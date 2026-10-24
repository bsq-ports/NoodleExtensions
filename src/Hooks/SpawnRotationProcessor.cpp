#include "beatsaber-hook/shared/hooking.hpp"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"

#include "GlobalNamespace/BeatmapEventData.hpp"
#include "GlobalNamespace/BeatmapEventTypeExtensions.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

#include "NEHooks.h"
#include "AssociatedData.h"

using namespace GlobalNamespace;

// MAKE_HOOK_MATCH(ProcessBeatmapEventData,
//                 &SpawnRotationProcessor::ProcessBeatmapEventData, bool,
//                 SpawnRotationProcessor *self,
//                 BeatmapEventData *normalEventData) {
//     if (!Hooks::isNoodleHookEnabled())
//         return ProcessBeatmapEventData(self, normalEventData);
//
//     auto *beatmapEventData = reinterpret_cast<CustomJSONData::CustomBeatmapEventData *>(normalEventData);
//     if (BeatmapEventTypeExtensions::IsRotationEvent(beatmapEventData->type)) {
//         if (!beatmapEventData->customData->value) {
//             return ProcessBeatmapEventData(self, beatmapEventData);
//         }
//
//         rapidjson::Value const &customData = *beatmapEventData->customData->value;
//         std::optional<float> rotation =
//             customData.HasMember("_rotation")
//                 ? std::optional{customData["_rotation"].GetFloat()}
//                 : std::nullopt;
//
//         if (rotation.has_value()) {
//             self->rotation += *rotation;
//             return true;
//         }
//     }
//     return ProcessBeatmapEventData(self, beatmapEventData);
// }

void InstallSpawnRotationProcessorHooks() {
  //    INSTALL_HOOK(NELogger::Logger, ProcessBeatmapEventData);
}

NEInstallHooks(InstallSpawnRotationProcessorHooks);