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

#ifndef MY_CELL_H_
#define MY_CELL_H_

#include <biodynamo.h>

namespace bdm {
namespace soma_clustering {

// Define my custom cell, which extends Cell by adding an extra
// data member cell_type.
class MyCell : public Cell {
  BDM_AGENT_HEADER(MyCell, Cell);

 public:
  MyCell() {}
  explicit MyCell(const Real3& position, int cell_type)
      : Base(position), cell_type_(cell_type) {}
  virtual ~MyCell() {}

  int GetCellType() const { return cell_type_; }

  bool GetVisualizationData(const std::string& name,
                            VisualizationData* values) const override {
    if (name == "cell_type_") {
      *values = std::vector<int>{cell_type_};
      return true;
    }
    return Base::GetVisualizationData(name, values);
  }

 private:
  int cell_type_;
};

}  // namespace soma_clustering
}  // namespace bdm

#endif  // MY_CELL_H_
