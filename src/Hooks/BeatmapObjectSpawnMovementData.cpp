#include "NECaches.h"
#include "beatsaber-hook/shared/hooking.hpp"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapEventData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/StaticBeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/NoteCutDirection.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/NoteLineLayer.hpp"
#include "GlobalNamespace/NoteSpawnData.hpp"
#include "GlobalNamespace/ObstacleSpawnData.hpp"
#include "GlobalNamespace/SliderSpawnData.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "System/ValueType.hpp"

#include "AssociatedData.h"
#include "NEHooks.h"
#include "NELogger.h"
#include "SpawnDataHelper.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

#include <cmath>

using namespace GlobalNamespace;
using namespace NEVector;

// assigned in BeatmapObjectSpawnController::Start hook
BeatmapObjectSpawnController* beatmapObjectSpawnController;



MAKE_HOOK_MATCH(GetSliderSpawnData, &BeatmapObjectSpawnMovementData::GetSliderSpawnData, SliderSpawnData,
                BeatmapObjectSpawnMovementData* self, SliderData* normalSliderData) {

  if (!Hooks::isNoodleHookEnabled()) return GetSliderSpawnData(self, normalSliderData);

  if (!i2c::functions::class_is_assignable_from(i2c::class_of<CustomJSONData::CustomSliderData*>(), normalSliderData->klass))
    return GetSliderSpawnData(self, normalSliderData);

  auto* sliderData = reinterpret_cast<CustomJSONData::CustomSliderData*>(normalSliderData);
  auto result = GetSliderSpawnData(self, normalSliderData);

  // No need to create a custom ObstacleSpawnData if there is no custom data to begin with
  if (!sliderData->customData->value) {
    return result;
  }
  BeatmapObjectAssociatedData const& ad = getAD(sliderData->customData);

  bool gravityOverride = ad.objectData.disableNoteGravity.value_or(false);

  float offset = self->noteLinesCount / 2.f;
  float headLineIndex = ad.objectData.startX ? *ad.objectData.startX + offset : sliderData->headLineIndex;
  float headLineLayer = ad.objectData.startY.value_or(sliderData->headLineLayer.value__);
  float headStartLinelayer = ad.startNoteLineLayer;
  float tailLineIndex = ad.objectData.tailStartX ? *ad.objectData.tailStartX + offset : sliderData->tailLineIndex;
  float tailLineLayer = ad.objectData.tailStartY.value_or(sliderData->tailLineLayer.value__);
  float tailStartLineLayer = ad.tailStartNoteLineLayer;

  Vector3 headOffset =
      SpawnDataHelper::GetNoteOffset(self, headLineIndex, gravityOverride ? headLineLayer : headStartLinelayer);
  Vector3 tailOffset =
      SpawnDataHelper::GetNoteOffset(self, tailLineIndex, gravityOverride ? tailLineLayer : tailStartLineLayer);

  float headGravity =
      SpawnDataHelper::GetGravityBase(headLineLayer, gravityOverride ? headLineLayer : headStartLinelayer);
  float tailGravity =
      SpawnDataHelper::GetGravityBase(tailLineLayer, gravityOverride ? tailLineLayer : tailStartLineLayer);

  result = SliderSpawnData(headOffset, headGravity, tailOffset, tailGravity);

  return result;
}

MAKE_HOOK_MATCH(GetObstacleSpawnData, &BeatmapObjectSpawnMovementData::GetObstacleSpawnData, ObstacleSpawnData,
                BeatmapObjectSpawnMovementData* self, ObstacleData* normalObstacleData) {
  if (!Hooks::isNoodleHookEnabled()) return GetObstacleSpawnData(self, normalObstacleData);

  if (!i2c::functions::class_is_assignable_from(i2c::class_of<CustomJSONData::CustomObstacleData*>(), normalObstacleData->klass))
    return GetObstacleSpawnData(self, normalObstacleData);

  auto* obstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData*>(normalObstacleData);
  ObstacleSpawnData result = GetObstacleSpawnData(self, obstacleData);

  // No need to create a custom ObstacleSpawnData if there is no custom data to begin with
  if (!obstacleData->customData) {
    return result;
  }
  BeatmapObjectAssociatedData const& ad = getAD(obstacleData->customData);

  float lineIndex =
      ad.objectData.startX ? (*ad.objectData.startX + (self->noteLinesCount / 2.f)) : obstacleData->lineIndex;
  float lineLayer = ad.objectData.startY.value_or(obstacleData->lineLayer.value__);

  Vector3 obstacleOffset = SpawnDataHelper::GetObstacleOffset(self, lineIndex, lineLayer);
  obstacleOffset.y += NECaches::JumpOffsetYProvider->jumpOffsetY;

  std::optional<float> height = ad.objectData.height;
  std::optional<float> width = ad.objectData.width;

  float const layerHeight = StaticBeatmapObjectSpawnMovementData::LineYPosForLineLayer(NoteLineLayer::Upper) -
                            StaticBeatmapObjectSpawnMovementData::LineYPosForLineLayer(NoteLineLayer::Base);
  float obstacleHeight;
  if (height.has_value()) {
    obstacleHeight = height.value() * layerHeight;
  } else {
    // _topObstaclePosY =/= _obstacleTopPosY
    obstacleHeight =
        std::min(obstacleData->height * layerHeight,
                 self->_obstacleTopPosY - obstacleOffset.y);
  }

  float obstacleWidth =
      width.value_or(obstacleData->width) * GlobalNamespace::StaticBeatmapObjectSpawnMovementData::kNoteLinesDistance;
  // Adjust X offset so width is centered relative to the single-line width
  obstacleOffset.x +=
      (obstacleWidth - GlobalNamespace::StaticBeatmapObjectSpawnMovementData::kNoteLinesDistance) * 0.5f;

  result = ObstacleSpawnData(obstacleOffset, obstacleWidth, obstacleHeight);

  return result;
}

MAKE_HOOK_MATCH(GetJumpingNoteSpawnData, &BeatmapObjectSpawnMovementData::GetJumpingNoteSpawnData, NoteSpawnData,
                BeatmapObjectSpawnMovementData* self, NoteData* normalNoteData) {
  if (!Hooks::isNoodleHookEnabled()) return GetJumpingNoteSpawnData(self, normalNoteData);

  auto noteDataCast = i2c::try_cast<CustomJSONData::CustomNoteData*>(normalNoteData);
  if (!noteDataCast) return GetJumpingNoteSpawnData(self, normalNoteData);

  auto noteData = noteDataCast;
  if (!noteData->customData) {
    return GetJumpingNoteSpawnData(self, normalNoteData);
  }

  BeatmapObjectAssociatedData& ad = getAD(noteData->customData);

  float offset = self->noteLinesCount / 2.0f;

  auto const flipLineIndex = ad.flipX;

  bool const gravityOverride = ad.objectData.disableNoteGravity.value_or(false);

  float lineIndex = ad.objectData.startX ? (*ad.objectData.startX + offset) : noteData->lineIndex;
  float lineLayer = ad.objectData.startY.value_or(noteData->noteLineLayer.value__);
  float const startLineLayer = ad.startNoteLineLayer;

  float offsetStartRow = flipLineIndex.value_or(lineIndex);
  float offsetStartHeight = gravityOverride ? lineLayer : startLineLayer;

  Vector3 const noteOffset = SpawnDataHelper::GetNoteOffset(self, lineIndex, startLineLayer);
  Vector3 const noteOffset2 =
      (noteData->colorType != GlobalNamespace::ColorType::None)
          ? SpawnDataHelper::GetNoteOffset(self, offsetStartRow, offsetStartHeight)
          : (gravityOverride ? SpawnDataHelper::GetNoteOffset(self, lineIndex, lineLayer) : noteOffset);

  float gravity = SpawnDataHelper::GetGravityBase(lineLayer, gravityOverride ? lineLayer : startLineLayer);
  auto result = NoteSpawnData(noteOffset2, noteOffset2, noteOffset, gravity);

  return result;
}

void InstallBeatmapObjectSpawnMovementDataHooks() {
  INSTALL_HOOK(NELogger::Logger, GetObstacleSpawnData);
  INSTALL_HOOK(NELogger::Logger, GetJumpingNoteSpawnData);
  INSTALL_HOOK(NELogger::Logger, GetSliderSpawnData)
}

NEInstallHooks(InstallBeatmapObjectSpawnMovementDataHooks);
