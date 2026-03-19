#include "Animation/PlayerTrack.h"
#include "Animation/AnimationHelper.h"
#include "AssociatedData.h"
#include "GlobalNamespace/PlayerTransforms.hpp"
#include "GlobalNamespace/PlayerVRControllersManager.hpp"
#include "GlobalNamespace/VRCenterAdjust.hpp"
#include "NELogger.h"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Resources.hpp"
#include "GlobalNamespace/PauseController.hpp"
#include "GlobalNamespace/PauseMenuManager.hpp"
#include "GlobalNamespace/MultiplayerLocalActivePlayerInGameMenuController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/StaticBeatmapObjectSpawnMovementData.hpp"
#include "System/Action.hpp"
#include "NECaches.h"
#include "Zenject/DiContainer.hpp"

#include "UnityEngine/Transform.hpp"
#include "custom-types/shared/delegate.hpp"

using namespace TrackParenting;
using namespace UnityEngine;
using namespace GlobalNamespace;
using namespace System;
using namespace Animation;

// Events.cpp
extern BeatmapObjectSpawnController* spawnController;

std::unordered_map<PlayerTrackObject, SafePtrUnity<PlayerTrack>> PlayerTrack::playerTracks;

static Action* didPauseEventAction;
static Action* didResumeEventAction;

DEFINE_TYPE(TrackParenting, PlayerTrack);

void PlayerTrack::ctor() {
  startPos = NEVector::Vector3::zero();
  startRot = NEVector::Quaternion::identity();
  startLocalRot = NEVector::Quaternion::identity();
  startScale = NEVector::Vector3::one();
  if (!pauseController) pauseController.emplace(nullptr);
}

PlayerTrack* PlayerTrack::Create(PlayerTrackObject object) {
  auto playerTransforms = Resources::FindObjectsOfTypeAll<PlayerTransforms*>()->FirstOrDefault();
  if (!playerTransforms) {
    CJDLogger::Logger.fmtLog<Paper::LogLevel::ERR>("PlayerTransforms not found");
    return nullptr;
  }

  UnityEngine::Transform* target;
  auto playerVRControllersManager = Resources::FindObjectsOfTypeAll<PlayerVRControllersManager*>()->FirstOrDefault();
  switch (object) {
  case PlayerTrackObject::Head:
    target = playerTransforms->_headTransform;
    break;
  case PlayerTrackObject::LeftHand:
    target = playerVRControllersManager->_leftHandVRController->transform;
    break;
  case PlayerTrackObject::RightHand:
    target = playerVRControllersManager->_rightHandVRController->transform;
    break;
  case PlayerTrackObject::Root:
  default:
    target = playerTransforms->_originTransform->parent;
    break;
  }

  auto noodleObject = GameObject::New_ctor("NoodlePlayerTrack " + std::to_string((int)object));
  auto playerTrack = noodleObject->AddComponent<PlayerTrack*>();
  playerTrack->trackObject = object;
  auto origin = playerTrack->origin = noodleObject->transform;

  if (object == PlayerTrackObject::Root) {
    // Transform hierarchy manipulation: PLAYER PARENT -> NOODLE -> PLAYER
    origin->SetParent(target->parent, false);
    target->SetParent(origin, true);
  } else {
    // Can be null for inactive players in multiplayer
    if (playerTransforms->_originParentTransform == nullptr) {
        playerTransforms->_originParentTransform = playerTransforms->_originTransform->parent;
    }

    origin->SetParent(playerTransforms->_originParentTransform->transform, false);

    GameObject* roomOffset = GameObject::New_ctor("NoodleRoomOffset");
    roomOffset->SetActive(false);
    Transform* roomOffsetTransform = roomOffset->transform;
    roomOffset->AddComponent<VRCenterAdjust*>();
    roomOffsetTransform->SetParent(origin);
    roomOffset->SetActive(true);
    target->SetParent(roomOffsetTransform, true);
  }

  playerTrack->startLocalRot = playerTrack->origin->get_localRotation();
  playerTrack->startPos = playerTrack->origin->get_localPosition();

  playerTrack->pauseController = Object::FindObjectOfType<PauseController*>();

  if (playerTrack->pauseController) {
    std::function<void()> pause = [playerTrack]() mutable { playerTrack->OnDidPauseEvent(); };
    std::function<void()> resume = [playerTrack]() mutable { playerTrack->OnDidResumeEvent(); };
    didPauseEventAction = custom_types::MakeDelegate<Action*>(pause);
    playerTrack->pauseController->add_didPauseEvent(didPauseEventAction);
    didResumeEventAction = custom_types::MakeDelegate<Action*>(resume);
    playerTrack->pauseController->add_didResumeEvent(didResumeEventAction);
  }

  if (object == PlayerTrackObject::Root) {
    auto* pauseMenuManager = playerTrack->pauseController
                                 ? playerTrack->pauseController->_pauseMenuManager.ptr()
                                 : NECaches::GameplayCoreContainer->TryResolve<PauseMenuManager*>();
    auto multiPauseMenuManager =
        NECaches::GameplayCoreContainer->TryResolve<MultiplayerLocalActivePlayerInGameMenuController*>();
    if (pauseMenuManager) {
      CJDLogger::Logger.fmtLog<Paper::LogLevel::INF>("Setting transform to pause menu");
      pauseMenuManager->get_transform()->SetParent(playerTrack->origin, false);
    }

    if (multiPauseMenuManager) {
      CJDLogger::Logger.fmtLog<Paper::LogLevel::INF>("Setting multi transform to pause menu");
      multiPauseMenuManager->get_transform()->SetParent(playerTrack->origin, false);
    }
  }

  return playerTrack;
}

void PlayerTrack::AssignTrack(TrackW track, PlayerTrackObject object) {
  auto& playerTrack = PlayerTrack::playerTracks[object];

  if (playerTrack && playerTrack->track) {
    playerTrack->track.UnregisterGameObject(playerTrack->get_gameObject());
  }

  // Init
  if (!playerTrack) {
    playerTrack = Create(object);
  }

  if (!playerTrack) {
    NELogger::Logger.error("Failed to initialize player track {} {}", track ? track.GetName() : "", (int)object);
    return;
  }

  GameObject* noodleObject = playerTrack->origin->gameObject;

  // this is only used in v2
  playerTrack->set_enabled(track.v2);
  playerTrack->track = track;

  if (playerTrack && playerTrack->track) {
    playerTrack->track.RegisterGameObject(playerTrack->get_gameObject());
  }

  if (track.v2) {
    playerTrack->Update();
  } else {
    
      TrackW tracksArray[] = { track };
      playerTrack->trackController = Tracks::GameObjectTrackController::HandleTrackData(noodleObject, std::span<const TrackW>(tracksArray, 1), 0.6, track.v2, true)
            .value_or(nullptr);
    playerTrack->trackController->UpdateData(true);
  }
}

void PlayerTrack::OnDidPauseEvent() {
  NELogger::Logger.debug("PlayerTrack::OnDidPauseEvent");
  this->set_enabled(false);

  if (this->trackObject != PlayerTrackObject::Root) {
    origin->localPosition = startPos;
    origin->localRotation = startLocalRot;
  }

  if (trackController) {
    trackController->set_enabled(false);
  }
}

void PlayerTrack::OnDidResumeEvent() {
  NELogger::Logger.debug("PlayerTrack::OnDidResumeEvent");
  this->set_enabled(track.v2);

  if (trackController) {
    trackController->set_enabled(true);
  }
}

void PlayerTrack::OnDestroy() {
  NELogger::Logger.debug("PlayerTrack::OnDestroy");
  if (pauseController) {
    // NELogger::Logger.debug("Removing action didPauseEvent %p", didPauseEventAction);
    // pauseController->remove_didPauseEvent(didPauseEventAction);
  }
  trackController = nullptr;
  track = TrackW();
  if (auto it = PlayerTrack::playerTracks.find(this->trackObject); it != PlayerTrack::playerTracks.end()) {
    PlayerTrack::playerTracks.erase(it);
  }
}

// V2
void PlayerTrack::UpdateDataOld() {
  float noteLinesDistance = GlobalNamespace::StaticBeatmapObjectSpawnMovementData::kNoteLinesDistance;

  // DO NOT USE LAST CHECKED TIME HERE BECAUSE IT CAUSES BUGS
  // IT WAS NOT DESIGNED FOR USAGE WITH V2 TRACKS MATH
  std::optional<NEVector::Quaternion> rotation = track.GetPropertyNamed(PropertyNames::Rotation).GetQuat();
  std::optional<NEVector::Vector3> position =
      track.GetPropertyNamed(PropertyNames::Position).GetVec3();
  std::optional<NEVector::Quaternion> localRotation =
      track.GetPropertyNamed(PropertyNames::LocalRotation).GetQuat();

  if (NECaches::LeftHandedMode) {
    rotation = Animation::MirrorQuaternionNullable(rotation);
    localRotation = Animation::MirrorQuaternionNullable(localRotation);
    position = Animation::MirrorVectorNullable(position);
  }

  NEVector::Quaternion worldRotationQuaternion = startRot;
  NEVector::Vector3 positionVector = startPos;
  if (rotation.has_value() || position.has_value()) {
    NEVector::Quaternion rotationOffset = rotation.value_or(NEVector::Quaternion::identity());
    worldRotationQuaternion = worldRotationQuaternion * rotationOffset;
    NEVector::Vector3 positionOffset = position.value_or(NEVector::Vector3::zero());
    positionVector = worldRotationQuaternion * ((positionOffset * noteLinesDistance) + startPos);
  }

  worldRotationQuaternion = worldRotationQuaternion * startLocalRot;
  if (localRotation.has_value()) {
    worldRotationQuaternion = worldRotationQuaternion * *localRotation;
  }

  origin->set_localRotation(worldRotationQuaternion);
  origin->set_localPosition(positionVector);
}

void PlayerTrack::Update() {
  if (track && track.v2) {
    return UpdateDataOld();
  }
}