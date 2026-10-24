#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/GameplayCoreInstaller.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "GlobalNamespace/BeatmapDifficulty.hpp"
#include "GlobalNamespace/BeatmapDifficultyMethods.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnControllerHelpers.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/IReadonlyBeatmapData.hpp"
#include "GlobalNamespace/BeatmapLevel.hpp"
#include "GlobalNamespace/BeatmapBasicData.hpp"

#include "Zenject/DiContainer.hpp"

#include "NEObjectPool.hpp"
#include "NECaches.h"
#include "NEHooks.h"
#include <memory>

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(InstallBindings, &GameplayCoreInstaller::InstallBindings, void, GameplayCoreInstaller* self) {
  if (!Hooks::isNoodleHookEnabled()) return InstallBindings(self);

  auto* difficultyBeatmap = self->_sceneSetupData->beatmapBasicData;
  GameplayModifiers* gameplayModifiers = self->_sceneSetupData->gameplayModifiers;

  BeatmapObjectSpawnControllerHelpers::GetNoteJumpValues(
      self->_sceneSetupData->playerSpecificSettings, difficultyBeatmap->noteJumpStartBeatOffset,
      by_ref(NECaches::noteJumpValueType), by_ref(NECaches::noteJumpValue));

  NECaches::noteJumpStartBeatOffset = difficultyBeatmap->noteJumpStartBeatOffset +
                                      self->_sceneSetupData->playerSpecificSettings->noteJumpStartBeatOffset;
  NECaches::beatsPerMinute = self->_sceneSetupData->___beatmapLevel->beatsPerMinute;
  NECaches::numberOfLines = self->_sceneSetupData->transformedBeatmapData->numberOfLines;

  InstallBindings(self);

  NECaches::GameplayCoreContainer = self->Container;
  // NECaches::customBeatmapData =
  //     i2c::cast<CustomJSONData::CustomBeatmapData*>(self->_sceneSetupData->get_transformedBeatmapData());

  {
    // https://github.com/Aeroluna/Heck/blob/master/NoodleExtensions/HarmonyPatches/SmallFixes/InitializedSpawnMovementData.cs
    // this is needed to ensure that the spawn movement data is initialized before we use it
    // to create our NoodleMovementDataProvider
    NECaches::JumpOffsetYProvider = self->Container->Resolve<GlobalNamespace::IJumpOffsetYProvider*>();
    NECaches::InitData = self->Container->Resolve<GlobalNamespace::BeatmapObjectSpawnController::InitData*>();

    // force it to start and set the variable
    NECaches::beatmapObjectSpawnController = self->Container->Resolve<GlobalNamespace::BeatmapObjectSpawnController*>();
    NECaches::VariableMovementDataProvider = self->Container->Resolve<GlobalNamespace::VariableMovementDataProvider*>();
    NECaches::beatmapObjectSpawnController->Start();
    // Start is called by Unity later, we disable in a hook to prevent double start

    // Avoid duplicating the callbacks
    // NECaches::beatmapObjectSpawnController->_beatmapCallbacksController->RemoveBeatmapCallback(NECaches::beatmapObjectSpawnController->_noteDataCallbackWrapper);
    // NECaches::beatmapObjectSpawnController->_beatmapCallbacksController->RemoveBeatmapCallback(NECaches::beatmapObjectSpawnController->_obstacleDataCallbackWrapper);
    // NECaches::beatmapObjectSpawnController->_beatmapCallbacksController->RemoveBeatmapCallback(NECaches::beatmapObjectSpawnController->_sliderDataCallbackWrapper);

    NECaches::noodleMovementDataProviderPool =
        std::make_shared<NoodleExtensions::Pool::NoodleMovementDataProviderPool>(75);
  }
}

void InstallGameplayCoreInstallerHooks() {
  INSTALL_HOOK(NELogger::Logger, InstallBindings);
}

NEInstallHooks(InstallGameplayCoreInstallerHooks);
