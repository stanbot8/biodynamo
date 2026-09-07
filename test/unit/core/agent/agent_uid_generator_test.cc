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

#include "core/agent/agent_uid_generator.h"
#include <gtest/gtest.h>
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "unit/test_util/test_agent.h"

namespace bdm {

TEST(AgentUidGeneratorTest, NormalAndDefragmentationMode) {
  AgentUidGenerator generator;

  // Create num threads agent uids
  auto* tinfo = ThreadInfo::GetInstance();
  for (int i = 0; i < tinfo->GetMaxThreads(); ++i) {
    EXPECT_EQ(AgentUid(i), generator.GenerateUid());
  }

  // Reuse uids
#pragma omp parallel for schedule(static, 1)
  for (int i = 0; i < tinfo->GetMaxThreads(); ++i) {
    generator.ReuseAgentUid(AgentUid(i));
    EXPECT_EQ(AgentUid(i, 1), generator.GenerateUid());
  }

  // generate new uids
  for (int i = 0; i < tinfo->GetMaxThreads(); ++i) {
    EXPECT_EQ(AgentUid(i + tinfo->GetMaxThreads()), generator.GenerateUid());
  }
}

}  // namespace bdm
