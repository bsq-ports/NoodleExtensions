#include "NELogger.h"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "UnityEngine/Bounds.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Time.hpp"

#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/PlayerHeadAndObstacleInteraction.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "System/Collections/Generic/HashSet_1.hpp"
#include "System/Action_1.hpp"
#include "tracks/shared/Vector.h"
#include "AssociatedData.h"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "custom-json-data/shared/VList.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

// returns true to skip obstacle
static bool BoundsNullCheck(ObstacleController* obstacleController) {
  if (NEVector::Vector3(obstacleController->bounds.size) == NEVector::Vector3::zero()) {
    return true;
  }

  auto customObstacleData = i2c::try_cast<CustomJSONData::CustomObstacleData*>(obstacleController->_obstacleData);
  if (!customObstacleData) {
    return false;
  }

  auto const& ad = getAD(customObstacleData->customData);
  return ad.objectData.fake.value_or(false);
}

MAKE_HOOK_MATCH(PlayerHeadAndObstacleInteraction_RefreshIntersectingObstacles,
                &PlayerHeadAndObstacleInteraction::RefreshIntersectingObstacles, void,
                PlayerHeadAndObstacleInteraction* self, Vector3 worldPos) {

  bool noodleHookEnabled = Hooks::isNoodleHookEnabled();
  if (!noodleHookEnabled) {
    return PlayerHeadAndObstacleInteraction_RefreshIntersectingObstacles(self, worldPos);
  }

  int frameCount = Time::get_frameCount();
  if (self->_lastFrameNumCheck == frameCount) {
    return;
  }
  self->_lastFrameNumCheck = frameCount;
  for (auto obstacleController :
       ListW<UnityW<ObstacleController>>(self->_beatmapObjectManager->activeObstacleControllers)) {

    // transpile start
    if (BoundsNullCheck(obstacleController)) {
      continue;
    }
    // transpile end

    if (obstacleController->isActiveAndEnabled) {
      if (!obstacleController->hasPassedAvoidedMark) {
        Vector3 vector = obstacleController->transform->InverseTransformPoint(worldPos);
        if (obstacleController->bounds.Contains(vector)) {
          if (!self->_intersectingObstacles->Contains(obstacleController)) {
            self->_intersectingObstacles->Add(obstacleController);
            auto action = self->headDidEnterObstacleEvent;
            if (action != nullptr) {
              action->Invoke(obstacleController);
            }
          }
        } else {
          self->_intersectingObstacles->Remove(obstacleController);
        }
      } else if (obstacleController->hasPassedAvoidedMark) {
        self->_intersectingObstacles->Remove(obstacleController);
      }
    }
  }
}

void InstallPlayerHeadAndObstacleInterationHooks() {
  INSTALL_HOOK_ORIG(NELogger::Logger, PlayerHeadAndObstacleInteraction_RefreshIntersectingObstacles);
}

NEInstallHooks(InstallPlayerHeadAndObstacleInterationHooks);