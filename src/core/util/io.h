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

#ifndef CORE_UTIL_IO_H_
#define CORE_UTIL_IO_H_

#include <string>

namespace bdm {

bool FileExists(const std::string& file_name);
void WriteToFile(const std::string& filename, const std::string& content);

}  // namespace bdm

#endif  // CORE_UTIL_IO_H_
