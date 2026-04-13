// -----------------------------------------------------------------------------
//
// Copyright (C) 2026 stanbot8 fork of BioDynaMo.
// Licensed under the Apache License, Version 2.0 (the "License").
// See the LICENSE file distributed with this work for details.
//
// -----------------------------------------------------------------------------

#include "core/agent/cell.h"
#include "core/diffusion/advection_diffusion_grid.h"
#include "core/environment/environment.h"
#include "core/simulation.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {

namespace {

// Build a small cube of cells so the environment sizes the diffusion grid
// to a known bounding box.
void PopulateBounds(real_t lo, real_t hi) {
  auto* rm = Simulation::GetActive()->GetResourceManager();
  auto* c1 = new Cell({lo, lo, lo});
  c1->SetDiameter(10);
  rm->AddAgent(c1);
  auto* c2 = new Cell({hi, hi, hi});
  c2->SetDiameter(10);
  rm->AddAgent(c2);
}

real_t TotalMass(const AdvectionDiffusionGrid& g) {
  const real_t* c = g.GetAllConcentrations();
  real_t sum = 0;
  for (size_t i = 0; i < g.GetNumBoxes(); i++)
    sum += c[i];
  return sum * g.GetBoxVolume();
}

}  // namespace

// Pure diffusion (velocity = 0) on AdvectionDiffusionGrid must preserve mass
// with closed edges, just like EulerGrid.
TEST(AdvectionDiffusionTest, PureDiffusionConservesMass) {
  Simulation simulation(TEST_NAME);
  auto* env = simulation.GetEnvironment();
  PopulateBounds(0, 100);

  auto* g = new AdvectionDiffusionGrid(0, "A", 0.1, 0.0, 20);
  env->ForcedUpdate();
  g->Initialize();
  g->SetBoundaryConditionType(BoundaryConditionType::kClosedBoundaries);

  g->ChangeConcentrationBy({50, 50, 50}, 1000.0);
  const real_t m0 = TotalMass(*g);
  EXPECT_GT(m0, 0);

  for (int i = 0; i < 50; i++)
    g->Diffuse(0.01);

  EXPECT_NEAR(TotalMass(*g), m0, m0 * 1e-6);
  delete g;
}

// Advection in +x moves mass toward the +x wall: the center-of-mass in x
// must increase over time when v = (+vx, 0, 0).
TEST(AdvectionDiffusionTest, AdvectionShiftsMassDownstream) {
  Simulation simulation(TEST_NAME);
  auto* env = simulation.GetEnvironment();
  PopulateBounds(0, 100);

  auto* g = new AdvectionDiffusionGrid(0, "A", 0.01, 0.0, 20);
  env->ForcedUpdate();
  g->Initialize();
  g->SetBoundaryConditionType(BoundaryConditionType::kClosedBoundaries);

  g->ChangeConcentrationBy({50, 50, 50}, 1000.0);

  // Set uniform +x velocity on every voxel.
  const size_t n = g->GetNumBoxes();
  for (size_t i = 0; i < n; i++) {
    g->SetVelocity(i, Real3{2.0, 0, 0});
  }

  // Center-of-mass in x before and after.
  auto com_x = [&]() {
    const real_t* c = g->GetAllConcentrations();
    const real_t bl = g->GetBoxLength();
    const auto dims = g->GetDimensions();
    const size_t res = g->GetResolution();
    real_t num = 0, den = 0;
    for (size_t z = 0; z < res; z++) {
      for (size_t y = 0; y < res; y++) {
        for (size_t x = 0; x < res; x++) {
          const real_t v = c[x + y * res + z * res * res];
          const real_t xc = dims[0] + (x + 0.5) * bl;
          num += v * xc;
          den += v;
        }
      }
    }
    return num / den;
  };

  const real_t x0 = com_x();
  for (int i = 0; i < 20; i++)
    g->Diffuse(0.05);
  const real_t x1 = com_x();

  EXPECT_GT(x1, x0 + 0.5);  // must have moved noticeably downstream
  delete g;
}

// A sealed barrier (permeability = 0) on one plane must prevent mass from
// crossing it, even with advection pushing toward it.
TEST(AdvectionDiffusionTest, SealedBarrierBlocksTransport) {
  Simulation simulation(TEST_NAME);
  auto* env = simulation.GetEnvironment();
  PopulateBounds(0, 100);

  const int res = 20;
  auto* g = new AdvectionDiffusionGrid(0, "A", 0.01, 0.0, res);
  env->ForcedUpdate();
  g->Initialize();
  g->SetBoundaryConditionType(BoundaryConditionType::kClosedBoundaries);

  // Seed mass in the left half only.
  g->ChangeConcentrationBy({25, 50, 50}, 1000.0);

  // Uniform +x velocity pushes toward +x.
  const size_t n = g->GetNumBoxes();
  for (size_t i = 0; i < n; i++) {
    g->SetVelocity(i, Real3{2.0, 0, 0});
  }

  // Seal the +x face of every voxel in the plane x == res/2 - 1.
  // All such voxels sit at real-x = dims[0] + (res/2 - 0.5) * bl.
  const real_t bl = g->GetBoxLength();
  const auto dims = g->GetDimensions();
  const real_t seal_x = dims[0] + (res / 2 - 0.5) * bl;
  for (int y = 0; y < res; y++) {
    for (int z = 0; z < res; z++) {
      const real_t ry = dims[0] + (y + 0.5) * bl;
      const real_t rz = dims[0] + (z + 0.5) * bl;
      g->SetPermeability({seal_x, ry, rz}, 0, 0.0);
    }
  }

  for (int i = 0; i < 50; i++)
    g->Diffuse(0.05);

  // Sum mass in the right half: should be effectively zero.
  const real_t* c = g->GetAllConcentrations();
  real_t right_mass = 0;
  for (int z = 0; z < res; z++) {
    for (int y = 0; y < res; y++) {
      for (int x = res / 2; x < res; x++) {
        right_mass += c[x + y * res + z * res * res];
      }
    }
  }
  EXPECT_NEAR(right_mass, 0.0, 1e-9);
  delete g;
}

}  // namespace bdm
