#include "GlobalNamespace/MissionLevelScenesTransitionSetupData.hpp"
#include "GlobalNamespace/StandardLevelScenesTransitionSetupData.hpp"
#include "GlobalNamespace/EnvironmentsListModel.hpp"
#include "GlobalNamespace/BeatmapKey.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "System/ValueTuple_3.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "beatsaber-hook/shared/hooking.hpp"
#include "NEHooks.h"
#include "SceneTransitionHelper.hpp"

using namespace GlobalNamespace;
using namespace NoodleExtensions;

MAKE_HOOK_MATCH(MissionLevelScenesTransitionSetupData_Init, &MissionLevelScenesTransitionSetupData::Init, void,
                MissionLevelScenesTransitionSetupData* self, ::StringW missionId,
                ::by_ref<const ::GlobalNamespace::BeatmapKey> beatmapKey, ::GlobalNamespace::BeatmapLevel* beatmapLevel,
                ::GlobalNamespace::OverrideEnvironmentSettings* overrideEnvironmentSettings,
                ::ArrayW<::GlobalNamespace::MissionObjective*> missionObjectives,
                ::GlobalNamespace::ColorScheme* playerOverrideColorScheme, bool playerOverrideLightshowColors,
                ::GlobalNamespace::GameplayModifiers* gameplayModifiers,
                ::GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings,
                ::GlobalNamespace::EnvironmentsListModel* environmentsListModel,
                ::GlobalNamespace::AudioClipAsyncLoader* audioClipAsyncLoader,
                ::GlobalNamespace::SettingsManager* settingsManager,
                ::GlobalNamespace::GameplayAdditionalInformation* gameplayAdditionalInformation,
                ::GlobalNamespace::BeatmapDataLoader* beatmapDataLoader, ::StringW backButtonText,
                ::GlobalNamespace::BeatmapLevelsModel* beatmapLevelsModel,
                ::GlobalNamespace::IBeatmapLevelData* beatmapLevelData) {
  auto customLevel = i2c::try_cast<SongCore::SongLoader::CustomBeatmapLevel*>(beatmapLevel);
  auto environment = StandardLevelScenesTransitionSetupData::GetEnvironmentInfo(
                         *beatmapKey, beatmapLevel, overrideEnvironmentSettings, environmentsListModel)
                         .Item2.ptr();
  SceneTransitionHelper::Patch(customLevel, *beatmapKey, environment, playerSpecificSettings);
  MissionLevelScenesTransitionSetupData_Init(
      self, missionId, beatmapKey, beatmapLevel, overrideEnvironmentSettings, missionObjectives,
      playerOverrideColorScheme, playerOverrideLightshowColors, gameplayModifiers, playerSpecificSettings,
      environmentsListModel, audioClipAsyncLoader, settingsManager, gameplayAdditionalInformation, beatmapDataLoader,
      backButtonText, beatmapLevelsModel, beatmapLevelData);
}

void InstallMissionLevelScenesTransitionSetupDataHook() {
  INSTALL_HOOK(NELogger::Logger, MissionLevelScenesTransitionSetupData_Init);
}
NEInstallHooks(InstallMissionLevelScenesTransitionSetupDataHook);
