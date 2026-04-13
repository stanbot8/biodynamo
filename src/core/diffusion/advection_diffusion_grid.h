// -----------------------------------------------------------------------------
//
// Copyright (C) 2026 stanbot8 fork of BioDynaMo.
// Licensed under the Apache License, Version 2.0 (the "License").
// See the LICENSE file distributed with this work for details.
//
// -----------------------------------------------------------------------------

#ifndef CORE_DIFFUSION_ADVECTION_DIFFUSION_GRID_H_
#define CORE_DIFFUSION_ADVECTION_DIFFUSION_GRID_H_

#include <array>
#include <string>
#include <utility>

#include "core/container/math_array.h"
#include "core/container/parallel_resize_vector.h"
#include "core/diffusion/diffusion_grid.h"

namespace bdm {

/// Advection-diffusion on a regular grid: d_t c = D laplacian(c) - div(v c) -
/// mu c.
///
/// First-order upwind advection plus the FTCS diffusion stencil from EulerGrid.
/// Per-face permeability (px_/py_/pz_) multiplies both the diffusive and
/// advective flux: 1 = open, 0 = sealed. Face index i is the face between
/// voxels i and i+1 along that axis.
///
/// Stability: dt * (6 D / dx^2 + |v|_max / dx) < 1. A violation is logged
/// once on the first Diffuse() call.
///
/// Introduced for the OHSU 2026 cytoplasmic tradewind model
/// (Nature Commun. s41467-026-70688-6), which requires advective transport
/// through an actin-myosin condensate barrier.
class AdvectionDiffusionGrid : public DiffusionGrid {
 public:
  AdvectionDiffusionGrid() = default;
  explicit AdvectionDiffusionGrid(const TRootIOCtor*) {}
  AdvectionDiffusionGrid(int substance_id, std::string substance_name,
                         real_t dc, real_t mu, int resolution = 10)
      : DiffusionGrid(substance_id, std::move(substance_name), dc, mu,
                      resolution) {}

  void Initialize() override;
  void Update() override;

  void DiffuseWithClosedEdge(real_t dt) override;
  void DiffuseWithOpenEdge(real_t dt) override;
  void DiffuseWithDirichlet(real_t dt) override;
  void DiffuseWithNeumann(real_t dt) override;
  void DiffuseWithPeriodic(real_t dt) override;

  void SetVelocity(const Real3& position, const Real3& v);
  void SetVelocity(size_t idx, const Real3& v);
  Real3 GetVelocity(const Real3& position) const;

  /// axis: 0=x, 1=y, 2=z. value is clamped to [0, 1].
  void SetPermeability(const Real3& position, int axis, real_t value);
  real_t GetPermeability(size_t idx, int axis) const;

  /// Reset all permeabilities to 1 and velocities to zero.
  void ClearFlowField();

  const Real3* GetAllVelocities() const { return velocity_.data(); }

 private:
  void EnsureFlowFieldSized();

  ParallelResizeVector<Real3> velocity_ = {};
  ParallelResizeVector<real_t> px_ = {};
  ParallelResizeVector<real_t> py_ = {};
  ParallelResizeVector<real_t> pz_ = {};

  BDM_CLASS_DEF_OVERRIDE(AdvectionDiffusionGrid, 1);
};

}  // namespace bdm

#endif  // CORE_DIFFUSION_ADVECTION_DIFFUSION_GRID_H_
