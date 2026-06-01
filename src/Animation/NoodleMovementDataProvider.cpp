#include "Animation/NoodleMovementDataProvider.hpp"
#include "AssociatedData.h"

#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/CoreMathUtils.hpp"
#include "NECaches.h"
#include "SpawnDataHelper.h"

DEFINE_TYPE(NoodleExtensions, NoodleMovementDataProvider);

NoodleExtensions::NoodleMovementDataProvider*
NoodleExtensions::NoodleMovementDataProvider::InitObject(GlobalNamespace::BeatmapObjectData* beatmapObjectData) {
  jumpDistanceOverride = std::nullopt;
  jumpDurationOverride = std::nullopt;
  halfJumpDurationOverride = std::nullopt;
  spawnAheadTimeOverride = std::nullopt;
  noteJumpSpeedOverride = std::nullopt;
  moveStartPositionOverride = std::nullopt;
  moveEndPositionOverride = std::nullopt;
  jumpEndPositionOverride = std::nullopt;

  static auto* customObstacleDataClass = classof(CustomJSONData::CustomObstacleData*);
  static auto* customNoteDataClass = classof(CustomJSONData::CustomNoteData*);
  static auto* customSliderDataClass = classof(CustomJSONData::CustomSliderData*);

  if (!beatmapObjectData) return this;

  CustomJSONData::JSONWrapper* customDataWrapper = nullptr;
  if (beatmapObjectData->klass == customObstacleDataClass) {
    customDataWrapper = ((CustomJSONData::CustomObstacleData*)beatmapObjectData)->customData;
  } else if (beatmapObjectData->klass == customNoteDataClass) {
    customDataWrapper = ((CustomJSONData::CustomNoteData*)beatmapObjectData)->customData;
  } else if (beatmapObjectData->klass == customSliderDataClass) {
    customDataWrapper = ((CustomJSONData::CustomSliderData*)beatmapObjectData)->customData;
  }
  BeatmapObjectAssociatedData& ad = getAD(customDataWrapper);

  if (!ad.parsed) return this;

  if (!ad.objectData.noteJumpMovementSpeed.has_value() && !ad.objectData.noteJumpStartBeatOffset.has_value())
    return this;

  noteJumpSpeedOverride = ad.objectData.noteJumpMovementSpeed;

  float njs = get_noteJumpSpeed();
  float spawnOffset = ad.objectData.noteJumpStartBeatOffset.value_or(noteJumpStartBeatOffset);
  auto valueType = noteJumpSpeedOverride.has_value() || ad.objectData.noteJumpStartBeatOffset ?
      GlobalNamespace::BeatmapObjectSpawnMovementData::NoteJumpValueType::BeatOffset : noteJumpValueType;
  switch (valueType) {
  case GlobalNamespace::BeatmapObjectSpawnMovementData::NoteJumpValueType::JumpDuration: {
    jumpDurationOverride = spawnOffset * 2.0f;
    halfJumpDurationOverride = spawnOffset;
    break;
  }

  case GlobalNamespace::BeatmapObjectSpawnMovementData::NoteJumpValueType::BeatOffset: {
    float halfJumpDurationInBeats = GlobalNamespace::CoreMathUtils::CalculateHalfJumpDurationInBeats(
        beatmapObjectSpawnMovementData->_startHalfJumpDurationInBeats,
        beatmapObjectSpawnMovementData->_maxHalfJumpDistance, njs, oneBeatDuration, spawnOffset);

    float halfJump = oneBeatDuration * halfJumpDurationInBeats;
    halfJumpDurationOverride = halfJump;
    jumpDurationOverride = halfJump * 2.0f;
    break;
  }

  default:
    NELogger::Logger.error("Unknown NoteJumpValueType {}", (int)noteJumpValueType);
    break;
  }

  spawnAheadTimeOverride = GlobalNamespace::VariableMovementDataProvider::kMoveDuration + get_halfJumpDuration();

  float jumpDist = njs * get_jumpDuration();
  jumpDistanceOverride = jumpDist;
  float halfJumpDistance = jumpDist * 0.5f;
  NEVector::Vector3 center = beatmapObjectSpawnMovementData->centerPos;
  NEVector::Vector3 forward = NEVector::Vector3::forward();

  moveStartPositionOverride =
      center + (forward * (GlobalNamespace::VariableMovementDataProvider::kMoveDistance + halfJumpDistance));
  moveEndPositionOverride = center + (forward * halfJumpDistance);
  jumpEndPositionOverride = center - (forward * halfJumpDistance);
  return this;
}

bool NoodleExtensions::NoodleMovementDataProvider::get_wasUpdatedThisFrame() {
  return original->get_wasUpdatedThisFrame();
}

float NoodleExtensions::NoodleMovementDataProvider::get_jumpDistance() {
  if (!jumpDistanceOverride.has_value()) {
    return original->get_jumpDistance();
  }
  return jumpDistanceOverride.value();
}

float NoodleExtensions::NoodleMovementDataProvider::get_jumpDuration() {
  if (!jumpDurationOverride.has_value()) {
    return original->get_jumpDuration();
  }
  return jumpDurationOverride.value();
}

float NoodleExtensions::NoodleMovementDataProvider::get_halfJumpDuration() {
  if (!halfJumpDurationOverride.has_value()) {
    return original->get_halfJumpDuration();
  }
  return halfJumpDurationOverride.value();
}

float NoodleExtensions::NoodleMovementDataProvider::get_moveDuration() {
  // TODO: Revise if we want to allow overriding move duration
  // return original->get_moveDuration();
  
  
  // method resolves to this constant in vanilla
  return GlobalNamespace::VariableMovementDataProvider::kMoveDuration;
}

float NoodleExtensions::NoodleMovementDataProvider::get_spawnAheadTime() {
  if (!spawnAheadTimeOverride.has_value()) {
    return original->get_spawnAheadTime();
  }
  return spawnAheadTimeOverride.value();
}

float NoodleExtensions::NoodleMovementDataProvider::get_waitingDuration() {
  return 0.0f;
}

float NoodleExtensions::NoodleMovementDataProvider::get_noteJumpSpeed() {
  if (!noteJumpSpeedOverride.has_value()) {
    return original->get_noteJumpSpeed();
  }
  return noteJumpSpeedOverride.value();
}

NEVector::Vector3 NoodleExtensions::NoodleMovementDataProvider::get_moveStartPosition() {
  if (!moveStartPositionOverride.has_value()) {
    return original->get_moveStartPosition();
  }
  return moveStartPositionOverride.value();
}

NEVector::Vector3 NoodleExtensions::NoodleMovementDataProvider::get_moveEndPosition() {
  if (!moveEndPositionOverride.has_value()) {
    return original->get_moveEndPosition();
  }
  return moveEndPositionOverride.value();
}

NEVector::Vector3 NoodleExtensions::NoodleMovementDataProvider::get_jumpEndPosition() {
  if (!jumpEndPositionOverride.has_value()) {
    return original->get_jumpEndPosition();
  }
  return jumpEndPositionOverride.value();
}

void NoodleExtensions::NoodleMovementDataProvider::Init(
    float startHalfJumpDurationInBeats, float maxHalfJumpDistance, float noteJumpMovementSpeed,
    float minRelativeNoteJumpSpeed, float bpm,
    GlobalNamespace::BeatmapObjectSpawnMovementData::NoteJumpValueType noteJumpValueType, float noteJumpValue,
    NEVector::Vector3 centerPosition, NEVector::Vector3 forwardVector) {}

float NoodleExtensions::NoodleMovementDataProvider::CalculateCurrentNoteJumpGravity(float gravityBase) {
  float halfJumpDur = get_halfJumpDuration();
  return 2.0f * gravityBase / (halfJumpDur * halfJumpDur);
}

float NoodleExtensions::NoodleMovementDataProvider::NoteJumpGravityForLineLayerWithoutJumpOffset(
    float highestJumpPosY, float beforeJumpLineLayer) {
  float num = get_jumpDistance() / get_noteJumpSpeed() * 0.5f;
  return 2.0f * (highestJumpPosY - SpawnDataHelper::LineYPosForLineLayer(beforeJumpLineLayer)) / (num * num);
}

float NoodleExtensions::NoodleMovementDataProvider::JumpPosYForLineLayerAtDistanceFromPlayerWithoutJumpOffset(
    float highestJumpPosY, float distanceFromPlayer) {
  float num = ((get_jumpDistance() * 0.5f) - distanceFromPlayer) / get_noteJumpSpeed();
  float num2 = NoteJumpGravityForLineLayerWithoutJumpOffset(highestJumpPosY, 0);
  float num3 = num2 * get_jumpDuration() * 0.5f;
  return SpawnDataHelper::LineYPosForLineLayer(0) + (num3 * num) - (num2 * num * num * 0.5f);
}

void NoodleExtensions::NoodleMovementDataProvider::ctor() {
  beatmapObjectSpawnMovementData = NECaches::beatmapObjectSpawnController->_beatmapObjectSpawnMovementData;
  original.emplace(NECaches::VariableMovementDataProvider.ptr());

  // initialize base data
  // TODO: Check if we need more initialization
  noteJumpValueType = NECaches::InitData->noteJumpValueType;
  noteJumpStartBeatOffset = NECaches::InitData->noteJumpValue;
  oneBeatDuration = SpawnDataHelper::OneBeatDuration(NECaches::InitData->beatsPerMinute);
}