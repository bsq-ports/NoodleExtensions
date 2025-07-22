#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/PlayerTransforms.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"

#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"

#include "NEHooks.h"

#include "tracks/shared/Vector.h"
#include <optional>

using namespace GlobalNamespace;
using namespace UnityEngine;

static Transform* _parentTransform;
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
  if (!Hooks::isNoodleHookEnabled()) return PlayerTransforms_HeadsetOffsetZ(self);

  if (self->_beatmapKey.hasValue && self->____beatmapKey.Value.beatmapCharacteristic->containsRotationEvents) {
    return;
  }

  if (!_parentTransform) {
    _parentTransform = self->_originParentTransform;
    // _noodlePlayerTransformManager.Active ? _noodlePlayerTransformManager.Head : originParentTransform;
  }

  self->_headPseudoLocalZOnlyPos = Sombrero::FastVector3(_parentTransform->forward) *
                                   HeadOffsetZ(self->_headPseudoLocalPos, self->_originParentTransform);
}

void InstallPlayerTransformsHooks() {
  INSTALL_HOOK(NELogger::Logger, PlayerTransforms_Awake);
  INSTALL_HOOK(NELogger::Logger, PlayerTransforms_HeadsetOffsetZ);
}

NEInstallHooks(InstallPlayerTransformsHooks);
