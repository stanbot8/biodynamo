#!/bin/bash
# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
# BioDynaMo collaboration. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

valgrind \
  --track-origins=yes \
  --num-callers=25 \
  --leak-resolution=high \
  --tool=memcheck \
  --leak-check=full \
  --show-leak-kinds=all \
  --suppressions="$(dirname "${BASH_SOURCE[0]}")/valgrind-bdm.supp" \
  --show-reachable=no \
  --error-exitcode=1 \
  "$@"
