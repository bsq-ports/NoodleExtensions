#include "NELogger.h"
#include "VariableMovementHelper.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"

#include "GlobalNamespace/SliderController.hpp"
#include "GlobalNamespace/SliderMovement.hpp"
#include "GlobalNamespace/SliderIntensityEffect.hpp"
#include "GlobalNamespace/CutoutAnimateEffect.hpp"

#include "System/Action.hpp"
#include "System/Action_1.hpp"

#include "NEUtils.hpp"
#include "Animation/AnimationHelper.h"
#include "tracks/shared/TimeSourceHelper.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "NECaches.h"

#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace NoodleExtensions;

void NECaches::ClearSliderCaches() {
  NECaches::sliderCache.clear();
}

MAKE_HOOK_MATCH(SliderController_IsNoteStartOfThisSlider, &SliderController::IsNoteStartOfThisSlider, bool,
                SliderController* self, NoteData* noteData) {

  if (!Hooks::isNoodleHookEnabled()) return SliderController_IsNoteStartOfThisSlider(self, noteData);

  if (!Approximately(noteData->time, self->_sliderData->time) ||
      noteData->colorType != self->_sliderData->colorType)
  {
      return false;
  }

  static auto CustomSliderKlass = classof(CustomJSONData::CustomSliderData*);
  static auto CustomNoteKlass = classof(CustomJSONData::CustomNoteData*);
  if (self->_sliderData->klass != CustomSliderKlass ||
      noteData->klass != CustomNoteKlass) {
    return SliderController_IsNoteStartOfThisSlider(self, noteData);
  }
  auto* customSliderData = reinterpret_cast<CustomJSONData::CustomSliderData*>(self->_sliderData);
  auto* customNoteData = reinterpret_cast<CustomJSONData::CustomNoteData*>(noteData);
  if (!customSliderData->customData || !customNoteData->customData) {
    return SliderController_IsNoteStartOfThisSlider(self, noteData);
  }
  BeatmapObjectAssociatedData& sliderAD = getAD(customSliderData->customData);
  BeatmapObjectAssociatedData& noteAD = getAD(customNoteData->customData);
  if (!sliderAD.parsed || !noteAD.parsed) {
    return SliderController_IsNoteStartOfThisSlider(self, noteData);
  }

  auto movementData = NECaches::beatmapObjectSpawnController->beatmapObjectSpawnMovementData;
  int offset = movementData->noteLinesCount / 2;
  float headIndex = sliderAD.objectData.startX.has_value() ?
                    sliderAD.objectData.startX.value() + offset : self->_sliderData->headLineIndex;
  float noteIndex = noteAD.objectData.startX.has_value() ?
                    noteAD.objectData.startX.value() + offset : noteData->lineIndex;
  float headLayer = sliderAD.objectData.startY.value_or(
                    static_cast<int>(self->_sliderData->headLineLayer));
  float noteLayer = noteAD.objectData.startY.value_or(
                    static_cast<int>(noteData->noteLineLayer));

  return Approximately(headIndex, noteIndex) && Approximately(headLayer, noteLayer);
}

MAKE_HOOK_MATCH(SliderController_Init, &SliderController::Init, void, SliderController* self,
                SliderController::LengthType lengthType, SliderData* sliderData, ByRef<SliderSpawnData> sliderSpawnData,
                float noteUniformScale, float randomValue) {

  SliderController_Init(self, lengthType, sliderData, sliderSpawnData, noteUniformScale, randomValue);

  if (!Hooks::isNoodleHookEnabled()) return;

  static auto CustomKlass = classof(CustomJSONData::CustomSliderData*);

  if (sliderData->klass != CustomKlass) return;

  auto* customSliderData = reinterpret_cast<CustomJSONData::CustomSliderData*>(sliderData);

  Transform* transform = self->get_transform();
  transform->set_localScale(NEVector::Vector3::one()); // This is a fix for animation due to notes being
  // recycled

  if (!customSliderData->customData) return;
  BeatmapObjectAssociatedData& ad = getAD(customSliderData->customData);

  if (!ad.parsed) return;

  // Transpile GetCustomNjs or already taken care of by NoodleMovementDataProvider?

  auto& sliderCache = NECaches::getSliderCache(self->_sliderMovement);
  sliderCache.cutoutAnimateEffect = self->_cutoutAnimateEffect;

  NEVector::Quaternion localRotation = NEVector::Quaternion::identity();
  if (ad.objectData.rotation || ad.objectData.localRotation) {
    if (ad.objectData.localRotation) {
      localRotation = *ad.objectData.localRotation;
    }

    if (ad.objectData.rotation) {
      NEVector::Quaternion worldRotationQuatnerion = *ad.objectData.rotation;
      self->_sliderMovement->_worldRotation = worldRotationQuatnerion;

      worldRotationQuatnerion = worldRotationQuatnerion * localRotation;
      transform->set_localRotation(worldRotationQuatnerion);
    } else {
      transform->set_localRotation(NEVector::Quaternion(transform->get_localRotation()) * localRotation);
    }
  }

  auto scale = NEVector::Vector3(ad.objectData.scaleX.value_or(1.0f), ad.objectData.scaleY.value_or(1.0f),
                                 ad.objectData.scaleZ.value_or(1.0f));
  transform->set_localScale(scale);
  ad.internalScale = scale;

  ad.worldRotation = self->_sliderMovement->_worldRotation;
  ad.localRotation = localRotation;
}

MAKE_HOOK_MATCH(SliderMovement_ManualUpdate, &SliderMovement::ManualUpdate, void, SliderMovement* self) {

  if (!Hooks::isNoodleHookEnabled()) return SliderMovement_ManualUpdate(self);

  static auto CustomKlass = classof(CustomJSONData::CustomSliderData*);
  if (self->_sliderData->klass != CustomKlass) return SliderMovement_ManualUpdate(self);

  auto* customSliderData = reinterpret_cast<CustomJSONData::CustomSliderData*>(self->_sliderData);
  if (!customSliderData->customData) return SliderMovement_ManualUpdate(self);

  BeatmapObjectAssociatedData& ad = getAD(customSliderData->customData);
  auto const& tracks = TracksAD::getAD(customSliderData->customData).tracks;

  if (tracks.empty() && !ad.animationData.parsed) {
    return SliderMovement_ManualUpdate(self);
  }

  auto movement = VariableMovementW(self->_variableMovementDataProvider);
  auto sliderData = self->_sliderData;
  float headNoteTime = sliderData->time;
  float tailNoteTime = sliderData->tailTime;
  float jumpDuration = movement.jumpDuration;

  float duration = (jumpDuration * 0.75f) + (tailNoteTime - headNoteTime);
  float normalizedTime;
  float timeSinceTailNoteJump;
  float halfJumpDuration = jumpDuration * 0.5f;

  auto time = NoodleExtensions::getTimeProp(tracks);
  if (time) {
      normalizedTime = time.value();
      self->_timeSinceHeadNoteJump = normalizedTime * duration;
      timeSinceTailNoteJump = (self->_timeSinceHeadNoteJump + (headNoteTime - halfJumpDuration)) -
                              (tailNoteTime - halfJumpDuration);
  } else {
      float songTime = TimeSourceHelper::getSongTime(self->_audioTimeSyncController);
      self->_timeSinceHeadNoteJump = songTime - (headNoteTime - halfJumpDuration);
      normalizedTime = self->_timeSinceHeadNoteJump / duration;
      timeSinceTailNoteJump = songTime - (tailNoteTime - halfJumpDuration);
  }

  float normalizedHeadTime = self->_timeSinceHeadNoteJump / jumpDuration;
  float normalizedTailTime = timeSinceTailNoteJump / jumpDuration;

  normalizedTime = std::max(normalizedTime, 0.0f);
  auto offset = AnimationHelper::GetObjectOffset(ad.animationData, tracks, normalizedTime);

  auto transform = self->get_transform();
  self->_localPosition = NEVector::Vector3::zero();
  NEVector::Quaternion worldRotation = self->_worldRotation;

  if (offset.rotationOffset.has_value() || offset.localRotationOffset.has_value()) {
    worldRotation = ad.worldRotation;
    NEVector::Quaternion localRotation = ad.localRotation;

    NEVector::Quaternion worldRotationQuaternion = worldRotation;
    if (offset.rotationOffset.has_value()) {
      worldRotationQuaternion = worldRotationQuaternion * *offset.rotationOffset;
      worldRotation = worldRotationQuaternion;
    }

    worldRotationQuaternion = worldRotationQuaternion * localRotation;

    if (offset.localRotationOffset.has_value()) {
      worldRotationQuaternion = worldRotationQuaternion * *offset.localRotationOffset;
    }

    transform->set_localRotation(worldRotationQuaternion);
  }

  if (offset.scaleOffset.has_value()) {
    transform->set_localScale(*offset.scaleOffset * ad.internalScale);
  }

  auto& sliderCache = NECaches::getSliderCache(self);
  if (offset.dissolve.has_value()) {
    CutoutAnimateEffect*& cutoutAnimateEffect = sliderCache.cutoutAnimateEffect;
    cutoutAnimateEffect->SetCutout(1 - offset.dissolve.value());
  }

  auto definitePosition = AnimationHelper::GetDefinitePositionOffset(ad.animationData, tracks, normalizedTime);
  if (definitePosition.has_value()) {
    transform->set_localPosition(definitePosition.value());
    return;
  }

  if (offset.positionOffset.has_value()) {
    self->_localPosition = *offset.positionOffset;
  }

  float headOffsetZ = self->_sliderSpawnData.headNoteOffset.z;
  float a = movement.moveEndPosition.z + headOffsetZ;
  float b = movement.jumpEndPosition.z + headOffsetZ;
  self->_localPosition.z += std::lerp(a, b, normalizedHeadTime);
  transform->localPosition = worldRotation * self->_localPosition;

  // TRANSPILE

  if (!self->_headDidMovePastCutMarkReported && normalizedHeadTime > 0.5f) {
		self->_headDidMovePastCutMarkReported = true;
		if(self->headDidMovePastCutMarkEvent)
      self->headDidMovePastCutMarkEvent->Invoke();
	} if (!self->_tailDidMovePastCutMarkReported && normalizedTailTime > 0.5f) {
		self->_tailDidMovePastCutMarkReported = true;
		if (self->tailDidMovePastCutMarkEvent)
      self->tailDidMovePastCutMarkEvent->Invoke();
	} if (self->_movementEndReported && normalizedTailTime > 0.75f) {
		self->_movementEndReported = true;
		if (self->movementDidFinishEvent)
      self->movementDidFinishEvent->Invoke();
	}
	if (self->movementDidMoveEvent)
    self->movementDidMoveEvent->Invoke(self->_timeSinceHeadNoteJump);
}

void InstallSliderControllerHooks() {
  INSTALL_HOOK(NELogger::Logger, SliderController_IsNoteStartOfThisSlider);
  INSTALL_HOOK(NELogger::Logger, SliderController_Init);
  INSTALL_HOOK(NELogger::Logger, SliderMovement_ManualUpdate);
}

NEInstallHooks(InstallSliderControllerHooks);