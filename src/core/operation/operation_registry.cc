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

#include "core/operation/operation_registry.h"

#include <memory>

namespace bdm {

OperationRegistry *OperationRegistry::GetInstance() {
  static OperationRegistry operation_registry;
  return &operation_registry;
}

Operation *OperationRegistry::NewOperation(const std::string &op_name) {
  auto definition = operations_.find(op_name);
  if (definition == operations_.end()) {
    std::string msg = "Operation not found in registry: " + op_name;
    Log::Fatal("OperationRegistry::NewOperation", msg);
  }

  auto operation =
      std::make_unique<Operation>(op_name, definition->second.frequency);
  for (size_t target = 0; target < definition->second.factories.size();
       ++target) {
    auto factory = definition->second.factories[target];
    if (factory) {
      operation->AddOperationImpl(static_cast<OpComputeTarget>(target),
                                  factory());
    }
  }
  return operation.release();
}

bool OperationRegistry::AddOperationImpl(const std::string &op_name,
                                         OpComputeTarget target,
                                         OperationImplFactory factory,
                                         uint32_t frequency) {
  auto result = operations_.emplace(
      op_name, OperationDefinition{frequency, {}});
  auto& factories = result.first->second.factories;
  if (factories.size() < static_cast<size_t>(target + 1)) {
    factories.resize(target + 1, nullptr);
  }
  if (factories[target]) {
    Log::Fatal("OperationRegistry::AddOperationImpl", "Operation '", op_name,
               "' with implementation '", OpComputeTargetString(target),
               "' already exists in the registry!");
  }
  factories[target] = factory;
  return true;
}

OperationRegistry::OperationRegistry() = default;

}  // namespace bdm
