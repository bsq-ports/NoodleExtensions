#pragma once

#include <optional>

#include "tracks/shared/Vector.h"

#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/NoteLineLayer.hpp"
#include "GlobalNamespace/StaticBeatmapObjectSpawnMovementData.hpp"
#include "NELogger.h"
#include "SpawnDataHelper.h"

#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "NECaches.h"

namespace GlobalNamespace {
class BeatmapObjectSpawnMovementData;
class BeatmapObjectData;
class BeatmapObjectSpawnMovementData;
struct NoteLineLayer;
} // namespace GlobalNamespace

namespace SpawnDataHelper {

using namespace GlobalNamespace;

static inline float const kHalfJumpDistanceEpsilon = 0.001f;

// CoreMathUtils.CalculateHalfJumpDurationInBeats
constexpr float CalculateHalfJumpDurationInBeats(float startHalfJumpDurationInBeats, float maxHalfJumpDistance,
                                                 float noteJumpMovementSpeed, float oneBeatDuration,
                                                 float noteJumpStartBeatOffset) {
  float num1 = startHalfJumpDurationInBeats;
  float num2 = noteJumpMovementSpeed * oneBeatDuration;
  float num3 = num2 * num1;
  for (maxHalfJumpDistance -= 1.0f / 1000.0f; (double)num3 > (double)maxHalfJumpDistance; num3 = num2 * num1)
    num1 /= 2.0f;
  float jumpDurationInBeats = num1 + noteJumpStartBeatOffset;
  if ((double)jumpDurationInBeats < 0.25) jumpDurationInBeats = 0.25f;
  return jumpDurationInBeats;
}

constexpr float OneBeatDuration(float bpm) {
  if (bpm <= 0.0f) {
    return 0.0f;
  }
  return 60.0f / bpm;
}

constexpr float GetJumpDuration(std::optional<float> inputNjs, std::optional<float> inputOffset) {

  if (!inputNjs && !inputOffset) {
    return NECaches::VariableMovementDataProvider->get_jumpDuration();
  }

  float njs = inputNjs.value_or(NECaches::VariableMovementDataProvider->get_noteJumpSpeed());
  float spawnOffset = inputOffset.value_or(NECaches::InitData->noteJumpValue);
  auto valueType = NECaches::InitData->noteJumpValueType;
  if (valueType == GlobalNamespace::BeatmapObjectSpawnMovementData::NoteJumpValueType::JumpDuration &&
      !inputNjs.has_value() && !inputOffset.has_value()) {
    return spawnOffset * 2.0f;
  }

  auto movementData = NECaches::beatmapObjectSpawnController->beatmapObjectSpawnMovementData;

  float oneBeatDuration = OneBeatDuration(NECaches::InitData->beatsPerMinute);
  float halfJumpDurationInBeats =
      CalculateHalfJumpDurationInBeats(movementData->_startHalfJumpDurationInBeats, movementData->_maxHalfJumpDistance,
                                       njs, oneBeatDuration, spawnOffset);

  return oneBeatDuration * halfJumpDurationInBeats * 2.0f;
}

inline float GetSpawnAheadTime(std::optional<float> inputNjs, std::optional<float> inputOffset) {
  float moveDuration = GlobalNamespace::VariableMovementDataProvider::kMoveDuration;
  return moveDuration + (GetJumpDuration(inputNjs, inputOffset) * 0.5f);
}

float HighestJumpPosYForLineLayer(float lineLayer);

float GetGravityBase(float noteLineLayer, float beforeJumpLineLayer);

float LineYPosForLineLayer(float height);

constexpr NEVector::Vector2 Get2DNoteOffset(float lineIndex, int noteLinesCount, float lineLayer) {
  float distance = -(float(noteLinesCount) - 1.0f) * 0.5f;
  return { (distance + lineIndex) * GlobalNamespace::StaticBeatmapObjectSpawnMovementData::kNoteLinesDistance,
           LineYPosForLineLayer(lineLayer) };
}

constexpr NEVector::Vector3 GetNoteOffset(GlobalNamespace::BeatmapObjectSpawnMovementData* spawnMovementData,
                                          float lineIndex, float lineLayer) {
  NEVector::Vector2 coords = Get2DNoteOffset(lineIndex, spawnMovementData->noteLinesCount, lineLayer);
  return NEVector::Vector3(0, coords.y, 0) + (NEVector::Vector3(spawnMovementData->_rightVec) * coords.x);
}

constexpr UnityEngine::Vector3 GetObstacleOffset(GlobalNamespace::BeatmapObjectSpawnMovementData* spawnMovementData,
                                                 float lineIndex, float lineLayer) {
  UnityEngine::Vector3 result = GetNoteOffset(spawnMovementData, lineIndex, lineLayer);
  result.y += GlobalNamespace::StaticBeatmapObjectSpawnMovementData::kObstacleVerticalOffset;
  return result;
}

} // namespace SpawnDataHelper