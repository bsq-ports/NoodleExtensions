#include "beatsaber-hook/shared/safeptr.hpp"
#pragma once

#include "Animation/PlayerTrack.h"
#include "custom-types/shared/macros.hpp"
#include "GlobalNamespace/IVariableMovementDataProvider.hpp"
#include "GlobalNamespace/VariableMovementDataProvider.hpp"
#include "GlobalNamespace/BeatmapObjectData.hpp"
#include "System/Object.hpp"

DECLARE_CLASS_CODEGEN_INTERFACES(NoodleExtensions, NoodleMovementDataProvider, System::Object, GlobalNamespace::IVariableMovementDataProvider*) {
  DECLARE_CTOR(ctor);

  DECLARE_INSTANCE_METHOD(NoodleMovementDataProvider*, InitObject,
                          GlobalNamespace::BeatmapObjectData * beatmapObjectData);

  DECLARE_OVERRIDE_METHOD_MATCH(bool, get_wasUpdatedThisFrame, &GlobalNamespace::IVariableMovementDataProvider::get_wasUpdatedThisFrame);
  DECLARE_OVERRIDE_METHOD_MATCH(float, get_jumpDistance, &GlobalNamespace::IVariableMovementDataProvider::get_jumpDistance);
  DECLARE_OVERRIDE_METHOD_MATCH(float, get_jumpDuration, &GlobalNamespace::IVariableMovementDataProvider::get_jumpDuration);
  DECLARE_OVERRIDE_METHOD_MATCH(float, get_halfJumpDuration, &GlobalNamespace::IVariableMovementDataProvider::get_halfJumpDuration);
  DECLARE_OVERRIDE_METHOD_MATCH(float, get_moveDuration, &GlobalNamespace::IVariableMovementDataProvider::get_moveDuration);
  DECLARE_OVERRIDE_METHOD_MATCH(float, get_spawnAheadTime, &GlobalNamespace::IVariableMovementDataProvider::get_spawnAheadTime);
  DECLARE_OVERRIDE_METHOD_MATCH(float, get_waitingDuration, &GlobalNamespace::IVariableMovementDataProvider::get_waitingDuration);
  DECLARE_OVERRIDE_METHOD_MATCH(float, get_noteJumpSpeed, &GlobalNamespace::IVariableMovementDataProvider::get_noteJumpSpeed);
  DECLARE_OVERRIDE_METHOD_MATCH(NEVector::Vector3, get_moveStartPosition, &GlobalNamespace::IVariableMovementDataProvider::get_moveStartPosition);
  DECLARE_OVERRIDE_METHOD_MATCH(NEVector::Vector3, get_moveEndPosition, &GlobalNamespace::IVariableMovementDataProvider::get_moveEndPosition);
  DECLARE_OVERRIDE_METHOD_MATCH(NEVector::Vector3, get_jumpEndPosition, &GlobalNamespace::IVariableMovementDataProvider::get_jumpEndPosition);

  DECLARE_OVERRIDE_METHOD_MATCH(
    void, Init, 
    &GlobalNamespace::IVariableMovementDataProvider::Init, float startHalfJumpDurationInBeats,
    float maxHalfJumpDistance,
    float noteJumpMovementSpeed,
    float minRelativeNoteJumpSpeed,
    float bpm,
    GlobalNamespace::BeatmapObjectSpawnMovementData::NoteJumpValueType noteJumpValueType,
    float noteJumpValue,
    NEVector::Vector3 centerPosition,
    NEVector::Vector3 forwardVector);

  DECLARE_OVERRIDE_METHOD_MATCH(
    float, CalculateCurrentNoteJumpGravity, 
    &GlobalNamespace::IVariableMovementDataProvider::CalculateCurrentNoteJumpGravity, float gravityBase);

  DECLARE_OVERRIDE_METHOD_MATCH(
    float, JumpPosYForLineLayerAtDistanceFromPlayerWithoutJumpOffset, 
    &GlobalNamespace::IVariableMovementDataProvider::JumpPosYForLineLayerAtDistanceFromPlayerWithoutJumpOffset, float highestJumpPosY, float distanceFromPlayer);

public:
  DECLARE_SIMPLE_DTOR();
  
  safe_ptr<GlobalNamespace::VariableMovementDataProvider*> original;
  safe_ptr<GlobalNamespace::BeatmapObjectSpawnMovementData*> beatmapObjectSpawnMovementData;
  float noteJumpStartBeatOffset;
  float oneBeatDuration;
  GlobalNamespace::BeatmapObjectSpawnMovementData::NoteJumpValueType noteJumpValueType;

  float NoteJumpGravityForLineLayerWithoutJumpOffset(float highestJumpPosY, float beforeJumpLineLayer);
  
  std::optional<float> jumpDistanceOverride;
  std::optional<float> jumpDurationOverride;
  std::optional<float> halfJumpDurationOverride;
  std::optional<float> spawnAheadTimeOverride;
  std::optional<float> noteJumpSpeedOverride;
  std::optional<NEVector::Vector3> moveStartPositionOverride;
  std::optional<NEVector::Vector3> moveEndPositionOverride;
  std::optional<NEVector::Vector3> jumpEndPositionOverride;
};