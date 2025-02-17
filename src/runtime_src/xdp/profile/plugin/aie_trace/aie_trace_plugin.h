/**
 * Copyright (C) 2020-2022 Xilinx, Inc
 * Copyright (C) 2022 Advanced Micro Devices, Inc. - All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You may
 * not use this file except in compliance with the License. A copy of the
 * License is located at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef AIE_TRACE_PLUGIN_H
#define AIE_TRACE_PLUGIN_H

#include "xdp/profile/plugin/vp_base/vp_base_plugin.h"
#include "core/edge/common/aie_parser.h"
#include "xaiefal/xaiefal.hpp"

extern "C" {
#include <xaiengine.h>
}

namespace xdp {

  class DeviceIntf;
  class AIETraceOffload;
  class AIETraceLogger;

  using tile_type = xrt_core::edge::aie::tile_type;

  class AieTracePlugin : public XDPPlugin
  {
    public:
      XDP_EXPORT
      AieTracePlugin();

      XDP_EXPORT
      ~AieTracePlugin();

      XDP_EXPORT
      void updateAIEDevice(void* handle);

      XDP_EXPORT
      void flushAIEDevice(void* handle);

      XDP_EXPORT
      void finishFlushAIEDevice(void* handle);

      XDP_EXPORT
      virtual void writeAll(bool openNewFiles);

      XDP_EXPORT
      static bool alive();

    private:
      inline uint32_t bcIdToEvent(int bcId);
      void releaseCurrentTileCounters(int numCoreCounters, int numMemoryCounters);
      bool setMetrics(uint64_t deviceId, void* handle);
      bool setMetricsSettings(uint64_t deviceId, void* handle);

      bool checkAieDeviceAndRuntimeMetrics(uint64_t deviceId, void* handle);

      void getConfigMetricsForTiles(std::vector<std::string> metricsSettings,
                                    std::vector<std::string> graphmetricsSettings,
                                    void* handle);

      void setFlushMetrics(uint64_t deviceId, void* handle);
      void setTraceStartControl(void* handle);

      // Aie resource manager utility functions
      bool tileHasFreeRsc(xaiefal::XAieDev* aieDevice, XAie_LocType& loc, const std::string& metricSet);
      void printTileStats(xaiefal::XAieDev* aieDevice, const tile_type& tile);
      bool configureStartDelay(xaiefal::XAieMod& core);
      bool configureStartIteration(xaiefal::XAieMod& core);

      // Utility functions
      std::string getMetricSet(void* handle, const std::string& metricStr, bool ignoreOldConfig = false);
      std::vector<tile_type> getTilesForTracing(void* handle);

    private:

      // Indicate whether the Aie Trace Plugin is in valid state
      static bool live;

      // Runtime or compile-time specified trace metrics?
      bool runtimeMetrics = true;

      // These flags are used to decide configuration at various points

      // Start Delay in cycles
      bool mUseDelay = false;
      uint64_t mDelayCycles = 0;
      bool mUseOneDelayCtr = true;

      // Start Delay using graph iterator
      bool mUseGraphIterator = false;
      uint32_t mIterationCount = 0;

      // Start using user control
      bool mUseUserControl = false;

      bool continuousTrace;
      uint64_t offloadIntervalUs;
      unsigned int aie_trace_file_dump_int_s;

      // Trace Runtime Status
      AieRC mConfigStatus = XAIE_OK;
      std::map<uint64_t, void*> deviceIdToHandle;

      typedef std::tuple<AIETraceOffload*, 
                         AIETraceLogger*,
                         DeviceIntf*> AIEData;

      std::map<uint32_t, AIEData>  aieOffloaders;

      XAie_DevInst*     aieDevInst = nullptr;
      xaiefal::XAieDev* aieDevice  = nullptr;

      // Types
      typedef XAie_Events            EventType;
      typedef std::vector<EventType> EventVector;
      typedef std::vector<uint32_t>  ValueVector;

      // Trace metrics
      std::string metricSet;    
      std::set<std::string> metricSets;
      std::map<std::string, EventVector> coreEventSets;
      std::map<std::string, EventVector> memoryEventSets;

      // AIE profile counters
      std::vector<xrt_core::edge::aie::tile_type> mCoreCounterTiles;
      std::vector<std::shared_ptr<xaiefal::XAiePerfCounter>> mCoreCounters;
      std::vector<std::shared_ptr<xaiefal::XAiePerfCounter>> mMemoryCounters;

      // Counter metrics (same for all sets)
      EventType   coreTraceStartEvent;
      EventType   coreTraceEndEvent;
      EventVector coreCounterStartEvents;
      EventVector coreCounterEndEvents;
      ValueVector coreCounterEventValues;

      EventVector memoryCounterStartEvents;
      EventVector memoryCounterEndEvents;
      EventVector memoryCounterResetEvents;
      ValueVector memoryCounterEventValues;

      /* Currently only "aie" tile metrics is supported for graph/tile based trace.
       * So, a single map for tile and resolved metric is sufficient.
       * In future, when mem_tile and interface_tile metrics will be supported, we will
       * need separate map for each type or a vector of maps for all types together.
       */
      std::map<tile_type, std::string> mConfigMetrics;
//      std::vector<std::map<tile_type, std::string>> mConfigMetrics;
  };
    
}   
    
#endif
