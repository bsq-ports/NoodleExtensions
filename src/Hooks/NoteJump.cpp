#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/Easing.hpp"
#include "GlobalNamespace/NoteJump.hpp"
#include "GlobalNamespace/PlayerTransforms.hpp"
#include "System/Action.hpp"
#include "System/Action_1.hpp"
#include "UnityEngine/Transform.hpp"

#include "Animation/AnimationHelper.h"
#include "NEHooks.h"
#include "tracks/shared/TimeSourceHelper.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

extern BeatmapObjectAssociatedData* noteUpdateAD;
extern TracksAD::TracksVector noteTracks;

float noteTimeAdjust(float original, float jumpDuration);

constexpr static float InOutQuad(float t) {
  if (t >= 0.5f) {
    return -1.0f + (4.0f - 2.0f * t) * t;
  }
  return 2.0f * t * t;
}

constexpr float NoteMissedTimeAdjust(float beatTime, float jumpDuration, float num) {
  return num + (beatTime - (jumpDuration * 0.5f));
}

void NoteJump_ManualUpdateNoteLookTranspile(NoteJump* self, Transform* selfTransform, float const normalTime) {
  if (noteUpdateAD && noteUpdateAD->objectData.disableNoteLook) {
    self->_rotatedObject->set_localRotation(self->_endRotation);
    return;
  }
  Transform* baseTransform = selfTransform; // lazy
  NEVector::Vector3 baseTransformPosition(baseTransform->get_position());
  NEVector::Quaternion baseTransformRotation(baseTransform->get_rotation());

  NEVector::Quaternion a =
      normalTime < 0.125
          ? NEVector::Quaternion::Slerp(baseTransformRotation * NEVector::Quaternion(self->_startRotation),
                                        baseTransformRotation * NEVector::Quaternion(self->_middleRotation),
                                        std::sin(normalTime * M_PI * 4))
          : NEVector::Quaternion::Slerp(baseTransformRotation * NEVector::Quaternion(self->_middleRotation),
                                        baseTransformRotation * NEVector::Quaternion(self->_endRotation),
                                        std::sin((normalTime - 0.125) * M_PI * 2));

  // Aero doesn't know what's happening anymore
  NEVector::Quaternion worldRot = self->_inverseWorldRotation;
  auto baseTransformParent = baseTransform->get_parent();
  if (baseTransformParent) {
    // Handle parenting
    worldRot = worldRot * (NEVector::Quaternion)NEVector::Quaternion::Inverse(baseTransformParent->get_rotation());
  }

  if (self->_rotateTowardsPlayer) {
    Transform* parentTransform = self->_playerTransforms->_originParentTransform;
    NEVector::Vector3 vector = parentTransform->TransformPoint(self->_playerTransforms->headPseudoLocalPos);
    NEVector::Quaternion inverse = NEVector::Quaternion::Inverse(worldRot);
    NEVector::Vector3 upVector = inverse * NEVector::Vector3::up();
    NEVector::Vector3 position = baseTransformPosition;

    float baseUpMagnitude = NEVector::Vector3::Dot(worldRot * position, NEVector::Vector3::up());
    float headUpMagnitude = 
      NEVector::Vector3::Dot(worldRot * vector, NEVector::Vector3::up());
    float mult = std::lerp(headUpMagnitude, baseUpMagnitude, 0.8f) - headUpMagnitude;
    vector = vector + (upVector * mult);

    // more wtf
    NEVector::Vector3 normalized = NEVector::Quaternion(baseTransformRotation) *
                                  (worldRot * Sombrero::vector3subtract(baseTransformPosition, vector).get_normalized());

    NEVector::Quaternion b = NEVector::Quaternion::LookRotation(normalized, self->_rotatedObject->get_up());

    self->_rotatedObject->set_rotation(NEVector::Quaternion::Lerp(a, b, normalTime * 2));
  } else {
    self->_rotatedObject->set_rotation(a);
  }
}

MAKE_HOOK_MATCH(NoteJump_ManualUpdate, &NoteJump::ManualUpdate, Vector3, NoteJump* self) {
  if (!Hooks::isNoodleHookEnabled()) return NoteJump_ManualUpdate(self);

  if (!self->_missedMarkReported) {
    self->_halfJumpDuration = self->_variableMovementDataProvider->halfJumpDuration;
    self->_jumpDuration = self->_variableMovementDataProvider->jumpDuration;
    self->_gravity = self->_variableMovementDataProvider->CalculateCurrentNoteJumpGravity(self->_gravityBase);
    self->_startPos = Sombrero::FastVector3(self->_variableMovementDataProvider->moveEndPosition) +
                      Sombrero::FastVector3(self->_startOffset);
    self->_endPos = Sombrero::FastVector3(self->_variableMovementDataProvider->jumpEndPosition) +
                    Sombrero::FastVector3(self->_endOffset);
    self->_missedTime = self->_noteTime + 0.15f;
  }
  auto selfTransform = self->get_transform();
  float songTime = TimeSourceHelper::getSongTime(self->_audioTimeSyncController);
  float elapsedTime = songTime - (self->_noteTime - self->_halfJumpDuration);
  // transpile here
  if (noteUpdateAD) {
    elapsedTime = noteTimeAdjust(elapsedTime, self->_jumpDuration);
  }
  //
  float normalTime = elapsedTime / self->_jumpDuration;
  if (self->_startPos.x == self->_endPos.x) {
    self->_localPosition.x = self->_startPos.x;
  } else if (normalTime < 0.25f) {
    self->_localPosition.x = self->_startPos.x + (self->_endPos.x - self->_startPos.x) * InOutQuad(normalTime * 4.f);
  } else {
    self->_localPosition.x = self->_endPos.x;
  }
  self->_localPosition.z = std::lerp(self->_startPos.z, self->_endPos.z, normalTime);
  float num3 = self->_gravity * self->_halfJumpDuration;
  self->_localPosition.y = self->_startPos.y + num3 * elapsedTime - self->_gravity * elapsedTime * elapsedTime * 0.5f;
  if (self->_yAvoidance != 0.f && normalTime < 0.25f) {
    float num4 = 0.5f - std::cos(normalTime * 8.f * 3.1415927f) * 0.5f;
    self->_localPosition.y = self->_localPosition.y + num4 * self->_yAvoidance;
  }

  // transpile here
  // https://github.com/Aeroluna/NoodleExtensions/blob/2147129bfd480a718d99d8c2ca8c45df0502c5d1/NoodleExtensions/HarmonyPatches/NoteJump.cs#L115-L126
  bool definitePosition = false;

  if (noteUpdateAD) {
    std::optional<NEVector::Vector3> position =
        AnimationHelper::GetDefinitePositionOffset(noteUpdateAD->animationData, noteTracks, normalTime);
    if (position.has_value()) {
      self->_localPosition = *position + noteUpdateAD->noteOffset;
      definitePosition = true;
    }
  }
  //

  if (normalTime < 0.5) {
    NoteJump_ManualUpdateNoteLookTranspile(self, selfTransform, normalTime);
  }

  if (!self->_jumpStartedReported) {
    self->_jumpStartedReported = true;
    auto action = self->noteJumpDidStartEvent;
    if (action != nullptr) {
      action->Invoke();
    }
  }

  if (normalTime >= 0.5f && !self->_halfJumpMarkReported) {
    self->_halfJumpMarkReported = true;
    auto action2 = self->noteJumpDidPassHalfEvent;
    if (action2 != nullptr) {
      action2->Invoke();
    }
  }
  if (normalTime >= 0.75f && !self->_threeQuartersMarkReported) {
    self->_threeQuartersMarkReported = true;
    auto action3 = self->noteJumpDidPassThreeQuartersEvent;
    if (action3 != nullptr) {
      action3->Invoke(self);
    }
  }

  {
    float num = elapsedTime;
    float missedCompare = NoteMissedTimeAdjust(self->_noteTime, self->_jumpDuration, num);

    if (missedCompare >= self->_missedTime && !self->_missedMarkReported) {
      self->_missedMarkReported = true;
      auto action4 = self->noteJumpDidPassMissedMarkerEvent;
      if (action4 != nullptr) {
        action4->Invoke();
      }
    }
  }
  // transpile here
  if (self->_threeQuartersMarkReported && !definitePosition) {
    //
    float num5 = (normalTime - 0.75f) / 0.25f;
    num5 = num5 * num5 * num5;
    self->_localPosition.z = self->_localPosition.z - std::lerp(0.0f, self->_endDistanceOffset, num5);
  }
  if (normalTime >= 1.0f) {
    if (!self->_missedMarkReported) {
      self->_missedMarkReported = true;
      auto action5 = self->noteJumpDidPassMissedMarkerEvent;
      if (action5 != nullptr) {
        action5->Invoke();
      }
    }
    auto action6 = self->noteJumpDidFinishEvent;
    if (action6 != nullptr) {
      action6->Invoke();
    }
  }
  auto vector2 = Sombrero::FastQuaternion(self->_worldRotation) * Sombrero::FastVector3(self->_localPosition);
  self->transform->localPosition = vector2;
  auto action7 = self->noteJumpDidUpdateProgressEvent;
  if (action7 != nullptr) {
    action7->Invoke(normalTime);
  }
  return vector2;
}

void InstallNoteJumpHooks() {
  INSTALL_HOOK_ORIG(NELogger::Logger, NoteJump_ManualUpdate);
}

NEInstallHooks(InstallNoteJumpHooks);
