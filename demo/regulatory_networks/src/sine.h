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

#ifndef SINE_H_
#define SINE_H_

#include <fstream>
#include <iostream>

#include <boost/array.hpp>
#include "boost/numeric/odeint.hpp"
#include "boost/phoenix/core.hpp"
#include "boost/phoenix/operator.hpp"
#include "core/util/math.h"

typedef boost::numeric::ublas::vector<double> b_vector_t;
typedef boost::numeric::ublas::matrix<double> b_matrix_t;

namespace sine {

constexpr double kIntegrationTolerance = 1e-6;
constexpr double kIntegrationStep = 1e-3;
constexpr double kEndTime = 4.0 * bdm::Math::kPi;

struct ODE_system {
  void operator()(const b_vector_t& x, b_vector_t& dxdt, double t) const {
    dxdt[0] = A * cos(t);
  }
  //
  const double A = 10.0;
};

struct ODE_output {
  void operator()(const b_vector_t& x, double t) {
    std::clog << t << ',' << x[0] << std::endl;
  }
};
inline int Simulate(int argc, const char** argv) {
  std::ofstream fout("sine.csv");
  // save the original buffer of std::clog
  std::streambuf* orig_clog_buff = std::clog.rdbuf();
  // redirect std::clog to point to the above file
  std::clog.rdbuf(fout.rdbuf());

  b_vector_t x(1);
  x[0] = 0.0;

  typedef boost::numeric::odeint::runge_kutta_dopri5<b_vector_t> ode_int;

  // set-up the Runge-Kutta integrator
  auto stepper = boost::numeric::odeint::make_dense_output<ode_int>(
      kIntegrationTolerance, kIntegrationTolerance);

  // perform the time-integration
  integrate_const(stepper, ODE_system(), x, 0.0, kEndTime, kIntegrationStep,
                  ODE_output());
  // restore the original buffer of std::clog
  std::clog.rdbuf(orig_clog_buff);

  return 0;
}

}  // namespace sine

#endif  // SINE_H_
