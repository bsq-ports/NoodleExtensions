#include "Animation/Events.h"
#include "GlobalNamespace/BeatmapData.hpp"
#include "GlobalNamespace/BeatmapCallbacksController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Vector3.hpp"

#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "custom-json-data/shared/CustomEventData.h"
#include "custom-types/shared/register.hpp"

#include "Animation/ParentObject.h"
#include "Animation/PlayerTrack.h"
#include "AssociatedData.h"
#include "NELogger.h"
#include "tracks/shared/TimeSourceHelper.h"
#include "tracks/shared/Vector.h"
#include "NEHooks.h"

using namespace Events;
using namespace GlobalNamespace;
using namespace TrackParenting;
using namespace NEVector;

void LoadNoodleEvent(TracksAD::BeatmapAssociatedData& beatmapAD, CustomJSONData::CustomEventData const* customEventData,
                     bool v2);

void CustomEventCallback(BeatmapCallbacksController* callbackController,
                         CustomJSONData::CustomEventData* customEventData) {
  PAPER_IL2CPP_CATCH_HANDLER(
      if (!Hooks::isNoodleHookEnabled()) return;

      bool isType = false;

      auto typeHash = customEventData->typeHash;

#define TYPE_GET(jsonName, varName)                                                                                    \
  static auto jsonNameHash_##varName = std::hash<std::string_view>()(jsonName);                                        \
  if (!isType && typeHash == (jsonNameHash_##varName)) isType = true;

      TYPE_GET("AssignTrackParent", AssignTrackParent) TYPE_GET("AssignPlayerToTrack", AssignPlayerToTrack)

          if (!isType) { return; }

      auto const& ad = getEventAD(customEventData);

      // fail safe, idek why this needs to be done smh
      // CJD you bugger
      if (!ad.parsed) {
        auto* customBeatmapData = il2cpp_utils::cast<CustomJSONData::CustomBeatmapData>(callbackController->_beatmapData);
        TracksAD::BeatmapAssociatedData& beatmapAD = TracksAD::getBeatmapAD(customBeatmapData->customData);
        auto v2 = customBeatmapData->v2orEarlier;
        LoadNoodleEvent(beatmapAD, customEventData, v2);
      }

      if (ad.parentTrackEventData) {
        ParentObject::AssignTrack(*ad.parentTrackEventData);
      } else if (ad.playerTrackEventData) {
        PlayerTrack::AssignTrack(ad.playerTrackEventData->track, ad.playerTrackEventData->target.value_or(PlayerTrackObject::Root)); 
      })
}

void NEEvents::AddEventCallbacks() {
  CustomJSONData::CustomEventCallbacks::AddCustomEventCallback(&CustomEventCallback);
  custom_types::Register::AutoRegister();
}
