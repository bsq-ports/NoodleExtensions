#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/BombNoteController.hpp"
#include "GlobalNamespace/CuttableBySaber.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(BombNoteController_Init, &BombNoteController::Init, void, BombNoteController* self, NoteData* noteData, by_ref<GlobalNamespace::NoteSpawnData const> noteSpawnData) {
  BombNoteController_Init(self, noteData, noteSpawnData);
  if (!Hooks::isNoodleHookEnabled()) return;

  if (!FakeNoteHelper::GetCuttable(noteData)) {
    self->_cuttableBySaber->set_canBeCut(false);
  }
}

void InstallBombNoteControllerHooks() {
  INSTALL_HOOK(NELogger::Logger, BombNoteController_Init);
}
NEInstallHooks(InstallBombNoteControllerHooks);