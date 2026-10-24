
#include "beatsaber-hook/shared/hooking.hpp"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"

#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapDataLoader.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"

#include "Animation/AnimationHelper.h"
#include "tracks/shared/Animation/Track.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "NELogger.h"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"

#include <optional>

using namespace GlobalNamespace;

// Needed for obstacles, idk why too lazy to figure out
void LoadNoodleObjects(CustomJSONData::CustomBeatmapData* beatmap, BeatmapObjectSpawnMovementData* movementData,
                       BeatmapObjectSpawnController::InitData* initData);
//
// MAKE_HOOK_MATCH(GetBeatmapDataFromBeatmapSaveData,
//                &BeatmapDataLoader::GetBeatmapDataFromBeatmapSaveData,
//                BeatmapData *, BeatmapDataLoader *self,
//                List<BeatmapSaveData::NoteData *> *notesSaveData,
//                List<BeatmapSaveData::WaypointData *> *waypointsSaveData,
//                List<BeatmapSaveData::ObstacleData *> *obstaclesSaveData,
//                List<BeatmapSaveData::EventData *> *eventsSaveData,
//                BeatmapSaveData::SpecialEventKeywordFiltersData
//                    *evironmentSpecialEventFilterData,
//                float startBpm, float shuffle, float shufflePeriod) {
//    // This should call CJD's hook
//    auto *result =
//        (CustomJSONData::CustomBeatmapData *)GetBeatmapDataFromBeatmapSaveData(
//            self, notesSaveData, waypointsSaveData, obstaclesSaveData,
//            eventsSaveData, evironmentSpecialEventFilterData, startBpm, shuffle,
//            shufflePeriod);
//
//    LoadNoodleObjects(result);
//
//    return result;
//}

void InstallBeatmapDataLoaderHooks() {
  //    INSTALL_HOOK(NELogger::Logger, GetBeatmapDataFromBeatmapSaveData);
}

NEInstallHooks(InstallBeatmapDataLoaderHooks);