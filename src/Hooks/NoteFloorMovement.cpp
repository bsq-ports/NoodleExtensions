#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/NoteFloorMovement.hpp"
#include "UnityEngine/Transform.hpp"
#include "System/Action.hpp"

#include "Animation/AnimationHelper.h"
#include "tracks/shared/TimeSourceHelper.h"
#include "NEHooks.h"
#include "NELogger.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace System;

extern BeatmapObjectAssociatedData* noteUpdateAD;
extern TracksAD::TracksVector noteTracks;

static NEVector::Vector3 DefinitePositionTranspile(NEVector::Vector3 original, NoteFloorMovement* noteFloorMovement) {
  if (!noteUpdateAD) {
    return original;
  }

  std::optional<NEVector::Vector3> position =
      AnimationHelper::GetDefinitePositionOffset(noteUpdateAD->animationData, noteTracks, 0);
  if (!position.has_value()) {
    return original;
  }

  NEVector::Vector3 noteOffset = noteUpdateAD->noteOffset;
  NEVector::Vector3 endPos = noteFloorMovement->endPos;
  return original + (position.value() + noteOffset - endPos);
}

MAKE_HOOK_MATCH(NoteFloorMovement_ManualUpdate, &NoteFloorMovement::ManualUpdate, Vector3, NoteFloorMovement* self) {
  if (!Hooks::isNoodleHookEnabled()) return NoteFloorMovement_ManualUpdate(self);
  float num = TimeSourceHelper::getSongTime(self->_audioTimeSyncController) -
              (self->_beatTime - self->_variableMovementDataProvider->moveDuration -
               self->_variableMovementDataProvider->halfJumpDuration);

  self->_localPosition = NEVector::Vector3::Lerp(
      NEVector::Vector3(self->_variableMovementDataProvider->moveStartPosition) + self->_moveStartOffset,
      NEVector::Vector3(self->_variableMovementDataProvider->moveEndPosition) + self->_moveEndOffset,
      num / self->_variableMovementDataProvider->moveDuration);
  self->_localPosition = DefinitePositionTranspile(self->_localPosition, self);

  NEVector::Vector3 vector = NEVector::Quaternion(self->_worldRotation) * NEVector::Vector3(self->_localPosition);
  self->get_transform()->set_localPosition(vector);
  return vector;
}

MAKE_HOOK_MATCH(NoteFloorMovement_SetToStart, &NoteFloorMovement::SetToStart, UnityEngine::Vector3,
                NoteFloorMovement* self) {
  if (!Hooks::isNoodleHookEnabled()) return NoteFloorMovement_SetToStart(self);

  auto ret = NoteFloorMovement_SetToStart(self);

  static auto Quaternion_Euler =
      il2cpp_utils::il2cpp_type_check::FPtrWrapper<static_cast<UnityEngine::Quaternion (*)(UnityEngine::Vector3)>(
          &UnityEngine::Quaternion::Euler)>::get();

  if (noteUpdateAD && noteUpdateAD->objectData.disableNoteLook) {
    self->_rotatedObject->set_localRotation(Quaternion_Euler({ 0, 0, noteUpdateAD->endRotation }));
  }

  return ret;
}

void InstallNoteFloorMovementHooks() {
  INSTALL_HOOK_ORIG(NELogger::Logger, NoteFloorMovement_ManualUpdate);
  INSTALL_HOOK(NELogger::Logger, NoteFloorMovement_SetToStart);
}

NEInstallHooks(InstallNoteFloorMovementHooks);
