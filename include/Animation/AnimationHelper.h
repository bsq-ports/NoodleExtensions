#pragma once
#include "beatsaber-hook/shared/rapidjson.hpp"

#include "tracks/shared/Animation/Track.h"

#include "tracks/shared/Animation/Animation.h"
#include "AssociatedData.h"
#include "NECaches.h"

namespace AnimationHelper {
struct ObjectOffset {
  std::optional<NEVector::Vector3> positionOffset;
  std::optional<NEVector::Quaternion> rotationOffset;
  std::optional<NEVector::Vector3> scaleOffset;
  std::optional<NEVector::Quaternion> localRotationOffset;
  std::optional<float> dissolve;
  std::optional<float> dissolveArrow;
  std::optional<float> cuttable;
};

std::optional<NEVector::Vector3> GetDefinitePositionOffset(AnimationObjectData const& animationData,
                                                           std::span<TrackW const> tracks, float time);
ObjectOffset GetObjectOffset(AnimationObjectData const& customData, std::span<TrackW const> tracks, float time);

} // end namespace AnimationHelper