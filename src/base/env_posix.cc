// Copyright 2017 Wu Tao
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fcntl.h>
#include <limits.h>
#include <mutex>
#include <sys/stat.h>
#include <sys/uio.h>
#include <thread>

#include "base/env.h"
#include "base/errno.h"
#include "base/logging.h"
#include "base/port.h"

#include <boost/filesystem.hpp>
#include <fmt/format.h>
#include <silly/likely.h>

namespace consensus {

using std::vector;
using std::string;

// Retry on EINTR for functions like read() that return -1 on error.
#define RETRY_ON_EINTR(err, expr)                                                                  \
  do {                                                                                             \
    static_assert(std::is_signed<decltype(err)>::value == true, #err " must be a signed integer"); \
    (err) = (expr);                                                                                \
  } while ((err) == -1 && errno == EINTR)

#define RETURN_BOOST_EC(code)                       \
  do {                                              \
    if (PREDICT_FALSE(bool(code)))                  \
      return IOError(code.message(), code.value()); \
  } while (0)

Status StatusWithErrno(Error::ErrorCodes code, const Slice& context, int err_number) {
  return Status::Make(code, context) << ": " << ErrnoToString(err_number);
}

static Status IOError(const Slice& context, int err_number) {
  switch (err_number) {
    case ENOENT:
      return StatusWithErrno(Error::NotFound, context, err_number);
    case EEXIST:
      return StatusWithErrno(Error::AlreadyPresent, context, err_number);
    case EOPNOTSUPP:
      return StatusWithErrno(Error::NotSupported, context, err_number);
  }
  return StatusWithErrno(Error::IOError, context, err_number);
}

static Status DoSync(int fd, const Slice& filename) {
  if (fdatasync(fd) < 0) {
    return IOError(filename, errno);
  }
  return Status::OK();
}

static StatusWith<int> DoOpen(const Slice& filename, Env::CreateMode mode) {
  int flags = O_RDWR;
  switch (mode) {
    case Env::CREATE_IF_NON_EXISTING_TRUNCATE:
      flags |= O_CREAT | O_TRUNC;
      break;
    case Env::CREATE_NON_EXISTING:
      flags |= O_CREAT | O_EXCL;
      break;
    case Env::OPEN_EXISTING:
      break;
    default:
      return Status::Make(Error::NotSupported, fmt::format("Unknown create mode {}", mode));
  }
  const int f = open(filename.data(), flags, 0666);
  if (f < 0) {
    return IOError(filename, errno);
  }
  return f;
}

StatusWith<uint64_t> DoGetFileSize(const Slice& fname) {
  struct stat sbuf;
  if (stat(fname.data(), &sbuf) != 0) {
    return IOError(fname, errno);
  } else {
    return static_cast<uint64_t>(sbuf.st_size);
  }
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

// Use non-memory mapped POSIX files to write data to a file.
//
// TODO (perf) investigate zeroing a pre-allocated allocated area in
// order to further improve Sync() performance.
class PosixWritableFile : public WritableFile {
 public:
  PosixWritableFile(const Slice& fname, int fd, uint64_t file_size, bool sync_on_close)
      : filename_(fname.ToString()),
        fd_(fd),
        sync_on_close_(sync_on_close),
        filesize_(file_size),
        pre_allocated_size_(0),
        pending_sync_(false) {}

  ~PosixWritableFile() {
    if (fd_ >= 0) {
      WARN_NOT_OK(Close(), "Failed to close " + filename_);
    }
  }

  Status Append(const Slice& data) override {
    const char* src = data.data();
    size_t left = data.size();
    while (left != 0) {
      ssize_t done = write(fd_, src, left);
      if (done < 0) {
        if (errno == EINTR) {
          continue;
        }
        return IOError(filename_, errno);
      }
      left -= done;
      src += done;
    }
    filesize_ += data.size();
    return Status::OK();
  }

  Status PositionedAppend(const Slice& data, uint64_t offset) override {
    DLOG_ASSERT(offset <= std::numeric_limits<off_t>::max());
    const char* src = data.data();
    size_t left = data.size();
    while (left != 0) {
      ssize_t done = pwrite(fd_, src, left, static_cast<off_t>(offset));
      if (done < 0) {
        if (errno == EINTR) {
          continue;
        }
        return IOError(filename_, errno);
      }
      left -= done;
      offset += done;
      src += done;
    }
    filesize_ = offset;
    return Status::OK();
  }

  Status PreAllocate(uint64_t size) override {
    uint64_t offset = std::max(filesize_, pre_allocated_size_);
    if (fallocate(fd_, 0, offset, size) < 0) {
      if (errno == EOPNOTSUPP) {
        LOG(WARNING) << "The filesystem does not support fallocate().";
      } else if (errno == ENOSYS) {
        LOG(WARNING) << "The kernel does not implement fallocate().";
      } else {
        return IOError(filename_, errno);
      }
    }
    pre_allocated_size_ = offset + size;
    return Status::OK();
  }

  Status Close() override {
    Status s;

    // If we've allocated more space than we used, truncate to the
    // actual size of the file and perform Sync().
    if (filesize_ < pre_allocated_size_) {
      int ret;
      RETRY_ON_EINTR(ret, ftruncate(fd_, filesize_));
      if (ret != 0) {
        s = IOError(filename_, errno);
        pending_sync_ = true;
      }
    }

    if (sync_on_close_) {
      Status sync_status = Sync();
      if (!sync_status.OK()) {
        LOG(ERROR) << "Unable to Sync " << filename_ << ": " << sync_status.ToString();
        if (s.OK()) {
          s = sync_status;
        }
      }
    }

    if (close(fd_) < 0) {
      if (s.OK()) {
        s = IOError(filename_, errno);
      }
    }

    fd_ = -1;
    return s;
  }

  Status Flush(FlushMode mode) override {
#if defined(__linux__)
    unsigned int flags = SYNC_FILE_RANGE_WRITE;
    if (mode == FLUSH_SYNC) {
      flags |= SYNC_FILE_RANGE_WAIT_BEFORE;
      flags |= SYNC_FILE_RANGE_WAIT_AFTER;
    }
    if (sync_file_range(fd_, 0, 0, flags) < 0) {
      return IOError(filename_, errno);
    }
#else
    if (mode == FLUSH_SYNC && fsync(fd_) < 0) {
      return IOError(filename_, errno);
    }
#endif
    return Status::OK();
  }

  Status Sync() override {
    if (pending_sync_) {
      pending_sync_ = false;
      RETURN_NOT_OK(DoSync(fd_, filename_));
    }
    return Status::OK();
  }

  uint64_t Size() const override {
    return filesize_;
  }

  // NOTE: The file offset will not be changed.
  Status Truncate(size_t size) override {
    Status s;
    int r = ftruncate(fd_, size);
    if (r < 0) {
      s = IOError(filename_, errno);
    } else {
      filesize_ = size;
    }
    return s;
  }

  const string& filename() const override {
    return filename_;
  }

 private:
  const std::string filename_;
  int fd_;
  bool sync_on_close_;
  uint64_t filesize_;
  uint64_t pre_allocated_size_;
  bool pending_sync_;
};

// pread() based random-access
class PosixRandomAccessFile : public RandomAccessFile {
 private:
  std::string filename_;
  int fd_;

 public:
  PosixRandomAccessFile(const Slice& fname, int fd) : filename_(fname.ToString()), fd_(fd) {}
  virtual ~PosixRandomAccessFile() {
    close(fd_);
  }

  virtual Status Read(uint64_t offset, size_t n, Slice* result, char* scratch) const override {
    Status s;
    ssize_t r;
    RETRY_ON_EINTR(r, pread(fd_, scratch, n, offset));
    if (r < 0) {
      // An error: return a non-ok status.
      s = IOError(filename_, errno);
    }
    *result = Slice(scratch, static_cast<size_t>(r));
    return s;
  }

  virtual StatusWith<uint64_t> Size() const override {
    return DoGetFileSize(filename_);
  }

  virtual const std::string& filename() const override {
    return filename_;
  }
};

class PosixEnv final : public Env {
 public:
  PosixEnv() = default;
  ~PosixEnv() = default;

  StatusWith<WritableFile*> NewWritableFile(const Slice& fname,
                                            CreateMode mode = CREATE_IF_NON_EXISTING_TRUNCATE,
                                            bool sync_on_close = false) override {
    uint64_t file_size = 0;
    if (mode == OPEN_EXISTING) {
      ASSIGN_IF_OK(GetFileSize(fname), file_size);
    }

    int fd;
    ASSIGN_IF_OK(DoOpen(fname, mode), fd);

    return new PosixWritableFile(fname, fd, file_size, sync_on_close);
  }

  StatusWith<RandomAccessFile*> NewRandomAccessFile(const Slice& fname) override {
    int fd = open(fname.data(), O_RDONLY);
    if (fd < 0) {
      return IOError(fname, errno);
    }
    return new PosixRandomAccessFile(fname, fd);
  }

  StatusWith<uint64_t> GetFileSize(const Slice& fname) override {
    return DoGetFileSize(fname);
  }

  Status CreateDir(const Slice& dirname) override {
    boost::system::error_code code;
    boost::filesystem::create_directory(dirname.data(), code);
    RETURN_BOOST_EC(code);
    return Status::OK();
  }

  Status CreateDirIfMissing(const Slice& dirname) override {
    boost::system::error_code code;
    boost::filesystem::create_directories(dirname.data(), code);
    RETURN_BOOST_EC(code);
    return Status::OK();
  }

  Status DeleteRecursively(const Slice& name) override {
    boost::system::error_code code;
    boost::filesystem::remove_all(name.data(), code);
    RETURN_BOOST_EC(code);
    return Status::OK();
  }

  Status DeleteFile(const Slice& fname) override {
    boost::system::error_code code;
    boost::filesystem::remove(fname.data(), code);
    RETURN_BOOST_EC(code);
    return Status::OK();
  }
};

Env* Env::Default() {
  static PosixEnv* env = new PosixEnv();
  return env;
}

}  // namespace consensus