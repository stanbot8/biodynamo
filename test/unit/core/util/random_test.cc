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

#include "core/util/random.h"

#include <cmath>
#include <stdexcept>

#include <gtest/gtest.h>

#include "unit/test_util/test_util.h"

namespace bdm {
namespace {

constexpr uint64_t kSeed = 42;

TEST(RandomTest, SeedReproducesSequence) {
  Random first;
  Random second;
  first.SetSeed(kSeed);
  second.SetSeed(kSeed);

  EXPECT_EQ(kSeed, first.GetSeed());
  for (uint64_t i = 0; i < 10; ++i) {
    EXPECT_REAL_EQ(first.Uniform(), second.Uniform());
    EXPECT_REAL_EQ(first.Gaus(3, 4), second.Gaus(3, 4));
    EXPECT_REAL_EQ(first.Exp(5), second.Exp(5));
    EXPECT_REAL_EQ(first.PoissonD(6), second.PoissonD(6));
    EXPECT_REAL_EQ(first.BreitWigner(7, 8), second.BreitWigner(7, 8));
    EXPECT_EQ(first.Integer(9), second.Integer(9));
    EXPECT_EQ(first.Binomial(10, 0.25), second.Binomial(10, 0.25));
    EXPECT_EQ(first.Poisson(11), second.Poisson(11));
  }
}

TEST(RandomTest, UniformValuesStayWithinBounds) {
  Random random;
  random.SetSeed(kSeed);

  for (uint64_t i = 0; i < 100; ++i) {
    const auto zero_based = random.Uniform(5);
    EXPECT_GE(zero_based, 0);
    EXPECT_LT(zero_based, 5);
    const auto bounded = random.Uniform(-3, 4);
    EXPECT_GE(bounded, -3);
    EXPECT_LT(bounded, 4);
  }

  const auto values = random.UniformArray<12>(-2, 3);
  for (const auto value : values) {
    EXPECT_GE(value, -2);
    EXPECT_LT(value, 3);
  }
}

TEST(RandomTest, DistributionFactoriesShareTheRandomEngine) {
  Random first;
  Random second;
  first.SetSeed(kSeed);
  second.SetSeed(kSeed);

  auto uniform_first = first.GetUniformRng(3, 4);
  auto uniform_second = second.GetUniformRng(3, 4);
  EXPECT_REAL_EQ(uniform_first.Sample(), uniform_second.Sample());
  EXPECT_EQ(uniform_first.SampleArray<5>(), uniform_second.SampleArray<5>());

  auto gaus_first = first.GetGausRng(3, 4);
  auto gaus_second = second.GetGausRng(3, 4);
  EXPECT_EQ(gaus_first.Sample3(), gaus_second.Sample3());

  auto exp_first = first.GetExpRng(5);
  auto exp_second = second.GetExpRng(5);
  EXPECT_EQ(exp_first.Sample2(), exp_second.Sample2());

  auto poisson_d_first = first.GetPoissonDRng(6);
  auto poisson_d_second = second.GetPoissonDRng(6);
  EXPECT_REAL_EQ(poisson_d_first.Sample(), poisson_d_second.Sample());

  auto breit_wigner_first = first.GetBreitWignerRng(7, 8);
  auto breit_wigner_second = second.GetBreitWignerRng(7, 8);
  EXPECT_REAL_EQ(breit_wigner_first.Sample(), breit_wigner_second.Sample());

  auto binomial_first = first.GetBinomialRng(10, 0.25);
  auto binomial_second = second.GetBinomialRng(10, 0.25);
  EXPECT_EQ(binomial_first.Sample(), binomial_second.Sample());

  auto poisson_first = first.GetPoissonRng(11);
  auto poisson_second = second.GetPoissonRng(11);
  EXPECT_EQ(poisson_first.Sample(), poisson_second.Sample());
}

TEST(RandomTest, DistributionValuesRespectTheirDomains) {
  Random random;
  random.SetSeed(kSeed);

  for (uint64_t i = 0; i < 100; ++i) {
    EXPECT_TRUE(std::isfinite(random.Gaus()));
    EXPECT_GE(random.Exp(2), 0);
    EXPECT_GE(random.PoissonD(3), 0);
    EXPECT_TRUE(std::isfinite(random.BreitWigner()));
    EXPECT_LT(random.Integer(7), 7u);
    const auto binomial = random.Binomial(8, 0.25);
    EXPECT_GE(binomial, 0);
    EXPECT_LE(binomial, 8);
    EXPECT_GE(random.Poisson(9), 0);
  }

  EXPECT_EQ(random.Exp(0), 0);
}

TEST(RandomTest, CircleAndSphereSamplesLieOnSurface) {
  Random random;
  random.SetSeed(kSeed);

  for (uint64_t i = 0; i < 100; ++i) {
    EXPECT_NEAR(random.Circle(3).Norm(), 3, abs_error<real_t>::value * 10);
    EXPECT_NEAR(random.Sphere(4).Norm(), 4, abs_error<real_t>::value * 10);
  }
}

TEST(RandomTest, RejectsInvalidDistributionParameters) {
  Random random;

  EXPECT_THROW(random.Uniform(2, 1), std::invalid_argument);
  EXPECT_THROW(random.Gaus(0, 0), std::invalid_argument);
  EXPECT_THROW(random.Exp(-1), std::invalid_argument);
  EXPECT_THROW(random.PoissonD(-1), std::invalid_argument);
  EXPECT_THROW(random.BreitWigner(0, 0), std::invalid_argument);
  EXPECT_THROW(random.Integer(0), std::invalid_argument);
  EXPECT_THROW(random.Binomial(-1, 0.5), std::invalid_argument);
  EXPECT_THROW(random.Binomial(1, -0.1), std::invalid_argument);
  EXPECT_THROW(random.Binomial(1, 1.1), std::invalid_argument);
  EXPECT_THROW(random.Poisson(-1), std::invalid_argument);
  EXPECT_THROW(random.Circle(-1), std::invalid_argument);
  EXPECT_THROW(random.Sphere(-1), std::invalid_argument);
}

TEST(RandomTest, CopyPreservesEngineState) {
  Random original;
  original.SetSeed(kSeed);
  original.Uniform();

  Random copy = original;
  EXPECT_REAL_EQ(original.Uniform(), copy.Uniform());
  EXPECT_REAL_EQ(original.Gaus(), copy.Gaus());
}

}  // namespace
}  // namespace bdm
