#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapCallbacksController.hpp"
#include "GlobalNamespace/CallbacksInTime.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "GlobalNamespace/SortedList_1.hpp"
#include "GlobalNamespace/SortedList_2.hpp"
#include "GlobalNamespace/BeatmapCallbacksUpdater.hpp"
#include "System/Collections/Generic/HashSet_1.hpp"
#include "System/Collections/Generic/Dictionary_2.hpp"
#include "System/Action.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "Animation/Events.h"
#include "AssociatedData.h"
#include "tracks/shared/TimeSourceHelper.h"
#include "NEHooks.h"
#include "NELogger.h"
#include "SharedUpdate.h"
#include "NECaches.h"
#include "SpawnDataHelper.h"

using namespace GlobalNamespace;

BeatmapCallbacksController* controller;
static GlobalNamespace::IReadonlyBeatmapData* beatmapData;

static BeatmapObjectSpawnController::InitData* initData;
static GlobalNamespace::BeatmapObjectSpawnMovementData* movementData;

inline float GetSpawnAheadTime(BeatmapObjectSpawnController::InitData* initData,
                               BeatmapObjectSpawnMovementData* movementData, std::optional<float> inputNjs,
                               std::optional<float> inputOffset) {
  return movementData->moveDuration +
         (SpawnDataHelper::GetJumpDuration(initData, movementData, inputNjs, inputOffset) * 0.5f);
}

inline float ObjectSortGetTime(BeatmapDataItem* n) {
  static auto* customObstacleDataClass = classof(CustomJSONData::CustomObstacleData*);
  static auto* customNoteDataClass = classof(CustomJSONData::CustomNoteData*);

  float* aheadTime;
  CustomJSONData::JSONWrapper* customDataWrapper;

  if (n->klass == customObstacleDataClass) {
    auto* obstacle = reinterpret_cast<CustomJSONData::CustomObstacleData*>(n);
    aheadTime = &obstacle->aheadTimeNoodle;
    customDataWrapper = obstacle->customData;
  } else if (n->klass == customNoteDataClass) {
    auto* note = reinterpret_cast<CustomJSONData::CustomNoteData*>(n);
    aheadTime = &note->aheadTimeNoodle;
    customDataWrapper = note->customData;
  } else {
    return n->time;
  }

  auto const& ad = getAD(customDataWrapper);

  auto const njs = ad.objectData.noteJumpMovementSpeed;           // .value_or(NECaches::noteJumpMovementSpeed);
  auto const spawnOffset = ad.objectData.noteJumpStartBeatOffset; //.value_or(NECaches::noteJumpStartBeatOffset);

  *aheadTime = GetSpawnAheadTime(initData, movementData, njs, spawnOffset);

  return n->time - *aheadTime;
}

constexpr bool ObjectTimeCompare(BeatmapDataItem* a, BeatmapDataItem* b) {
  return ObjectSortGetTime(a) < ObjectSortGetTime(b);
}

System::Collections::Generic::LinkedList_1<BeatmapDataItem*>*
SortAndOrderList(CustomJSONData::CustomBeatmapData* beatmapData) {
  initData = NECaches::GameplayCoreContainer->Resolve<BeatmapObjectSpawnController::InitData*>();
  movementData = GlobalNamespace::BeatmapObjectSpawnMovementData::New_ctor();
  movementData->Init(initData->noteLinesCount, initData->noteJumpMovementSpeed, initData->beatsPerMinute,
                     initData->noteJumpValueType, initData->noteJumpValue, nullptr, NEVector::Vector3::right(),
                     NEVector::Vector3::forward());

  auto items = beatmapData->GetAllBeatmapItemsCpp();

  std::stable_sort(items.begin(), items.end(), ObjectTimeCompare);

  initData = nullptr;
  movementData = nullptr;

  auto newList = SafePtr(System::Collections::Generic::LinkedList_1<BeatmapDataItem*>::New_ctor());
  auto newListPtr = static_cast<System::Collections::Generic::LinkedList_1<BeatmapDataItem*>*>(newList);
  if (items.empty()) return newListPtr;

  for (auto const& o : items) {
    newList->AddLast(o);
  }

  return newListPtr;
}

MAKE_HOOK_MATCH(BeatmapCallbacksUpdater_LateUpdate, &BeatmapCallbacksUpdater::LateUpdate, void,
                BeatmapCallbacksUpdater* self) {
  auto selfController = self->_beatmapCallbacksController;

  // Reset to avoid overriding non NE maps
  //    if ((controller || beatmapData) && (controller != selfController || selfController->beatmapData != beatmapData))
  //    {
  //        CustomJSONData::CustomEventCallbacks::firstNode.emplace(nullptr);
  //    }

  if (!Hooks::isNoodleHookEnabled()) {
    controller = nullptr;
    beatmapData = nullptr;
    return BeatmapCallbacksUpdater_LateUpdate(self);
  }

  auto beatmapOpt = il2cpp_utils::try_cast<CustomJSONData::CustomBeatmapData>(selfController->_beatmapData);
  if (beatmapOpt && controller != selfController || selfController->_beatmapData != beatmapData) {

    CJDLogger::Logger.fmtLog<Paper::LogLevel::INF>("Using noodle sorted node");
    controller = selfController;
    beatmapData = selfController->_beatmapData;

    auto items = SortAndOrderList(beatmapOpt.value());

    auto first = items->get_First();
    CustomJSONData::CustomEventCallbacks::firstNode.emplace(first);
  }

  return BeatmapCallbacksUpdater_LateUpdate(self);
}

void InstallBeatmapObjectCallbackControllerHooks() {
  INSTALL_HOOK(NELogger::Logger, BeatmapCallbacksUpdater_LateUpdate);
}

NEInstallHooks(InstallBeatmapObjectCallbackControllerHooks);