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

#include "wal/log_manager.h"
#include "base/logging.h"
#include "wal/log_writer.h"
#include "wal/readable_log_segment.h"
#include "wal/segment_meta.h"

#include <fmt/format.h>
#include <silly/likely.h>
#include <yaraft/memory_storage.h>
#include <yaraft/pb_utils.h>

namespace consensus {
namespace wal {

static bool isWal(const std::string& fname) {
  // TODO(optimize)
  size_t len = fname.length();
  return len > 4 && fname.substr(len - 4, 4) == ".wal";
}

static void parseWalName(const std::string& fname, uint64_t* segId, uint64_t* segStart) {
  sscanf(fname.c_str(), "%zu-%zu.wal", segId, segStart);
}

// Returns: Error::YARaftERR / OK
Status AppendToMemStore(yaraft::pb::Entry& e, yaraft::MemoryStorage* memstore) {
  auto& vec = memstore->TEST_Entries();

  if (!vec.empty()) {
    if (e.term() < vec.rbegin()->term()) {
      return Status::Make(
          Error::YARaftERR,
          fmt::format(
              "new entry [index:{}, term:{}] has lower term than last entry [index:{}, term:{}]",
              e.index(), e.term(), vec.rbegin()->index(), vec.rbegin()->term()));
    }

    int del = vec.size() - 1;
    while (del >= 0) {
      if (vec[del].index() < e.index()) {
        break;
      }
      del--;
    }

    size_t sz = vec.size();
    for (int i = del + 1; i < sz; i++) {
      vec.pop_back();
    }
  }

  memstore->Append(e);
  return Status::OK();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

LogManager::LogManager(const Slice& logsDir)
    : lastIndex_(0), empty_(true), logsDir_(logsDir.ToString()) {}

LogManager::~LogManager() {}

StatusWith<LogManager*> LogManager::Recover(const std::string& logsDir,
                                            yaraft::MemoryStorage* memstore) {
  std::vector<std::string> files;
  RETURN_NOT_OK(Env::Default()->GetChildren(logsDir, &files));

  // finds all files with suffix ".wal"
  std::map<uint64_t, uint64_t> wals;  // ordered by segId
  for (const auto& f : files) {
    if (isWal(f)) {
      uint64_t segId, segStart;
      parseWalName(f, &segId, &segStart);
      wals[segId] = segStart;
    }
  }

  std::unique_ptr<LogManager> m(new LogManager(logsDir));
  if (wals.empty()) {
    LOG(WARNING) << fmt::format("Recovering from {} with no logs", logsDir);
    return m.release();
  }

  m->empty_ = false;

  LOG(INFO) << fmt::format("Recovering from {} wals, starts from {}-{}, ends at {}-{}", wals.size(),
                           wals.begin()->first, wals.begin()->second, wals.rbegin()->first,
                           wals.rbegin()->second);

  for (auto it = wals.begin(); it != wals.end(); it++) {
    ReadableLogSegment* seg;
    std::string fname = logsDir + "/" + fmt::format("{}-{}.wal", it->first, it->second);
    ASSIGN_IF_OK(ReadableLogSegment::Create(fname), seg);
    std::unique_ptr<ReadableLogSegment> d(seg);

    bool head = true;
    while (!seg->Eof()) {
      if (head) {
        ReadableLogSegment::Header header;
        ASSIGN_IF_OK(seg->ReadHeader(), header);

        SegmentMetaData meta;
        meta.fileName = std::move(fname);
        meta.committed = header.committed;
        m->files_.push_back(std::move(meta));
        head = false;
        continue;
      }

      yaraft::pb::Entry entry;
      ASSIGN_IF_OK(seg->ReadEntry(), entry);
      m->lastIndex_ = entry.index();
      RETURN_NOT_OK(AppendToMemStore(entry, memstore));
    }
  }
  LogWriter* writer;
  ASSIGN_IF_OK(LogWriter::New(m.get()), writer);
  m->current_.reset(writer);

  return m.release();
}

// mark the specified segment as committed
Status LogManager::MarkCommitted(uint64_t segId) const {
  WritableFile* wf;
  ASSIGN_IF_OK(
      Env::Default()->NewWritableFile(files_[segId].fileName, Env::CreateMode::OPEN_EXISTING, true),
      wf);
  std::unique_ptr<WritableFile> d(wf);

  char committed[1] = {static_cast<char>(1)};
  RETURN_NOT_OK(wf->PositionedAppend(Slice(committed, 1), 0));
  return wf->Close();
}

Status LogManager::AppendEntries(const yaraft::pb::Message& msg) {
  if (msg.entries().empty()) {
    return Status::OK();
  }

  uint64_t beginIdx = msg.entries().begin()->index();
  if (empty_) {
    lastIndex_ = beginIdx - 1;  // start at the first entry received.
    empty_ = false;
  }

  // the overlapped part of logs will not be deleted until snapshotting.
  return doAppend(msg.entries().begin(), msg.entries().end());
}

Status LogManager::doAppend(ConstPBEntriesIterator begin, ConstPBEntriesIterator end) {
  auto segStart = begin;
  auto it = segStart;
  while (true) {
    if (!current_) {
      LogWriter* w;
      ASSIGN_IF_OK(LogWriter::New(this), w);
      current_.reset(w);
    }

    ASSIGN_IF_OK(current_->AppendEntries(segStart, end), it);
    lastIndex_ = it->index();

    if (it == end) {
      // write complete
      break;
    }

    SegmentMetaData meta;
    ASSIGN_IF_OK(current_->Finish(), meta);
    delete current_.release();
    files_.push_back(meta);

    segStart = it;
  }
  return Status::OK();
}

Status LogManager::Close() {
  if (current_) {
    return current_->Finish().GetStatus();
  }
  return Status::OK();
}

}  // namespace wal
}  // namespace consensus