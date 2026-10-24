#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/MultiplayerLevelScenesTransitionSetupData.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"

#include "Animation/ParentObject.h"
#include "NEHooks.h"
#include "NECaches.h"
#include "SceneTransitionHelper.hpp"

using namespace GlobalNamespace;
using namespace TrackParenting;
using namespace CustomJSONData;
using namespace NoodleExtensions;

MAKE_HOOK_MATCH(MultiplayerLevelScenesTransitionSetupData_Init,
                &MultiplayerLevelScenesTransitionSetupData::InitAndSetupScenes, void,
                MultiplayerLevelScenesTransitionSetupData* self) {

  auto customBeatmapLevel = i2c::try_cast<SongCore::SongLoader::CustomBeatmapLevel*>(self->get_beatmapLevel());
  if (!customBeatmapLevel) {
    MultiplayerLevelScenesTransitionSetupData_Init(self);
    return;
  }

  // TODO: Fix environment override
  SceneTransitionHelper::Patch(customBeatmapLevel, self->beatmapKey,
                               self->_environmentInfo.ptr(),
                               self->gameplayCoreSceneSetupData->playerSpecificSettings);

  MultiplayerLevelScenesTransitionSetupData_Init(self);
}

void InstallMultiplayerLevelScenesTransitionSetupDataHooks() {
  INSTALL_HOOK(NELogger::Logger, MultiplayerLevelScenesTransitionSetupData_Init);
}
NEInstallHooks(InstallMultiplayerLevelScenesTransitionSetupDataHooks);
