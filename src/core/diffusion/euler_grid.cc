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

#include "core/diffusion/euler_grid.h"
#include "core/resource_manager.h"
#include "core/simulation.h"

namespace bdm {

void EulerGrid::DiffuseWithClosedEdge(real_t dt) {

  const size_t res = GetResolution();
  const real_t bl = GetBoxLength();
  const real_t dc0 = GetDiffusionCoefficients()[0];
  const real_t mu = GetDecayConstant();
  real_t* c1 = GetConcentrationPtr();
  real_t* c2 = GetScratchPtr();
  const auto nx = res;
  const auto ny = res;
  const auto nz = res;

  const real_t ibl2 = 1 / (bl * bl);
  const real_t d = 1 - dc0;

  constexpr size_t YBF = 16;
#pragma omp parallel for collapse(2)
  for (size_t yy = 0; yy < ny; yy += YBF) {
    for (size_t z = 0; z < nz; z++) {
      size_t ymax = yy + YBF;
      if (ymax >= ny) {
        ymax = ny;
      }
      for (size_t y = yy; y < ymax; y++) {
        size_t x{0};
        size_t c{0};
        size_t n{0};
        size_t s{0};
        size_t b{0};
        size_t t{0};
        c = x + y * nx + z * nx * ny;
#pragma omp simd
        for (x = 1; x < nx - 1; x++) {
          ++c;

          if (y == 0 || y == (ny - 1) || z == 0 || z == (nz - 1)) {
            continue;
          }

          n = c - nx;
          s = c + nx;
          b = c - nx * ny;
          t = c + nx * ny;

          c2[c] = c1[c] * (1 - mu * dt) +
                   (d * dt * ibl2) *
                       (c1[c - 1] - 2 * c1[c] + c1[c + 1] + c1[s] -
                        2 * c1[c] + c1[n] + c1[b] - 2 * c1[c] + c1[t]);
        }
      }  // tile ny
    }    // tile nz
  }      // block ny
  SwapBuffers();
}

void EulerGrid::DiffuseWithOpenEdge(real_t dt) {

  const size_t res = GetResolution();
  const real_t bl = GetBoxLength();
  const real_t dc0 = GetDiffusionCoefficients()[0];
  const real_t mu = GetDecayConstant();
  real_t* c1 = GetConcentrationPtr();
  real_t* c2 = GetScratchPtr();
  const auto nx = res;
  const auto ny = res;
  const auto nz = res;

  const real_t ibl2 = 1 / (bl * bl);
  const real_t d = 1 - dc0;
  std::array<int, 4> l;

  constexpr size_t YBF = 16;
#pragma omp parallel for collapse(2)
  for (size_t yy = 0; yy < ny; yy += YBF) {
    for (size_t z = 0; z < nz; z++) {
      size_t ymax = yy + YBF;
      if (ymax >= ny) {
        ymax = ny;
      }
      for (size_t y = yy; y < ymax; y++) {
        size_t x{0};
        size_t c{0};
        size_t n{0};
        size_t s{0};
        size_t b{0};
        size_t t{0};
        c = x + y * nx + z * nx * ny;

        l.fill(1);

        if (y == 0) {
          n = c;
          l[0] = 0;
        } else {
          n = c - nx;
        }

        if (y == ny - 1) {
          s = c;
          l[1] = 0;
        } else {
          s = c + nx;
        }

        if (z == 0) {
          b = c;
          l[2] = 0;
        } else {
          b = c - nx * ny;
        }

        if (z == nz - 1) {
          t = c;
          l[3] = 0;
        } else {
          t = c + nx * ny;
        }

        c2[c] = c1[c] * (1 - mu * dt) +
                 (d * dt * ibl2) *
                     (0 - 2 * c1[c] + c1[c + 1] + c1[s] - 2 * c1[c] +
                      c1[n] + c1[b] - 2 * c1[c] + c1[t]);
#pragma omp simd
        for (x = 1; x < nx - 1; x++) {
          ++c;
          ++n;
          ++s;
          ++b;
          ++t;
          c2[c] =
              c1[c] * (1 - mu * dt) +
              (d * dt * ibl2) * (c1[c - 1] - 2 * c1[c] + c1[c + 1] +
                                 l[0] * c1[s] - 2 * c1[c] + l[1] * c1[n] +
                                 l[2] * c1[b] - 2 * c1[c] + l[3] * c1[t]);
        }
        ++c;
        ++n;
        ++s;
        ++b;
        ++t;
        c2[c] = c1[c] * (1 - mu * dt) +
                 (d * dt * ibl2) *
                     (c1[c - 1] - 2 * c1[c] + 0 + c1[s] - 2 * c1[c] +
                      c1[n] + c1[b] - 2 * c1[c] + c1[t]);
      }  // tile ny
    }    // tile nz
  }      // block ny
  SwapBuffers();
}

void EulerGrid::DiffuseWithDirichlet(real_t dt) {

  const size_t res = GetResolution();
  const real_t bl = GetBoxLength();
  const auto& gd = GetDimensions();
  const real_t dc0 = GetDiffusionCoefficients()[0];
  const real_t mu = GetDecayConstant();
  real_t* c1 = GetConcentrationPtr();
  real_t* c2 = GetScratchPtr();
  const auto nx = res;
  const auto ny = res;
  const auto nz = res;

  const real_t ibl2 = 1 / (bl * bl);
  const real_t d = 1 - dc0;

  const auto sim_time = GetSimulatedTime();

  constexpr size_t YBF = 16;
#pragma omp parallel for collapse(2)
  for (size_t yy = 0; yy < ny; yy += YBF) {
    for (size_t z = 0; z < nz; z++) {
      size_t ymax = yy + YBF;
      if (ymax >= ny) {
        ymax = ny;
      }
      for (size_t y = yy; y < ymax; y++) {
        size_t x{0};
        size_t c{0};
        size_t n{0};
        size_t s{0};
        size_t b{0};
        size_t t{0};
        c = x + y * nx + z * nx * ny;
#pragma omp simd
        for (x = 0; x < nx; x++) {
          if (x == 0 || x == (nx - 1) || y == 0 || y == (ny - 1) || z == 0 ||
              z == (nz - 1)) {
            // For all boxes on the boundary, we simply evaluate the boundary
            real_t real_x = gd[0] + x * bl;
            real_t real_y = gd[0] + y * bl;
            real_t real_z = gd[0] + z * bl;
            c2[c] =
                GetBoundaryCondition()->Evaluate(real_x, real_y, real_z, sim_time);
          } else {
            // For inner boxes, we compute the regular stencil update
            n = c - nx;
            s = c + nx;
            b = c - nx * ny;
            t = c + nx * ny;

            c2[c] = c1[c] * (1 - mu * dt) +
                     (d * dt * ibl2) *
                         (c1[c - 1] - 2 * c1[c] + c1[c + 1] + c1[s] -
                          2 * c1[c] + c1[n] + c1[b] - 2 * c1[c] + c1[t]);
          }
          ++c;
        }
      }  // tile ny
    }    // tile nz
  }      // block ny
  SwapBuffers();
}

void EulerGrid::DiffuseWithNeumann(real_t dt) {

  const size_t res = GetResolution();
  const real_t bl = GetBoxLength();
  const auto& gd = GetDimensions();
  const real_t dc0 = GetDiffusionCoefficients()[0];
  const real_t mu = GetDecayConstant();
  real_t* c1 = GetConcentrationPtr();
  real_t* c2 = GetScratchPtr();
  const size_t nx = res;
  const size_t ny = res;
  const size_t nz = res;
  const size_t num_boxes = nx * ny * nz;

  const real_t ibl2 = 1 / (bl * bl);
  const real_t d = 1 - dc0;

  const auto sim_time = GetSimulatedTime();

  constexpr size_t YBF = 16;
#pragma omp parallel for collapse(2)
  for (size_t yy = 0; yy < ny; yy += YBF) {
    for (size_t z = 0; z < nz; z++) {
      size_t ymax = yy + YBF;
      if (ymax >= ny) {
        ymax = ny;
      }
      for (size_t y = yy; y < ymax; y++) {
        size_t x{0};
        size_t c{0};
        size_t n{0};
        size_t s{0};
        size_t b{0};
        size_t t{0};
        c = x + y * nx + z * nx * ny;
#pragma omp simd
        for (x = 0; x < nx; x++) {
          n = c - nx;
          s = c + nx;
          b = c - nx * ny;
          t = c + nx * ny;

          // Clamp to avoid out of bounds access. Clamped values are initialized
          // to a wrong value but will be overwritten by the boundary condition
          // evaluation. All other values are correct.
          real_t left{c1[std::clamp(c - 1, size_t{0}, num_boxes - 1)]};
          real_t right{c1[std::clamp(c + 1, size_t{0}, num_boxes - 1)]};
          real_t bottom{c1[std::clamp(b, size_t{0}, num_boxes - 1)]};
          real_t top{c1[std::clamp(t, size_t{0}, num_boxes - 1)]};
          real_t north{c1[std::clamp(n, size_t{0}, num_boxes - 1)]};
          real_t south{c1[std::clamp(s, size_t{0}, num_boxes - 1)]};
          real_t center_factor{6.0};

          if (x == 0 || x == (nx - 1) || y == 0 || y == (ny - 1) || z == 0 ||
              z == (nz - 1)) {
            real_t real_x = gd[0] + x * bl;
            real_t real_y = gd[0] + y * bl;
            real_t real_z = gd[0] + z * bl;
            real_t boundary_value =
                -bl *
                GetBoundaryCondition()->Evaluate(real_x, real_y, real_z, sim_time);

            if (x == 0) {
              left = boundary_value;
              center_factor -= 1.0;
            } else if (x == (nx - 1)) {
              right = boundary_value;
              center_factor -= 1.0;
            }

            if (y == 0) {
              north = boundary_value;
              center_factor -= 1.0;
            } else if (y == (ny - 1)) {
              south = boundary_value;
              center_factor -= 1.0;
            }

            if (z == 0) {
              bottom = boundary_value;
              center_factor -= 1.0;
            } else if (z == (nz - 1)) {
              top = boundary_value;
              center_factor -= 1.0;
            }
          }

          c2[c] = c1[c] * (1 - mu * dt) +
                   (d * dt * ibl2) * (left + right + south + north + top +
                                      bottom - center_factor * c1[c]);

          ++c;
        }
      }  // tile ny
    }    // tile nz
  }      // block ny
  SwapBuffers();
}

void EulerGrid::DiffuseWithPeriodic(real_t dt) {

  const size_t res = GetResolution();
  const real_t bl = GetBoxLength();
  const real_t dc0 = GetDiffusionCoefficients()[0];
  const real_t mu = GetDecayConstant();
  real_t* c1 = GetConcentrationPtr();
  real_t* c2 = GetScratchPtr();
  const size_t nx = res;
  const size_t ny = res;
  const size_t nz = res;

  const real_t dx = bl;
  const real_t d = 1 - dc0;

  constexpr size_t YBF = 16;
#pragma omp parallel for collapse(2)
  for (size_t yy = 0; yy < ny; yy += YBF) {
    for (size_t z = 0; z < nz; z++) {
      size_t ymax = yy + YBF;
      if (ymax >= ny) {
        ymax = ny;
      }
      for (size_t y = yy; y < ymax; y++) {
        size_t l{0};
        size_t r{0};
        size_t n{0};
        size_t s{0};
        size_t b{0};
        size_t t{0};
        size_t c = y * nx + z * nx * ny;
#pragma omp simd
        for (size_t x = 0; x < nx; x++) {
          l = c - 1;
          r = c + 1;
          n = c - nx;
          s = c + nx;
          b = c - nx * ny;
          t = c + nx * ny;

          // Adapt neighbor indices for boundary boxes for periodic boundary
          if (x == 0) {
            l = nx - 1 + y * nx + z * nx * ny;
          } else if (x == (nx - 1)) {
            r = 0 + y * nx + z * nx * ny;
          }

          if (y == 0) {
            n = x + (ny - 1) * nx + z * nx * ny;
          } else if (y == (ny - 1)) {
            s = x + 0 * nx + z * nx * ny;
          }

          if (z == 0) {
            b = x + y * nx + (nz - 1) * nx * ny;
          } else if (z == (nz - 1)) {
            t = x + y * nx + 0 * nx * ny;
          }

          // Stencil update
          c2[c] = c1[c] * (1 - (mu * dt)) +
                   ((d * dt / (dx * dx)) * (c1[l] + c1[r] + c1[n] + c1[s] +
                                            c1[t] + c1[b] - 6.0 * c1[c]));

          ++c;
        }
      }  // tile ny
    }    // tile nz
  }      // block ny
  SwapBuffers();
}

}  // namespace bdm
