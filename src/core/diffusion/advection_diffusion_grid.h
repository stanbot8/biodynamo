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

/// Advection-diffusion grid with per-voxel velocity and per-face permeability.
///
/// Solves  d_t c = D laplacian(c) - div(v c) - mu c
///
/// Motivation: OHSU 2026 (Nature Commun. s41467-026-70688-6) showed that
/// soluble proteins in motile cells are delivered to the leading edge by
/// intracellular fluid flow through an actin-myosin condensate barrier, not
/// by pure diffusion. Pure Fickian diffusion is wrong for motile/polarized
/// cells; this grid adds the advective term and a per-face permeability
/// field for internal barriers (condensate interfaces).
///
/// Advection uses first-order upwind (stable, diffusive). Diffusion uses
/// the same FTCS stencil as EulerGrid. Permeability multiplies BOTH the
/// diffusive and advective flux at each face: 1.0 = open, 0.0 = sealed.
/// Faces are indexed by the lower voxel along that axis (px_[i] is the
/// face between voxel i and i+1 in x).
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

  /// Set the velocity at the voxel containing the given position.
  void SetVelocity(const Real3& position, const Real3& v);
  /// Set the velocity at a voxel by flat index.
  void SetVelocity(size_t idx, const Real3& v);
  Real3 GetVelocity(const Real3& position) const;

  /// Set permeability for the face between voxel at `position` and its
  /// +axis neighbor. axis: 0=x, 1=y, 2=z. value in [0, 1].
  void SetPermeability(const Real3& position, int axis, real_t value);
  real_t GetPermeability(size_t idx, int axis) const;

  /// Reset all permeabilities to 1 (fully open) and velocities to zero.
  void ClearFlowField();

  const Real3* GetAllVelocities() const { return velocity_.data(); }

 private:
  void EnsureFlowFieldSized();

  /// Per-voxel velocity [um / time-unit].
  ParallelResizeVector<Real3> velocity_ = {};
  /// Per-face permeability for +x, +y, +z faces. Face at index i is between
  /// voxel i and its +axis neighbor. Values default to 1 (open).
  ParallelResizeVector<real_t> px_ = {};
  ParallelResizeVector<real_t> py_ = {};
  ParallelResizeVector<real_t> pz_ = {};

  BDM_CLASS_DEF_OVERRIDE(AdvectionDiffusionGrid, 1);
};

}  // namespace bdm

#endif  // CORE_DIFFUSION_ADVECTION_DIFFUSION_GRID_H_
