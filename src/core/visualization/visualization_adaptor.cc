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

#include "core/visualization/visualization_adaptor.h"
#include <string>
#include "core/param/param.h"
#include "core/simulation.h"
#include "core/util/log.h"

#ifdef USE_PARAVIEW
#include "core/visualization/paraview/adaptor.h"
#endif

namespace bdm {

VisualizationAdaptor *VisualizationAdaptor::Create(const std::string &adaptor) {
  auto *param = Simulation::GetActive()->GetParam();
  if (!(param->insitu_visualization || param->export_visualization)) {
    return nullptr;
  }
#ifdef USE_PARAVIEW
  if (adaptor == "paraview") {
    return new ParaviewAdaptor();
  }
#endif

  Log::Error("VisualizationAdaptor::Create", "Visualization adaptor '", adaptor,
             "' is not available in this build.");
  return nullptr;
}

}  // namespace bdm
