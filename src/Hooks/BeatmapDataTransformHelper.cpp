#include "Constants.hpp"
#include "beatsaber-hook/shared/hooking.hpp"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"

#include "GlobalNamespace/IJumpOffsetYProvider.hpp"
#include "GlobalNamespace/PlayerHeightDetector.hpp"
#include "GlobalNamespace/PlayerHeightToJumpOffsetYProvider.hpp"
#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapDataObstaclesMergingTransform.hpp"
#include "GlobalNamespace/BeatmapDataTransformHelper.hpp"
#include "GlobalNamespace/BeatmapLineData.hpp"
#include "GlobalNamespace/EnvironmentEffectsFilterPreset.hpp"
#include "GlobalNamespace/EnvironmentIntensityReductionOptions.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/IReadonlyBeatmapData.hpp"
#include "GlobalNamespace/PracticeSettings.hpp"
#include "System/Collections/Generic/IEnumerable_1.hpp"
#include "System/Func_2.hpp"
#include "System/Linq/Enumerable.hpp"
#include "System/Linq/IOrderedEnumerable_1.hpp"
#include "UnityEngine/Resources.hpp"

#include "GlobalNamespace/BeatmapDataBasicInfo.hpp"

#include "AssociatedData.h"
#include "NEHooks.h"
#include "NELogger.h"
#include "NECaches.h"
#include "SceneTransitionHelper.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "custom-json-data/shared/CustomBeatmapSaveDatav3.h"
#include "GlobalNamespace/SortedList_1.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "SpawnDataHelper.h"
#include "custom-json-data/shared/JsonUtils.h"
#include "tracks/shared/AssociatedData.h"
#include "tracks/shared/Json.h"

#include <optional>

using namespace GlobalNamespace;
using namespace System::Collections::Generic;

//
// IReadonlyBeatmapData *ReorderLineData(IReadonlyBeatmapData *beatmapData) {
//    auto *customBeatmapData = static_cast<CustomJSONData::CustomBeatmapData *>(beatmapData->GetCopy());
//    if (!customObstacleDataClass) {
//        customObstacleDataClass = i2c::class_of<CustomJSONData::CustomObstacleData *>();
//        customNoteDataClass = i2c::class_of<CustomJSONData::CustomNoteData *>();
//    }
//
//
//
//    auto notes = customBeatmapData->GetBeatmapItemsCpp<NoteData*>();
//    auto obstacles = customBeatmapData->GetBeatmapItemsCpp<ObstacleData*>();
//
//    std::vector<BeatmapObjectData*> objects;
//    objects.reserve(notes.size() + obstacles.size());
//
//    std::copy(notes.begin(), notes.end(), std::back_inserter(objects));
//    std::copy(obstacles.begin(), obstacles.end(), std::back_inserter(objects));
//
//    // loop through all objects in all lines of the beatmapData
//    for (BeatmapObjectData *beatmapObjectData : objects) {
//
//    }
//
//    return reinterpret_cast<IReadonlyBeatmapData *>(customBeatmapData);
//}

extern System::Collections::Generic::LinkedList_1<BeatmapDataItem*>*
SortAndOrderList(CustomJSONData::CustomBeatmapData* beatmapData);

void LoadNoodleObjects(CustomJSONData::CustomBeatmapData* beatmap) {
  NELogger::Logger.info("BeatmapData klass name is {}", beatmap->klass->name);

  static auto* customObstacleDataClass = i2c::class_of<CustomJSONData::CustomObstacleData*>();
  static auto* customNoteDataClass = i2c::class_of<CustomJSONData::CustomNoteData*>();
  static auto* customSliderDataClass = i2c::class_of<CustomJSONData::CustomSliderData*>();

  TracksAD::BeatmapAssociatedData& beatmapAD = TracksAD::getBeatmapAD(beatmap->customData);

  if (!beatmapAD.valid) {
    TracksAD::readBeatmapDataAD(beatmap);
  }

  auto v2 = beatmap->v2orEarlier;

  bool mirror = true;

  if (beatmap->customData->value) {
    rapidjson::Value const& data = *beatmap->customData->value;
    mirror = NEJSON::ReadOptionalBool(data, v2 ? "_questNoteMirror" : "questNoteMirror").value_or(true);
  }

  for (BeatmapObjectData* beatmapObjectData : beatmap->beatmapObjectDatas) {
    CustomJSONData::CustomNoteData* noteData =
        i2c::try_cast<CustomJSONData::CustomNoteData*>(beatmapObjectData);
    CustomJSONData::CustomObstacleData* obstacleData =
        i2c::try_cast<CustomJSONData::CustomObstacleData*>(beatmapObjectData);
    CustomJSONData::CustomSliderData* sliderData =
        i2c::try_cast<CustomJSONData::CustomSliderData*>(beatmapObjectData);
    CustomJSONData::JSONWrapper* customDataWrapper = nullptr;
    if (obstacleData) {
      customDataWrapper = obstacleData->customData;
    }
    if (noteData) {
      customDataWrapper = noteData->customData;
    }
    if (sliderData) {
      customDataWrapper = sliderData->customData;
    }

    BeatmapObjectAssociatedData& ad = getAD(customDataWrapper);

    ad.mirror = mirror;

    if (customDataWrapper->value) {
      rapidjson::Value const& customData = *customDataWrapper->value;

      if (ad.parsed) continue;

      auto v3Fake = ad.objectData.fake;
      ad.objectData = ObjectCustomData(customData, noteData, obstacleData, v2);
      if (v3Fake) ad.objectData.fake = v3Fake;

      if (!ad.flipX || !ad.flipY) {
        auto [x, y] = NEJSON::ReadOptionalPair(customData, v2 ? NoodleExtensions::Constants::V2_FLIP
                                                              : NoodleExtensions::Constants::FLIP);
        ad.flipX = x;
        ad.flipY = y;
      }

      auto animationKey = v2 ? NoodleExtensions::Constants::V2_ANIMATION : NoodleExtensions::Constants::ANIMATION;
      if (customData.HasMember(animationKey.data())) {
        rapidjson::Value const& animation = customData[animationKey.data()];
        ad.animationData = { beatmapAD, animation, v2 };
      } else {
        ad.animationData = AnimationObjectData();
      }
    }
    ad.parsed = true;
  }
}

void LoadNoodleEvent(TracksAD::BeatmapAssociatedData& beatmapAD, CustomJSONData::CustomEventData const* customEventData,
                     bool v2) {
  bool isType = false;

  auto typeHash = customEventData->typeHash;

#define TYPE_GET(jsonName, varName)                                                                                    \
  static auto jsonNameHash_##varName = std::hash<std::string_view>()(jsonName);                                        \
  if (!isType && typeHash == (jsonNameHash_##varName)) isType = true;

  TYPE_GET(NoodleExtensions::Constants::ASSIGN_TRACK_PARENT, AssignTrackParent)
  TYPE_GET(NoodleExtensions::Constants::ASSIGN_PLAYER_TO_TRACK, AssignPlayerToTrack)

  if (!isType) {
    return;
  }
  CRASH_UNLESS(customEventData->customData->value);
  rapidjson::Value const& eventData = *customEventData->customData->value;
  auto& eventAD = getEventAD(customEventData);

  if (eventAD.parsed) return;

  if (typeHash == jsonNameHash_AssignTrackParent) {
    eventAD.parentTrackEventData.emplace(eventData, beatmapAD, v2);
  } else if (typeHash == jsonNameHash_AssignPlayerToTrack) {
    std::string_view trackName(
        eventData[v2 ? NoodleExtensions::Constants::V2_TRACK.data() : NoodleExtensions::Constants::TRACK.data()]
            .GetString());
    std::string_view trackTarget(
        NEJSON::ReadOptionalString(eventData, v2 ? NoodleExtensions::Constants::V2_TARGET.data()
                                                 : NoodleExtensions::Constants::TARGET.data())
            .value_or("Root"));
    TrackW track = beatmapAD.getTrack(trackName);
    NELogger::Logger.debug("Assigning player to track {} at {}, with target {}", trackName.data(), track.GetName(),
                           trackTarget.data());
    eventAD.playerTrackEventData.emplace(track, trackTarget);

    NECaches::hasPlayerTransfrom = true;
  }

  eventAD.parsed = true;
}

void LoadNoodleEvents(CustomJSONData::CustomBeatmapData* beatmap) {
  auto& beatmapAD = TracksAD::getBeatmapAD(beatmap->customData);

  auto v2 = beatmap->v2orEarlier;

  if (!beatmapAD.valid) {
    TracksAD::readBeatmapDataAD(beatmap);
  }

  // Parse events
  for (auto const& customEventData : beatmap->customEventDatas) {
    LoadNoodleEvent(beatmapAD, customEventData, v2);
  }
}

MAKE_HOOK_MATCH(BeatmapDataTransformHelper_CreateTransformedBeatmapData,
                &BeatmapDataTransformHelper::CreateTransformedBeatmapData, IReadonlyBeatmapData*,
                ::GlobalNamespace::IReadonlyBeatmapData* beatmapData, ::GlobalNamespace::BeatmapKey beatmapKey,
                ::GlobalNamespace::BeatmapLevel* beatmapLevel, ::GlobalNamespace::GameplayModifiers* gameplayModifiers,
                ::GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings,
                ::GlobalNamespace::EnvironmentInfoSO* originalEnvironmentInfo, bool screenDisplacementEffects) {
  auto result = BeatmapDataTransformHelper_CreateTransformedBeatmapData(
      beatmapData, beatmapKey, beatmapLevel, gameplayModifiers, playerSpecificSettings, originalEnvironmentInfo,
      screenDisplacementEffects);

  if (!Hooks::isNoodleHookEnabled()) return result;

  if (auto customBeatmap = i2c::try_cast<CustomJSONData::CustomBeatmapData*>(result)) {
    auto& beatmapAD = TracksAD::getBeatmapAD(customBeatmap->customData);

    if (!beatmapAD.valid) {
      TracksAD::readBeatmapDataAD(customBeatmap);
    }

    LoadNoodleObjects(customBeatmap);
    LoadNoodleEvents(customBeatmap);

    NECaches::hasLocalSpaceTrail = false;
    auto customData = customBeatmap->customData;
    if (customData != nullptr) {
      if (customData->value.has_value()) {
        auto localSpaceTrail =
            NEJSON::ReadOptionalBool(customData->value.value(), NoodleExtensions::Constants::TRAIL_LOCAL_SPACE);
        if (localSpaceTrail.has_value()) {
          NECaches::hasLocalSpaceTrail = localSpaceTrail.value();
        }
      }
    }
  }

  return result;
}

void HandleFakeObjects(CustomJSONData::v3::CustomBeatmapSaveData* beatmap) {
  using namespace CustomJSONData::v3;
  if (!beatmap->levelCustomData || !beatmap->customData) return;

  auto noodle = NoodleExtensions::SceneTransitionHelper::CheckIfNoodle(*beatmap->levelCustomData);

  if (!noodle) return;

  rapidjson::Value const& customData = *beatmap->customData;

  // not ideal to set fieldbut works for now

#define PARSE_ARRAY(key, array, parse)                                                                                 \
  auto key##it = customData.FindMember(#key);                                                                          \
  if (key##it != customData.MemberEnd()) {                                                                             \
    for (auto& it : key##it->value.GetArray()) {                                                                 \
      auto item = parse(it);                                                                                           \
      if (!item || !item->customData) continue;                                                                        \
      reinterpret_cast<rapidjson::Value&>(item->customData.value()).GetObject().AddMember(                                                            \
          rapidjson::StringRef(NoodleExtensions::Constants::INTERNAL_FAKE_NOTE.data()), rapidjson::Value(true).Move(),        \
          alloc);                                                                                                      \
    }                                                                                                                  \
  }

  // auto& ad = getAD(item->customData);                                                                              \
   // ad.objectData.fake = true;                                                                                       \
      array->Add(item);                                                                                                \

  // TODO: set counts properly
  // basic->_obstaclesCount_k__BackingField = unsafeBeatmap->obstacles->_size;
  // basic->_cuttableNotesCount_k__BackingField = unsafeBeatmap->colorNotes->_size;
  // basic->_bombsCount_k__BackingField = unsafeBeatmap->bombNotes->_size;

  auto alloc = beatmap->doc->GetAllocator();

  PARSE_ARRAY(fakeColorNotes, unsafeBeatmap->colorNotes, Parser::DeserializeColorNote);
  PARSE_ARRAY(fakeBombNotes, unsafeBeatmap->bombNotes, Parser::DeserializeBombNote);
  PARSE_ARRAY(fakeObstacles, unsafeBeatmap->obstacles, Parser::DeserializeObstacle);
  PARSE_ARRAY(fakeBurstSliders, unsafeBeatmap->burstSliders, Parser::DeserializeBurstSlider);
  PARSE_ARRAY(fakeSliders, unsafeBeatmap->sliders, Parser::DeserializeSlider);
}

void InstallBeatmapDataTransformHelperHooks() {
  // Modloader::requireMod("CustomJSONData");
  CustomJSONData::v3::Parser::ParsedEvent += HandleFakeObjects;
  // SongCore::API::Loading::GetSongsLoadedEvent() += HandleFakeObjects;
  //  RuntimeSongLoader::API::AddBeatmapDataBasicInfoLoadedEvent(HandleFakeObjects);

  INSTALL_HOOK(NELogger::Logger, BeatmapDataTransformHelper_CreateTransformedBeatmapData);
}

NEInstallHooks(InstallBeatmapDataTransformHelperHooks);
