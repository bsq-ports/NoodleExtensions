#include "NEHooks.h"
#include "FakeNoteHelper.h"

#include "tracks/shared/Vector.h"

#include "beatsaber-hook/shared/hooking.hpp"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"

#include "GlobalNamespace/ObstacleSaberSparkleEffectManager.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/ObstacleSaberSparkleEffect.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "GlobalNamespace/SaberTypeExtensions.hpp"

#include "UnityEngine/Color.hpp"

#include "custom-json-data/shared/VList.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace NoodleExtensions;

/*MAKE_HOOK_MATCH(ObstacleSaberSparkleEffectManager_GetBurnMarkPos, &ObstacleSaberSparkleEffectManager::GetBurnMarkPos,
                bool, ObstacleSaberSparkleEffectManager* self, UnityEngine::Bounds bounds,
                UnityEngine::Transform* transform, UnityEngine::Vector3 bladeBottomPos,
                UnityEngine::Vector3 bladeTopPos, by_ref<UnityEngine::Vector3> burnMarkPos) {
  if (!Hooks::isNoodleHookEnabled())
    return ObstacleSaberSparkleEffectManager_GetBurnMarkPos(self, bounds, transform, bladeBottomPos, bladeTopPos,
                                                            burnMarkPos);

  if (NEVector::Vector3(bounds.get_size()) == NEVector::Vector3::zero()) {
    // burnMarkPos is out ref, must assign
    burnMarkPos.heldRef = NEVector::Vector3::zero();
    return false;
  }

  return ObstacleSaberSparkleEffectManager_GetBurnMarkPos(self, bounds, transform, bladeBottomPos, bladeTopPos,
                                                          burnMarkPos);
}

void ObstacleSaberSparkleEffectManagerHook() {
  INSTALL_HOOK(NELogger::Logger, ObstacleSaberSparkleEffectManager_GetBurnMarkPos);
}

NEInstallHooks(ObstacleSaberSparkleEffectManagerHook)*/