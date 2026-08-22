---
title: "Simulation Parameter Tutorial"
date: "2020-09-08"
path: "/docs/userguide/simulation_parameter_tutorial/"
meta_title: "BioDynaMo User Guide"
meta_description: "Define and configure simulation parameters with TOML."
toc: true
image: ""
sidebar: "userguide"
keywords:
  -parameter
  -toml
---

This tutorial uses the installed `parameters` demo to define a custom parameter
group and configure it with TOML.

### Copy and inspect the demo

```bash
biodynamo demo parameters
cd parameters
```

`src/parameters.h` defines two simulation-specific values:

```cpp
struct SimParam : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(SimParam, 1);

  real_t foo = 3.14;
  int bar = -42;

 protected:
  void AssignFromConfig(
      const std::shared_ptr<cpptoml::table>& config) override {
    BDM_ASSIGN_CONFIG_VALUE(foo, "parameters.foo");
    BDM_ASSIGN_CONFIG_VALUE(bar, "parameters.bar");
  }
};
```

`src/parameters.cc` defines the group's unique identifier:

```cpp
const ParamGroupUid SimParam::kUid = ParamGroupUidGenerator::Get()->NewUid();
```

The simulation registers the group before creating `Simulation`, then reads the
effective values from the simulation-owned parameter set:

```cpp
Param::RegisterParamGroup(new SimParam());
Simulation simulation(argc, argv);

const auto* param = simulation.GetParam();
const auto* simulation_param = param->Get<SimParam>();
```

### Build and run

```bash
cmake -S . -B build
cmake --build build --parallel
./build/parameters
```

The initial output contains the defaults:

```text
Value of simulation time step 0.01
Value of foo                  3.14
Value of bar                  -42
Simulation completed successfully!
```

### Configure the simulation

Create `bdm.toml` in the demo directory:

```toml
[simulation]
time_step = 1.0

[parameters]
foo = 6.28
bar = 84
```

Run the binary again. BioDynaMo finds `bdm.toml` in the parent of the build
directory and prints the configured values:

```text
Value of simulation time step 1
Value of foo                  6.28
Value of bar                  84
Simulation completed successfully!
```

### Select a configuration explicitly

Rename the file and pass it on the command line:

```bash
mv bdm.toml experiment.toml
./build/parameters --config experiment.toml
```

The constructor can select the same file in source code:

```cpp
Simulation simulation(argc, argv, {"experiment.toml"});
```

Command line configuration is convenient for choosing experiments. Constructor
configuration keeps a simulation tied to an explicit configuration set.
