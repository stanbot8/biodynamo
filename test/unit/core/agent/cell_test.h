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

#ifndef UNIT_CORE_AGENT_CELL_TEST_H_
#define UNIT_CORE_AGENT_CELL_TEST_H_

#include <vector>

#include "core/agent/cell.h"
#include "core/agent/cell_division_event.h"
#include "gtest/gtest.h"
#include "unit/core/agent/agent_test.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace cell_test_internal {

/// Class used to get access to protected members
class TestCell : public Cell {
  BDM_AGENT_HEADER(TestCell, Cell);

 public:
  TestCell() = default;

  virtual ~TestCell() = default;

  void Initialize(const NewAgentEvent& event) override {
    Base::Initialize(event);

    if (event.GetUid() == CellDivisionEvent::kUid) {
      const auto& cdevent = static_cast<const CellDivisionEvent&>(event);
      auto* mother_cell = bdm_static_cast<TestCell*>(event.existing_agent);
      mother_cell->captured_volume_ratio = cdevent.volume_ratio;
      mother_cell->captured_phi_ = cdevent.phi;
      mother_cell->captured_theta_ = cdevent.theta;
    }
  }

  void TestTransformCoordinatesGlobalToPolar() {
    Real3 coord = {1, 2, 3};
    Base::SetPosition({9, 8, 7});
    auto result = Base::TransformCoordinatesGlobalToPolar(coord);

    EXPECT_NEAR(10.770329614269007, result[0], abs_error<real_t>::value);
    EXPECT_NEAR(1.9513027039072615, result[1], abs_error<real_t>::value);
    EXPECT_NEAR(-2.4980915447965089, result[2], abs_error<real_t>::value);
  }

  bool capture_input_parameters_ = false;
  real_t captured_volume_ratio = 0.0;
  real_t captured_phi_ = 0.0;
  real_t captured_theta_ = 0.0;

  FRIEND_TEST(CellTest, DivideVolumeRatioPhiTheta);
};

}  // namespace cell_test_internal

}  // namespace bdm

#endif  // UNIT_CORE_AGENT_CELL_TEST_H_
