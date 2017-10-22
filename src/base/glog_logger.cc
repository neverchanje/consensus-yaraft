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

#include "consensus/base/glog_logger.h"

#include <glog/logging.h>

namespace consensus {

void GLogLogger::Log(yaraft::LogLevel level, int line, const char *file, const yaraft::Slice &log) {
  google::LogSeverity severity;
  switch (level) {
    case yaraft::INFO:
      severity = google::INFO;
      break;
    case yaraft::WARNING:
      severity = google::WARNING;
      break;
    case yaraft::ERROR:
      severity = google::ERROR;
      break;
    case yaraft::FATAL:
      severity = google::FATAL;
      break;
    default:
      assert(false);
  }

  google::LogMessage(file, line, severity).stream() << log;
}

}  // namespace consensus
