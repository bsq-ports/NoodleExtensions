#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/MirroredObstacleController.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "UnityEngine/Transform.hpp"

#include "Animation/AnimationHelper.h"
#include "Animation/ParentObject.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace TrackParenting;

MAKE_HOOK_MATCH(MirroredObstacleController_Mirror, &MirroredObstacleController::Mirror, void,
                MirroredObstacleController* self, ObstacleController* obstacleController) {
  if (Hooks::isNoodleHookEnabled()) {
    self->set_hide(true);
    return;
  }

  MirroredObstacleController_Mirror(self, obstacleController);
}

MAKE_HOOK_MATCH(MirroredObstacleController_UpdatePositionAndRotation,
                &MirroredObstacleController::UpdatePositionAndRotation, void, MirroredObstacleController* self) {
  // static auto *customObstacleDataClass = i2c::class_of<CustomJSONData::CustomObstacleData *>();
  // if (self->followedObstacle && self->followedObstacle->_obstacleData->klass == customObstacleDataClass) {
  if (Hooks::isNoodleHookEnabled()) {
    self->set_hide(true);
    return;
  }
  // }

  // if (self->followedTransform->get_position().y < 0) {
  //     // Hide without disabling update
  //     self->transform->set_position(Vector3(0, 100, 0));
  // }

  // Vector3 position = self->followedTransform->get_position();
  // Quaternion quaternion = self->followedTransform->get_rotation();
  // position.y = -position.y;

  // // TODO:
  // // quaternion = quaternion.Reflect(Vector3::get_up());

  // self->transform->SetPositionAndRotation(position, quaternion);

  // if (self->followedTransform->get_localScale() !=
  //     self->transform->get_localScale()) {
  //     self->transform->set_localScale(
  //         self->followedTransform->get_localScale());
  // }

  MirroredObstacleController_UpdatePositionAndRotation(self);
}

void InstallMirroredObstacleControllerHooks() {
  INSTALL_HOOK(NELogger::Logger, MirroredObstacleController_Mirror);
  INSTALL_HOOK(NELogger::Logger, MirroredObstacleController_UpdatePositionAndRotation);
}

NEInstallHooks(InstallMirroredObstacleControllerHooks);