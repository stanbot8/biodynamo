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

// Deprecated: cpptoml has been replaced by tomlplusplus.
// Update your includes from "core/util/cpptoml.h" to "core/util/toml_config.h"
// and use TomlConfig (from param_group.h) instead of cpptoml::table.
#pragma message( \
    "cpptoml.h is deprecated. Use #include \"core/util/toml_config.h\" instead.")

#include "core/util/toml_config.h"
