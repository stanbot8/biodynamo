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

#ifndef UNIT_CORE_CONTAINER_SHARED_DATA_H_
#define UNIT_CORE_CONTAINER_SHARED_DATA_H_

#include <gtest/gtest.h>
#include "core/container/shared_data.h"

namespace bdm {
namespace shared_data_test {

inline void RunCacheLineAlignmentTest() {
  // Test standard data types int, float, double
  // Test alignment of int
  EXPECT_EQ(
      std::alignment_of<typename SharedData<int>::Data::value_type>::value,
      kCacheLineSize);
  // Test alignment of float
  EXPECT_EQ(
      std::alignment_of<typename SharedData<float>::Data::value_type>::value,
      kCacheLineSize);
  // Test alignment of double
  EXPECT_EQ(
      std::alignment_of<typename SharedData<double>::Data::value_type>::value,
      kCacheLineSize);
  // Test size of vector components int
  EXPECT_EQ(sizeof(typename SharedData<int>::Data::value_type), kCacheLineSize);
  // Test size of vector components float
  EXPECT_EQ(sizeof(typename SharedData<float>::Data::value_type),
            kCacheLineSize);
  // Test size of vector components double
  EXPECT_EQ(sizeof(typename SharedData<double>::Data::value_type),
            kCacheLineSize);

  // Test a cache line fully filled with doubles.
  // Test alignment of double[max_double], e.g. max cache line capacity
  EXPECT_EQ(
      std::alignment_of<typename SharedData<
          double[kCacheLineSize / sizeof(double)]>::Data::value_type>::value,
      kCacheLineSize);
  // Test size of vector components double[max_double], e.g. max cache line
  // capacity
  EXPECT_EQ(sizeof(typename SharedData<
                   double[kCacheLineSize / sizeof(double)]>::Data::value_type),
            kCacheLineSize);

  // Test some custom data structures
  // Test alignment of data that fills 1 cache line
  EXPECT_EQ(std::alignment_of<typename SharedData<
                char[kCacheLineSize - 1]>::Data::value_type>::value,
            kCacheLineSize);
  // Test alignment of data that fills 2 cache lines
  EXPECT_EQ(std::alignment_of<typename SharedData<
                char[kCacheLineSize + 1]>::Data::value_type>::value,
            kCacheLineSize);
  // Test alignment of data that fills 3 cache lines
  EXPECT_EQ(std::alignment_of<typename SharedData<
                char[2 * kCacheLineSize + 1]>::Data::value_type>::value,
            kCacheLineSize);
  // Test size of data that fills 1 cache line
  EXPECT_EQ(
      sizeof(typename SharedData<char[kCacheLineSize - 1]>::Data::value_type),
      kCacheLineSize);
  // Test size of data that fills 2 cache lines
  EXPECT_EQ(
      sizeof(typename SharedData<char[kCacheLineSize + 1]>::Data::value_type),
      2 * kCacheLineSize);
  // Test size of data that fills 3 cache lines
  EXPECT_EQ(
      sizeof(
          typename SharedData<char[2 * kCacheLineSize + 1]>::Data::value_type),
      3 * kCacheLineSize);
}

}  // namespace shared_data_test
}  // namespace bdm

#endif  // UNIT_CORE_CONTAINER_SHARED_DATA_H_
