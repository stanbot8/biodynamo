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

namespace bdm {
namespace {

constexpr real_t kPi =
    // Preserve pi when real_t is configured as double.
    static_cast<real_t>(3.141592653589793238462643383279502884L);

real_t RequireNonnegative(real_t value, const char* message) {
  if (value < 0) {
    throw std::invalid_argument(message);
  }
  return value;
}

real_t RequirePositive(real_t value, const char* message) {
  if (value <= 0) {
    throw std::invalid_argument(message);
  }
  return value;
}

real_t RequireOrdered(real_t min, real_t max) {
  if (max < min) {
    throw std::invalid_argument(
        "uniform distribution maximum must not be less than minimum");
  }
  return max;
}

int RequireNonnegative(int value, const char* message) {
  if (value < 0) {
    throw std::invalid_argument(message);
  }
  return value;
}

real_t RequireProbability(real_t probability) {
  if (probability < 0 || probability > 1) {
    throw std::invalid_argument(
        "binomial distribution probability must be between zero and one");
  }
  return probability;
}

}  // namespace

UniformRng::UniformRng(std::mt19937_64* engine, real_t min, real_t max)
    : DistributionRng(engine), distribution_(min, RequireOrdered(min, max)) {}

real_t UniformRng::SampleImpl(std::mt19937_64& engine) {
  return distribution_(engine);
}

GausRng::GausRng(std::mt19937_64* engine, real_t mean, real_t sigma)
    : DistributionRng(engine),
      distribution_(mean,
                    RequirePositive(
                        sigma, "normal distribution sigma must be positive")) {}

real_t GausRng::SampleImpl(std::mt19937_64& engine) {
  return distribution_(engine);
}

ExpRng::ExpRng(std::mt19937_64* engine, real_t tau)
    : DistributionRng(engine),
      tau_(RequireNonnegative(
          tau, "exponential distribution tau must be nonnegative")),
      distribution_(tau_ > 0 ? 1 / tau_ : 1) {}

real_t ExpRng::SampleImpl(std::mt19937_64& engine) {
  return tau_ == 0 ? 0 : distribution_(engine);
}

PoissonDRng::PoissonDRng(std::mt19937_64* engine, real_t mean)
    : DistributionRng(engine),
      distribution_(RequireNonnegative(
          mean, "Poisson distribution mean must be nonnegative")) {}

real_t PoissonDRng::SampleImpl(std::mt19937_64& engine) {
  return static_cast<real_t>(distribution_(engine));
}

BreitWignerRng::BreitWignerRng(std::mt19937_64* engine, real_t mean,
                               real_t gamma)
    : DistributionRng(engine),
      distribution_(
          mean,
          RequirePositive(gamma, "Breit-Wigner gamma must be positive") / 2) {}

real_t BreitWignerRng::SampleImpl(std::mt19937_64& engine) {
  return distribution_(engine);
}

BinomialRng::BinomialRng(std::mt19937_64* engine, int trials,
                         real_t probability)
    : DistributionRng(engine),
      distribution_(
          RequireNonnegative(
              trials, "binomial distribution trials must be nonnegative"),
          RequireProbability(probability)) {}

int BinomialRng::SampleImpl(std::mt19937_64& engine) {
  return distribution_(engine);
}

PoissonRng::PoissonRng(std::mt19937_64* engine, real_t mean)
    : DistributionRng(engine),
      distribution_(RequireNonnegative(
          mean, "Poisson distribution mean must be nonnegative")) {}

int PoissonRng::SampleImpl(std::mt19937_64& engine) {
  return distribution_(engine);
}

real_t Random::Uniform(real_t max) { return Uniform(0, max); }

real_t Random::Uniform(real_t min, real_t max) {
  return std::uniform_real_distribution<real_t>(
      min, RequireOrdered(min, max))(engine_);
}

real_t Random::Gaus(real_t mean, real_t sigma) {
  return GetGausRng(mean, sigma).Sample();
}

real_t Random::Exp(real_t tau) { return GetExpRng(tau).Sample(); }

real_t Random::PoissonD(real_t mean) { return GetPoissonDRng(mean).Sample(); }

real_t Random::BreitWigner(real_t mean, real_t gamma) {
  return GetBreitWignerRng(mean, gamma).Sample();
}

unsigned Random::Integer(int max) {
  if (max <= 0) {
    throw std::invalid_argument(
        "integer distribution maximum must be positive");
  }
  return std::uniform_int_distribution<unsigned>(0, max - 1)(engine_);
}

int Random::Binomial(int trials, real_t probability) {
  return GetBinomialRng(trials, probability).Sample();
}

int Random::Poisson(real_t mean) { return GetPoissonRng(mean).Sample(); }

MathArray<real_t, 2> Random::Circle(real_t radius) {
  RequireNonnegative(radius, "circle radius must be nonnegative");
  const auto angle = Uniform(0, 2 * kPi);
  return {radius * std::cos(angle), radius * std::sin(angle)};
}

MathArray<real_t, 3> Random::Sphere(real_t radius) {
  RequireNonnegative(radius, "sphere radius must be nonnegative");
  const auto z = Uniform(-radius, radius);
  const auto angle = Uniform(0, 2 * kPi);
  const auto radial = std::sqrt(radius * radius - z * z);
  return {radial * std::cos(angle), radial * std::sin(angle), z};
}

void Random::SetSeed(uint64_t seed) {
  seed_ = seed;
  engine_.seed(seed);
}

uint64_t Random::GetSeed() const { return seed_; }

UniformRng Random::GetUniformRng(real_t min, real_t max) {
  return UniformRng(&engine_, min, max);
}

GausRng Random::GetGausRng(real_t mean, real_t sigma) {
  return GausRng(&engine_, mean, sigma);
}

ExpRng Random::GetExpRng(real_t tau) { return ExpRng(&engine_, tau); }

PoissonDRng Random::GetPoissonDRng(real_t mean) {
  return PoissonDRng(&engine_, mean);
}

BreitWignerRng Random::GetBreitWignerRng(real_t mean, real_t gamma) {
  return BreitWignerRng(&engine_, mean, gamma);
}

BinomialRng Random::GetBinomialRng(int trials, real_t probability) {
  return BinomialRng(&engine_, trials, probability);
}

PoissonRng Random::GetPoissonRng(real_t mean) {
  return PoissonRng(&engine_, mean);
}

}  // namespace bdm
