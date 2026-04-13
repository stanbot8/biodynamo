// -----------------------------------------------------------------------------
//
// Copyright (C) 2026 stanbot8 fork of BioDynaMo.
// Licensed under the Apache License, Version 2.0 (the "License").
// See the LICENSE file distributed with this work for details.
//
// -----------------------------------------------------------------------------
//
// Tradewind demo: seeds a cubic domain at the rear wall, applies a uniform
// +x advection, and places a sealed barrier plane near the front with a
// small central gap. Writes x-profiles and a leading-edge mass time series
// to stdout as CSV. Set TRADEWIND_NO_WIND=1 to disable the advection for the
// pure-diffusion baseline.
//
// Reference: OHSU 2026 (Nature Commun. s41467-026-70688-6).

#include "tradewind.h"

#include <cmath>
#include <iostream>
#include <string>

#include "core/agent/cell.h"
#include "core/diffusion/advection_diffusion_grid.h"
#include "core/environment/environment.h"
#include "core/resource_manager.h"
#include "core/simulation.h"

namespace bdm {

namespace {

// The cell is a cube [0, L]^3 in simulated units. We use a single dummy
// cell to force the environment bounds, then work directly on the grid.
constexpr real_t kL = 40.0;     // um: rough cell dimension
constexpr real_t kDiff = 0.05;  // um^2 / step
constexpr real_t kWind = 4.0;   // um / step (+x), ~fast trade wind
constexpr int kRes = 40;        // voxels per axis
constexpr real_t kDt = 0.02;
constexpr int kSteps = 400;
constexpr real_t kBarrierFrac = 0.75;  // barrier at x = 0.75 * L
constexpr real_t kGapHalfWidth = 3.0;  // gap half-size in y,z (um)

void SeedRearSource(AdvectionDiffusionGrid* g) {
  const real_t bl = g->GetBoxLength();
  const auto dims = g->GetDimensions();
  // Strip at x = 2*bl (a couple voxels from the rear wall), full y/z.
  const real_t x = dims[0] + 2 * bl;
  for (int yi = 0; yi < kRes; yi++) {
    for (int zi = 0; zi < kRes; zi++) {
      const real_t y = dims[0] + (yi + 0.5) * bl;
      const real_t z = dims[0] + (zi + 0.5) * bl;
      g->ChangeConcentrationBy({x, y, z}, 100.0);
    }
  }
}

void SetUniformWind(AdvectionDiffusionGrid* g, real_t vx) {
  for (size_t i = 0; i < g->GetNumBoxes(); i++) {
    g->SetVelocity(i, Real3{vx, 0, 0});
  }
}

// Seals the +x face of every voxel in the barrier plane, except a small
// square gap centered on (y0, z0).
void InstallBarrier(AdvectionDiffusionGrid* g) {
  const real_t bl = g->GetBoxLength();
  const auto dims = g->GetDimensions();
  const real_t L = dims[1] - dims[0];
  const real_t y0 = dims[0] + 0.5 * L;
  const real_t z0 = dims[0] + 0.5 * L;
  // Pick the voxel index whose +x face sits at x = dims[0] + barrier_frac * L.
  const int bx = static_cast<int>(kBarrierFrac * kRes) - 1;
  const real_t bx_center = dims[0] + (bx + 0.5) * bl;
  for (int yi = 0; yi < kRes; yi++) {
    for (int zi = 0; zi < kRes; zi++) {
      const real_t y = dims[0] + (yi + 0.5) * bl;
      const real_t z = dims[0] + (zi + 0.5) * bl;
      const bool in_gap = std::fabs(y - y0) <= kGapHalfWidth &&
                          std::fabs(z - z0) <= kGapHalfWidth;
      if (!in_gap) {
        g->SetPermeability({bx_center, y, z}, 0, 0.0);
      }
    }
  }
}

real_t LeadingEdgeMass(const AdvectionDiffusionGrid& g) {
  const real_t bl = g.GetBoxLength();
  const auto dims = g.GetDimensions();
  const real_t x_cut = dims[0] + kBarrierFrac * (dims[1] - dims[0]);
  const real_t* c = g.GetAllConcentrations();
  const size_t res = g.GetResolution();
  real_t sum = 0;
  for (size_t z = 0; z < res; z++) {
    for (size_t y = 0; y < res; y++) {
      for (size_t x = 0; x < res; x++) {
        const real_t rx = dims[0] + (x + 0.5) * bl;
        if (rx > x_cut) {
          sum += c[x + y * res + z * res * res];
        }
      }
    }
  }
  return sum * g.GetBoxVolume();
}

void PrintXProfile(const AdvectionDiffusionGrid& g, const std::string& tag) {
  const real_t bl = g.GetBoxLength();
  const auto dims = g.GetDimensions();
  const real_t* c = g.GetAllConcentrations();
  const size_t res = g.GetResolution();
  std::cout << "# x-profile tag=" << tag << "\n# x,mean_c\n";
  for (size_t x = 0; x < res; x++) {
    real_t sum = 0;
    for (size_t z = 0; z < res; z++) {
      for (size_t y = 0; y < res; y++) {
        sum += c[x + y * res + z * res * res];
      }
    }
    const real_t rx = dims[0] + (x + 0.5) * bl;
    std::cout << rx << "," << sum / (res * res) << "\n";
  }
}

}  // namespace

int Simulate(int argc, const char** argv) {
  Simulation simulation(argc, argv);

  // BDM's Simulation consumes argv and rejects unknown flags, so use
  // an env var for the demo-only toggle. Set TRADEWIND_NO_WIND=1 to run
  // the pure-diffusion baseline.
  const char* flag = std::getenv("TRADEWIND_NO_WIND");
  const bool with_wind = !(flag && std::string(flag) == "1");

  auto* rm = simulation.GetResourceManager();
  auto* env = simulation.GetEnvironment();

  // Dummy cells to anchor environment bounds to roughly [0, L]^3.
  auto* c0 = new Cell({0, 0, 0});
  c0->SetDiameter(1);
  rm->AddAgent(c0);
  auto* c1 = new Cell({kL, kL, kL});
  c1->SetDiameter(1);
  rm->AddAgent(c1);
  env->ForcedUpdate();

  auto* g = new AdvectionDiffusionGrid(0, "A", kDiff, 0.0, kRes);
  g->Initialize();
  g->SetBoundaryConditionType(BoundaryConditionType::kClosedBoundaries);

  SeedRearSource(g);
  if (with_wind)
    SetUniformWind(g, kWind);
  InstallBarrier(g);

  PrintXProfile(*g, "t=0");
  std::cout << "# leading_edge_mass,t\n";
  std::cout << "0," << LeadingEdgeMass(*g) << "\n";

  for (int step = 1; step <= kSteps; step++) {
    g->Diffuse(kDt);
    if (step % 50 == 0) {
      std::cout << step * kDt << "," << LeadingEdgeMass(*g) << "\n";
    }
  }

  PrintXProfile(*g, with_wind ? "t_end_wind" : "t_end_nowind");

  delete g;
  return 0;
}

}  // namespace bdm

int main(int argc, const char** argv) { return bdm::Simulate(argc, argv); }
