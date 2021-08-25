/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/String.h>
#include <chrono>
#include <vector>
#include "watchman/ChildProcess.h"
#include "watchman/LRUCache.h"
#include "watchman/scm/SCM.h"

namespace watchman {

class GitStatusAccumulator {
 public:
  void add(w_string_piece status);

  SCM::StatusResult finalize() const;

 private:
  // -1 = removed
  // 0 = changed
  // 1 = added
  std::unordered_map<w_string, int> byFile_;
};

class Git : public SCM {
 public:
  Git(w_string_piece rootPath, w_string_piece scmRoot);
  w_string mergeBaseWith(w_string_piece commitId, w_string requestId = nullptr)
      const override;
  std::vector<w_string> getFilesChangedSinceMergeBaseWith(
      w_string_piece commitId,
      w_string requestId = nullptr) const override;
  StatusResult getFilesChangedBetweenCommits(
      std::vector<std::string> commits,
      w_string requestId = nullptr) const override;
  std::chrono::time_point<std::chrono::system_clock> getCommitDate(
      w_string_piece commitId,
      w_string requestId = nullptr) const override;
  std::vector<w_string> getCommitsPriorToAndIncluding(
      w_string_piece commitId,
      int numCommits,
      w_string requestId = nullptr) const override;

 private:
  std::string indexPath_;
  mutable LRUCache<std::string, std::vector<w_string>> commitsPrior_;
  mutable LRUCache<std::string, w_string> mergeBases_;
  mutable LRUCache<std::string, w_string> filesChangedBetweenCommits_;
  mutable LRUCache<std::string, std::vector<w_string>>
      filesChangedSinceMergeBaseWith_;

  ChildProcess::Options makeGitOptions(w_string requestId) const;
  struct timespec getIndexMtime() const;
};

} // namespace watchman