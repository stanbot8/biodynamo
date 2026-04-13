// -----------------------------------------------------------------------------
//
// Copyright (C) 2026 stanbot8 fork of BioDynaMo.
// Licensed under the Apache License, Version 2.0 (the "License").
// See the LICENSE file distributed with this work for details.
//
// -----------------------------------------------------------------------------

#include "core/diffusion/advection_diffusion_grid.h"

#include <algorithm>
#include <cmath>

#include "core/util/log.h"

namespace bdm {

void AdvectionDiffusionGrid::Initialize() {
  DiffusionGrid::Initialize();
  EnsureFlowFieldSized();
}

void AdvectionDiffusionGrid::Update() {
  DiffusionGrid::Update();
  EnsureFlowFieldSized();
}

void AdvectionDiffusionGrid::EnsureFlowFieldSized() {
  if (velocity_.size() != GetNumBoxes()) {
    velocity_.resize(GetNumBoxes());
    std::fill(velocity_.begin(), velocity_.end(), Real3{0, 0, 0});
  }
  if (px_.size() != GetNumBoxes()) {
    px_.resize(GetNumBoxes());
    py_.resize(GetNumBoxes());
    pz_.resize(GetNumBoxes());
    std::fill(px_.begin(), px_.end(), real_t{1});
    std::fill(py_.begin(), py_.end(), real_t{1});
    std::fill(pz_.begin(), pz_.end(), real_t{1});
  }
}

void AdvectionDiffusionGrid::ClearFlowField() {
  EnsureFlowFieldSized();
  std::fill(velocity_.begin(), velocity_.end(), Real3{0, 0, 0});
  std::fill(px_.begin(), px_.end(), real_t{1});
  std::fill(py_.begin(), py_.end(), real_t{1});
  std::fill(pz_.begin(), pz_.end(), real_t{1});
}

void AdvectionDiffusionGrid::SetVelocity(const Real3& position,
                                         const Real3& v) {
  EnsureFlowFieldSized();
  velocity_[GetBoxIndex(position)] = v;
}

void AdvectionDiffusionGrid::SetVelocity(size_t idx, const Real3& v) {
  EnsureFlowFieldSized();
  velocity_[idx] = v;
}

Real3 AdvectionDiffusionGrid::GetVelocity(const Real3& position) const {
  if (velocity_.size() != GetNumBoxes()) {
    return {0, 0, 0};
  }
  return velocity_[GetBoxIndex(position)];
}

void AdvectionDiffusionGrid::SetPermeability(const Real3& position, int axis,
                                             real_t value) {
  EnsureFlowFieldSized();
  const size_t idx = GetBoxIndex(position);
  value = std::clamp(value, real_t{0}, real_t{1});
  if (axis == 0) {
    px_[idx] = value;
  } else if (axis == 1) {
    py_[idx] = value;
  } else if (axis == 2) {
    pz_[idx] = value;
  } else {
    Log::Fatal("AdvectionDiffusionGrid::SetPermeability",
               "axis must be 0 (x), 1 (y), or 2 (z)");
  }
}

real_t AdvectionDiffusionGrid::GetPermeability(size_t idx, int axis) const {
  if (px_.size() != GetNumBoxes()) {
    return 1;
  }
  if (axis == 0)
    return px_[idx];
  if (axis == 1)
    return py_[idx];
  if (axis == 2)
    return pz_[idx];
  return 1;
}

namespace {

inline real_t UpwindFlux(real_t v_face, real_t c_lo, real_t c_hi) {
  return v_face >= 0 ? v_face * c_lo : v_face * c_hi;
}

}  // namespace

void AdvectionDiffusionGrid::DiffuseWithClosedEdge(real_t dt) {
  EnsureFlowFieldSized();

  const size_t nx = GetResolution();
  const size_t ny = nx;
  const size_t nz = nx;
  const real_t dx = GetBoxLength();
  const real_t inv_dx = 1 / dx;
  const real_t inv_dx2 = inv_dx * inv_dx;
  const real_t d = GetRawDiffusionCoefficient();
  const real_t mu = GetDecayConstant();
  real_t* c1 = GetConcentrationPtr();
  real_t* c2 = GetScratchPtr();

  if (!cfl_warned_) {
    real_t vmax = 0;
    for (size_t i = 0; i < velocity_.size(); i++) {
      const auto& v = velocity_[i];
      const real_t mag = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
      if (mag > vmax)
        vmax = mag;
    }
    const real_t cfl = dt * (6 * d * inv_dx2 + vmax * inv_dx);
    if (cfl >= 1) {
      Log::Warning("AdvectionDiffusionGrid",
                   "CFL violated: dt * (6 D / dx^2 + |v|_max / dx) = ", cfl,
                   " >= 1. Scheme is unstable. Substance: '",
                   GetContinuumName(), "' dt=", dt, " D=", d, " dx=", dx,
                   " |v|_max=", vmax);
      cfl_warned_ = true;
    }
  }

#pragma omp parallel for collapse(2)
  for (size_t z = 0; z < nz; z++) {
    for (size_t y = 0; y < ny; y++) {
      for (size_t x = 0; x < nx; x++) {
        const size_t c = x + y * nx + z * nx * ny;
        const real_t cc = c1[c];
        real_t flux_in = 0;

        // dc/dt = -div(v c): inflow at -face contributes +v*c_upwind/dx,
        // outflow at +face contributes -v*c_upwind/dx.
        if (x > 0) {
          const size_t l = c - 1;
          const real_t p = px_[l];
          const real_t v_face = 0.5 * (velocity_[l][0] + velocity_[c][0]);
          const real_t diff = d * inv_dx2 * (c1[l] - cc);
          const real_t adv = UpwindFlux(v_face, c1[l], cc) * inv_dx;
          flux_in += p * (diff + adv);
        }
        if (x + 1 < nx) {
          const size_t r = c + 1;
          const real_t p = px_[c];
          const real_t v_face = 0.5 * (velocity_[c][0] + velocity_[r][0]);
          const real_t diff = d * inv_dx2 * (c1[r] - cc);
          const real_t adv = -UpwindFlux(v_face, cc, c1[r]) * inv_dx;
          flux_in += p * (diff + adv);
        }
        if (y > 0) {
          const size_t n = c - nx;
          const real_t p = py_[n];
          const real_t v_face = 0.5 * (velocity_[n][1] + velocity_[c][1]);
          const real_t diff = d * inv_dx2 * (c1[n] - cc);
          const real_t adv = UpwindFlux(v_face, c1[n], cc) * inv_dx;
          flux_in += p * (diff + adv);
        }
        if (y + 1 < ny) {
          const size_t s = c + nx;
          const real_t p = py_[c];
          const real_t v_face = 0.5 * (velocity_[c][1] + velocity_[s][1]);
          const real_t diff = d * inv_dx2 * (c1[s] - cc);
          const real_t adv = -UpwindFlux(v_face, cc, c1[s]) * inv_dx;
          flux_in += p * (diff + adv);
        }
        if (z > 0) {
          const size_t b = c - nx * ny;
          const real_t p = pz_[b];
          const real_t v_face = 0.5 * (velocity_[b][2] + velocity_[c][2]);
          const real_t diff = d * inv_dx2 * (c1[b] - cc);
          const real_t adv = UpwindFlux(v_face, c1[b], cc) * inv_dx;
          flux_in += p * (diff + adv);
        }
        if (z + 1 < nz) {
          const size_t t = c + nx * ny;
          const real_t p = pz_[c];
          const real_t v_face = 0.5 * (velocity_[c][2] + velocity_[t][2]);
          const real_t diff = d * inv_dx2 * (c1[t] - cc);
          const real_t adv = -UpwindFlux(v_face, cc, c1[t]) * inv_dx;
          flux_in += p * (diff + adv);
        }

        c2[c] = cc * (1 - mu * dt) + dt * flux_in;
      }
    }
  }
  SwapBuffers();
}

// Non-closed-edge variants delegate: advection-aware inflow boundaries need
// careful upwind handling, and the closed-edge case matches the cell-interior
// setup the feature targets.
void AdvectionDiffusionGrid::DiffuseWithOpenEdge(real_t dt) {
  DiffuseWithClosedEdge(dt);
}
void AdvectionDiffusionGrid::DiffuseWithDirichlet(real_t dt) {
  DiffuseWithClosedEdge(dt);
}
void AdvectionDiffusionGrid::DiffuseWithNeumann(real_t dt) {
  DiffuseWithClosedEdge(dt);
}
void AdvectionDiffusionGrid::DiffuseWithPeriodic(real_t dt) {
  DiffuseWithClosedEdge(dt);
}

}  // namespace bdm
