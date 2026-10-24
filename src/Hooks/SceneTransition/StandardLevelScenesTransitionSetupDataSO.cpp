#include "GlobalNamespace/GameMode.hpp"
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

MAKE_HOOK_MATCH(StandardLevelScenesTransitionSetupData_Init, &StandardLevelScenesTransitionSetupData::Init, void,
                StandardLevelScenesTransitionSetupData* self, ::GlobalNamespace::GameMode gameMode,
                ::by_ref<const ::GlobalNamespace::BeatmapKey> beatmapKey, ::GlobalNamespace::BeatmapLevel* beatmapLevel,
                ::GlobalNamespace::OverrideEnvironmentSettings* overrideEnvironmentSettings,
                ::GlobalNamespace::ColorScheme* playerOverrideColorScheme, bool playerOverrideLightshowColors,
                ::GlobalNamespace::GameplayModifiers* gameplayModifiers,
                ::GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings,
                ::GlobalNamespace::PracticeSettings* practiceSettings,
                ::GlobalNamespace::EnvironmentsListModel* environmentsListModel,
                ::GlobalNamespace::AudioClipAsyncLoader* audioClipAsyncLoader,
                ::GlobalNamespace::SettingsManager* settingsManager,
                ::GlobalNamespace::GameplayAdditionalInformation* gameplayAdditionalInformation,
                ::GlobalNamespace::BeatmapDataLoader* beatmapDataLoader,
                ::GlobalNamespace::BeatmapLevelsEntitlementModel* beatmapLevelsEntitlementModel,
                ::GlobalNamespace::BeatmapLevelsModel* beatmapLevelsModel,
                ::GlobalNamespace::IBeatmapLevelData* beatmapLevelData) {
  auto customLevel = i2c::try_cast<SongCore::SongLoader::CustomBeatmapLevel*>(beatmapLevel);
  auto environment = StandardLevelScenesTransitionSetupData::GetEnvironmentInfo(
                         *beatmapKey, beatmapLevel, overrideEnvironmentSettings, environmentsListModel)
                         .Item2.ptr();
  SceneTransitionHelper::Patch(customLevel, *beatmapKey, environment, playerSpecificSettings);
  StandardLevelScenesTransitionSetupData_Init(
      self, gameMode, beatmapKey, beatmapLevel, overrideEnvironmentSettings, playerOverrideColorScheme,
      playerOverrideLightshowColors, gameplayModifiers, playerSpecificSettings, practiceSettings, environmentsListModel,
      audioClipAsyncLoader, settingsManager, gameplayAdditionalInformation, beatmapDataLoader,
      beatmapLevelsEntitlementModel, beatmapLevelsModel, beatmapLevelData);
}

void InstallStandardLevelScenesTransitionSetupDataHook() {
  INSTALL_HOOK(NELogger::Logger, StandardLevelScenesTransitionSetupData_Init);
}
NEInstallHooks(InstallStandardLevelScenesTransitionSetupDataHook);
