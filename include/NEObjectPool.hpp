// https://github.com/Kautenja/object-pool/blob/master/include/object_pool.hpp

#pragma once

#include <list>
#include "Animation/NoodleMovementDataProvider.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "NELogger.h"

namespace NoodleExtensions::Pool {

class NoodleMovementDataProviderPool {
 private:
  std::vector<SafePtr<NoodleMovementDataProvider>> free = {};

 public:
  NoodleMovementDataProviderPool(int count) : free() {
    for (int i = 0; i < count; ++i) {
      NELogger::Logger.info("hi");
      put(NoodleMovementDataProvider::New_ctor());
    }
  }
  ~NoodleMovementDataProviderPool() {
    free.clear();
  }

  NoodleMovementDataProvider* get(GlobalNamespace::BeatmapObjectData* beatmapObjectData) {
    if (free.empty())
    {
      auto obj = NoodleMovementDataProvider::New_ctor()->InitObject(beatmapObjectData);
      put(obj);
      return obj;
    }

    NoodleMovementDataProvider* obj = free.end()->ptr();
    free.pop_back();
    return obj->InitObject(beatmapObjectData);
  }

  inline void put(NoodleMovementDataProvider* obj) { free.emplace_back(obj); }
};

}
