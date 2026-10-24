#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/ObstacleData.hpp"

#include "AssociatedData.h"

#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(ObstacleData_Mirror, &ObstacleData::Mirror, void, ObstacleData* self, int lineCount) {
  static auto CustomObstacleDataKlass = i2c::class_of<CustomJSONData::CustomObstacleData*>();
  if (self->klass != CustomObstacleDataKlass) {
    return ObstacleData_Mirror(self, lineCount);
  }

  auto customObstacleData = reinterpret_cast<CustomJSONData::CustomObstacleData*>(self);

  auto& ad = getAD(customObstacleData->customData);

  float width = self->width;

  std::optional<float> scaleX;

  if (ad.objectData.width) {
    scaleX = ad.objectData.width;

    if (scaleX) width = *scaleX;
  }

  if (ad.objectData.startX) {
    ad.objectData.startX = (*ad.objectData.startX + width) * -1.0f;
  } else if (scaleX) {
    float lineIndex = self->lineIndex - 2;
    ad.objectData.startX = (lineIndex + width) * -1.0f;
  }

  if (ad.objectData.localRotation) {
    ad.objectData.localRotation->y *= -1;
    ad.objectData.localRotation->z *= -1;
  }

  if (ad.objectData.rotation) {
    ad.objectData.rotation->y *= -1;

    // TODO: If this breaks, I blame Aero for having single float as rotation values
    ad.objectData.rotation->z *= -1;
  }

  return ObstacleData_Mirror(self, lineCount);
}

void InstallObstacleDataHooks() {
  INSTALL_HOOK(NELogger::Logger, ObstacleData_Mirror);
}

NEInstallHooks(InstallObstacleDataHooks);