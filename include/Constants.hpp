#pragma once

#include <string_view>

#include "tracks/shared/AssociatedData.h"

namespace NoodleExtensions::Constants {
using namespace TracksAD::Constants;

inline static constexpr const std::string_view V2_CUT_DIRECTION = "_cutDirection";
inline static constexpr const std::string_view V2_CUTTABLE = "_interactable";
inline static constexpr const std::string_view V2_DEFINITE_POSITION = "_definitePosition";
inline static constexpr const std::string_view V2_DISSOLVE = "_dissolve";
inline static constexpr const std::string_view V2_DISSOLVE_ARROW = "_dissolveArrow";
inline static constexpr const std::string_view V2_FAKE_NOTE = "_fake";
inline static constexpr const std::string_view V2_FLIP = "_flip";
inline static constexpr const std::string_view V2_NOTE_GRAVITY_DISABLE = "_disableNoteGravity";
inline static constexpr const std::string_view V2_NOTE_JUMP_SPEED = "_noteJumpMovementSpeed";
inline static constexpr const std::string_view V2_NOTE_LOOK_DISABLE = "_disableNoteLook";
inline static constexpr const std::string_view V2_NOTE_SPAWN_OFFSET = "_noteJumpStartBeatOffset";
inline static constexpr const std::string_view V2_TIME = "_time";
inline static constexpr const std::string_view V2_WORLD_POSITION_STAYS = "_worldPositionStays";
inline static constexpr const std::string_view V2_PARENT_TRACK = "_parentTrack";
inline static constexpr const std::string_view V2_CHILDREN_TRACKS = "_childrenTracks";
inline static constexpr const std::string_view V2_TARGET = "_target";

inline static constexpr const std::string_view TRAIL_LOCAL_SPACE = "_localSpaceSaberTrail";
inline static constexpr const std::string_view NOTE_OFFSET = "coordinates";
inline static constexpr const std::string_view TAIL_NOTE_OFFSET = "tailCoordinates";
inline static constexpr const std::string_view OBSTACLE_SIZE = "size";
inline static constexpr const std::string_view WORLD_ROTATION = "worldRotation";
inline static constexpr const std::string_view INTERACTABLE = "interactable";
inline static constexpr const std::string_view UNINTERACTABLE = "uninteractable";
inline static constexpr const std::string_view OFFSET_POSITION = "offsetPosition";
inline static constexpr const std::string_view OFFSET_ROTATION = "offsetWorldRotation";
inline static constexpr const std::string_view DEFINITE_POSITION = "definitePosition";
inline static constexpr const std::string_view DISSOLVE = "dissolve";
inline static constexpr const std::string_view DISSOLVE_ARROW = "dissolveArrow";
inline static constexpr const std::string_view FLIP = "flip";
inline static constexpr const std::string_view NOTE_BADCUT_DIRECTION_DISABLE = "disableBadCutDirection";
inline static constexpr const std::string_view NOTE_BADCUT_SABERTYPE_DISABLE = "disableBadCutSaberType";
inline static constexpr const std::string_view NOTE_BADCUT_SPEED_DISABLE = "disableBadCutSpeed";
inline static constexpr const std::string_view NOTE_GRAVITY_DISABLE = "disableNoteGravity";
inline static constexpr const std::string_view NOTE_JUMP_SPEED = "noteJumpMovementSpeed";
inline static constexpr const std::string_view NOTE_LOOK_DISABLE = "disableNoteLook";
inline static constexpr const std::string_view NOTE_SPAWN_OFFSET = "noteJumpStartBeatOffset";
inline static constexpr const std::string_view TIME = "time";
inline static constexpr const std::string_view WORLD_POSITION_STAYS = "worldPositionStays";
inline static constexpr const std::string_view PARENT_TRACK = "parentTrack";
inline static constexpr const std::string_view CHILDREN_TRACKS = "childrenTracks";
inline static constexpr const std::string_view LINK = "link";
inline static constexpr const std::string_view TARGET = "target";

inline static constexpr const std::string_view INTERNAL_STARTNOTELINELAYER = "NE_startNoteLineLayer";
inline static constexpr const std::string_view INTERNAL_TAILSTARTNOTELINELAYER = "NE_tailStartNoteLineLayer";
inline static constexpr const std::string_view INTERNAL_FLIPYSIDE = "NE_flipYSide";
inline static constexpr const std::string_view INTERNAL_FLIPLINEINDEX = "NE_flipLineIndex";
inline static constexpr const std::string_view INTERNAL_FAKE_NOTE = "NE_fake";

inline static constexpr const std::string_view ASSIGN_PLAYER_TO_TRACK = "AssignPlayerToTrack";
inline static constexpr const std::string_view ASSIGN_TRACK_PARENT = "AssignTrackParent";
} // namespace NoodleExtensions::Constants