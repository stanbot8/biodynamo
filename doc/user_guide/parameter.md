---
title: "Parameter"
date: "2019-01-01"
path: "/docs/userguide/parameter/"
meta_title: "BioDynaMo User Guide"
meta_description: "Configure BioDynaMo simulations."
toc: true
image: ""
sidebar: "userguide"
keywords:
  -parameter
  -configuration
  -code
  -setup
---

Parameters tailor BioDynaMo to a simulation. Core parameters are documented in
the [`Param` API](/api/structbdm_1_1Param.html). Simulations and modules can add
their own `ParamGroup` implementations.

### Read parameters

The active simulation owns the effective parameter set:

```cpp
const auto* param = Simulation::GetActive()->GetParam();
std::cout << param->simulation_time_step << std::endl;
std::cout << param->Get<neuroscience::Param>()->neurite_max_length_ << std::endl;
```

### TOML configuration

Place `bdm.toml` in the working directory or its parent directory. BioDynaMo
loads it automatically. The following file changes the time step and enables
exported visualization:

```toml
[simulation]
time_step = 0.1

[visualization]
export = true

[[visualize_agent]]
name = "Cell"
additional_data_members = ["density_"]
```

Required visualization members supplied by an agent are available
automatically. A custom name in `additional_data_members` also requires the
agent to override `GetVisualizationData` and return a `std::vector<real_t>`,
`std::vector<int>`, or `std::vector<uint64_t>` for that name.

Pass a different file with `-c FILE` or `--config FILE`. Repeat the option to
load several files in order. A simulation can also supply configuration files
to its constructor:

```cpp
Simulation simulation(argc, argv, {"experiment.toml"});
```

Custom parameter groups implement `AssignFromConfig` and read their values from
the parsed TOML table.

### Command line options

Run a simulation with `--help` to list its options. Core options cover compute
targets, visualization, verbosity, and TOML configuration.

Add a simulation-specific option before constructing `Simulation`:

```cpp
CommandLineOptions options(argc, argv);
options.AddOption<uint64_t>("n, num-cells", "10",
                            "The total number of cells");
Simulation simulation(options);
auto num_cells = options.Get<uint64_t>("num-cells");
```

### Source configuration

Use a constructor callback for values fixed by the simulation source:

```cpp
auto set_param = [](Param* param) {
  param->bound_space = Param::BoundSpaceMode::kClosed;
  param->min_bound = 0;
  param->max_bound = 250;
};

Simulation simulation(argc, argv, set_param);
```

The simulation exposes parameters as read-only state after initialization.
