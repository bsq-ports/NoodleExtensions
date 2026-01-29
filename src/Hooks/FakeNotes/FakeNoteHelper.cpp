#include "FakeNoteHelper.h"

#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/ObstacleController.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

#include "NELogger.h"
#include "AssociatedData.h"

#include "System/Collections/Generic/List_1.hpp"
#include "System/Collections/Generic/IList_1.hpp"
#include "custom-json-data/shared/VList.h"

using namespace GlobalNamespace;

/**
 * @brief Checks if a note is a fake note.
 * 
 * @param noteData The note data to check.
 * @return true if the note is fake, false if the note is not fake.
 */
bool FakeNoteHelper::GetFakeNote(NoteData* noteData) {
  auto customNoteData = il2cpp_utils::try_cast<CustomJSONData::CustomNoteData>(noteData);
  if (!customNoteData || !customNoteData.value()->customData) {
    return false;
  }
  BeatmapObjectAssociatedData& ad = getAD(customNoteData.value()->customData);
  return ad.objectData.fake.value_or(false);
}

bool FakeNoteHelper::GetCuttable(NoteData* noteData) {
  auto customNoteData = il2cpp_utils::try_cast<CustomJSONData::CustomNoteData>(noteData);
  if (!customNoteData || !customNoteData.value()->customData) {
    return true;
  }
  BeatmapObjectAssociatedData& ad = getAD(customNoteData.value()->customData);
  return !ad.objectData.uninteractable.value_or(false);
}

bool FakeNoteHelper::GetAttractableArc(SliderData* arcData) {
  auto customArcData = il2cpp_utils::try_cast<CustomJSONData::CustomSliderData>(arcData);
  if (!customArcData || !customArcData.value()->customData) {
    return true;
  }
  BeatmapObjectAssociatedData& ad = getAD(customArcData.value()->customData);
  return !ad.objectData.uninteractable.value_or(false);
}

System::Collections::Generic::List_1<GlobalNamespace::ObstacleController*>*
FakeNoteHelper::ObstacleFakeCheck(VList<GlobalNamespace::ObstacleController*> intersectingObstacles) {
  //    auto filteredInner = List<GlobalNamespace::ObstacleController*>::New_ctor();

  for (auto const& obstacle : intersectingObstacles) {
    if (!obstacle || !obstacle->obstacleData ||
        il2cpp_utils::AssignableFrom<CustomJSONData::CustomObstacleData*>(obstacle->obstacleData->klass))
      continue;

    auto customData = reinterpret_cast<CustomJSONData::CustomObstacleData*>(obstacle->obstacleData)->customData;

    bool add = !customData || !customData->value;

    if (!add) {
      auto const& ad = getAD(customData);
      add = !ad.objectData.fake.value_or(false);
    }

    if (!add) {
      intersectingObstacles->Remove(obstacle);
    }
  }

  return intersectingObstacles;
}
