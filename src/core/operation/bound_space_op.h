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

#ifndef CORE_OPERATION_BOUND_SPACE_OP_H_
#define CORE_OPERATION_BOUND_SPACE_OP_H_

#include "core/agent/agent.h"
#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"
#include "core/param/param.h"
#include "core/simulation.h"

namespace bdm {

inline void ApplyBoundingBox(Agent* agent, Param::BoundSpaceMode mode,
                             const Param* param) {
  // Need to create a small distance from the positive edge of each dimension;
  // otherwise it will fall out of the boundary of the simulation space
  real_t eps = 1e-10;
  auto pos = agent->GetPosition();
  if (mode == Param::BoundSpaceMode::kClosed) {
    bool updated = false;
    for (int i = 0; i < 3; i++) {
      real_t lb = param->GetMinBound(i);
      real_t rb = param->GetMaxBound(i);
      if (pos[i] < lb) {
        pos[i] = lb;
        updated = true;
      } else if (pos[i] >= rb) {
        pos[i] = rb - eps;
        updated = true;
      }
    }
    if (updated) {
      agent->SetPosition(pos);
    }
  } else if (mode == Param::BoundSpaceMode::kTorus) {
    for (int i = 0; i < 3; i++) {
      real_t lb = param->GetMinBound(i);
      real_t rb = param->GetMaxBound(i);
      auto length = rb - lb;
      if (pos[i] < lb) {
        auto d = std::abs(lb - pos[i]);
        if (d > length) {
          d = std::fmod(d, length);
        }
        pos[i] = rb - d;
      } else if (pos[i] > rb) {
        auto d = std::abs(pos[i] - rb);
        if (d > length) {
          d = std::fmod(d, length);
        }
        pos[i] = lb + d;
      }
    }
    agent->SetPosition(pos);
  }
}

/// Backward-compatible overload with scalar bounds.
inline void ApplyBoundingBox(Agent* agent, Param::BoundSpaceMode mode,
                             real_t lb, real_t rb) {
  Param p;
  p.min_bound = lb;
  p.max_bound = rb;
  ApplyBoundingBox(agent, mode, &p);
}

/// Keeps the agents contained within the bounds as defined in
/// param.h
struct BoundSpace : public AgentOperationImpl {
  BDM_OP_HEADER(BoundSpace);

  void operator()(Agent* agent) override {
    auto* param = Simulation::GetActive()->GetParam();
    if (param->bound_space) {
      ApplyBoundingBox(agent, param->bound_space, param);
    }
  }
};

}  // namespace bdm

#endif  // CORE_OPERATION_BOUND_SPACE_OP_H_
