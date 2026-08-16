// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "core/util/io.h"

#include <fstream>

namespace bdm {

bool FileExists(const std::string& file_name) {
  std::ifstream infile(file_name);
  return infile.good();
}

void WriteToFile(const std::string& filename, const std::string& content) {
  std::ofstream ofs(filename);
  ofs << content;
}

}  // namespace bdm
