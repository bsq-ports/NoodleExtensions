#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"

#include "NEHooks.h"


extern GlobalNamespace::BeatmapObjectSpawnController* beatmapObjectSpawnController;

// https://github.com/Aeroluna/Heck/blob/master/NoodleExtensions/HarmonyPatches/SmallFixes/InitializedSpawnMovementData.cs#L61
// Disable, we already started it in GameplayCoreInstaller
// to ensure that the movement data is initialized before we use it
// we need to avoid a double start though
MAKE_HOOK_MATCH(BeatmapObjectSpawnController_Start, &GlobalNamespace::BeatmapObjectSpawnController::Start, void, GlobalNamespace::BeatmapObjectSpawnController* self) {
  beatmapObjectSpawnController = self;

  if (!Hooks::isNoodleHookEnabled()) {
    BeatmapObjectSpawnController_Start(self);
    return;
  }

  // avoid double start
  if (self->_isInitialized) return;

  BeatmapObjectSpawnController_Start(self);
}


void InstallBeatmapObjectSpawnControllerHooks() {
  INSTALL_HOOK(NELogger::Logger, BeatmapObjectSpawnController_Start);
}

NEInstallHooks(InstallBeatmapObjectSpawnControllerHooks);