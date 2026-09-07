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

#include "core/visualization/paraview/vtk_agents.h"
// std
#include <set>
#include <string>
#include <type_traits>
#include <vector>
// ParaView
#include <vtkAOSDataArrayTemplate.h>
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkCellArray.h>
#include <vtkCellType.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
// BioDynaMo
#include "core/agent/agent.h"
#include "core/param/param.h"
#include "core/shape.h"
#include "core/simulation.h"
#include "core/visualization/paraview/vtu_writer.h"

namespace bdm {

// -----------------------------------------------------------------------------
VtkAgents::VtkAgents(const char* type_name,
                     vtkCPDataDescription* data_description) {
  auto* param = Simulation::GetActive()->GetParam();
  data_.push_back(vtkUnstructuredGrid::New());
  name_ = type_name;

  if (!param->export_visualization) {
    data_description->AddInput(type_name);
    data_description->GetInputDescriptionByName(type_name)->SetGrid(data_[0]);
  }
}

// -----------------------------------------------------------------------------
VtkAgents::~VtkAgents() {
  name_ = "";
  for (auto& el : data_) {
    el->Delete();
  }
  data_.clear();
}

// -----------------------------------------------------------------------------
vtkUnstructuredGrid* VtkAgents::GetData(uint64_t idx) { return data_[idx]; }

// -----------------------------------------------------------------------------
Shape VtkAgents::GetShape() const { return shape_; }

// -----------------------------------------------------------------------------
const std::string& VtkAgents::GetTypeName() const { return name_; }

// -----------------------------------------------------------------------------
void VtkAgents::Update(const std::vector<Agent*>* agents) {
  if (!agents->empty() && !metadata_initialized_) {
    InitializeMetadata(*agents->front());
  }
  UpdateGrid(0, agents, 0, agents->size());
}

// -----------------------------------------------------------------------------
void VtkAgents::WriteToFile(uint64_t step) const {
  auto* sim = Simulation::GetActive();
  auto filename_prefix = Concat(name_, "-", step);

  VtuWriter writer;
  writer(sim->GetOutputDir(), filename_prefix, data_[0]);
}

// -----------------------------------------------------------------------------
void VtkAgents::InitializeMetadata(const Agent& agent) {
  shape_ = agent.GetShape();
  std::set<std::string> dm_set = agent.GetRequiredVisDataMembers();
  auto* param = Simulation::GetActive()->GetParam();
  auto configured = param->visualize_agents.find(name_);
  if (configured != param->visualize_agents.end()) {
    dm_set.insert(configured->second.begin(), configured->second.end());
  }
  data_members_.assign(dm_set.begin(), dm_set.end());
  metadata_initialized_ = true;
}

// -----------------------------------------------------------------------------
void VtkAgents::UpdateGrid(uint64_t tid, const std::vector<Agent*>* agents,
                           uint64_t start, uint64_t end) {
  auto* grid = data_[tid];
  grid->Initialize();

  vtkNew<vtkPoints> points;
  points->SetNumberOfPoints(end - start);
  for (uint64_t i = start; i < end; ++i) {
    const auto& position = (*agents)[i]->GetPosition();
    points->SetPoint(i - start, position[0], position[1], position[2]);
  }
  grid->SetPoints(points);

  vtkNew<vtkCellArray> vertices;
  vertices->AllocateExact(end - start, end - start);
  for (vtkIdType i = 0; i < static_cast<vtkIdType>(end - start); ++i) {
    vertices->InsertNextCell(1, &i);
  }
  grid->SetCells(VTK_VERTEX, vertices);

  for (const auto& name : data_members_) {
    if (name == "position_") {
      continue;
    }

    if (start == end) {
      continue;
    }
    VisualizationData first_value;
    if (!(*agents)[start]->GetVisualizationData(name, &first_value)) {
      Log::Fatal("VtkAgents::UpdateGrid", "Agent type '", name_,
                 "' does not expose visualization data member '", name, "'.");
    }

    std::visit(
        [&](const auto& initial_values) {
          using Values = std::decay_t<decltype(initial_values)>;
          using Value = typename Values::value_type;
          if (initial_values.empty()) {
            Log::Fatal("VtkAgents::UpdateGrid", "Agent type '", name_,
                       "' exposes empty visualization data member '", name,
                       "'.");
          }

          vtkNew<vtkAOSDataArrayTemplate<Value>> array;
          array->SetName(name.c_str());
          array->SetNumberOfComponents(static_cast<int>(initial_values.size()));
          array->SetNumberOfTuples(end - start);

          for (uint64_t i = start; i < end; ++i) {
            VisualizationData current_value;
            if (!(*agents)[i]->GetVisualizationData(name, &current_value)) {
              Log::Fatal("VtkAgents::UpdateGrid", "Agent type '", name_,
                         "' does not expose visualization data member '", name,
                         "'.");
            }
            auto* current_values = std::get_if<Values>(&current_value);
            if (current_values == nullptr ||
                current_values->size() != initial_values.size()) {
              Log::Fatal("VtkAgents::UpdateGrid", "Agent type '", name_,
                         "' exposes inconsistent visualization data member '",
                         name, "'.");
            }
            for (size_t component = 0; component < current_values->size();
                 ++component) {
              array->SetTypedComponent(static_cast<vtkIdType>(i - start),
                                       static_cast<int>(component),
                                       (*current_values)[component]);
            }
          }
          grid->GetPointData()->AddArray(array);
        },
        first_value);
  }
}

}  // namespace bdm
