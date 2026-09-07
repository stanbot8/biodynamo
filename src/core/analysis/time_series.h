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
#ifndef CORE_ANALYSIS_TIME_SERIES_H_
#define CORE_ANALYSIS_TIME_SERIES_H_

#include <functional>
#include <unordered_map>
#include <vector>
#include "core/analysis/reduce.h"
#include "core/real_t.h"

namespace bdm {

class Simulation;

namespace experimental {

/// DataTransformer class that can be used to transform the data of a time
/// series entry.
/// \code
/// DataTransformer transformer;
/// ts.AddTransformedData("my-entry", "my-entry-transformed", transformer);
/// \endcode
/// This adds a new entry to the time series with the name
/// "my-entry-transformed" whose data has been transformed according to
/// the member functions.
class DataTransformer {
 public:
  virtual ~DataTransformer() = default;

  virtual void TransformXValues(const std::vector<real_t>& old_x_values,
                                std::vector<real_t>& new_x_values) const = 0;
  virtual void TransformYValues(const std::vector<real_t>& old_y_values,
                                std::vector<real_t>& new_y_values) const = 0;
  virtual void TransformYErrorLow(
      const std::vector<real_t>& old_y_error_low,
      std::vector<real_t>& new_y_error_low) const = 0;
  virtual void TransformYErrorHigh(
      const std::vector<real_t>& old_y_error_high,
      std::vector<real_t>& new_y_error_high) const = 0;
};

/// This class implements a linear transformation of the data of a time series.
/// It is used to transform the data of a time series entry. The transformation
/// is applied to the x-vales, y-values, y-error-low, and y-error-high.
/// Each transformation is linear and has its own slope and intercept.
/// \code
/// LinearTransformer transformer;
/// transformer.SetXSlope(2.0);
/// transformer.SetXIntercept(1.0);
/// ts.AddTransformedData("my-entry", "my-entry-transformed", transformer);
/// \endcode
/// This adds a new entry to the time series with the name
/// "my-entry-transformed" whose data has been transformed according to
/// the member functions.
class LinearTransformer : public DataTransformer {
 public:
  /// @brief  Transforms the x-values according to the linear transformation
  /// @param old_x_values x-values of the time series entry
  /// @param new_x_values x_slope_ * old_x_values + x_intercept_
  void TransformXValues(const std::vector<real_t>& old_x_values,
                        std::vector<real_t>& new_x_values) const final;

  /// @brief  Transforms the y-values according to the linear transformation
  /// @param old_y_values y-values of the time series entry
  /// @param new_y_values y_slope_ * old_y_values + y_intercept_
  void TransformYValues(const std::vector<real_t>& old_y_values,
                        std::vector<real_t>& new_y_values) const final;

  /// @brief  Transforms the y-error-low according to the linear transformation
  /// @param old_y_error_low y-error-low of the time series entry
  /// @param new_y_error_low y_error_low_slope_ * old_y_error_low +
  /// y_error_low_intercept_
  void TransformYErrorLow(const std::vector<real_t>& old_y_error_low,
                          std::vector<real_t>& new_y_error_low) const final;

  /// @brief  Transforms the y-error-high according to the linear transformation
  /// @param old_y_error_high y-error-high of the time series entry
  /// @param new_y_error_high y_error_high_slope_ * old_y_error_high +
  /// y_error_high_intercept_
  void TransformYErrorHigh(const std::vector<real_t>& old_y_error_high,
                           std::vector<real_t>& new_y_error_high) const final;

  void SetXSlope(real_t slope);
  void SetXIntercept(real_t intercept);
  void SetYSlope(real_t slope);
  void SetYIntercept(real_t intercept);
  void SetYErrorLowSlope(real_t slope);
  void SetYErrorLowIntercept(real_t intercept);
  void SetYErrorHighSlope(real_t slope);
  void SetYErrorHighIntercept(real_t intercept);

 private:
  real_t x_slope_ = 1.0;
  real_t x_intercept_ = 0.0;
  real_t y_slope_ = 1.0;
  real_t y_intercept_ = 0.0;
  real_t y_error_low_slope_ = 1.0;
  real_t y_error_low_intercept_ = 0.0;
  real_t y_error_high_slope_ = 1.0;
  real_t y_error_high_intercept_ = 0.0;
};

/// This class simplifies the collection of time series data during a
/// simulation. Every entry has an id and data arrays storing x-values,
/// y-values, y-error-low, and y-error-high.
class TimeSeries {
 public:
  struct Data {
    Data();
    Data(real_t (*ycollector)(Simulation*), real_t (*xcollector)(Simulation*));
    Data(Reducer<real_t>* y_reducer_collector,
         real_t (*xcollector)(Simulation*));
    Data(const Data&);
    ~Data();

    Data& operator=(const Data& other);

    Reducer<real_t>* y_reducer_collector = nullptr;
    real_t (*ycollector)(Simulation*) = nullptr;
    real_t (*xcollector)(Simulation*) = nullptr;
    std::vector<real_t> x_values;
    std::vector<real_t> y_values;
    std::vector<real_t> y_error_low;
    std::vector<real_t> y_error_high;
  };

  /// This function combines several time series into one.
  /// All time series in parameter `time_series` must have the same entries.
  /// All entries must have the exact same x values.
  /// The parameter `merger` takes a function which describes how each data
  /// point should be combined.
  /// Let's assume the following example: We take the median of each value
  /// and define error_low = mean - minimum, and error_high = maximum - mean.
  /// All time series objects have one entry called "entry-0" with the same
  /// x-values.
  /// \code
  /// std::vector<TimeSeries> tss(3);
  /// tss[0].Add("entry-0", {1, 2}, {2, 5});
  /// tss[1].Add("entry-0", {1, 2}, {4, 8});
  /// tss[2].Add("entry-0", {1, 2}, {1, 13});
  /// TimeSeries merged;
  /// TimeSeries::Merge(
  ///     &merged, tss,
  ///     merger);
  /// \endcode
  /// After these operations, `merged` will contain one entry with id "entry-0"
  /// with the following arrays: \n
  /// `x-values:     {1, 2}` \n
  /// `y-values:     {2, 8}` \n
  /// `y-error-low:  {1, 3}` \n
  /// `y-error-high: {2, 5}` \n
  /// Of course any other merger can be used too: e.g. mean + stddev
  static void Merge(
      TimeSeries* merged, const std::vector<TimeSeries>& time_series,
      const std::function<void(const std::vector<real_t>&, real_t*, real_t*,
                               real_t*)>& merger);

  /// Computes the mean squared error between `ts1` and `ts2`
  static real_t ComputeError(const TimeSeries& ts1, const TimeSeries& ts2);

  TimeSeries();
  TimeSeries(const TimeSeries& other);
  TimeSeries(TimeSeries&& other) noexcept;

  TimeSeries& operator=(TimeSeries&& other) noexcept;
  TimeSeries& operator=(const TimeSeries& other);

  /// Adds a new collector which is executed at each iteration.
  /// e.g. to track the number of agents in the simulation:
  /// \code
  /// auto* ts = simulation.GetTimeSeries();
  /// auto get_num_agents = [](Simulation* sim) {
  ///   return static_cast<real_t>(sim->GetResourceManager()->GetNumAgents());
  /// };
  /// ts->AddCollector("num-agents", get_num_agents);
  /// \endcode
  /// The optional x-value collector allows to modify the x-values.
  /// If no x-value collector is given, x-values will correspond to the
  /// simulation time.
  void AddCollector(const std::string& id, real_t (*ycollector)(Simulation*),
                    real_t (*xcollector)(Simulation*) = nullptr);

  /// Adds a reducer collector which is executed at each iteration.\n
  /// The benefit (in comparison with `AddCollector` using a function pointer
  /// to collect y-values) is that multiple reducers can be combined.
  /// This mechanism is more cache-friendly and calculates the result
  /// much faster.\n
  /// `Update` calculates the values for reducers before function pointers.
  /// Thus, a function pointer collector can use the result of a reducer
  /// collector. This is illustrated in the example below.
  /// Let's assume we want to track the fraction of infected agents in an
  /// epidemiological simulation.
  /// \code
  /// auto is_infected = [](Agent* a) {
  ///   return bdm_static_cast<Person*>(a)->state_ == State::kInfected;
  /// };
  /// ts->AddCollector("infected", new Counter<real_t>(is_infected);
  ///
  /// auto infected_rate = [](Simulation* sim) {
  ///   auto num_agents = sim->GetResourceManager()->GetNumAgents();
  ///   auto count = ts->GetYValues("infected").back();
  ///   return count / static_cast<real_t>(num_agents);
  /// };
  /// ts->AddCollector("infected_rate", infected_rate);
  /// \endcode
  void AddCollector(const std::string& id, Reducer<real_t>* y_reducer_collector,
                    real_t (*xcollector)(Simulation*) = nullptr);

  /// Add new entry with data that is not collected during a simulation.
  /// This function can for example be used to add experimental data
  /// which can be later plotted together with the simulation results
  /// using an analysis tool.
  /// \code
  /// time_series.Add("experimental-data", {0, 1, 2}, {3, 4, 5});
  /// \endcode
  void Add(const std::string& id, const std::vector<real_t>& x_values,
           const std::vector<real_t>& y_values);

  void Add(const std::string& id, const std::vector<real_t>& x_values,
           const std::vector<real_t>& y_values,
           const std::vector<real_t>& y_error);

  void Add(const std::string& id, const std::vector<real_t>& x_values,
           const std::vector<real_t>& y_values,
           const std::vector<real_t>& y_error_low,
           const std::vector<real_t>& y_error_high);

  /// Add the entries of another TimeSeries instance to this one.
  /// Let's assume that `ts` contains the entries:
  /// "entry1" and "entry2" and that suffix is set to "-from-ts".
  /// In this scenario the following entries will be added to this
  /// object: "entry1-from-ts", "entry2-from-ts"
  void Add(const TimeSeries& ts, const std::string& suffix);

  /// Copies the data of an existing entry and adds it to this object. During
  /// the copy process, the data is transformed using the given `transformer`.
  /// This allows to e.g. convert the x-values from simulation time to
  /// simulation steps, or vice versa. A common use case are linear or log
  /// scaling of the x-axis.
  void AddTransformedData(const std::string& old_id,
                          const std::string& transformed_id,
                          const DataTransformer& transformer);

  /// Adds a new data point to all time series with a collector.
  void Update();

  /// Returns whether a times series with given id exists in this object.
  bool Contains(const std::string& id) const;
  uint64_t Size() const;
  const std::vector<real_t>& GetXValues(const std::string& id) const;
  const std::vector<real_t>& GetYValues(const std::string& id) const;
  const std::vector<real_t>& GetYErrorLow(const std::string& id) const;
  const std::vector<real_t>& GetYErrorHigh(const std::string& id) const;

  /// Print all time series entry names to stdout
  void ListEntries() const;

 private:
  std::unordered_map<std::string, Data> data_;
};

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_ANALYSIS_TIME_SERIES_H_
