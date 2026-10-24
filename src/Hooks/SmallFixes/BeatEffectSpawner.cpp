#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/BeatEffectSpawner.hpp"
#include "GlobalNamespace/BeatEffect.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/ColorType.hpp"
#include "GlobalNamespace/ColorManager.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/MemoryPoolContainer_1.hpp"
#include "GlobalNamespace/ILazyCopyHashSet_1.hpp"
#include "GlobalNamespace/BloomFogSO.hpp"
#include "GlobalNamespace/NoteFloorMovement.hpp"
#include "GlobalNamespace/NoteMovement.hpp"
#include "GlobalNamespace/LazyCopyHashSet_1.hpp"

#include "System/Action_1.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"

#include "tracks/shared/TimeSourceHelper.h"

#include "tracks/shared/Vector.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

#include "sombrero/shared/FastColor.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;

constexpr static NEVector::Vector3 GetNoteControllerPosition(Transform* transform) {
  return transform->get_position();
}

constexpr static NEVector::Quaternion GetNoteControllerRotation(Transform* transform) {
  return transform->get_rotation();
}

MAKE_HOOK_MATCH(BeatEffectSpawner_HandleNoteDidStartJump, &BeatEffectSpawner::HandleNoteDidStartJump, void,
                BeatEffectSpawner* self, NoteController* noteController) {
  if (!Hooks::isNoodleHookEnabled()) return BeatEffectSpawner_HandleNoteDidStartJump(self, noteController);

  if (self->_initData->hideNoteSpawnEffect) {
    return;
  }
  if (noteController->hidden) {
    return;
  }
  if (noteController->_noteData->_time_k__BackingField + 0.1f < self->_audioTimeSyncController->_songTime) {
    return;
  }
  ColorType colorType = noteController->_noteData->colorType;
  Sombrero::FastColor color =
      (colorType != ColorType::None) ? self->_colorManager->ColorForType(colorType) : self->_bombColorEffect;
  auto beatEffect = self->_beatEffectPoolContainer->Spawn();
  beatEffect->didFinishEvent->Add(reinterpret_cast<IBeatEffectDidFinishEvent*>(self));

  // TRANSPILE HERE
  auto transform = noteController->get_transform();
  NEVector::Vector3 jumpStartPos(GetNoteControllerPosition(transform));
  NEVector::Quaternion worldRotation(GetNoteControllerRotation(transform));

  beatEffect->get_transform()->SetPositionAndRotation(jumpStartPos - NEVector::Vector3(0.0f, 0.15f, 0.0f),
                                                      NEVector::Quaternion::identity());
  // end transpile

  beatEffect->Init(color * self->_bloomFog->noteSpawnIntensity, self->_effectDuration, worldRotation);
}

void InstallBeatEffectSpawnerHooks() {
  INSTALL_HOOK_ORIG(NELogger::Logger, BeatEffectSpawner_HandleNoteDidStartJump);
}

NEInstallHooks(InstallBeatEffectSpawnerHooks);