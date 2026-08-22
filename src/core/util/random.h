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

#ifndef CORE_UTIL_RANDOM_H_
#define CORE_UTIL_RANDOM_H_

#include <cstdint>
#include <random>

#include "core/container/math_array.h"

namespace bdm {

template <typename TSample>
class DistributionRng {
 public:
  virtual ~DistributionRng() = default;

  TSample Sample() { return SampleImpl(*engine_); }
  MathArray<TSample, 2> Sample2() { return {Sample(), Sample()}; }
  MathArray<TSample, 3> Sample3() { return {Sample(), Sample(), Sample()}; }

  template <uint64_t N>
  MathArray<TSample, N> SampleArray() {
    MathArray<TSample, N> result;
    for (uint64_t i = 0; i < N; ++i) {
      result[i] = Sample();
    }
    return result;
  }

 protected:
  explicit DistributionRng(std::mt19937_64* engine) : engine_(engine) {}
  virtual TSample SampleImpl(std::mt19937_64& engine) = 0;

 private:
  std::mt19937_64* engine_;
};

class UniformRng : public DistributionRng<real_t> {
 public:
  UniformRng(std::mt19937_64* engine, real_t min, real_t max);

 private:
  real_t SampleImpl(std::mt19937_64& engine) override;
  std::uniform_real_distribution<real_t> distribution_;
};

class GausRng : public DistributionRng<real_t> {
 public:
  GausRng(std::mt19937_64* engine, real_t mean, real_t sigma);

 private:
  real_t SampleImpl(std::mt19937_64& engine) override;
  std::normal_distribution<real_t> distribution_;
};

class ExpRng : public DistributionRng<real_t> {
 public:
  ExpRng(std::mt19937_64* engine, real_t tau);

 private:
  real_t SampleImpl(std::mt19937_64& engine) override;
  real_t tau_;
  std::exponential_distribution<real_t> distribution_;
};

class PoissonDRng : public DistributionRng<real_t> {
 public:
  PoissonDRng(std::mt19937_64* engine, real_t mean);

 private:
  real_t SampleImpl(std::mt19937_64& engine) override;
  std::poisson_distribution<uint64_t> distribution_;
};

class BreitWignerRng : public DistributionRng<real_t> {
 public:
  BreitWignerRng(std::mt19937_64* engine, real_t mean, real_t gamma);

 private:
  real_t SampleImpl(std::mt19937_64& engine) override;
  std::cauchy_distribution<real_t> distribution_;
};

class BinomialRng : public DistributionRng<int> {
 public:
  BinomialRng(std::mt19937_64* engine, int trials, real_t probability);

 private:
  int SampleImpl(std::mt19937_64& engine) override;
  std::binomial_distribution<int> distribution_;
};

class PoissonRng : public DistributionRng<int> {
 public:
  PoissonRng(std::mt19937_64* engine, real_t mean);

 private:
  int SampleImpl(std::mt19937_64& engine) override;
  std::poisson_distribution<int> distribution_;
};

class Random {
 public:
  using result_type = std::mt19937_64::result_type;

  Random() = default;
  Random(const Random&) = default;
  Random& operator=(const Random&) = default;
  ~Random() = default;

  static constexpr result_type min() { return std::mt19937_64::min(); }
  static constexpr result_type max() { return std::mt19937_64::max(); }
  result_type operator()() { return engine_(); }

  real_t Uniform(real_t max = 1.0);
  real_t Uniform(real_t min, real_t max);

  template <uint64_t N>
  MathArray<real_t, N> UniformArray(real_t max = 1.0) {
    return UniformArray<N>(0, max);
  }

  template <uint64_t N>
  MathArray<real_t, N> UniformArray(real_t min, real_t max) {
    MathArray<real_t, N> result;
    for (uint64_t i = 0; i < N; ++i) {
      result[i] = Uniform(min, max);
    }
    return result;
  }

  real_t Gaus(real_t mean = 0.0, real_t sigma = 1.0);
  real_t Exp(real_t tau);
  real_t PoissonD(real_t mean);
  real_t BreitWigner(real_t mean = 0, real_t gamma = 1);
  unsigned Integer(int max);
  int Binomial(int trials, real_t probability);
  int Poisson(real_t mean);
  MathArray<real_t, 2> Circle(real_t radius);
  MathArray<real_t, 3> Sphere(real_t radius);

  void SetSeed(uint64_t seed);
  uint64_t GetSeed() const;

  UniformRng GetUniformRng(real_t min = 0, real_t max = 1);
  GausRng GetGausRng(real_t mean = 0, real_t sigma = 1);
  ExpRng GetExpRng(real_t tau);
  PoissonDRng GetPoissonDRng(real_t mean);
  BreitWignerRng GetBreitWignerRng(real_t mean = 0, real_t gamma = 1);
  BinomialRng GetBinomialRng(int trials, real_t probability);
  PoissonRng GetPoissonRng(real_t mean);

 private:
  std::mt19937_64 engine_;
  uint64_t seed_ = std::mt19937_64::default_seed;
};

}  // namespace bdm

#endif  // CORE_UTIL_RANDOM_H_
