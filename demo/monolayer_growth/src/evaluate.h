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

#ifndef EVALUATE_H_
#define EVALUATE_H_

#include <cmath>
#include <fstream>
#include <string>
#include <vector>
#include "biodynamo.h"
#include "sim_param.h"

namespace bdm {

inline void SetupResultCollection(Simulation* sim) {
  auto* ts = sim->GetTimeSeries();

  // Monolayer size
  auto get_env_dims = [](Simulation* sim) {
    auto* env = dynamic_cast<UniformGridEnvironment*>(sim->GetEnvironment());
    real_t env_dim_x, env_dim_y;
    env_dim_x = env->GetDimensions()[1] - env->GetDimensions()[0];
    env_dim_y = env->GetDimensions()[3] - env->GetDimensions()[2];
    return std::max(env_dim_x, env_dim_y);
  };

  // Number of cells
  auto get_num_cells = [](Simulation* sim) {
    auto* rm = sim->GetResourceManager();
    return static_cast<real_t>(rm->GetNumAgents());
  };

  // Time in days
  auto get_time = [](Simulation* sim) {
    auto* scheduler = sim->GetScheduler();
    const auto* param = sim->GetParam();
    const auto* sparam = param->Get<SimParam>();
    return (real_t)(sparam->t0 + scheduler->GetSimulatedTime() / 24.);
  };
  ts->AddCollector("total_cells", get_num_cells, get_time);
  ts->AddCollector("env_dims", get_env_dims, get_time);
}

inline void ExportResults(const std::string& filename = "result.csv") {
  const std::string folder = Simulation::GetActive()->GetOutputDir();
  auto* ts = Simulation::GetActive()->GetTimeSeries();
  const auto& times = ts->GetXValues("env_dims");
  const auto& sizes = ts->GetYValues("env_dims");
  std::ofstream output(Concat(folder, "/", filename));
  output << "source,time_days,size_um\n";
  for (size_t i = 0; i < times.size(); ++i) {
    output << "simulation," << times[i] << ',' << sizes[i] << '\n';
  }

  constexpr real_t kHoursPerDay = 24.0;
  const std::vector<real_t> experimental_times = {
      336 / kHoursPerDay, 386 / kHoursPerDay, 408 / kHoursPerDay,
      481 / kHoursPerDay, 506 / kHoursPerDay, 646 / kHoursPerDay};
  const std::vector<real_t> experimental_sizes = {1140, 1400, 1590,
                                                  2040, 2250, 3040};
  for (size_t i = 0; i < experimental_times.size(); ++i) {
    output << "experiment," << experimental_times[i] << ','
           << experimental_sizes[i] << '\n';
  }
}

}  // namespace bdm

#endif  // EVALUATE_H_
