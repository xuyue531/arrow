// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "arrow/flight/types.h"

#include <memory>
#include <sstream>
#include <utility>

#include "arrow/io/memory.h"
#include "arrow/ipc/dictionary.h"
#include "arrow/ipc/reader.h"
#include "arrow/status.h"

namespace arrow {
namespace flight {

bool FlightDescriptor::Equals(const FlightDescriptor& other) const {
  if (type != other.type) {
    return false;
  }
  switch (type) {
    case PATH:
      return path == other.path;
    case CMD:
      return cmd == other.cmd;
    default:
      return false;
  }
}

std::string FlightDescriptor::ToString() const {
  std::stringstream ss;
  ss << "FlightDescriptor<";
  switch (type) {
    case PATH: {
      bool first = true;
      ss << "path = '";
      for (const auto& p : path) {
        if (!first) {
          ss << "/";
        }
        first = false;
        ss << p;
      }
      ss << "'";
      break;
    }
    case CMD:
      ss << "cmd = '" << cmd << "'";
      break;
    default:
      break;
  }
  ss << ">";
  return ss.str();
}

Status FlightInfo::GetSchema(ipc::DictionaryMemo* dictionary_memo,
                             std::shared_ptr<Schema>* out) const {
  if (reconstructed_schema_) {
    *out = schema_;
    return Status::OK();
  }
  io::BufferReader schema_reader(data_.schema);
  RETURN_NOT_OK(ipc::ReadSchema(&schema_reader, dictionary_memo, &schema_));
  reconstructed_schema_ = true;
  *out = schema_;
  return Status::OK();
}

SimpleFlightListing::SimpleFlightListing(const std::vector<FlightInfo>& flights)
    : position_(0), flights_(flights) {}

SimpleFlightListing::SimpleFlightListing(std::vector<FlightInfo>&& flights)
    : position_(0), flights_(std::move(flights)) {}

Status SimpleFlightListing::Next(std::unique_ptr<FlightInfo>* info) {
  if (position_ >= static_cast<int>(flights_.size())) {
    *info = nullptr;
    return Status::OK();
  }
  *info = std::unique_ptr<FlightInfo>(new FlightInfo(std::move(flights_[position_++])));
  return Status::OK();
}

SimpleResultStream::SimpleResultStream(std::vector<Result>&& results)
    : results_(std::move(results)), position_(0) {}

Status SimpleResultStream::Next(std::unique_ptr<Result>* result) {
  if (position_ >= results_.size()) {
    *result = nullptr;
    return Status::OK();
  }
  *result = std::unique_ptr<Result>(new Result(std::move(results_[position_++])));
  return Status::OK();
}

}  // namespace flight
}  // namespace arrow
