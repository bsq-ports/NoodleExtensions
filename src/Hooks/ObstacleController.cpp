#include "beatsaber-hook/shared/safeptr.hpp"
#include "NELogger.h"
#include "beatsaber-hook/shared/hooking.hpp"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"

#include "System/Action_1.hpp"
#include "System/Action_2.hpp"
#include "GlobalNamespace/ConditionalMaterialSwitcher.hpp"
#include "GlobalNamespace/CutoutAnimateEffect.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/ObstacleDissolve.hpp"
#include "GlobalNamespace/ParametricBoxFakeGlowController.hpp"
#include "GlobalNamespace/ParametricBoxFrameController.hpp"
#include "GlobalNamespace/SimpleColorSO.hpp"
#include "GlobalNamespace/StretchableObstacle.hpp"
#include "GlobalNamespace/StaticBeatmapObjectSpawnMovementData.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Renderer.hpp"
#include "UnityEngine/Transform.hpp"

#include "Animation/AnimationHelper.h"
#include "Animation/ParentObject.h"
#include "AssociatedData.h"
#include "NEUtils.hpp"
#include "NEConfig.h"
#include "NEHooks.h"
#include "NECaches.h"
#include "VariableMovementHelper.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

#include "tracks/shared/AssociatedData.h"
#include "tracks/shared/TimeSourceHelper.h"

#define ID "Noodle"
#include "chroma/shared/ObstacleAPI.hpp"
#undef ID

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace TrackParenting;

static safe_ptr<System::Collections::Generic::List_1<UnityW<ObstacleController>>*> activeObstacles;
safe_ptr<System::Collections::Generic::List_1<UnityW<ObstacleController>>*>& getActiveObstacles() {
  if (!activeObstacles) activeObstacles = System::Collections::Generic::List_1<UnityW<ObstacleController>>::New_ctor();

  return activeObstacles;
}

bool mapLoaded = false;

void OnObstacleChangeColor(GlobalNamespace::ObstacleControllerBase* oc, Sombrero::FastColor const& color) {
  NECaches::getObstacleCache(oc).color = color;
}

void NECaches::ClearObstacleCaches() {
  NECaches::obstacleCache.clear();
  mapLoaded = false;
}

float obstacleTimeAdjust(ObstacleController* oc, float original, std::span<TrackW> tracks) {
  auto movement = VariableMovementW(oc->_variableMovementDataProvider);

  float moveDuration = movement.moveDuration;
  if (original <= moveDuration) return original;

  auto time = NoodleExtensions::getTimeProp(tracks);
  if (!time) return original;

  return (time.value() * (oc->_finishMovementTime - moveDuration)) + moveDuration;
}

MAKE_HOOK_MATCH(ObstacleController_Init, &ObstacleController::Init, void, ObstacleController* self,
                ObstacleData* normalObstacleData, by_ref<GlobalNamespace::ObstacleSpawnData const> obstacleSpawnData) {
  if (!Hooks::isNoodleHookEnabled()) return ObstacleController_Init(self, normalObstacleData, obstacleSpawnData);
  ObstacleController_Init(self, normalObstacleData, obstacleSpawnData);

  static auto CustomKlass = i2c::class_of<CustomJSONData::CustomObstacleData*>();

  if (normalObstacleData->klass != CustomKlass) return;

  auto* obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData*>(normalObstacleData);

  Transform* transform = self->get_transform();
  transform->set_localScale(NEVector::Vector3::one());

  if (!obstacleData->customData) {
    NELogger::Logger.warn("Obstacle at time {} has no customData", obstacleData->time);
    return;
  }

  auto& obstacleCache = NECaches::getObstacleCache(self);

  if (getNEConfig().materialBehaviour.GetValue() == (int)MaterialBehaviour::SMART_COLOR) {
    // lazy initialize since Chroma clears the callbacks on map load and ordering that is not easy
    if (!mapLoaded) {
      static auto callbackOpt = Chroma::ObstacleAPI::getObstacleChangedColorCallbackSafe();
      mapLoaded = true;
      if (callbackOpt) {
        Chroma::ObstacleAPI::ObstacleCallback& callback = *callbackOpt;
        // remove if it already exists
        callback -= &OnObstacleChangeColor;
        callback += &OnObstacleChangeColor;
      }
    }
  }

  BeatmapObjectAssociatedData& ad = getAD(obstacleData->customData);

  if (!ad.parsed) {
    NELogger::Logger.warn("Obstacle at time {} was not parsed", obstacleData->time);
    return;
  }

  //   // TODO: reimplement smarter dissolve enabling based on color alpha and brightness
  // ArrayW<ConditionalMaterialSwitcher*>& materialSwitchers = obstacleCache.conditionalMaterialSwitchers;
  // if (!materialSwitchers) {
  //   materialSwitchers = self->GetComponentsInChildren<ConditionalMaterialSwitcher*>();
  // }

  // // Reset only if NE dissolve is enabled
  // if (getNEConfig().enableObstacleDissolve.GetValue()) {
  //   for (auto* materialSwitcher : materialSwitchers) {
  //     materialSwitcher->_renderer->set_sharedMaterial(materialSwitcher->_material0);
  //   }
  // }
  obstacleCache.dissolveEnabled = false;

  auto const setBounds = [&ad, &self]() constexpr {
    if (ad.objectData.uninteractable.value_or(false)) {
      self->bounds.set_size(NEVector::Vector3::zero());
      getActiveObstacles()->Remove(self);
    } else {
      getActiveObstacles()->Add(self);
    }

    ad.boundsSize = self->bounds.get_size();
  };

  if (!obstacleData->customData->value) {
    setBounds();
    return;
  }

  // transpiler GetWorldRotation
  {
    if (ad.objectData.rotation) {
      auto const& rotation = *ad.objectData.rotation;

      self->_worldRotation = rotation;
      transform->localRotation = rotation;
    }
    ad.worldRotation = self->_worldRotation;
  }

  // postfix rotation code
  {
    NEVector::Quaternion localRotation = NEVector::Quaternion::identity();
    auto const& localrot = ad.objectData.localRotation;
    if (localrot.has_value()) {
      localRotation = *localrot;
      transform->localRotation = NEVector::Quaternion(self->_worldRotation) * localRotation;
    }

    ad.localRotation = localRotation;
  }

  if (ad.objectData.localRotation) {
    auto const& localRotation = *ad.objectData.localRotation;
    transform->localRotation = NEVector::Quaternion(self->_worldRotation) * localRotation;
  }

  auto scale = NEVector::Vector3(ad.objectData.scaleX.value_or(1.0f), ad.objectData.scaleY.value_or(1.0f),
                                 ad.objectData.scaleZ.value_or(1.0f));
  ad.internalScale = scale;
  transform->set_localScale(scale);

  // GetCustomWidth transpiler
  if (ad.objectData.width) {
    self->_width = *ad.objectData.width * StaticBeatmapObjectSpawnMovementData::kNoteLinesDistance;
  }

  if (ad.objectData.length) {
    self->_length = *ad.objectData.length * StaticBeatmapObjectSpawnMovementData::kNoteLinesDistance;
  }

  ad.moveStartPos = obstacleSpawnData->moveOffset;
  // ad.moveEndPos = self->_midPos;
  // ad.jumpEndPos = self->_endPos;

  // Note offset is jumpEndPosition + spawn move offset (z removed), matching C# LATEST
  auto movement = VariableMovementW(self->_variableMovementDataProvider);

  Vector3 noteOffset = NEVector::Vector3(movement.jumpEndPosition) + obstacleSpawnData->moveOffset;
  noteOffset.z = 0;
  ad.noteOffset = noteOffset;

  self->_stretchableObstacle->SetAllProperties(self->_width * 0.98f, self->_height, self->_length, self->_color,
                                               TimeSourceHelper::getSongTime(self->_audioTimeSyncController));
  self->_bounds = self->_stretchableObstacle->bounds;
  setBounds();
}

static void ObstacleController_ManualUpdateTranspile(ObstacleController* self, float const elapsedTime,
                                                     VariableMovementW movement) {
  // TRANSPILE HERE
  float num = elapsedTime;
  // TRANSPILE HERE
  NEVector::Vector3 posForTime = self->GetPosForTime(num);
  self->transform->localPosition = NEVector::Quaternion(self->_worldRotation) * posForTime;
  self->_length = self->GetObstacleLength();

  if (movement.wasUpdatedThisFrame) {
    self->_stretchableObstacle->SetSizeAndOffset(self->_width, self->_height, self->_length,
                                                 TimeSourceHelper::getSongTime(self->_audioTimeSyncController));
  }
  auto action = self->didUpdateProgress;
  if (action != nullptr) {
    action->Invoke(self, num);
  }
  if (!self->_passedThreeQuartersOfJumpDurationReported && num > self->_passedThreeQuartersOfJumpDurationTime) {
    self->_passedThreeQuartersOfJumpDurationReported = true;
    auto action2 = self->passedThreeQuartersOfJumpDurationEvent;
    if (action2 != nullptr) {
      action2->Invoke(self);
    }
  }
  if (!self->_passedAvoidedMarkReported && num > self->_passedAvoidedMarkTime) {
    self->_passedAvoidedMarkReported = true;
    auto action3 = self->passedAvoidedMarkEvent;
    if (action3 != nullptr) {
      action3->Invoke(self);
    }
  }
  if (num > self->_finishMovementTime) {
    auto action4 = self->finishedMovementEvent;
    if (action4 == nullptr) {
      return;
    }
    action4->Invoke(self);
  }
}

MAKE_HOOK_MATCH(ObstacleController_ManualUpdate, &ObstacleController::ManualUpdate, void, ObstacleController* self) {
  if (!Hooks::isNoodleHookEnabled()) return ObstacleController_ManualUpdate(self);

  static auto CustomKlass = i2c::class_of<CustomJSONData::CustomObstacleData*>();

  if (self->_obstacleData->klass != CustomKlass) return ObstacleController_ManualUpdate(self);

  auto* obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData*>(self->_obstacleData);

  BeatmapObjectAssociatedData& ad = getAD(obstacleData->customData);

  if (ad.doUnhide) {
    self->Hide(false);
    ad.doUnhide = false;
  }
  if (!obstacleData->customData->value) {
    ObstacleController_ManualUpdate(self);
    return;
  }

  auto const& tracks = TracksAD::getAD(obstacleData->customData).tracks;

  if (tracks.empty() && !ad.parsed) {
    ObstacleController_ManualUpdate(self);
    return;
  }

  VariableMovementW movement = VariableMovementW(self->_variableMovementDataProvider);

  if (!self->_passedAvoidedMarkReported) {
    self->_startTimeOffset = self->_obstacleData->get_time() - movement.moveDuration - movement.halfJumpDuration;
    self->_passedThreeQuartersOfJumpDurationTime = movement.moveDuration + movement.jumpDuration * 0.75f;
    self->_passedAvoidedMarkTime = movement.moveDuration + movement.halfJumpDuration + self->_obstacleDuration + 0.15f;
    self->_finishMovementTime = movement.moveDuration + movement.jumpDuration + self->_obstacleDuration;
    self->_startPos = NEVector::Vector3(movement.moveStartPosition) + self->_obstacleSpawnData.moveOffset;
    self->_midPos = NEVector::Vector3(movement.moveEndPosition) + self->_obstacleSpawnData.moveOffset;
    self->_endPos = NEVector::Vector3(movement.jumpEndPosition) + self->_obstacleSpawnData.moveOffset;
  }

  float moveDuration = movement.moveDuration;
  float jumpDuration = movement.jumpDuration;
  float obstacleDuration = self->_obstacleData->_duration_k__BackingField;

  float const songTime = TimeSourceHelper::getSongTime(self->_audioTimeSyncController);
  float const elapsedTime = songTime - self->_startTimeOffset;
  float const obstacleOriginalTime = (elapsedTime - moveDuration) / (jumpDuration + obstacleDuration);
  float normalTime;

  auto animatedTime = NoodleExtensions::getTimeProp(tracks);

  if (animatedTime) {
    normalTime = *animatedTime;
  } else {
    normalTime = obstacleOriginalTime;
  }

  // auto context = TracksAD::getBeatmapAD(NECaches::customBeatmapData->customData).internal_tracks_context;
  AnimationHelper::ObjectOffset offset = AnimationHelper::GetObjectOffset(ad.animationData, tracks, normalTime);

  if (offset.positionOffset.has_value()) {
    NEVector::Vector3 moveOffset = ad.moveStartPos;
    self->_obstacleSpawnData = GlobalNamespace::ObstacleSpawnData(moveOffset + offset.positionOffset.value(),
                                                                  self->_obstacleSpawnData.obstacleWidth,
                                                                  self->_obstacleSpawnData.obstacleHeight);
  }

  Transform* transform = self->get_transform();

  if (offset.rotationOffset.has_value() || offset.localRotationOffset.has_value()) {
    NEVector::Quaternion const& worldRotation = ad.worldRotation;
    NEVector::Quaternion const& localRotation = ad.localRotation;

    NEVector::Quaternion worldRotationQuaternion = worldRotation;
    if (offset.rotationOffset.has_value()) {
      worldRotationQuaternion = worldRotationQuaternion * *offset.rotationOffset;
      self->_worldRotation = worldRotationQuaternion;
    }

    worldRotationQuaternion = worldRotationQuaternion * localRotation;

    if (offset.localRotationOffset.has_value()) {
      worldRotationQuaternion = worldRotationQuaternion * *offset.localRotationOffset;
    }

    transform->set_localRotation(worldRotationQuaternion);
  }

  if (offset.cuttable.has_value()) {
    if (*offset.cuttable >= 1.0f) {
      self->bounds.set_size(NEVector::Vector3::zero());
    } else {
      self->bounds.set_size(ad.boundsSize);
    }
  }

  if (offset.scaleOffset.has_value()) {
    transform->set_localScale(*offset.scaleOffset * ad.internalScale);
  }

  auto& obstacleCache = NECaches::getObstacleCache(self);

  //   // TODO: reimplement smarter dissolve enabling based on color alpha and brightness

  // if (obstacleCache.cachedData != self->_obstacleData) {
  //   obstacleCache.cachedData = self->_obstacleData;
  //   // Obstacles are pooled. Clear obstacle when initialized if it's not colored or update to its new color (probably
  //   // redundantly)
  //   auto color = Chroma::ObstacleAPI::getObstacleControllerColorSafe(self);
  //   if (color) {
  //     obstacleCache.color = *color;
  //   } else {
  //     obstacleCache.color = std::nullopt;
  //   }
  // }

  bool obstacleDissolveConfig = true; // getNEConfig().enableObstacleDissolve.GetValue();
  if (offset.dissolve.has_value()) {
    float dissolve;

    if (obstacleDissolveConfig) {
      dissolve = 1 - *offset.dissolve;
    } else {
      dissolve = *offset.dissolve >= 0 ? 0 : 1;
    }

    bool wasEnabled = obstacleCache.dissolveEnabled;
    obstacleCache.dissolveEnabled = obstacleDissolveConfig;

    //   // TODO: reimplement smarter dissolve enabling based on color alpha and brightness
    // if (obstacleCache.dissolveEnabled) {
    //   if (getNEConfig().materialBehaviour.GetValue() == (int)MaterialBehaviour::BASIC) {
    //     obstacleCache.dissolveEnabled |= dissolve > 0.0f;
    //   } else {
    //     bool transparent = true;

    //     if (getNEConfig().materialBehaviour.GetValue() == (int)MaterialBehaviour::SMART_COLOR) {
    //       auto const& colorIt = obstacleCache.color;

    //       // multiply rgb by alpha?
    //       if (colorIt &&
    //           (colorIt->a > 1.0f || NEVector::Vector3(colorIt->r, colorIt->g, colorIt->b).Magnitude() >= 1)) {
    //         transparent = false;
    //       }
    //     }

    //     if (transparent) obstacleCache.dissolveEnabled = dissolve > 0.0f;
    //   }
    // }

    // if (wasEnabled != obstacleCache.dissolveEnabled) {
    //   ArrayW<ConditionalMaterialSwitcher*> materialSwitchers = obstacleCache.conditionalMaterialSwitchers;
    //   for (auto* materialSwitcher : materialSwitchers) {
    //     materialSwitcher->_renderer->set_sharedMaterial(obstacleCache.dissolveEnabled ? materialSwitcher->_material1
    //                                                                                   :
    //                                                                                   materialSwitcher->_material0);
    //   }
    // }

    CutoutAnimateEffect*& cutoutAnimationEffect = obstacleCache.cutoutAnimateEffect;
    if (!cutoutAnimationEffect) {
      obstacleCache.obstacleDissolve =
          obstacleCache.obstacleDissolve ?: self->get_gameObject()->GetComponent<ObstacleDissolve*>();
      cutoutAnimationEffect = obstacleCache.obstacleDissolve->_cutoutAnimateEffect;
    }

    cutoutAnimationEffect->SetCutout(dissolve);
  }

  auto animatedTimeAdjusted = obstacleTimeAdjust(self, elapsedTime, tracks);
  return ObstacleController_ManualUpdateTranspile(self, animatedTimeAdjusted, movement);
}

MAKE_HOOK_MATCH(ObstacleController_GetPosForTime, &ObstacleController::GetPosForTime, Vector3, ObstacleController* self,
                float time) {
  if (!Hooks::isNoodleHookEnabled()) return ObstacleController_GetPosForTime(self, time);

  //    static auto CustomObstacleDataKlass = i2c::class_of<CustomJSONData::CustomObstacleData *>();
  //    CRASH_UNLESS(self);
  //    CRASH_UNLESS(self->_obstacleData);
  //    NELogger::Logger.debug("ObstacleController::GetPosForTime %p", self->_obstacleData);
  //    CRASH_UNLESS(self->_obstacleData->klass);
  //    CRASH_UNLESS(CustomObstacleDataKlass);
  //
  //    if (!self || !self->_obstacleData || self->_obstacleData->klass != CustomObstacleDataKlass) {
  //        return ObstacleController_GetPosForTime(self, time);
  //    }
  auto* obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData*>(self->_obstacleData);

  static auto CustomKlass = i2c::class_of<CustomJSONData::CustomObstacleData*>();

  if (self->_obstacleData->klass != CustomKlass || !obstacleData->customData->value) {
    return ObstacleController_GetPosForTime(self, time);
  }
  BeatmapObjectAssociatedData const& ad = getAD(obstacleData->customData);

  auto const& tracks = TracksAD::getAD(obstacleData->customData).tracks;

  VariableMovementW movement = VariableMovementW(self->_variableMovementDataProvider);

  float moveDuration = movement.moveDuration;
  float jumpDuration = movement.jumpDuration;
  float obstacleDuration = self->_obstacleData->_duration_k__BackingField;

  float jumpTime = (time - moveDuration) / (jumpDuration + obstacleDuration);
  jumpTime = std::clamp(jumpTime, 0.0f, 1.0f);

  // auto context = TracksAD::getBeatmapAD(NECaches::customBeatmapData->customData).internal_tracks_context;

  std::optional<NEVector::Vector3> position =
      AnimationHelper::GetDefinitePositionOffset(ad.animationData, tracks, jumpTime);

  if (!position.has_value()) return ObstacleController_GetPosForTime(self, time);

  NEVector::Vector3 const& noteOffset = ad.noteOffset;
  NEVector::Vector3 definitePosition = *position + noteOffset;
  if (time < moveDuration) {
    NEVector::Vector3 result = NEVector::Vector3::LerpUnclamped(self->_startPos, self->_midPos, time / moveDuration);
    return result + (definitePosition - static_cast<NEVector::Vector3>(self->_midPos));
  } else {
    return definitePosition;
  }
}

MAKE_HOOK_MATCH(ObstacleController_GetObstacleLength, &ObstacleController::GetObstacleLength, float,
                ObstacleController* self) {
  if (!Hooks::isNoodleHookEnabled()) return ObstacleController_GetObstacleLength(self);

  static auto CustomKlass = i2c::class_of<CustomJSONData::CustomObstacleData*>();

  if (self->_obstacleData->klass != CustomKlass) return ObstacleController_GetObstacleLength(self);

  auto* obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData*>(self->_obstacleData);
  if (!obstacleData->customData->value) return ObstacleController_GetObstacleLength(self);

  BeatmapObjectAssociatedData const& ad = getAD(obstacleData->customData);

  if (ad.objectData.length) {
    return *ad.objectData.length * GlobalNamespace::StaticBeatmapObjectSpawnMovementData::kNoteLinesDistance;
  }

  return ObstacleController_GetObstacleLength(self);
}

MAKE_HOOK_MATCH(ParametricBoxFakeGlowController_OnEnable, &ParametricBoxFakeGlowController::OnEnable, void,
                ParametricBoxFakeGlowController* self) {
  if (Hooks::isNoodleHookEnabled()) return;

  ParametricBoxFakeGlowController_OnEnable(self);
}

// Kaitlyn's fake glow overbounds fix (modified)
// https://github.com/ItsKaitlyn03/AnyTweaks-old/blob/a723e76506cd7cb8ab6f890b3d6a342f3618aaeb/src/hooks/ParametricBoxFakeGlowController.cpp#L23-L36
MAKE_HOOK_MATCH(ParametricBoxFakeGlowController_Refresh, &GlobalNamespace::ParametricBoxFakeGlowController::Refresh,
                void, GlobalNamespace::ParametricBoxFakeGlowController* self) {
  if (!Hooks::isNoodleHookEnabled()) return ParametricBoxFakeGlowController_Refresh(self);

  float initialEdgeSizeMultip = self->edgeSizeMultiplier;
  float minDimension = std::min({ self->width, self->height, std::abs(self->length) });
  float clampedMinDimension = std::max(minDimension, 0.1f);

  self->edgeSizeMultiplier = std::min(self->edgeSizeMultiplier, std::min(0.5f, clampedMinDimension * 13.5f));

  ParametricBoxFakeGlowController_Refresh(self);
}

// #include "beatsaber-hook/shared/binary.hpp"
// MAKE_HOOK(Object_New, nullptr, Il2CppObject *, Il2CppClass *klass) {
//     if (test && klass && klass->name && klass->namespaze) {
//         NELogger::Logger.info("Allocating a {}.{}", klass->namespaze, klass->name);
//         PrintBacktrace(10);
//     }
//     return Object_New(klass);
// }

void InstallObstacleControllerHooks() {
  INSTALL_HOOK_ORIG(NELogger::Logger, ObstacleController_Init);
  INSTALL_HOOK_ORIG(NELogger::Logger, ObstacleController_ManualUpdate);
  INSTALL_HOOK(NELogger::Logger, ObstacleController_GetPosForTime);
  INSTALL_HOOK(NELogger::Logger, ObstacleController_GetObstacleLength);

  // Fixes glow being too large on small walls.
  INSTALL_HOOK(NELogger::Logger, ParametricBoxFakeGlowController_Refresh);

  // Uncomment to disable fake glow.
  // INSTALL_HOOK(NELogger::Logger, ParametricBoxFakeGlowController_OnEnable);

  // Instruction on((const int32_t*) HookTracker::GetOrig(i2c::functions::object_new));
  // Instruction j2Ob_N_thunk(CRASH_UNLESS(on.findNthCall(1)->label));
  // logger.info("thunk addr %p %x", j2Ob_N_thunk.addr, *j2Ob_N_thunk.addr);
  // auto *j2Ob_N = CRASH_UNLESS(j2Ob_N_thunk.findNthDirectBranchWithoutLink(1));
  // INSTALL_HOOK(logger, Object_New, (void*) *j2Ob_N->label);
}

NEInstallHooks(InstallObstacleControllerHooks);
