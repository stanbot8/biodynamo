// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "core/param/command_line_options.h"
#include <utility>
#include "core/util/log.h"

namespace bdm {

using cxxopts::value;
using std::string;

CommandLineOptions::CommandLineOptions(int argc, const char** argv)
    : argc_(argc),
      argv_(argv),
      options_(argv[0], " -- BioDynaMo command line options\n") {
  AddCoreOptions();
  ExtractSimulationName(argv[0]);
}

cxxopts::OptionAdder CommandLineOptions::AddOption(string group) {
  if (parser_ != nullptr) {
    Log::Fatal("CommandLineOptions::AddOption",
               "Please add all your command line options before:\n  1) Using "
               "the Get() method.\n  2) Creating a Simulation object.");
  }
  return cxxopts::OptionAdder(options_, std::move(group));
}

std::string CommandLineOptions::GetSimulationName() const { return sim_name_; }

/// Parse the given command line arguments
void CommandLineOptions::Parse() {
  std::vector<std::string> arguments;
  arguments.reserve(argc_);
  for (int i = 0; i < argc_; ++i) {
    arguments.emplace_back(argv_[i]);
  }

  std::vector<char*> argument_pointers;
  argument_pointers.reserve(arguments.size() + 1);
  for (auto& argument : arguments) {
    argument_pointers.push_back(argument.data());
  }
  argument_pointers.push_back(nullptr);

  int argc_copy = argc_;
  char** argv_copy = argument_pointers.data();
  parser_.reset();

  try {
    parser_ = std::make_unique<cxxopts::ParseResult>(
        options_.parse(argc_copy, argv_copy));
  } catch (const cxxopts::option_not_exists_exception& option) {
    Log::Fatal("CommandLineOptions::ParseResult", option.what(),
               " Perhaps you are constructing multiple Simulation objects with "
               "different CommandLineOptions.");
  }

  if (first_parse_) {
    // Perform operations on core command line options
    HandleCoreOptions();
    first_parse_ = false;
  }
}

bool CommandLineOptions::IsSet(std::string option) {
  if (parser_ == nullptr) {
    this->Parse();
  }
  return parser_->count(option) == 0 ? false : true;
}

// clang-format off
void CommandLineOptions::AddCoreOptions() {
  options_.add_options("Core")
    ("h, help", "Print this help message.")
    ("version", "Print version number of BioDynaMo.")
    ("opencl", "Enable GPU acceleration through OpenCL.")
    ("cuda", "Enable GPU acceleration through CUDA.")
    ("visualize", "Enable exporting of visualization.")
    ("vis-frequency", "Set the frequency of exporting the visualization.", value<uint32_t>()->default_value("10"), "FREQ")
    ("v, verbose", "Verbose mode. Causes BioDynaMo to print debugging messages. Multiple "
      "-v options increases the verbosity. The maximum is 3.", value<bool>())
    ("c, config", "The TOML configuration that should be used. This option can be used multiple times.", value<std::vector<string>>()->default_value(""), "FILE");
}
// clang-format on

void CommandLineOptions::ExtractSimulationName(const char* path) {
  string s(path);
  auto pos = s.find_last_of('/');
  if (pos == std::string::npos) {
    sim_name_ = s;
  } else {
    sim_name_ = s.substr(pos + 1, s.length() - 1);
  }
}

void CommandLineOptions::HandleCoreOptions() {
  // Handle "help" argument
  if (parser_->count("help")) {
    auto groups = options_.groups();
    auto it = std::find(groups.begin(), groups.end(), "Core");
    std::rotate(it, it + 1, groups.end());
    std::cout << options_.help(groups) << std::endl;
    exit(0);
  }

  if (IsSet("version")) {
    std::cout << "BioDynaMo Version: " << Version::String() << std::endl;
    exit(0);
  }

  if (IsSet("verbose")) {
    auto verbosity = parser_->count("verbose");
    switch (verbosity) {
      case 1:
        Log::SetLevel(Log::Level::kWarning);
        break;
      case 2:
        Log::SetLevel(Log::Level::kInfo);
        break;
      default:
        Log::SetLevel(Log::Level::kDebug);
        break;
    }
  }
}

std::ostream& operator<<(std::ostream& os, const CommandLineOptions& clo) {
  for (int i = 0; i < clo.argc_; ++i) {
    os << clo.argv_[i] << " ";
  }
  return os;
}

}  // namespace bdm
