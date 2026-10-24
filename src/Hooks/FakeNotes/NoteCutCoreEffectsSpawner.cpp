#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/NoteCutCoreEffectsSpawner.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

std::optional<NoteCutCoreEffectsSpawner*> noteCutCoreEffectsSpawner;

MAKE_HOOK_MATCH(NoteCutCoreEffectsSpawner_Start, &NoteCutCoreEffectsSpawner::Start, void,
                NoteCutCoreEffectsSpawner* self) {
  if (!Hooks::isNoodleHookEnabled()) return NoteCutCoreEffectsSpawner_Start(self);

  NoteCutCoreEffectsSpawner_Start(self);
  if (self) noteCutCoreEffectsSpawner.emplace(self);
}

MAKE_HOOK_MATCH(NoteCutCoreEffectsSpawner_OnDestroy, &NoteCutCoreEffectsSpawner::OnDestroy, void,
                NoteCutCoreEffectsSpawner* self) {
  NoteCutCoreEffectsSpawner_OnDestroy(self);
  if (!Hooks::isNoodleHookEnabled()) return;
  noteCutCoreEffectsSpawner = std::nullopt;
}

void InstallNoteCutCoreEffectsSpawnerHooks() {
  INSTALL_HOOK(NELogger::Logger, NoteCutCoreEffectsSpawner_Start);
  INSTALL_HOOK(NELogger::Logger, NoteCutCoreEffectsSpawner_OnDestroy);
}
NEInstallHooks(InstallNoteCutCoreEffectsSpawnerHooks);