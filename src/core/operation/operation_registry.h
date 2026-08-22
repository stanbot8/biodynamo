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

#ifndef CORE_OPERATION_OPERATION_REGISTRY_H_
#define CORE_OPERATION_OPERATION_REGISTRY_H_

#include "core/operation/operation.h"

#include <unordered_map>
#include <vector>

namespace bdm {

/// A registry of operation implementations that can be scheduled for a
/// simulation. Since an operation can have multiple implementation (e.g. for
/// execution on CPU, CUDA, OpenCL), we need to register them separately.
class OperationRegistry {
 public:
  using OperationImplFactory = OperationImpl* (*)();

  /// Singleton class - returns the static instance
  static OperationRegistry *GetInstance();

  /// Creates an operation
  ///
  /// @param[in]  op_name  The operation's name
  ///
  /// @return     The operation pointer
  ///
  Operation *NewOperation(const std::string &op_name);

  /// Adds an operation implementation to the registry
  ///
  /// @param[in]  op_name    The operation's name
  /// @param[in]  target     The compute target
  /// @param[in]  factory    Creates the implementation for the compute target
  /// @param[in]  frequency  The frequency at which the operation is executed
  ///
  /// @return     Returns true when the operation is successfully added to
  ///             registry
  ///
  bool AddOperationImpl(const std::string &op_name, OpComputeTarget target,
                        OperationImplFactory factory, uint32_t frequency = 1);

 private:
  struct OperationDefinition {
    uint32_t frequency;
    std::vector<OperationImplFactory> factories;
  };

  /// The map containing the operations; accessible by their name
  std::unordered_map<std::string, OperationDefinition> operations_;

  OperationRegistry();
};

template <typename T>
OperationImpl* NewOperationImpl() {
  return new T();
}

/// A convenient macro to register a new operation implemented. To be used as:
/// BDM_REGISTER_OP(MyOp, "my operation", kCpu)
/// MyOp is required to have member: `static bool registered_`
#define BDM_REGISTER_OP(op, name, target)                                    \
  bool op::registered_ = OperationRegistry::GetInstance()->AddOperationImpl( \
      name, OpComputeTarget::target, &NewOperationImpl<op>);

/// \see BDM_REGISTER_OP
/// Adds parameter to specigy default execution frequency (\see
/// Operation::frequency_)
#define BDM_REGISTER_OP_WITH_FREQ(op, name, target, frequency)               \
  bool op::registered_ = OperationRegistry::GetInstance()->AddOperationImpl( \
      name, OpComputeTarget::target, &NewOperationImpl<op>, frequency);

/// A convenient macro to register a new operation implemented. To be used as:
/// BDM_REGISTER_OP(MyOp, "my operation", kCpu)
/// MyOp is required to have member: `static bool registered_`
#define BDM_REGISTER_TEMPLATE_OP(op, T, name, target)     \
  template <>                                             \
  bool op<T>::registered_ =                               \
      OperationRegistry::GetInstance()->AddOperationImpl( \
          name, OpComputeTarget::target, &NewOperationImpl<op<T>>);

/// A convenient function to get a new operation from the registry by its name
inline Operation *NewOperation(const std::string &name) {
  return OperationRegistry::GetInstance()->NewOperation(name);
}

/// A convenient macro to hide some of the boilerplate code from the user in
/// implementing new operations.
#define BDM_OP_HEADER(class_name) \
 private:                         \
  static bool registered_;        \
                                  \
 public:                          \
  class_name *Clone() override { return new class_name(*this); }

}  // namespace bdm

#endif  // CORE_OPERATION_OPERATION_REGISTRY_H_
