/*
 * Copyright 2015 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TRACE_EVENT_BENCHMARK_H_
#define TRACE_EVENT_BENCHMARK_H_

#include <string>
#include <vector>

#include "base/memory/ref_counted.h"
#include "base/memory/singleton.h"
#include "cobalt/trace_event/event_parser.h"

// The benchmarking system allows developers to quickly and easily write
// benchmarks that measure performance metrics recorded through Chromium's
// TRACE_EVENT system.  Sample benchmark usage can be found in
// cobalt/trace_event/sample_benchmark.cc.  Here is a quick example of how
// to record the performance metrics of 3 TRACE_EVENT scopes (and 1 of those
// is measured in 2 different ways):
//
//   TRACE_EVENT_BENCHMARK4(
//       SampleTestBenchmarkWithThreeTrackedEvents,
//       "LoopIteration", cobalt::trace_event::IN_SCOPE_DURATION,
//       "SubEventA", cobalt::trace_event::IN_SCOPE_DURATION,
//       "SubEventA", cobalt::trace_event::TIME_BETWEEN_EVENT_STARTS,
//       "SubEventB", cobalt::trace_event::IN_SCOPE_DURATION) {
//     const int kRenderIterationCount = 40;
//     for (int i = 0; i < kRenderIterationCount; ++i) {
//       TRACE_EVENT0("SampleBenchmark", "LoopIteration");
//       {
//         TRACE_EVENT0("SampleBenchmark", "SubEventA");
//         usleep(10000);
//       }
//       {
//         TRACE_EVENT0("SampleBenchmark", "SubEventB");
//         usleep(20000);
//       }
//     }
//   }
//
// Here, a loop is iterated over multiple times, and each time the timing
// results of the TRACE_EVENT calls are recorded and saved.  When the test is
// complete, 40 samples of each of the TRACE_EVENTS specified in the macro
// parameters are recorded.  After the benchmark completes, statistics about
// the results or the raw sample results themselves are produced.  Note that
// "SubEventA" is measured twice in two different ways.
//
// More sophisticated benchmarks can also be written by subclassing from
// cobalt::trace_event::Benchmark and implementing Experiment(),
// AnalyzeTraceEvent() and CompileResults().  Experiment() is the function
// that will execute to generate the tracing data, AnalyzeTraceEvent() is the
// function that is called as parsed events (e.g. high-level structures
// representing the TRACE_EVENT scopes and their hierarchies and timing
// information) are produced.  They can be analyzed and any interesting
// statistics can be extracted and recorded at this time.  Finally, when
// all events have been analyzed, CompileResults() is called so that the
// benchmark can return a list of the results it would like to publish.
// Once a benchmark is defined in this fashion, the
// TRACE_EVENT_REGISTER_BENCHMARK() macro should be called to make it visible
// to the benchmarking system.
// Once again, sample code can be found in
// cobalt/trace_event/sample_benchmark.cc.

namespace cobalt {
namespace trace_event {

// The base class for all benchmarks.  Declares the Benchmark interface that
// one should implement if one wishes to register a benchmark with the
// BenchmarkRegistrar.
class Benchmark {
 public:
  virtual ~Benchmark() {}

  struct Result {
    Result(const std::string& name, const std::vector<double>& samples)
        : name(name), samples(samples) {}
    Result(const std::string& name, double value) : name(name) {
      samples.push_back(value);
    }

    std::string name;
    std::vector<double> samples;
  };

  // The Experiment() function is executed within a ScopedEventParserTrace
  // scope, and all the resulting parsed events generated by TRACE_EVENT
  // calls are forwarded to AnalyzeTraceEvent() below.
  virtual void Experiment() = 0;
  // Handles a parsed event that is ready for analysis.
  virtual void AnalyzeTraceEvent(
      const scoped_refptr<EventParser::ScopedEvent>& event) = 0;
  // CompileResults() is called after all parsed events have been
  // observed.  It can then compile the resulting information in to a list
  // of results, one for each statistic of interest.
  virtual std::vector<Result> CompileResults() = 0;

  // The name of the benchmark.  This will be set when the benchmark is
  // registered with the BenchmarkRegistrar.
  const std::string& name() const { return name_; }

 private:
  std::string name_;
  friend class BenchmarkRegisterer;
};

// The BenchmarkRegistrar is a singleton that collects all defined Benchmarks
// in a central location so that they can be executed on demand.
class BenchmarkRegistrar {
 public:
  ~BenchmarkRegistrar();

  static BenchmarkRegistrar* GetInstance();

  // Register a benchmark with the central BenchmarkRegistrar so that it is
  // executed when ExecuteBenchmarks() is called.  RegisterBenchmark() is
  // typically called via the TRACE_EVENT_REGISTER_BENCHMARK macro.
  void RegisterBenchmark(Benchmark* benchmark);

  // Execute all registered benchmarks and report their results.
  void ExecuteBenchmarks();

 private:
  BenchmarkRegistrar();
  friend struct DefaultSingletonTraits<BenchmarkRegistrar>;

  void ExecuteBenchmark(Benchmark* benchmark);

  typedef std::vector<Benchmark*> BenchmarkList;
  BenchmarkList benchmarks_;

  DISALLOW_COPY_AND_ASSIGN(BenchmarkRegistrar);
};

// This is a helper class that makes it possible to register a given benchmark
// with the BenchmarkRegistrar at static initialization time.  It is within this
// call that the benchmark's name is specified.
class BenchmarkRegisterer {
 public:
  BenchmarkRegisterer(const std::string& name, Benchmark* benchmark) {
    benchmark->name_ = name;
    BenchmarkRegistrar::GetInstance()->RegisterBenchmark(benchmark);
  }
};

// Measurement types allow one to specify to the SIMPLE_BENCHMARK interface
// what quantity should be measured.
enum MeasurementType {
  // Measuring in-scope duration will sample the time between the start of the
  // event and the end of the event, ignoring its children.
  IN_SCOPE_DURATION,

  // Measuring flow duration will sample the time between the start of the event
  // and the latest end time of all the event's descendants.
  FLOW_DURATION,

  // Measuring the time between event starts will sample the time difference
  // between the start time of subsequent events of the same name.
  TIME_BETWEEN_EVENT_STARTS,
};

}  // namespace trace_event
}  // namespace cobalt

// After defining a new benchmark (by subclassing from
// cobalt::trace_event::Benchmark), this macro should be called on it to
// register it with the central BenchmarkRegistrar singleton so it can be found
// and executed later.
#define TRACE_EVENT_REGISTER_BENCHMARK(benchmark)                              \
  cobalt::trace_event::BenchmarkRegisterer g_benchmark_registerer_##benchmark( \
      #benchmark, new benchmark());

// Defines all variations of the TRACE_EVENT_BENCHMARK* macros.  It is
// isolated in its own header file so that it can be generated by pump.py.
#include "benchmark_internal.h"

#endif  // TRACE_EVENT_BENCHMARK_H_
