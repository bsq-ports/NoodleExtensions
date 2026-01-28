#pragma once

#include "custom-json-data/shared/JSONWrapper.h"
#include "custom-json-data/shared/CustomEventData.h"

#include "tracks/shared/Vector.h"

#include "tracks/shared/Animation/Track.h"
#include "tracks/shared/AssociatedData.h"
#include "tracks/shared/Animation/TransformData.hpp"

#include "Constants.hpp"

#include "GlobalNamespace/CutoutEffect.hpp"
#include "GlobalNamespace/DisappearingArrowControllerBase_1.hpp"
#include "GlobalNamespace/CutoutAnimateEffect.hpp"
#include <optional>

namespace GlobalNamespace {
class CutoutAnimateEffect;
class GameNoteController;
class ConditionalMaterialSwitcher;
} // namespace GlobalNamespace

namespace UnityEngine {
class Renderer;
}

struct AnimationObjectData {
  PointDefinitionW position = PointDefinitionW(nullptr);
  PointDefinitionW rotation = PointDefinitionW(nullptr);
  PointDefinitionW scale = PointDefinitionW(nullptr);
  PointDefinitionW localRotation = PointDefinitionW(nullptr);
  PointDefinitionW dissolve = PointDefinitionW(nullptr);
  PointDefinitionW dissolveArrow = PointDefinitionW(nullptr);
  PointDefinitionW cuttable = PointDefinitionW(nullptr);
  PointDefinitionW definitePosition = PointDefinitionW(nullptr);
  bool parsed = false;

  AnimationObjectData() = default;
  AnimationObjectData(TracksAD::BeatmapAssociatedData& beatmapAD, rapidjson::Value const& animation, bool v2);
};

struct ObjectCustomData {
  std::optional<float> startX;
  std::optional<float> startY;
  std::optional<float> tailStartX;
  std::optional<float> tailStartY;

  std::optional<NEVector::Quaternion> rotation;
  std::optional<NEVector::Quaternion> localRotation;
  std::optional<float> noteJumpMovementSpeed;
  std::optional<float> noteJumpStartBeatOffset;
  std::optional<bool> fake;
  std::optional<bool> uninteractable;

  // notes
  bool disableBadCutDirection;
  bool disableBadCutSaberType;
  bool disableBadCutSpeed;
  std::optional<bool> disableNoteGravity;
  bool disableNoteLook;
  std::optional<std::string> link;

  // obstacles [width, height, length]
  // legacy obstacle scaling
  std::optional<float> width;
  std::optional<float> height;
  std::optional<float> length;

  // new obstacle scaling
  std::optional<float> scaleX;
  std::optional<float> scaleY;
  std::optional<float> scaleZ;

  ObjectCustomData() = default;
  ObjectCustomData(rapidjson::Value const& customData, CustomJSONData::CustomNoteData* noteData,
                   CustomJSONData::CustomObstacleData* obstacleData, bool v2);
};

struct BeatmapObjectAssociatedData {
  BeatmapObjectAssociatedData() = default;
  BeatmapObjectAssociatedData(BeatmapObjectAssociatedData&&) = default;
  [[deprecated("Use default move semantics")]]
  BeatmapObjectAssociatedData(BeatmapObjectAssociatedData const&) = default;

  // Set in NotesInTimeRowProcessor.ProcessAllNotesInTimeRow
  float startNoteLineLayer = 0;
  float tailStartNoteLineLayer = 0;

  NEVector::Quaternion worldRotation;
  NEVector::Quaternion localRotation;
  NEVector::Vector3 moveStartPos;
  NEVector::Vector3 moveEndPos;
  NEVector::Vector3 jumpEndPos;
  NEVector::Vector3 noteOffset;
  NEVector::Vector3 boundsSize; // obstacles
  NEVector::Vector3 internalScale;
  float endRotation;

  float xOffset;
  AnimationObjectData animationData;
  ObjectCustomData objectData;
  bool mirror = true;

  // flip for notes
  std::optional<float> flipX;
  std::optional<float> flipY;

  // hide for obstacles
  bool doUnhide;

  bool parsed = false;

  void ResetState();
};

enum class PlayerTrackObject { Root, Head, LeftHand, RightHand };

struct PlayerTrackEventData {
  explicit PlayerTrackEventData(TrackW track, std::optional<std::string_view> target);

  TrackW track;
  std::optional<PlayerTrackObject> target;
};

struct ParentTrackEventData {
  explicit ParentTrackEventData(rapidjson::Value const& eventData, TracksAD::BeatmapAssociatedData& beatmapAD, bool v2);

  std::vector<TrackW> childrenTracks;
  std::optional<NEVector::Vector3> offsetPosition;
  TrackW parentTrack;
  Tracks::TransformData transformData;
  std::optional<NEVector::Quaternion> worldRotation;
  bool worldPositionStays;

};

struct BeatmapEventAssociatedData {
  // union?
  std::optional<PlayerTrackEventData> playerTrackEventData;
  std::optional<ParentTrackEventData> parentTrackEventData;

  bool parsed = false;
};

BeatmapEventAssociatedData& getEventAD(CustomJSONData::CustomEventData const* customData);
void clearEventADs();

constexpr BeatmapObjectAssociatedData& getAD(CustomJSONData::JSONWrapper* customData) {
  std::any& ad = customData->associatedData['N'];
  if (!ad.has_value()) ad = std::make_any<::BeatmapObjectAssociatedData>();
  return std::any_cast<::BeatmapObjectAssociatedData&>(ad);
}