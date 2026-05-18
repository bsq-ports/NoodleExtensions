#include "AssociatedData.h"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/PlayerTransforms.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"

#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"

#include "NEHooks.h"
#include "Animation/PlayerTrack.h"

#include "tracks/shared/Vector.h"
#include <optional>

using namespace GlobalNamespace;
using namespace UnityEngine;

static UnityW<Transform> _parentTransform;
static float HeadOffsetZ(Sombrero::FastVector3 headPsuedoLocalPos, Transform* originParentTransform) {
  // get magnitude in direction we care about rather than just z
  if (!_parentTransform) {
    _parentTransform = originParentTransform;
    // _noodlePlayerTransformManager.Active ? _noodlePlayerTransformManager.Head : originParentTransform;
  }
  return Sombrero::FastVector3::Dot(headPsuedoLocalPos, _parentTransform->forward);
}

MAKE_HOOK_MATCH(PlayerTransforms_Awake, &PlayerTransforms::Awake, void, PlayerTransforms* self) {
  if (!Hooks::isNoodleHookEnabled()) return PlayerTransforms_Awake(self);

  PlayerTransforms_Awake(self);
  self->_useOriginParentTransformForPseudoLocalCalculations = false;
}
MAKE_HOOK_MATCH(PlayerTransforms_HeadsetOffsetZ, &PlayerTransforms::Update, void, PlayerTransforms* self) {
  PlayerTransforms_HeadsetOffsetZ(self);
  if (!Hooks::isNoodleHookEnabled()) return;
  if (self->_overrideHeadPos) return;

  auto const& manager = TrackParenting::PlayerTrack::GetPlayerTracks();
  if (manager.empty()) return;

  // ensure we have a valid parent transform
  if (!_parentTransform) {
    _parentTransform = self->_originParentTransform;
  }

  // Make sure to set _headWorldPos to prevent collissions with walls when player is moved.
  if (UnityW<Transform> headTransform = self->_headTransform; headTransform.isAlive()) {
    self->_headWorldPos = headTransform->position;
    self->_headWorldRot = headTransform->rotation;
  }

  // head
  {
    if (auto it = manager.find(PlayerTrackObject::Head); it != manager.end()) {
      auto headObject = it->second->transform;
      if (headObject) {
        self->_headPseudoLocalPos = headObject->InverseTransformPoint(self->_headWorldPos);
        self->_headPseudoLocalRot =
            NEVector::Quaternion(NEVector::Quaternion::Inverse(headObject->rotation)) * self->_headWorldRot;
      }
    }
  }

  // right hand
  {
    if (auto it = manager.find(PlayerTrackObject::RightHand); it != manager.end()) {
      auto rightHandObject = it->second->transform;
      if (rightHandObject) {
        self->_rightHandPseudoLocalPos = rightHandObject->InverseTransformPoint(self->_rightHandTransform->position);
        self->_rightHandPseudoLocalRot =
            NEVector::Quaternion(NEVector::Quaternion::Inverse(rightHandObject->rotation)) *
            self->_rightHandTransform->rotation;
      }
    }
  }

  // left hand
  {
    if (auto it = manager.find(PlayerTrackObject::LeftHand); it != manager.end()) {
      auto leftHandObject = it->second->transform;
      if (leftHandObject) {
        self->_leftHandPseudoLocalPos = leftHandObject->InverseTransformPoint(self->_leftHandTransform->position);
        self->_leftHandPseudoLocalRot = NEVector::Quaternion(NEVector::Quaternion::Inverse(leftHandObject->rotation)) *
                                        self->_leftHandTransform->rotation;
      }
    }
  }

  if (self->_beatmapKey.hasValue && self->_beatmapKey.Value.beatmapCharacteristic->containsRotationEvents) {
    return;
  }

  self->_headPseudoLocalZOnlyPos = Sombrero::FastVector3(_parentTransform->forward) *
                                   HeadOffsetZ(self->_headPseudoLocalPos, self->_originParentTransform);
}

void InstallPlayerTransformsHooks() {
  INSTALL_HOOK(NELogger::Logger, PlayerTransforms_Awake);
  INSTALL_HOOK(NELogger::Logger, PlayerTransforms_HeadsetOffsetZ);
}

NEInstallHooks(InstallPlayerTransformsHooks);
