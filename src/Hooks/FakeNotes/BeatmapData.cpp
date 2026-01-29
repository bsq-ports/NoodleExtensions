// TODO: Fix with SongCore changes

#include "GlobalNamespace/BeatmapData.hpp"
#include "NELogger.h"
#include "beatsaber-hook/shared/utils/hooking.hpp"

#include "GlobalNamespace/BeatmapDataLoader.hpp"
#include "GlobalNamespace/BeatmapDataBasicInfo.hpp"

#include "custom-json-data/shared/CustomBeatmapSaveDatav3.h"

#include "NEHooks.h"
#include "NEJSON.h"
#include "SceneTransitionHelper.hpp"
#include "AssociatedData.h"

#include "BeatmapDataLoaderVersion3/BeatmapDataLoader.hpp"

#include "BeatmapDataLoaderVersion2_6_0AndEarlier/BeatmapDataLoader.hpp"
#include "BeatmapSaveDataVersion2_6_0AndEarlier/BeatmapSaveData.hpp"
#include "BeatmapSaveDataVersion2_6_0AndEarlier/NoteData.hpp"

#include "custom-json-data/shared/CustomBeatmapSaveDatav2.h"
#include "custom-json-data/shared/misc/BeatmapDataLoaderUtils.hpp"

#include "UnityEngine/JsonUtility.hpp"

#include "Constants.hpp"
#include "sombrero/shared/linq_functional.hpp"
using namespace System;
using namespace System::Collections::Generic;
using namespace GlobalNamespace;
using namespace CustomJSONData;
using namespace UnityEngine;

// return true if fake
// subtracts from object count if fake
template <typename T> static bool IsFake(T* o, bool v2) {
  auto const optData = o->customData;

  if (!optData || !optData) return false;

  rapidjson::Value const& customData = *optData;

  auto fake = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_FAKE_NOTE
                                                      : NoodleExtensions::Constants::INTERNAL_FAKE_NOTE);
  return fake.value_or(false);
}

template <typename U, typename T> auto FakeCount(ArrayW<T> list, bool v2) {
  int i = list.size();
  for (auto o : list) {
    auto note = il2cpp_utils::try_cast<U>(o);
    if (!note) continue;

    if (IsFake(*note, v2)) i--;
  }

  return i;
}

MAKE_HOOK_MATCH(V2_BeatmapDataLoader_GetBeatmapDataBasicInfoFromSaveDataJson,
                &BeatmapDataLoaderVersion2_6_0AndEarlier::BeatmapDataLoader::GetBeatmapDataBasicInfoFromSaveDataJson,
                GlobalNamespace::BeatmapDataBasicInfo*, StringW beatmapSaveDataJson) {
  if (!beatmapSaveDataJson) return V2_BeatmapDataLoader_GetBeatmapDataBasicInfoFromSaveDataJson(beatmapSaveDataJson);

  auto beatmapSaveData =
      JsonUtility::FromJson<BeatmapSaveDataVersion2_6_0AndEarlier::BeatmapSaveData*>(beatmapSaveDataJson);
  if (beatmapSaveData == nullptr || beatmapSaveData->notes == nullptr) {
    return nullptr;
  }
  ListW<BeatmapSaveDataVersion2_6_0AndEarlier::NoteData*> notes = beatmapSaveData->notes;

  auto notBombs = notes | Sombrero::Linq::Functional::Where([](BeatmapSaveDataVersion2_6_0AndEarlier::NoteData* x) {
                    return x->type != BeatmapSaveDataVersion2_6_0AndEarlier::NoteType::Bomb;
                  }) |
                  Sombrero::Linq::Functional::ToArray();
  auto bombs = notes | Sombrero::Linq::Functional::Where([](BeatmapSaveDataVersion2_6_0AndEarlier::NoteData* x) {
                 return x->type == BeatmapSaveDataVersion2_6_0AndEarlier::NoteType::Bomb;
               }) |
               Sombrero::Linq::Functional::ToArray();

  int noteCount = FakeCount<v2::CustomBeatmapSaveData_NoteData>(notBombs, true);
  int bombCount = FakeCount<v2::CustomBeatmapSaveData_NoteData>(bombs, true);
  int obstacleCount = FakeCount<v2::CustomBeatmapSaveData_ObstacleData>(beatmapSaveData->obstacles->ToArray(), true);

  return BeatmapDataBasicInfo::New_ctor(4, noteCount, 0, obstacleCount, bombCount);
}

static JSONWrapper* JSONWrapperOrNull(v3::CustomDataOpt const& val) {
  auto* wrapper = JSONWrapper::New_ctor();

  if (!val || !val->get().IsObject()) {
    return wrapper;
  }

  wrapper->value = val;

  return wrapper;
}

MAKE_HOOK_MATCH(V3_BeatmapDataLoader_GetBeatmapDataFromSaveData,
                &BeatmapDataLoaderVersion3::BeatmapDataLoader::GetBeatmapDataFromSaveData,
                GlobalNamespace::BeatmapData*, ::BeatmapSaveDataVersion3::BeatmapSaveData* beatmapSaveData,
                ::BeatmapSaveDataVersion4::LightshowSaveData* defaultLightshowSaveData,
                ::GlobalNamespace::BeatmapDifficulty beatmapDifficulty, float_t startBpm,
                bool loadingForDesignatedEnvironment, ::GlobalNamespace::EnvironmentKeywords* environmentKeywords,
                ::GlobalNamespace::IEnvironmentLightGroups* environmentLightGroups,
                ::GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings,
                ::GlobalNamespace::IBeatmapLightEventConverter* lightEventConverter,
                ::System::Diagnostics::Stopwatch* stopwatch) {
  using namespace CustomJSONData::v3;

  auto beatmap = V3_BeatmapDataLoader_GetBeatmapDataFromSaveData(
      beatmapSaveData, defaultLightshowSaveData, beatmapDifficulty, startBpm, loadingForDesignatedEnvironment,
      environmentKeywords, environmentLightGroups, playerSpecificSettings, lightEventConverter, stopwatch);

  auto customBeatmap = il2cpp_utils::try_cast<CustomBeatmapData>(beatmap).value_or(nullptr);
  if (!customBeatmap || !customBeatmap->customData->value) {
    return beatmap;
  }

  rapidjson::Value const& customData = customBeatmap->customData->value.value();

  ListW<BeatmapSaveDataVersion3::BpmChangeEventData*> bpmEvents = beatmapSaveData->bpmEvents;
  CustomJSONData::BpmTimeProcessor bpmTimeProcessor(startBpm, bpmEvents);

  auto const BeatToTime = [&bpmTimeProcessor](float beat) {
    auto time = bpmTimeProcessor.ConvertBeatToTime(beat);
    return time;
  };

#define PARSE_ARRAY(key, parse, convert)                                                                               \
  auto key##it = customData.FindMember(#key);                                                                          \
  if (key##it != customData.MemberEnd()) {                                                                             \
    bpmTimeProcessor.Reset();                                                                                          \
    for (auto const& it : key##it->value.GetArray()) {                                                                 \
      auto item = parse(it);                                                                                           \
      auto obj = convert(item);                                                                                        \
      if (!obj) continue;                                                                                              \
      auto& ad = getAD(obj->customData);                                                                               \
      ad.objectData.fake = true;                                                                                       \
      customBeatmap->AddBeatmapObjectDataOverride(obj);                                                                \
    }                                                                                                                  \
  }

  PARSE_ARRAY(fakeColorNotes, Parser::DeserializeColorNote, [&](v3::CustomBeatmapSaveData_ColorNoteData* data) {
    auto rotation = 0;
    return CreateCustomBasicNoteData(BeatToTime(data->b), data->b, rotation, data->line,
                                     ConvertNoteLineLayer(data->layer), ConvertColorType(data->color),
                                     ConvertNoteCutDirection(data->cutDirection), data->customData);
  });
  PARSE_ARRAY(fakeBombNotes, Parser::DeserializeBombNote, [&](v3::CustomBeatmapSaveData_BombNoteData* data) {
    auto rotation = 0;

    return CreateCustomBombNoteData(BeatToTime(data->b), data->b, rotation, data->line,
                                    ConvertNoteLineLayer(data->layer), data->customData);
  });
  PARSE_ARRAY(fakeObstacles, Parser::DeserializeObstacle, [&](v3::CustomBeatmapSaveData_ObstacleData* data) {
    float beat = BeatToTime(data->b);
    auto rotation = 0;
    auto* obstacle =
        CustomObstacleData::New_ctor(beat, data->b, data->b + data->duration, rotation, data->get_line(), GetNoteLineLayer(data->get_layer()),
                                     BeatToTime(data->b + data->duration) - beat, data->width, data->height);

    obstacle->customData = CustomJSONData::JSONWrapperOrNull(data->customData);

    return obstacle;
  });

  PARSE_ARRAY(
      fakeSliders, Parser::DeserializeSlider, [&](v3::CustomBeatmapSaveData_SliderData* data) -> CustomSliderData* {
        auto headRotation = 0;
        auto tailRotation = 0;
        return CustomSliderData_CreateCustomSliderData(
            ConvertColorType(data->get_colorType()), BeatToTime(data->b), data->b, headRotation, data->get_headLine(),
            ConvertNoteLineLayer(data->get_headLayer()), ConvertNoteLineLayer(data->get_headLayer()),
            data->get_headControlPointLengthMultiplier(), ConvertNoteCutDirection(data->get_headCutDirection()),
            BeatToTime(data->get_tailBeat()), tailRotation, data->get_tailLine(),
            ConvertNoteLineLayer(data->get_tailLayer()), ConvertNoteLineLayer(data->get_tailLayer()),
            data->get_tailControlPointLengthMultiplier(), ConvertNoteCutDirection(data->get_tailCutDirection()),
            ConvertSliderMidAnchorMode(data->get_sliderMidAnchorMode()), data->customData);
      });

  PARSE_ARRAY(fakeBurstSliders, Parser::DeserializeBurstSlider,
              [&](v3::CustomBeatmapSaveData_BurstSliderData* data) -> CustomSliderData* {
                auto headRotation = 0;
                auto tailRotation = 0;
                return CustomSliderData_CreateCustomBurstSliderData(
                    ConvertColorType(data->colorType), BeatToTime(data->beat), data->beat, headRotation, data->headLine,
                    ConvertNoteLineLayer(data->headLayer), ConvertNoteLineLayer(data->headLayer),
                    ConvertNoteCutDirection(data->headCutDirection), BeatToTime(data->tailBeat), data->tailBeat,
                    data->tailLine, ConvertNoteLineLayer(data->tailLayer), ConvertNoteLineLayer(data->tailLayer),
                    data->sliceCount, data->squishAmount, data->customData);
              });

  customBeatmap->ProcessRemainingData();
  customBeatmap->ProcessAndSortBeatmapData();

  return customBeatmap;
}

void HandleFakeV3Objects(v3::CustomBeatmapSaveData*) {}
void InstallBeatmapDataHooks(){
  // force CJD to be first, is this needed?
  // Modloader::requireMod("CustomJSONData");
  INSTALL_HOOK(NELogger::Logger, V2_BeatmapDataLoader_GetBeatmapDataBasicInfoFromSaveDataJson)
      INSTALL_HOOK(NELogger::Logger, V3_BeatmapDataLoader_GetBeatmapDataFromSaveData)

  // v3::Parser::ParsedEvent.addCallback(HandleFakeV3Objects);
}

NEInstallHooks(InstallBeatmapDataHooks);