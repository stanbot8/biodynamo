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

#include "core/agent/agent_uid.h"
#include <gtest/gtest.h>

namespace bdm {

TEST(AgentUidTest, DefaultCtor) {
  AgentUid uid;
  EXPECT_EQ(uid.GetIndex(), std::numeric_limits<uint32_t>::max());
  EXPECT_EQ(uid.GetReused(), std::numeric_limits<uint32_t>::max());
}

TEST(AgentUidTest, CtorOneParam) {
  AgentUid uid(123);
  EXPECT_EQ(uid.GetIndex(), 123u);
  EXPECT_EQ(uid.GetReused(), 0u);
}

TEST(AgentUidTest, CtorTwoParam) {
  AgentUid uid(123, 456);
  EXPECT_EQ(uid.GetIndex(), 123u);
  EXPECT_EQ(uid.GetReused(), 456u);
}

TEST(AgentUidTest, EqualsOperator) {
  AgentUid uid(123, 456);
  EXPECT_TRUE(uid == AgentUid(123, 456));
  EXPECT_FALSE(uid == AgentUid(456, 123));
  EXPECT_FALSE(uid == AgentUid(789, 987));

  EXPECT_FALSE(uid != AgentUid(123, 456));
  EXPECT_TRUE(uid != AgentUid(456, 123));
  EXPECT_TRUE(uid != AgentUid(789, 987));
}

TEST(AgentUidTest, LessThanOperator) {
  AgentUid uid(123, 456);
  EXPECT_FALSE(uid < uid);
  EXPECT_FALSE(uid < AgentUid(123, 455));
  EXPECT_TRUE(uid < AgentUid(123, 457));

  EXPECT_FALSE(uid < AgentUid(122, 456));
  EXPECT_TRUE(uid < AgentUid(124, 456));
}

TEST(AgentUidTest, PlosOperator) {
  AgentUid uid(123, 456);

  // integer
  {
    auto new_uid = uid + 7;
    EXPECT_EQ(new_uid, AgentUid(130, 456));
  }

  // unsigned integer
  {
    auto new_uid = uid + static_cast<uint64_t>(7u);
    EXPECT_EQ(new_uid, AgentUid(130, 456));
  }
}

TEST(AgentUidTest, MinusOperator) {
  AgentUid uid(123, 456);

  // integer
  {
    auto new_uid = uid - 7;
    EXPECT_EQ(new_uid, AgentUid(116, 456));
  }

  // unsigned integer
  {
    auto new_uid = uid - static_cast<uint64_t>(7u);
    EXPECT_EQ(new_uid, AgentUid(116, 456));
  }
}

TEST(AgentUidTest, uint64_tOperator) {
  AgentUid uid(123, 0);
  uint64_t idx = uid;
  EXPECT_EQ(idx, 123u);
}

TEST(AgentUidTest, uint64_tOperator2) {
  AgentUid uid(123, 2);
  uint64_t idx = uid;
  constexpr uint64_t expected_uid = (uint64_t{2} << 32) | 123u;
  EXPECT_EQ(idx, expected_uid);
}

}  // namespace bdm
