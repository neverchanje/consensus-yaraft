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

#pragma once

#include "base/slice.h"
#include "base/status.h"

namespace consensus {

// A file abstraction for sequential writing.  The implementation
// must provide buffering since callers may append small fragments
// at a time to the file.
class WritableFile {
 public:
  enum FlushMode { FLUSH_SYNC, FLUSH_ASYNC };

  WritableFile() {}
  virtual ~WritableFile() = default;

  virtual Status Append(const Slice &data) = 0;

  // If possible, uses scatter-gather I/O to efficiently append
  // multiple buffers to a file. Otherwise, falls back to regular I/O.
  //
  // For implementation specific quirks and details, see comments in
  // implementation source code (e.g., env_posix.cc)
  virtual Status AppendVector(const std::vector<Slice> &data_vector) = 0;

  // PositionedAppend data to the specified offset. The new EOF after append
  // must be larger than the previous EOF. This is to be used when writes are
  // not backed by OS buffers and hence has to always start from the start of
  // the sector. The implementation thus needs to also rewrite the last
  // partial sector.
  // Note: PositionAppend does not guarantee moving the file offset after the
  // write. A WritableFile object must support either Append or
  // PositionedAppend, so the users cannot mix the two.
  //
  // PositionedAppend() can only happen on the page/sector boundaries. For that
  // reason, if the last write was an incomplete sector we still need to rewind
  // back to the nearest sector/page and rewrite the portion of it with whatever
  // we need to add. We need to keep where we stop writing.
  //
  // PositionedAppend() can only write whole sectors. For that reason we have to
  // pad with zeros for the last write and trim the file when closing according
  // to the position we keep in the previous step.
  //
  // PositionedAppend() requires aligned buffer to be passed in. The alignment
  // required is queried via GetRequiredBufferAlignment()
  virtual Status PositionedAppend(const Slice &data, uint64_t offset) {
    return Status::Make(Error::NotSupported);
  }

  // Pre-allocates 'size' bytes for the file in the underlying filesystem.
  // size bytes are added to the current pre-allocated size or to the current
  // offset, whichever is bigger. In no case is the file truncated by this
  // operation.
  //
  // On some implementations, preallocation is done without initializing the
  // contents of the data blocks (as opposed to writing zeroes), requiring no
  // IO to the data blocks.
  //
  // In no case is the file truncated by this operation.
  virtual Status PreAllocate(uint64_t size) = 0;

  virtual Status Close() = 0;

  // Flush all dirty data (not metadata) to disk.
  //
  // If the flush mode is synchronous, will wait for flush to finish and
  // return a meaningful status.
  virtual Status Flush(FlushMode mode) = 0;

  virtual Status Sync() = 0;

  virtual uint64_t Size() const = 0;

  // Truncate is necessary to trim the file to the correct size
  // before closing. It is not always possible to keep track of the file
  // size due to whole pages writes. The behavior is undefined if called
  // with other writes to follow.
  virtual Status Truncate(uint64_t size) {
    return Status::Make(Error::NotSupported);
  }

  // Returns the filename provided when the WritableFile was constructed.
  virtual const std::string &filename() const = 0;

 private:
  // No copying allowed
  WritableFile(const WritableFile &);
  void operator=(const WritableFile &);
};

// A file abstraction for randomly reading the contents of a file.
class RandomAccessFile {
 public:
  RandomAccessFile() = default;
  virtual ~RandomAccessFile() = default;

  // Read up to "n" bytes from the file starting at "offset".
  // "scratch[0..n-1]" may be written by this routine.  Sets "*result"
  // to the data that was read (including if fewer than "n" bytes were
  // successfully read).  May set "*result" to point at data in
  // "scratch[0..n-1]", so "scratch[0..n-1]" must be live when
  // "*result" is used.  If an error was encountered, returns a non-OK
  // status.
  //
  // Safe for concurrent use by multiple threads.
  virtual Status Read(uint64_t offset, size_t n, Slice *result, char *scratch) const = 0;

  // Returns the size of the file
  virtual StatusWith<uint64_t> Size() const = 0;

  // Returns the filename provided when the RandomAccessFile was constructed.
  virtual const std::string &filename() const = 0;
};

class Env {
 public:
  // Governs if/how the file is created.
  //
  // enum value                      | file exists       | file does not exist
  // --------------------------------+-------------------+--------------------
  // CREATE_IF_NON_EXISTING_TRUNCATE | opens + truncates | creates
  // CREATE_NON_EXISTING             | fails             | creates
  // OPEN_EXISTING                   | opens             | fails
  enum CreateMode { CREATE_IF_NON_EXISTING_TRUNCATE, CREATE_NON_EXISTING, OPEN_EXISTING };

  Env() = default;
  virtual ~Env() = default;

  // Return a default environment suitable for the current operating
  // system.  Sophisticated users may wish to provide their own Env
  // implementation instead of relying on this default environment.
  //
  // The result of Default() belongs to kudu and must never be deleted.
  static Env *Default();

  // Create an object that writes to a new file with the specified
  // name.  Deletes any existing file with the same name and creates a
  // new file.  On success, stores a pointer to the new file in
  // *result and returns OK.  On failure stores NULL in *result and
  // returns non-OK.
  //
  // The returned file will only be accessed by one thread at a time.
  virtual StatusWith<WritableFile *> NewWritableFile(
      const Slice &fname, CreateMode mode = CREATE_IF_NON_EXISTING_TRUNCATE,
      bool sync_on_close = false) = 0;

  // Create a brand new random access read-only file with the
  // specified name.  On success, stores a pointer to the new file in
  // *result and returns OK.  On failure stores NULL in *result and
  // returns non-OK.  If the file does not exist, returns a non-OK
  // status.
  //
  // The returned file may be concurrently accessed by multiple threads.
  virtual StatusWith<RandomAccessFile *> NewRandomAccessFile(const Slice &fname) = 0;

  // Return the logical size of fname.
  virtual StatusWith<uint64_t> GetFileSize(const Slice &fname) = 0;

  // Create the specified directory. Returns error if directory exists.
  virtual Status CreateDir(const Slice &dirname) = 0;

  // Creates directory if missing. Return OK if it exists, or successful in creating.
  virtual Status CreateDirIfMissing(const Slice &dirname) = 0;

  // Recursively delete the specified directory.
  // This should operate safely, not following any symlinks, etc.
  virtual Status DeleteRecursively(const Slice &name) = 0;

  // Delete the named file.
  virtual Status DeleteFile(const Slice &fname) = 0;
};

}  // namespace consensus