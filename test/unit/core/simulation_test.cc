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

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "core/environment/environment.h"
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "unit/test_util/test_util.h"

namespace bdm {

class SimulationTest : public ::testing::Test {
 protected:
  static constexpr const char* kDefaultConfig = "bdm.toml";
  static constexpr const char* kConfigContent =
      "[simulation]\n"
      "unschedule_default_operations = [\"mechanical forces\"]\n"
      "random_seed = 123\n"
      "output_dir = \"result-dir\"\n"
      "time_step = 0.0125\n"
      "max_displacement = 2.0\n"
      "bound_space = \"closed\"\n"
      "min_bound = -100\n"
      "max_bound = 200\n"
      "diffusion_method = \"euler\"\n"
      "thread_safety_mechanism = \"automatic\"\n"
      "\n"
      "[visualization]\n"
      "insitu = false\n"
      "export = true\n"
      "pv_insitu_pipeline = \"my-insitu-script.py\"\n"
      "pv_insitu_pipelinearguments = \"param1=123\"\n"
      "interval = 100\n"
      "export_generate_pvsm = false\n"
      "compress_pv_files = false\n"
      "\n"
      "[[visualize_agent]]\n"
      "name = \"Cell\"\n"
      "\n"
      "[[visualize_agent]]\n"
      "name = \"Neurite\"\n"
      "additional_data_members = [\"spring_axis_\", \"tension_\"]\n"
      "\n"
      "[[visualize_diffusion]]\n"
      "name = \"Na\"\n"
      "concentration = false\n"
      "gradient = true\n"
      "\n"
      "[[visualize_diffusion]]\n"
      "name = \"K\"\n"
      "\n"
      "[performance]\n"
      "scheduling_batch_size = 123\n"
      "detect_static_agents = true\n"
      "cache_neighbors = true\n"
      "mem_mgr_growth_rate = 1.123\n"
      "mem_mgr_max_mem_per_thread_factor = 3\n"
      "minimize_memory_while_rebalancing = false\n"
      "\n"
      "[development]\n"
      "statistics = false\n"
      "debug_numa = true\n";

  void SetUp() override {
    std::remove(kDefaultConfig);
    Simulation::counter_ = 0;
  }

  void TearDown() override { std::remove(kDefaultConfig); }

  static void WriteConfig(const std::string& path) {
    std::ofstream config(path);
    config << kConfigContent;
  }

  static void ValidateConfig(const Param& param) {
    EXPECT_EQ(param.random_seed, 123u);
    EXPECT_EQ(param.output_dir, "result-dir");
    EXPECT_EQ(param.diffusion_method, "euler");
    EXPECT_REAL_EQ(param.simulation_time_step, real_t(0.0125));
    EXPECT_REAL_EQ(param.simulation_max_displacement, real_t(2));
    EXPECT_EQ(param.bound_space, Param::BoundSpaceMode::kClosed);
    EXPECT_EQ(param.min_bound, -100);
    EXPECT_EQ(param.max_bound, 200);
    EXPECT_EQ(param.thread_safety_mechanism,
              Param::ThreadSafetyMechanism::kAutomatic);
    EXPECT_EQ(param.unschedule_default_operations,
              std::vector<std::string>{"mechanical forces"});

    EXPECT_FALSE(param.insitu_visualization);
    EXPECT_TRUE(param.export_visualization);
    EXPECT_EQ(param.pv_insitu_pipeline, "my-insitu-script.py");
    EXPECT_EQ(param.pv_insitu_pipelinearguments, "param1=123");
    EXPECT_EQ(param.visualization_interval, 100u);
    EXPECT_FALSE(param.visualization_export_generate_pvsm);
    EXPECT_FALSE(param.visualization_compress_pv_files);
    ASSERT_EQ(param.visualize_agents.size(), 2u);
    EXPECT_TRUE(param.visualize_agents.at("Cell").empty());
    EXPECT_EQ(param.visualize_agents.at("Neurite"),
              (std::set<std::string>{"spring_axis_", "tension_"}));
    ASSERT_EQ(param.visualize_diffusion.size(), 2u);
    EXPECT_EQ(param.visualize_diffusion[0].name, "Na");
    EXPECT_FALSE(param.visualize_diffusion[0].concentration);
    EXPECT_TRUE(param.visualize_diffusion[0].gradient);
    EXPECT_EQ(param.visualize_diffusion[1].name, "K");

    EXPECT_EQ(param.scheduling_batch_size, 123u);
    EXPECT_TRUE(param.detect_static_agents);
    EXPECT_TRUE(param.cache_neighbors);
    EXPECT_NEAR(param.mem_mgr_growth_rate, 1.123, abs_error<real_t>::value);
    EXPECT_EQ(param.mem_mgr_max_mem_per_thread_factor, 3u);
    EXPECT_FALSE(param.minimize_memory_while_rebalancing);
    EXPECT_FALSE(param.statistics);
    EXPECT_TRUE(param.debug_numa);
  }
};

TEST_F(SimulationTest, LoadsDefaultTomlConfig) {
  WriteConfig(kDefaultConfig);
  Simulation simulation("my-simulation");

  EXPECT_EQ(simulation.GetUniqueName(), "my-simulation");
  ValidateConfig(*simulation.GetParam());
}

TEST_F(SimulationTest, LoadsConstructorTomlConfig) {
  constexpr auto kConfig = "constructor-config.toml";
  WriteConfig(kConfig);
  {
    Simulation simulation("my-simulation", {kConfig});
    ValidateConfig(*simulation.GetParam());
  }
  std::remove(kConfig);
}

TEST_F(SimulationTest, LoadsCommandLineTomlConfig) {
  constexpr auto kConfig = "command-line-config.toml";
  WriteConfig(kConfig);
  const char* argv[] = {"binary-name", "-c", kConfig};
  {
    Simulation simulation(3, argv);
    EXPECT_EQ(simulation.GetUniqueName(), "binary-name");
    ValidateConfig(*simulation.GetParam());
  }
  std::remove(kConfig);
}

TEST_F(SimulationTest, DerivesUniqueNamesFromProgramPath) {
  const char* first_argv[] = {"./binary-name"};
  Simulation first(1, first_argv);
  EXPECT_EQ(first.GetUniqueName(), "binary-name");

  const char* second_argv[] = {"binary-name"};
  Simulation second(1, second_argv);
  EXPECT_EQ(second.GetUniqueName(), "binary-name1");

  const char* third_argv[] = {"./build/binary-name"};
  Simulation third(1, third_argv);
  EXPECT_EQ(third.GetUniqueName(), "binary-name2");
}

TEST_F(SimulationTest, OutputDirectoryUsesUniqueName) {
  Simulation first("my-simulation");
  Simulation second("my-simulation");

  EXPECT_EQ(first.GetOutputDir(), "output/my-simulation");
  EXPECT_EQ(second.GetOutputDir(), "output/my-simulation1");
}

TEST_F(SimulationTest, EmptyNameUsesOutputRoot) {
  Simulation simulation("");

  EXPECT_EQ(simulation.GetUniqueName(), "");
  EXPECT_EQ(simulation.GetOutputDir(), "output");
}

TEST_F(SimulationTest, PreservesOutputDirectoryContentsWhenRequested) {
  const auto output = std::filesystem::path("output") / TEST_NAME;
  std::filesystem::create_directories(output / "subdir");

  auto preserve_contents = [](Param* param) {
    param->remove_output_dir_contents = false;
  };
  Simulation simulation(TEST_NAME, preserve_contents);

  EXPECT_FALSE(std::filesystem::is_empty(output));
}

TEST_F(SimulationTest, RemovesOutputDirectoryContentsByDefault) {
  const auto output = std::filesystem::path("output") / TEST_NAME;
  std::filesystem::create_directories(output / "subdir");

  Simulation simulation(TEST_NAME);

  EXPECT_TRUE(std::filesystem::is_empty(output));
}

TEST_F(SimulationTest, RetainsResourceManagerWhenSetToSameInstance) {
  Simulation simulation(TEST_NAME);
  auto* resource_manager = simulation.GetResourceManager();

  simulation.SetResourceManager(resource_manager);

  EXPECT_EQ(resource_manager->GetNumAgents(), 0u);
}

TEST_F(SimulationTest, RetainsEnvironmentWhenSetToSameInstance) {
  Simulation simulation(TEST_NAME);
  auto* environment = simulation.GetEnvironment();

  simulation.SetEnvironment(environment);
  environment->Clear();
}

}  // namespace bdm
