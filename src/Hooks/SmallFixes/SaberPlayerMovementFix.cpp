#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Vector3.hpp"
#include "beatsaber-hook/shared/utils/byref.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/SaberMovementData.hpp"
#include "GlobalNamespace/SaberSwingRatingCounter.hpp"
#include "GlobalNamespace/SaberTrail.hpp"
#include "GlobalNamespace/BladeMovementDataElement.hpp"
#include "GlobalNamespace/PlayerTransforms.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/IBladeMovementData.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"

#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"

#include "NEHooks.h"
#include "AssociatedData.h"
#include "NECaches.h"
#include <unordered_map>

using namespace GlobalNamespace;
using namespace UnityEngine;

static std::unordered_map<GlobalNamespace::IBladeMovementData*, GlobalNamespace::SaberMovementData*> _worldMovementData;
static SafePtrUnity<Transform> _origin;

void CheckOrigin() {
  if (_origin) return;

  // CustomModels breaks player transforms and thus we have to check if origin is valid before using it
  // https://github.com/bsq-ports/NoodleExtensions/issues/110
  auto playerTransform = Resources::FindObjectsOfTypeAll<PlayerTransforms*>()->FirstOrDefault();
  if (!playerTransform) {
    NELogger::Logger.warn("PlayerTransforms not found, cannot apply SaberPlayerMovementFix");
    return;
  }
  _origin = playerTransform->_originTransform;
}

Vector3 ComputeWorld(Vector3 original) {
  CheckOrigin();
  // CustomModels breaks player transforms and thus we have to check if origin is valid before using it
  if (!_origin) return original;

  return _origin->TransformPoint(original);
}

Vector3 InverseComputeWorld(Vector3 original) {
  CheckOrigin();
  // CustomModels breaks player transforms and thus we have to check if origin is valid before using it
  if (!_origin) return original;

  return _origin->InverseTransformPoint(original);
}

bool containsValue(SaberMovementData* data) {
  for (auto& pair : _worldMovementData) {
    if (pair.second == data) {
      return true;
    }
  }
  return false;
}

MAKE_HOOK_MATCH(SaberMovementData_ComputeAdditionalData, &SaberMovementData::ComputeAdditionalData, void,
                SaberMovementData* self, Vector3 topPos, Vector3 bottomPos, int idxOffset, ByRef<Vector3> segmentNormal,
                ByRef<float> segmentAngle) {
  if (!Hooks::isNoodleHookEnabled() || NECaches::hasLocalSpaceTrail || !NECaches::hasPlayerTransfrom) {
    return SaberMovementData_ComputeAdditionalData(self, topPos, bottomPos, idxOffset, segmentNormal, segmentAngle);
  }
  int num = self->_data.size();
  int num2 = self->_nextAddIndex + idxOffset;
  int num3 = num2 - 1;
  if (num3 < 0) {
    num3 += num;
  }
  if (self->_validCount > 0) {
    Sombrero::FastVector3 topPos2 = ComputeWorld(self->_data[num2].topPos);
    Sombrero::FastVector3 bottomPos2 = ComputeWorld(self->_data[num2].bottomPos);
    Sombrero::FastVector3 topPos3 = ComputeWorld(self->_data[num3].topPos);
    Sombrero::FastVector3 bottomPos3 = ComputeWorld(self->_data[num3].bottomPos);
    segmentNormal = self->ComputePlaneNormal(topPos2, bottomPos2, topPos3, bottomPos3);
    segmentAngle = Sombrero::FastVector3::Angle(topPos3 - bottomPos3, topPos2 - bottomPos2);
    return SaberMovementData_ComputeAdditionalData(self, topPos, bottomPos, idxOffset, segmentNormal, segmentAngle);
  }
  segmentNormal = Sombrero::FastVector3::zero();
  segmentAngle = 0.0f;
}

MAKE_HOOK_MATCH(SaberSwingRatingCounter_ProcessNewData, &SaberSwingRatingCounter::ProcessNewData, void,
                SaberSwingRatingCounter* self, BladeMovementDataElement newData, BladeMovementDataElement prevData,
                bool prevDataAreValid) {
  if (!Hooks::isNoodleHookEnabled() || NECaches::hasLocalSpaceTrail || !NECaches::hasPlayerTransfrom)
    return SaberSwingRatingCounter_ProcessNewData(self, newData, prevData, prevDataAreValid);

  newData.topPos = ComputeWorld(newData.topPos);
  newData.bottomPos = ComputeWorld(newData.bottomPos);
  prevData.topPos = ComputeWorld(prevData.topPos);
  prevData.bottomPos = ComputeWorld(prevData.bottomPos);

  SaberSwingRatingCounter_ProcessNewData(self, newData, prevData, prevDataAreValid);
}

MAKE_HOOK_MATCH(SaberMovementData_lastAddedData, &SaberMovementData::get_lastAddedData, BladeMovementDataElement,
                SaberMovementData* self) {
  if (!Hooks::isNoodleHookEnabled() || NECaches::hasLocalSpaceTrail || !NECaches::hasPlayerTransfrom)
    return SaberMovementData_lastAddedData(self);

  if (containsValue(self)) {
    return SaberMovementData_lastAddedData(self);
  }

  auto value = SaberMovementData_lastAddedData(self);

  value.topPos = ComputeWorld(value.topPos);
  value.bottomPos = ComputeWorld(value.bottomPos);

  return value;
}

MAKE_HOOK_MATCH(SaberMovementData_prevAddedData, &SaberMovementData::get_prevAddedData, BladeMovementDataElement,
                SaberMovementData* self) {
  if (!Hooks::isNoodleHookEnabled() || NECaches::hasLocalSpaceTrail || !NECaches::hasPlayerTransfrom)
    return SaberMovementData_prevAddedData(self);

  if (containsValue(self)) {
    return SaberMovementData_prevAddedData(self);
  }

  auto value = SaberMovementData_prevAddedData(self);

  value.topPos = ComputeWorld(value.topPos);
  value.bottomPos = ComputeWorld(value.bottomPos);

  return value;
}

MAKE_HOOK_MATCH(SaberMovementData_AddNewData, &SaberMovementData::AddNewData, void, SaberMovementData* self,
                Vector3 topPos, Vector3 bottomPos, float time) {
  if (!Hooks::isNoodleHookEnabled() || NECaches::hasLocalSpaceTrail || !NECaches::hasPlayerTransfrom)
    return SaberMovementData_AddNewData(self, topPos, bottomPos, time);

  if (containsValue(self)) {
    return SaberMovementData_AddNewData(self, topPos, bottomPos, time);
  }

  if (_worldMovementData.contains(self->i___GlobalNamespace__IBladeMovementData())) {
    _worldMovementData[self->i___GlobalNamespace__IBladeMovementData()]->AddNewData(topPos, bottomPos, time);
  }

  SaberMovementData_AddNewData(self, InverseComputeWorld(topPos), InverseComputeWorld(bottomPos), time);
}

MAKE_HOOK_MATCH(SaberTrail_Setup, &SaberTrail::Setup, void, SaberTrail* self, Color color,
                IBladeMovementData* movementData) {
  if (!Hooks::isNoodleHookEnabled() || !NECaches::hasPlayerTransfrom)
    return SaberTrail_Setup(self, color, movementData);

  if (NECaches::hasLocalSpaceTrail) {
    CheckOrigin();
    self->_trailRenderer->transform->SetParent(_origin->parent, false);
    return SaberTrail_Setup(self, color, movementData);
  }

  auto worldMovementData = SaberMovementData::New_ctor();
  _worldMovementData[movementData] = worldMovementData;
  SaberTrail_Setup(self, color, worldMovementData->i___GlobalNamespace__IBladeMovementData());
}

void InstallSaberPlayerMovementFixHooks() {
  INSTALL_HOOK(NELogger::Logger, SaberMovementData_ComputeAdditionalData);
  INSTALL_HOOK(NELogger::Logger, SaberSwingRatingCounter_ProcessNewData);
  INSTALL_HOOK(NELogger::Logger, SaberMovementData_lastAddedData);
  INSTALL_HOOK(NELogger::Logger, SaberMovementData_prevAddedData);
  INSTALL_HOOK(NELogger::Logger, SaberMovementData_AddNewData);
  INSTALL_HOOK(NELogger::Logger, SaberTrail_Setup);
}

NEInstallHooks(InstallSaberPlayerMovementFixHooks);