#include "beatsaber-hook/shared/safeptr.hpp"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/BasicBeatmapObjectManager.hpp"
#include "GlobalNamespace/IRandom.hpp"
#include "GlobalNamespace/DeterminismConfig.hpp"
#include "GlobalNamespace/ObstacleController.hpp"

#include "System/Collections/Generic/List_1.hpp"

#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

// added to by ObstacleController Init
safe_ptr<System::Collections::Generic::List_1<UnityW<ObstacleController>>*>& getActiveObstacles();

MAKE_HOOK_MATCH(BasicBeatmapObjectManager_Init, &BasicBeatmapObjectManager::Init, void, BasicBeatmapObjectManager* self,
                ::GlobalNamespace::BasicBeatmapObjectManager_InitData* initData, ::GlobalNamespace::IRandom* random,
                ::GlobalNamespace::DeterminismConfig* determinismConfig,
                ::GlobalNamespace::VariableMovementDataProvider* variableMovementDataProvider,
                ::GlobalNamespace::GameNoteController_Pool* basicGameNotePool,
                ::GlobalNamespace::GameNoteController_Pool* burstSliderHeadGameNotePool,
                ::GlobalNamespace::BurstSliderGameNoteController_Pool* burstSliderGameNotePool,
                ::GlobalNamespace::BombNoteController_Pool* bombNotePool,
                ::GlobalNamespace::ObstacleController_Pool* obstaclePool,
                ::GlobalNamespace::SliderController_Pool* sliderPools) {
  BasicBeatmapObjectManager_Init(self, initData, random, determinismConfig, variableMovementDataProvider, basicGameNotePool, burstSliderHeadGameNotePool,
                                 burstSliderGameNotePool, bombNotePool, obstaclePool, sliderPools);
  if (!Hooks::isNoodleHookEnabled()) return;

  // This makes sure that the list of obstacles is cleared every time a new BasicBeatmapObjectManager is initialized (on new level load or restart)
  // Without this, dead obstaclecontrollers would pile up in the list and cause nullref exceptions later on
  getActiveObstacles().emplace(ListW<UnityW<ObstacleController>>::New());
}

MAKE_HOOK_MATCH(BasicBeatmapObjectManager_get_activeObstacleControllers,
                &BasicBeatmapObjectManager::get_activeObstacleControllers,
                System::Collections::Generic::List_1<UnityW<GlobalNamespace::ObstacleController>>*,
                BasicBeatmapObjectManager* self) {
  if (!Hooks::isNoodleHookEnabled()) return BasicBeatmapObjectManager_get_activeObstacleControllers(self);

  return getActiveObstacles().ptr();
}

MAKE_HOOK_MATCH(BasicBeatmapObjectManager_DespawnInternal,
                static_cast<void (GlobalNamespace::BasicBeatmapObjectManager::*)(GlobalNamespace::ObstacleController*)>(
                    &GlobalNamespace::BasicBeatmapObjectManager::DespawnInternal),
                void, BasicBeatmapObjectManager* self, ObstacleController* obstacleController) {
  BasicBeatmapObjectManager_DespawnInternal(self, obstacleController);
  if (!Hooks::isNoodleHookEnabled()) return;

  getActiveObstacles()->Remove(obstacleController);
}

void InstallBasicBeatmapObjectManagerHooks() {
  INSTALL_HOOK(NELogger::Logger, BasicBeatmapObjectManager_Init);
  INSTALL_HOOK(NELogger::Logger, BasicBeatmapObjectManager_get_activeObstacleControllers)
  INSTALL_HOOK(NELogger::Logger, BasicBeatmapObjectManager_DespawnInternal)
}

NEInstallHooks(InstallBasicBeatmapObjectManagerHooks);
