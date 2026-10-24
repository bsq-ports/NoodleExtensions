#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/BadNoteCutEffectSpawner.hpp"
#include "GlobalNamespace/BombCutSoundEffectManager.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(BadNoteCutEffectSpawner_HandleNoteWasCut, &BadNoteCutEffectSpawner::HandleNoteWasCut, void,
                BadNoteCutEffectSpawner* self, GlobalNamespace::NoteController* noteController,
                by_ref<GlobalNamespace::NoteCutInfo const> noteCutInfo) {
  if (!Hooks::isNoodleHookEnabled()) return BadNoteCutEffectSpawner_HandleNoteWasCut(self, noteController, noteCutInfo);

  if (!FakeNoteHelper::GetFakeNote(noteController->_noteData)) {
    BadNoteCutEffectSpawner_HandleNoteWasCut(self, noteController, noteCutInfo);
  }
}

MAKE_HOOK_MATCH(BombCutSoundEffectManager_HandleNoteWasCut, &BombCutSoundEffectManager::HandleNoteWasCut, void,
                BombCutSoundEffectManager* self, GlobalNamespace::NoteController* noteController,
                by_ref<GlobalNamespace::NoteCutInfo const> noteCutInfo) {
  if (!Hooks::isNoodleHookEnabled()) return BombCutSoundEffectManager_HandleNoteWasCut(self, noteController, noteCutInfo);

  if (!FakeNoteHelper::GetFakeNote(noteController->_noteData)) {
    BombCutSoundEffectManager_HandleNoteWasCut(self, noteController, noteCutInfo);
  }
}

void InstallBadNoteCutEffectSpawnerHooks() {
  INSTALL_HOOK(NELogger::Logger, BadNoteCutEffectSpawner_HandleNoteWasCut);
  INSTALL_HOOK(NELogger::Logger, BombCutSoundEffectManager_HandleNoteWasCut);
}
NEInstallHooks(InstallBadNoteCutEffectSpawnerHooks);