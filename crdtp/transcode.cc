// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "encoding.h"

namespace crdtp {
class SingleThreadedPlatform : public json::Platform {
  bool StrToD(const char* str, double* result) const override {
    const char* saved_locale = std::setlocale(LC_NUMERIC, nullptr);
    char* end;
    *result = std::strtod(str, &end);
    std::setlocale(LC_NUMERIC, saved_locale);
    if (errno == ERANGE) {
      // errno must be reset, e.g. see the example here:
      // https://en.cppreference.com/w/cpp/string/byte/strtof
      errno = 0;
      return false;
    }
    return end == str + strlen(str);
  }

  std::unique_ptr<char[]> DToStr(double value) const override {
    std::stringstream ss;
    ss.imbue(std::locale("C"));
    ss << value;
    std::string str = ss.str();
    std::unique_ptr<char[]> result(new char[str.size() + 1]);
    memcpy(result.get(), str.c_str(), str.size() + 1);
    return result;
  }
};

int Transcode(const std::string& cmd,
              const std::string& input_file_name,
              const std::string& output_file_name) {
  std::ifstream input_file(input_file_name, std::ios::binary);
  if (!input_file.is_open()) {
    std::cerr << "failed to open " << input_file_name << "\n";
    return 1;
  }
  std::string in;
  while (input_file) {
    std::string buffer(1024, '\0');
    input_file.read(&buffer.front(), buffer.size());
    in += buffer.substr(0, input_file.gcount());
  }
  Status status;
  SingleThreadedPlatform platform;
  std::string out;
  if (cmd == "--json-to-cbor") {
    status = json::ConvertJSONToCBOR(platform, SpanFrom(in), &out);
  } else if (cmd == "--cbor-to-json") {
    status = json::ConvertCBORToJSON(platform, SpanFrom(in), &out);
  } else {
    std::cerr << "unknown command " << cmd << "\n";
    return 1;
  }
  if (!status.ok()) {
    std::cerr << "transcoding error: " << status.ToASCIIString() << "\n";
    return 1;
  }
  std::ofstream output_file(output_file_name, std::ios::binary);
  if (!output_file.is_open()) {
    std::cerr << "failed to open " << output_file_name << "\n";
    return 1;
  }
  output_file.write(out.data(), out.size());
  return 0;
}
}  // namespace crdtp

int main(int argc, char** argv) {
  if (argc == 4)
    return ::crdtp::Transcode(argv[1], argv[2], argv[3]);
  std::cerr << "usage: " << argv[0]
            << " --json-to-cbor <input-file> <output-file>\n"
            << "  or   " << argv[0]
            << " --cbor-to-json <input-file> <output-file>\n";
  return 1;
}
