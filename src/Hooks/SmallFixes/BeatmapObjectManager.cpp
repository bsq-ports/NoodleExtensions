#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/BasicBeatmapObjectManager.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "System/Action_1.hpp"
#include "System/Action_3.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"

#include "AssociatedData.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

static bool GetHiddenForType(BeatmapObjectManager* beatmapObjectManager) {
  if (i2c::functions::class_is_assignable_from(i2c::class_of<BasicBeatmapObjectManager*>(), beatmapObjectManager->klass)) {
    return true;
  }

  return beatmapObjectManager->spawnHidden;
}

MAKE_HOOK_MATCH(BeatmapObjectManager_SpawnObstacle, &BeatmapObjectManager::AddSpawnedObstacleController, void,
                BeatmapObjectManager* self, GlobalNamespace::ObstacleController* obstacleController,
                ObstacleSpawnData obstacleSpawnData) {
  if (!Hooks::isNoodleHookEnabled())
    return BeatmapObjectManager_SpawnObstacle(self, obstacleController, obstacleSpawnData);

  if (obstacleController == nullptr) {
    return;
  }
  self->SetObstacleEventCallbacks(obstacleController);
  if (self->obstacleWasSpawnedEvent) self->obstacleWasSpawnedEvent->Invoke(obstacleController);

  if (self->obstacleWasAddedEvent)
    self->obstacleWasAddedEvent->Invoke(obstacleController->_obstacleData, obstacleSpawnData,
                                        obstacleController->_obstacleData->rotation);

  self->_allBeatmapObjects->Add(obstacleController->i___GlobalNamespace__IBeatmapObjectController());
  obstacleController->ManualUpdate();
  // TRANSPILE HERE
  obstacleController->Hide(GetHiddenForType(self));
  //

  // POST FIX
  auto customObstacleData =
      i2c::try_cast<CustomJSONData::CustomObstacleData*>(obstacleController->_obstacleData);
  if (customObstacleData && customObstacleData->customData) {
    BeatmapObjectAssociatedData& ad = getAD(customObstacleData->customData);
    ad.doUnhide = true;
  }

  //
}

void InstallBeatmapObjectManagerSmallFixHooks() {
  INSTALL_HOOK_ORIG(NELogger::Logger, BeatmapObjectManager_SpawnObstacle);
}

NEInstallHooks(InstallBeatmapObjectManagerSmallFixHooks);