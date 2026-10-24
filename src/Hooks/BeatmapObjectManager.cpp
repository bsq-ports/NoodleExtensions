#include "tracks/shared/Animation/Track.h"
#include "tracks/shared/AssociatedData.h"
#include "Animation/ParentObject.h"

#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/SliderController.hpp"

#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(BeatmapObjectManager_Note_Despawn,
                static_cast<void (GlobalNamespace::BeatmapObjectManager::*)(GlobalNamespace::NoteController*)>(
                    &GlobalNamespace::BeatmapObjectManager::Despawn),
                void, BeatmapObjectManager* self, NoteController* noteController) {
  if (!Hooks::isNoodleHookEnabled()) return BeatmapObjectManager_Note_Despawn(self, noteController);

  auto customNoteData = i2c::try_cast<CustomJSONData::CustomNoteData*>(noteController->_noteData);

  if (customNoteData && customNoteData->customData && customNoteData->customData->value) {
    auto const& tracks = TracksAD::getAD(customNoteData->customData).tracks;
    if (!tracks.empty()) {
      auto go = noteController->get_gameObject();
      for (auto const& track : tracks) {
        track.UnregisterGameObject(go);
      }
    }
  }
  BeatmapObjectManager_Note_Despawn(self, noteController);
}

MAKE_HOOK_MATCH(BeatmapObjectManager_Obstacle_Despawn,
                static_cast<void (GlobalNamespace::BeatmapObjectManager::*)(GlobalNamespace::ObstacleController*)>(
                    &GlobalNamespace::BeatmapObjectManager::Despawn),
                void, BeatmapObjectManager* self, ObstacleController* obstacleController) {
  if (!Hooks::isNoodleHookEnabled()) return BeatmapObjectManager_Obstacle_Despawn(self, obstacleController);

  auto customObstacleData =
      i2c::try_cast<CustomJSONData::CustomObstacleData*>(obstacleController->_obstacleData);

  if (customObstacleData && customObstacleData->customData && customObstacleData->customData->value) {
    auto const& tracks = TracksAD::getAD(customObstacleData->customData).tracks;
    if (!tracks.empty()) {
      auto go = obstacleController->get_gameObject();
      for (auto const& track : tracks) {
        track.UnregisterGameObject(go);
      }
    }
  }
  BeatmapObjectManager_Obstacle_Despawn(self, obstacleController);
}

MAKE_HOOK_MATCH(BeatmapObjectManager_Slider_Despawn,
                static_cast<void (GlobalNamespace::BeatmapObjectManager::*)(GlobalNamespace::SliderController*)>(
                    &GlobalNamespace::BeatmapObjectManager::Despawn),
                void, BeatmapObjectManager* self, SliderController* sliderController) {
  if (!Hooks::isNoodleHookEnabled()) return BeatmapObjectManager_Slider_Despawn(self, sliderController);

  auto customSliderData =
      i2c::try_cast<CustomJSONData::CustomSliderData*>(sliderController->_sliderData);

  if (customSliderData && customSliderData->customData && customSliderData->customData->value) {
    auto const& tracks = TracksAD::getAD(customSliderData->customData).tracks;
    if (!tracks.empty()) {
      auto go = sliderController->get_gameObject();
      for (auto const& track : tracks) {
        track.UnregisterGameObject(go);
      }
    }
  }
  BeatmapObjectManager_Slider_Despawn(self, sliderController);
}

MAKE_HOOK_MATCH(BeatmapObjectManager_Note_Spawn, &GlobalNamespace::BeatmapObjectManager::AddSpawnedNoteController, void,
                BeatmapObjectManager* self, ::GlobalNamespace::NoteController* noteController,
                ::GlobalNamespace::NoteSpawnData noteSpawnData) {
  if (!Hooks::isNoodleHookEnabled()) return BeatmapObjectManager_Note_Spawn(self, noteController, noteSpawnData);
  BeatmapObjectManager_Note_Spawn(self, noteController, noteSpawnData);

  auto customNoteData = i2c::try_cast<CustomJSONData::CustomNoteData*>(noteController->_noteData);
  if (customNoteData && customNoteData->customData && customNoteData->customData->value) {
    auto const& tracks = TracksAD::getAD(customNoteData->customData).tracks;
    if (!tracks.empty()) {
      auto go = noteController->get_gameObject();
      for (auto const& track : tracks) {
        track.RegisterGameObject(go);
      }
    }
  }
}

MAKE_HOOK_MATCH(BeatmapObjectManager_Obstacle_Spawn,
                &GlobalNamespace::BeatmapObjectManager::AddSpawnedObstacleController, void,
                BeatmapObjectManager* self, ::GlobalNamespace::ObstacleController* obstacleController,
                ::GlobalNamespace::ObstacleSpawnData obstacleSpawnData) {
  if (!Hooks::isNoodleHookEnabled())
    return BeatmapObjectManager_Obstacle_Spawn(self, obstacleController, obstacleSpawnData);
  BeatmapObjectManager_Obstacle_Spawn(self, obstacleController, obstacleSpawnData);

  auto customObstacleData =
      i2c::try_cast<CustomJSONData::CustomObstacleData*>(obstacleController->_obstacleData);
  if (customObstacleData && customObstacleData->customData && customObstacleData->customData->value) {
    auto const& tracks = TracksAD::getAD(customObstacleData->customData).tracks;
    if (!tracks.empty()) {
      auto go = obstacleController->get_gameObject();
      for (auto const& track : tracks) {
        track.RegisterGameObject(go);
      }
    }
  }
}

MAKE_HOOK_MATCH(BeatmapObjectManager_Slider_Spawn,
                &GlobalNamespace::BeatmapObjectManager::AddSpawnedSliderController, void,
                BeatmapObjectManager* self, ::GlobalNamespace::SliderController* sliderController,
                ::GlobalNamespace::SliderSpawnData sliderSpawnData) {
  if (!Hooks::isNoodleHookEnabled()) return BeatmapObjectManager_Slider_Spawn(self, sliderController, sliderSpawnData);
  BeatmapObjectManager_Slider_Spawn(self, sliderController, sliderSpawnData);

  auto customSliderData =
      i2c::try_cast<CustomJSONData::CustomSliderData*>(sliderController->_sliderData);
  if (customSliderData && customSliderData->customData && customSliderData->customData->value) {
    auto const& tracks = TracksAD::getAD(customSliderData->customData).tracks;
    if (!tracks.empty()) {
      auto go = sliderController->get_gameObject();
      for (auto const& track : tracks) {
        track.RegisterGameObject(go);
      }
    }
  }
}

void InstallBeatmapObjectManagerHooksTrack() {
  INSTALL_HOOK(NELogger::Logger, BeatmapObjectManager_Note_Despawn);
  INSTALL_HOOK(NELogger::Logger, BeatmapObjectManager_Obstacle_Despawn);
  INSTALL_HOOK(NELogger::Logger, BeatmapObjectManager_Slider_Despawn);

  INSTALL_HOOK(NELogger::Logger, BeatmapObjectManager_Note_Spawn);
  INSTALL_HOOK(NELogger::Logger, BeatmapObjectManager_Obstacle_Spawn);
  INSTALL_HOOK(NELogger::Logger, BeatmapObjectManager_Slider_Spawn);
}

NEInstallHooks(InstallBeatmapObjectManagerHooksTrack);