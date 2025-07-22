#include "Animation/NoodleMovementDataProvider.hpp"
#include "NECaches.h"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/ObstacleData.hpp"
#include "GlobalNamespace/ObstacleSpawnData.hpp"

#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/BurstSliderGameNoteController.hpp"
#include "GlobalNamespace/NoteFloorMovement.hpp"
#include "GlobalNamespace/NoteMovement.hpp"
#include "GlobalNamespace/NoteWaiting.hpp"
#include "GlobalNamespace/NoteJump.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/NoteSpawnData.hpp"

#include "GlobalNamespace/SliderController.hpp"
#include "GlobalNamespace/SliderMovement.hpp"
#include "GlobalNamespace/SliderData.hpp"
#include "GlobalNamespace/SliderSpawnData.hpp"

#include "GlobalNamespace/BasicBeatmapObjectManager.hpp"

#include "NEHooks.h"
#include "NEObjectPool.hpp"
#include <memory>

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(ReplaceObstacleMovement, &ObstacleController::Init, void,
                ObstacleController* self, ObstacleData* obstacleData,
                ByRef<ObstacleSpawnData> obstacleSpawnData) {
  if (!Hooks::isNoodleHookEnabled())
    return ReplaceObstacleMovement(self, obstacleData, obstacleSpawnData);

  // auto provider = NECaches::noodleMovementDataProviderPool->get(obstacleData);
  auto provider = NoodleExtensions::NoodleMovementDataProvider::New_ctor();
  provider->original = reinterpret_cast<IVariableMovementDataProvider*>(NECaches::VariableMovementDataProvider.ptr());
  provider->InitObject(obstacleData);
  auto IProvider = reinterpret_cast<IVariableMovementDataProvider*>(provider);

  self->_variableMovementDataProvider = IProvider;

  ReplaceObstacleMovement(self, obstacleData, obstacleSpawnData);
}

MAKE_HOOK_MATCH(ReplaceNoteMovement, &NoteController::Init, void,
                NoteController* self, NoteData* noteData,
                ByRef<NoteSpawnData> noteSpawnData, float endRotation,
                float noteUniformScale, bool rotateTowardsPlayer, bool useRandomRotation) {
  if (!Hooks::isNoodleHookEnabled())
    return ReplaceNoteMovement(self, noteData, noteSpawnData, endRotation, noteUniformScale, rotateTowardsPlayer, useRandomRotation);

  // auto provider = NECaches::noodleMovementDataProviderPool->get(noteData);
  auto provider = NoodleExtensions::NoodleMovementDataProvider::New_ctor();
  provider->original = reinterpret_cast<IVariableMovementDataProvider*>(NECaches::VariableMovementDataProvider.ptr());
  provider->InitObject(noteData);
  auto IProvider = reinterpret_cast<IVariableMovementDataProvider*>(provider);

  auto noteMovement = self->_noteMovement;
  auto noteFloorMovement = noteMovement->_floorMovement;
  auto noteJump = noteMovement->_jump;
  auto noteWaiting = noteMovement->_waiting;

  noteMovement->_variableMovementDataProvider = IProvider;
  noteFloorMovement->_variableMovementDataProvider = IProvider;
  noteJump->_variableMovementDataProvider = IProvider;
  noteWaiting->_variableMovementDataProvider = IProvider;

  if (il2cpp_utils::AssignableFrom<BurstSliderGameNoteController*>(self->klass))
  {
    auto burstSliderGameNoteController = reinterpret_cast<BurstSliderGameNoteController*>(self);
    burstSliderGameNoteController->_variableMovementDataProvider = reinterpret_cast<IVariableMovementDataProvider*>(provider);
  }

  ReplaceNoteMovement(self, noteData, noteSpawnData, endRotation, noteUniformScale, rotateTowardsPlayer, useRandomRotation);
}

MAKE_HOOK_MATCH(ReplaceSliderMovement, &SliderController::Init, void,
                SliderController* self, SliderController::LengthType lengthType,
                SliderData* sliderData, ByRef<SliderSpawnData> sliderSpawnData,
                float noteUniformScale, float randomValue) {
  if (!Hooks::isNoodleHookEnabled())
    return ReplaceSliderMovement(self, lengthType, sliderData, sliderSpawnData, noteUniformScale, randomValue);

  // auto provider = NECaches::noodleMovementDataProviderPool->get(sliderData);
  auto provider = NoodleExtensions::NoodleMovementDataProvider::New_ctor();
  provider->original = reinterpret_cast<IVariableMovementDataProvider*>(NECaches::VariableMovementDataProvider.ptr());
  provider->InitObject(sliderData);
  auto IProvider = reinterpret_cast<IVariableMovementDataProvider*>(provider);
  self->_variableMovementDataProvider = IProvider;
  self->_sliderMovement->_variableMovementDataProvider = IProvider;

  ReplaceSliderMovement(self, lengthType, sliderData, sliderSpawnData, noteUniformScale, randomValue);
}

MAKE_HOOK_MATCH(DespawnObstacleMovement,
                static_cast<void (BasicBeatmapObjectManager::*)(ObstacleController*)>(&BasicBeatmapObjectManager::DespawnInternal),
                void, BasicBeatmapObjectManager* self, ObstacleController* obstacleController) {
  if (!Hooks::isNoodleHookEnabled())
    return DespawnObstacleMovement(self, obstacleController);

  if (self->_variableMovementDataProvider->klass == classof(NoodleExtensions::NoodleMovementDataProvider*)) {
    NECaches::noodleMovementDataProviderPool->put(reinterpret_cast<NoodleExtensions::NoodleMovementDataProvider*>(self->_variableMovementDataProvider));
  }

  DespawnObstacleMovement(self, obstacleController);
}

MAKE_HOOK_MATCH(DespawnNoteMovement,
                static_cast<void (BasicBeatmapObjectManager::*)(NoteController*)>(&BasicBeatmapObjectManager::DespawnInternal),
                void, BasicBeatmapObjectManager* self, NoteController* noteController) {
  if (!Hooks::isNoodleHookEnabled())
    return DespawnNoteMovement(self, noteController);

  if (self->_variableMovementDataProvider->klass == classof(NoodleExtensions::NoodleMovementDataProvider*)) {
    NECaches::noodleMovementDataProviderPool->put(reinterpret_cast<NoodleExtensions::NoodleMovementDataProvider*>(self->_variableMovementDataProvider));
  }

  DespawnNoteMovement(self, noteController);
}



MAKE_HOOK_MATCH(DespawnSliderMovement,
                static_cast<void (BasicBeatmapObjectManager::*)(SliderController*)>(&BasicBeatmapObjectManager::DespawnInternal),
                void, BasicBeatmapObjectManager* self, SliderController* sliderController) {
  if (!Hooks::isNoodleHookEnabled())
    return DespawnSliderMovement(self, sliderController);

  if (self->_variableMovementDataProvider->klass == classof(NoodleExtensions::NoodleMovementDataProvider*)) {
    NECaches::noodleMovementDataProviderPool->put(reinterpret_cast<NoodleExtensions::NoodleMovementDataProvider*>(self->_variableMovementDataProvider));
  }

  DespawnSliderMovement(self, sliderController);
}

void InstallReplaceMovementDataProviderHooks() {
  INSTALL_HOOK(NELogger::Logger, ReplaceObstacleMovement);
  INSTALL_HOOK(NELogger::Logger, ReplaceNoteMovement);
  INSTALL_HOOK(NELogger::Logger, ReplaceSliderMovement);
  INSTALL_HOOK(NELogger::Logger, DespawnObstacleMovement);
  INSTALL_HOOK(NELogger::Logger, DespawnNoteMovement);
  INSTALL_HOOK(NELogger::Logger, DespawnSliderMovement);
}

NEInstallHooks(InstallReplaceMovementDataProviderHooks);
