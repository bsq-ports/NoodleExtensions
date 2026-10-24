#include <functional>
#include "Animation/ParentObject.h"

#include "AssociatedData.h"
#include "NELogger.h"
#include "beatsaber-hook/shared/types.hpp"

#include <utility>
#include <array>
#include "Animation/AnimationHelper.h"
#include "UnityEngine/GameObject.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/StaticBeatmapObjectSpawnMovementData.hpp"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"

#include "NECaches.h"

using namespace TrackParenting;
using namespace UnityEngine;
using namespace GlobalNamespace;
using namespace Animation;

template <> struct std::hash<std::pair<TrackW, ParentObject*>> {
  std::size_t operator()(std::pair<TrackW, ParentObject*> const& k) const {
    return std::hash<int>()(k.first.track._0) ^ std::hash<ParentObject*>()(k.second);
  }
};
// emulate delegates here by associating each Track + Parent to a callback
// and since callbacks are one time use, we cannot recycle themm when removing them
// those will cause a crash since they're freed when RemoveGameObjectCallback is called
static std::unordered_map<std::pair<TrackW, ParentObject*>, TrackW::CWrappedCallback> gameObjectModificationCallbacks;

static void RemoveCallback(TrackW track, ParentObject* object) {
  auto pair = std::pair(track, object);
  track.RemoveGameObjectCallback(gameObjectModificationCallbacks[pair]);
  gameObjectModificationCallbacks.erase(pair);
}

template <typename F> static void AddCallback(TrackW track, ParentObject* object, F&& f) {
  auto pair = std::pair(track, object);
  auto callback = track.RegisterGameObjectCallback(std::forward<F>(f));
  gameObjectModificationCallbacks[pair] = callback;
}

// Events.cpp
extern BeatmapObjectSpawnController* spawnController;

DEFINE_TYPE(TrackParenting, ParentObject);

void ParentObject::OnEnable() {
  OnTransformParentChanged();
}

void ParentObject::Update() {
  UpdateData(false);
}

void ParentObject::OnTransformParentChanged() {
  UpdateData(true);
}

void ParentObject::UpdateData(bool force) {
  if (!track) return;

  if (track.v2) {
    UpdateDataOld(force);
    return;
  }
}

void ParentObject::UpdateDataOld(bool forced) {
  if (forced) {
    lastCheckedTime = TimeUnit();
  }
  float noteLinesDistance = GlobalNamespace::StaticBeatmapObjectSpawnMovementData::kNoteLinesDistance;

  // DO NOT USE LAST CHECKED TIME HERE BECAUSE IT CAUSES BUGS
  // IT WAS NOT DESIGNED FOR USAGE WITH V2 TRACKS MATH
  auto const rotation = track.GetPropertyNamed(PropertyNames::Rotation).GetQuat();
  auto const localRotation = track.GetPropertyNamed(PropertyNames::LocalRotation).GetQuat();
  auto const position = track.GetPropertyNamed(PropertyNames::Position).GetVec3();
  auto const scale = track.GetPropertyNamed(PropertyNames::Scale).GetVec3();

  NEVector::Quaternion worldRotationQuaternion = startRot;
  NEVector::Vector3 positionVector = worldRotationQuaternion * (startPos * noteLinesDistance);
  if (rotation.has_value() || position.has_value()) {
    NEVector::Quaternion rotationOffset = rotation.value_or(NEVector::Quaternion::identity());
    worldRotationQuaternion = worldRotationQuaternion * rotationOffset;
    NEVector::Vector3 positionOffset = position.value_or(NEVector::Vector3::zero());
    positionVector = worldRotationQuaternion * ((positionOffset + startPos) * noteLinesDistance);
  }

  worldRotationQuaternion = worldRotationQuaternion * startLocalRot;

  if (localRotation.has_value()) {
    worldRotationQuaternion = worldRotationQuaternion * *localRotation;
  }

  Vector3 scaleVector = startScale;
  if (scale.has_value()) {
    scaleVector = startScale * scale.value();
  }

  origin->set_localRotation(worldRotationQuaternion);
  origin->set_localPosition(positionVector);
  origin->set_localScale(scaleVector);

  lastCheckedTime = getCurrentTime();
}

static void logTransform(Transform* transform, int hierarchy = 0) {
  if (hierarchy != 0) {
    std::string tab = std::string(hierarchy * 4, ' ');
    NELogger::Logger.debug("{}{}Child: {} {}", hierarchy, tab.c_str(), transform->get_gameObject()->get_name(),
                           transform->GetChildCount());
  } else {
    NELogger::Logger.debug("Self: {} {}", transform->get_gameObject()->get_name(), transform->GetChildCount());
  }
  for (int i = 0; i < transform->GetChildCount(); i++) {
    auto childTransform = transform->GetChild(i);
    logTransform(childTransform, hierarchy + 1);
  }
}

void ParentObject::AssignTrack(ParentTrackEventData const& parentTrackEventData) {
  static ConstString ParentName("ParentObject");

  GameObject* parentGameObject = GameObject::New_ctor(ParentName);
  ParentObject* instance = parentGameObject->AddComponent<ParentObject*>();

  instance->origin = parentGameObject->get_transform();
  instance->track = parentTrackEventData.parentTrack;
  instance->worldPositionStays = parentTrackEventData.worldPositionStays;

  Transform* transform = instance->origin;
  NELogger::Logger.debug("Assigning ParentObject {} to [{}] v2 {}", parentTrackEventData.parentTrack.GetName(),
                         fmt::join(parentTrackEventData.childrenTracks |
                                       std::views::transform([](auto& t) { return std::string(t.GetName()); }),
                                   ", "),
                         parentTrackEventData.parentTrack.v2);

  parentTrackEventData.parentTrack.RegisterGameObject(parentGameObject);

  for (auto track : parentTrackEventData.childrenTracks) {
    if (track == parentTrackEventData.parentTrack) {
      NELogger::Logger.error("How could a track contain itself?");
      continue;
    }

    for (auto parentObject : ParentController::parentObjects) {
      // this code is ugly but whatever, keep the original above as a reference
      if (!parentObject) continue;

      // track->gameObjectModificationEvent -= { &ParentObject::HandleGameObject, parentObject };
      RemoveCallback(track, parentObject);
      parentObject->childrenTracks.erase(track);
    }

    NELogger::Logger.debug("Reparenting {} from {} to {}", track.GetGameObjects().size(), track.GetName(),
                           parentTrackEventData.parentTrack.GetName());
    for (auto gameObject : track.GetGameObjects()) {
      instance->ParentToObject(gameObject->get_transform());
    }

    instance->childrenTracks.emplace(track);

    // track.gameObjectModificationEvent += { &ParentObject::HandleGameObject, instance };
    RemoveCallback(track, instance);
    AddCallback(track, instance, [instance, track](UnityEngine::GameObject* go, bool added) {
      NELogger::Logger.debug("ParentObject child {} Track Callback: {} {}", track.GetName(), go->get_name(),
                             added ? "Added" : "Removed");
      instance->HandleGameObject(instance->track, go, !added);
    });
  }

  ParentController::parentObjects.emplace_back(instance);

  if (instance->track.v2) {

    auto startPos = parentTrackEventData.offsetPosition;
    auto startRot = parentTrackEventData.worldRotation;
    auto startLocalRot = parentTrackEventData.transformData.localRotation;
    auto startScale = parentTrackEventData.transformData.scale;

    // set initial values
    if (startPos.has_value()) {
      instance->startPos = *startPos;
      transform->set_localPosition(instance->startPos * StaticBeatmapObjectSpawnMovementData::kNoteLinesDistance);
    }

    if (startRot.has_value()) {
      instance->startRot = *startRot;
      instance->startLocalRot = instance->startRot;
      transform->set_localPosition(instance->startRot * NEVector::Vector3(transform->get_localPosition()));
      transform->set_localRotation(instance->startRot);
    }

    if (startLocalRot.has_value()) {
      instance->startLocalRot = instance->startRot * *startLocalRot;
      transform->set_localRotation(NEVector::Quaternion(transform->get_localRotation()) * instance->startLocalRot);
    }

    if (startScale.has_value()) {
      instance->startScale = *startScale;
      transform->set_localScale(instance->startScale);
    }
  } else {
    // delegate to GameObjectTrackController
    instance->enabled = false;
    // TODO: Left handed
    parentTrackEventData.transformData.Apply(transform, false, false);

    std::array<TrackW, 1> _tracks = { parentTrackEventData.parentTrack };
    Tracks::GameObjectTrackController::HandleTrackData(
        parentGameObject, _tracks, StaticBeatmapObjectSpawnMovementData::kNoteLinesDistance, false, false);
  }
}

void ParentObject::ParentToObject(Transform* childTransform) {
  childTransform->SetParent(origin, worldPositionStays);
  // SetParent(childTransform, origin, worldPositionStays);
}

void ParentObject::ResetTransformParent(Transform* transform) {
  transform->SetParent(nullptr, false);
}

void ParentObject::HandleGameObject(TrackW track, UnityEngine::GameObject* go, bool removed) {
  if (removed) {
    ResetTransformParent(go->get_transform());
  } else {
    ParentToObject(go->get_transform());
  }
}

ParentObject::~ParentObject() {
  for (auto childTrack : childrenTracks) {
    // childTrack->gameObjectModificationEvent -= { &ParentObject::HandleGameObject, this };
    RemoveCallback(childTrack, this);
  }
  // just in case
  // track->gameObjectModificationEvent -= { &ParentObject::HandleGameObject, this };

  // ParentController::parentObjects.erase(
  //     std::remove(ParentController::parentObjects.begin(), ParentController::parentObjects.end(), this),
  //     ParentController::parentObjects.end());
}

void ParentController::OnDestroy() {
  NELogger::Logger.debug("Clearing parent objects");
  parentObjects.clear();
  gameObjectModificationCallbacks.clear();
}