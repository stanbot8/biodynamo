# -----------------------------------------------------------------------------
#
# Copyright (C) The BioDynaMo Project.
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

# To avoid the repetition of the --cxx-compiler and --build-type arguments, store it in
# a variable. These two options can be omitted: bdm-config will then use the default
# compiler and the build type Release.
#BUILD_TYPE=--opt
#BUILD_TYPE=--debug
BUILD_TYPE=--opt --debug
BDM_CONFIG=bdm-config $(BUILD_TYPE)
CXXFLAGS=`$(BDM_CONFIG) --cxxflags`
LINKFLAGS=`$(BDM_CONFIG) --ldflags --libs`

all: makefile_project

# Compile all source files
makefile_project.o: src/makefile_project.cc
	$(CXX) $(CXXFLAGS) -c $?

# Generate binary (link object files and libraries)
makefile_project: makefile_project.o
	$(CXX) $? -o $@ $(LINKFLAGS)

clean:
	rm -f *.o makefile_project
