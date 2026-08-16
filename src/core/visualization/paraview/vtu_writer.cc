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

#include "core/visualization/paraview/vtu_writer.h"

#include <vtkNew.h>
#include <vtkXMLPUnstructuredGridWriter.h>

#include "core/param/param.h"
#include "core/simulation.h"
#include "core/util/string.h"

namespace bdm {

void VtuWriter::operator()(const std::string& folder,
                           const std::string& file_prefix,
                           vtkUnstructuredGrid* grid) const {
  auto* param = Simulation::GetActive()->GetParam();
  vtkNew<vtkXMLPUnstructuredGridWriter> writer;
  auto filename = Concat(folder, "/", file_prefix, ".pvtu");
  writer->SetFileName(filename.c_str());
  writer->SetInputData(grid);
  writer->SetNumberOfPieces(1);
  writer->SetStartPiece(0);
  writer->SetEndPiece(0);
  writer->SetDataModeToBinary();
  writer->SetEncodeAppendedData(false);
  if (!param->visualization_compress_pv_files) {
    writer->SetCompressorTypeToNone();
  }
  writer->Write();
}

}  // namespace bdm
