#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/GameNoteController.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(NoteDidStartJump, &GameNoteController::NoteDidStartJump, void, GameNoteController* self) {
  if (!Hooks::isNoodleHookEnabled()) return NoteDidStartJump(self);

  if (FakeNoteHelper::GetCuttable(self->_noteData)) {
    NoteDidStartJump(self);
  }
}

void InstallGameNoteControllerHooks() {
  INSTALL_HOOK(NELogger::Logger, NoteDidStartJump);
}
NEInstallHooks(InstallGameNoteControllerHooks);