#include "AssociatedData.h"

#include <utility>
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "tracks/shared/Animation/Animation.h"
#include "NEJSON.h"

using namespace TracksAD;
using namespace NEVector;

AnimationObjectData::AnimationObjectData(BeatmapAssociatedData& beatmapAD, rapidjson::Value const& animation, bool v2)
    : parsed(true) {
  position = beatmapAD.getPointDefinition(
      animation, v2 ? NoodleExtensions::Constants::V2_POSITION : NoodleExtensions::Constants::OFFSET_POSITION,
      Tracks::ffi::WrapBaseValueType::Vec3);
  rotation = beatmapAD.getPointDefinition(
      animation, v2 ? NoodleExtensions::Constants::V2_ROTATION : NoodleExtensions::Constants::OFFSET_ROTATION,
      Tracks::ffi::WrapBaseValueType::Quat);
  scale = beatmapAD.getPointDefinition(animation,
                                       v2 ? NoodleExtensions::Constants::V2_SCALE : NoodleExtensions::Constants::SCALE,
                                       Tracks::ffi::WrapBaseValueType::Vec3);
  localRotation = beatmapAD.getPointDefinition(
      animation, v2 ? NoodleExtensions::Constants::V2_LOCAL_ROTATION : NoodleExtensions::Constants::LOCAL_ROTATION,
      Tracks::ffi::WrapBaseValueType::Quat);
  dissolve = beatmapAD.getPointDefinition(
      animation, v2 ? NoodleExtensions::Constants::V2_DISSOLVE : NoodleExtensions::Constants::DISSOLVE,
      Tracks::ffi::WrapBaseValueType::Float);
  dissolveArrow = beatmapAD.getPointDefinition(
      animation, v2 ? NoodleExtensions::Constants::V2_DISSOLVE_ARROW : NoodleExtensions::Constants::DISSOLVE_ARROW,
      Tracks::ffi::WrapBaseValueType::Float);
  cuttable = beatmapAD.getPointDefinition(
      animation, v2 ? NoodleExtensions::Constants::V2_CUTTABLE : NoodleExtensions::Constants::INTERACTABLE,
      Tracks::ffi::WrapBaseValueType::Float);
  definitePosition = beatmapAD.getPointDefinition(animation,
                                                  v2 ? NoodleExtensions::Constants::V2_DEFINITE_POSITION
                                                     : NoodleExtensions::Constants::DEFINITE_POSITION,
                                                  Tracks::ffi::WrapBaseValueType::Vec3);
}

ObjectCustomData::ObjectCustomData(rapidjson::Value const& customData, CustomJSONData::CustomNoteData* noteData,
                                   CustomJSONData::CustomObstacleData* obstacleData, bool v2) {
  auto [x, y] = NEJSON::ReadOptionalPair(customData, v2 ? NoodleExtensions::Constants::V2_POSITION
                                                        : NoodleExtensions::Constants::NOTE_OFFSET);
  auto [tailX, tailY] = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::TAIL_NOTE_OFFSET);
  startX = x;
  startY = y;

  tailStartX = tailX;
  tailStartY = tailY;

  // TODO: Mirror X

  rotation = NEJSON::ReadOptionalRotation(customData, v2 ? NoodleExtensions::Constants::V2_ROTATION
                                                         : NoodleExtensions::Constants::WORLD_ROTATION);
  localRotation = NEJSON::ReadOptionalRotation(customData, v2 ? NoodleExtensions::Constants::V2_LOCAL_ROTATION
                                                              : NoodleExtensions::Constants::LOCAL_ROTATION);
  noteJumpMovementSpeed = NEJSON::ReadOptionalFloat(customData, v2 ? NoodleExtensions::Constants::V2_NOTE_JUMP_SPEED
                                                                   : NoodleExtensions::Constants::NOTE_JUMP_SPEED);
  noteJumpStartBeatOffset = NEJSON::ReadOptionalFloat(customData, v2 ? NoodleExtensions::Constants::V2_NOTE_SPAWN_OFFSET
                                                                     : NoodleExtensions::Constants::NOTE_SPAWN_OFFSET);
  fake = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_FAKE_NOTE
                                                 : NoodleExtensions::Constants::INTERNAL_FAKE_NOTE);
  uninteractable = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_CUTTABLE
                                                           : NoodleExtensions::Constants::UNINTERACTABLE);

  if (v2 && uninteractable.has_value()) {
    // In v2, uninteractable is the inverse of cuttable
    // only set value if it exists
    uninteractable = !uninteractable.value();
  }

  if (noteData && fake.value_or(false)) {
    noteData->set_scoringType(GlobalNamespace::NoteData::ScoringType::NoScore);
  }

  if (v2) {
    auto cutDirOpt = NEJSON::ReadOptionalFloat(customData, NoodleExtensions::Constants::V2_CUT_DIRECTION);

    if (cutDirOpt) {
      // TODO: MIRROR
      //            noteData->SetCutDirectionAngleOffset(Mirror(*cutDirOpt, leftHanded));
      noteData->SetCutDirectionAngleOffset(*cutDirOpt);
      if (noteData->cutDirection != GlobalNamespace::NoteCutDirection::Any) {
        noteData->ChangeNoteCutDirection(GlobalNamespace::NoteCutDirection::Down);
      }
    }
  }

  // TODO: FLIP X

  if (!v2) {
    disableBadCutDirection = NEJSON::ReadOptionalBool(customData, NoodleExtensions::Constants::NOTE_BADCUT_DIRECTION_DISABLE).value_or(false);
    disableBadCutSaberType = NEJSON::ReadOptionalBool(customData, NoodleExtensions::Constants::NOTE_BADCUT_SABERTYPE_DISABLE).value_or(false);
    disableBadCutSpeed = NEJSON::ReadOptionalBool(customData, NoodleExtensions::Constants::NOTE_BADCUT_SPEED_DISABLE).value_or(false);
  }

  disableNoteGravity = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_NOTE_GRAVITY_DISABLE
                                                               : NoodleExtensions::Constants::NOTE_GRAVITY_DISABLE);
  disableNoteLook = NEJSON::ReadOptionalBool(customData, v2 ? NoodleExtensions::Constants::V2_NOTE_LOOK_DISABLE
                                                            : NoodleExtensions::Constants::NOTE_LOOK_DISABLE)
                        .value_or(false);
  auto legacyScaleArray = NEJSON::ReadOptionalScale(customData, v2 ? NoodleExtensions::Constants::V2_SCALE
                                                                   : NoodleExtensions::Constants::OBSTACLE_SIZE);

  if (legacyScaleArray) {
    width = legacyScaleArray->at(0);
    height = legacyScaleArray->at(1);
    length = legacyScaleArray->at(2);
  }

  if (!v2) {
    auto scale = NEJSON::ReadOptionalScale(customData, NoodleExtensions::Constants::SCALE);

    if (scale) {
      scaleX = scale->at(0);
      scaleY = scale->at(1);
      scaleZ = scale->at(2);
    }
  }
  link = NEJSON::ReadOptionalString(customData, NoodleExtensions::Constants::LINK);

  // TODO: MIRROR WIDTH AND OBSTACLE START X

  //    float width = obstacleData->width;
  //    if (scale) {
  //        width = (*scale)[0].value_or(width);
  //    }
  //
  //    if (startX) {
  //        startX = (*startX + width) * -1;
  //    } else if (scale && (*scale)[0]){
  //        auto lineIndex = obstacleData->lineIndex - 2;
  //        (*scale)[0] = (lineIndex + width) * -1;
  //    }
}

void ::BeatmapObjectAssociatedData::ResetState() {
  parsed = false;
}

ParentTrackEventData::ParentTrackEventData(rapidjson::Value const& eventData, BeatmapAssociatedData& beatmapAD,
                                           bool v2) {
  parentTrack = beatmapAD.getTrack(eventData[v2 ? NoodleExtensions::Constants::V2_PARENT_TRACK.data()
                                                : NoodleExtensions::Constants::PARENT_TRACK.data()]
                                       .GetString());

  rapidjson::Value const& rawChildrenTracks = eventData[v2 ? NoodleExtensions::Constants::V2_CHILDREN_TRACKS.data()
                                                           : NoodleExtensions::Constants::CHILDREN_TRACKS.data()];

  if (rawChildrenTracks.IsArray()) {
    childrenTracks.reserve(rawChildrenTracks.Size());
    for (rapidjson::Value::ConstValueIterator itr = rawChildrenTracks.Begin(); itr != rawChildrenTracks.End(); itr++) {
      TrackW child = beatmapAD.getTrack(itr->GetString());
      // NELogger::Logger.debug("Assigning track {}(%p) to parent track {}(%p)", itr->GetString(), child,
      // eventData["_parentTrack"].GetString(), track);
      childrenTracks.emplace_back(child);
    }
  } else {
    childrenTracks = { beatmapAD.getTrack(rawChildrenTracks.GetString()) };
  }

  offsetPosition = NEJSON::ReadOptionalVector3(eventData, v2 ? NoodleExtensions::Constants::V2_POSITION
                                                             : NoodleExtensions::Constants::OFFSET_POSITION);
  worldRotation = NEJSON::ReadOptionalRotation(eventData, v2 ? NoodleExtensions::Constants::V2_ROTATION
                                                             : NoodleExtensions::Constants::WORLD_ROTATION);
  transformData = Tracks::TransformData(eventData, v2);

  worldPositionStays = NEJSON::ReadOptionalBool(eventData, v2 ? NoodleExtensions::Constants::V2_WORLD_POSITION_STAYS
                                                              : NoodleExtensions::Constants::WORLD_POSITION_STAYS)
                           .value_or(false);
}

PlayerTrackEventData::PlayerTrackEventData(TrackW track, std::optional<std::string_view> targetOpt) : track(track) {
  if (targetOpt) {
    auto targetStr = *targetOpt;

    if (targetStr == "Root") {
      this->target = PlayerTrackObject::Root;
    }
    if (targetStr == "Head") {
      this->target = PlayerTrackObject::Head;
    }
    if (targetStr == "LeftHand") {
      this->target = PlayerTrackObject::LeftHand;
    }
    if (targetStr == "RightHand") {
      this->target = PlayerTrackObject::RightHand;
    }
  }
}

static std::unordered_map<CustomJSONData::CustomEventData const*, BeatmapEventAssociatedData> eventDataMap;
static std::unordered_map<GlobalNamespace::BeatmapObjectData const*, ::BeatmapObjectAssociatedData> obstacleDataMap;

::BeatmapEventAssociatedData& getEventAD(CustomJSONData::CustomEventData const* customData) {
  return eventDataMap[customData];
}

void clearEventADs() {
  eventDataMap.clear();
}

::BeatmapObjectAssociatedData& getAD(GlobalNamespace::BeatmapObjectData* objectData) {
  return obstacleDataMap[objectData];
}

void clearObjectADs() {
  obstacleDataMap.clear();
}
