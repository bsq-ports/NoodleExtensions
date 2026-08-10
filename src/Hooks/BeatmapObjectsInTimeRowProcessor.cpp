#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"

#include "GlobalNamespace/BeatmapObjectsInTimeRowProcessor.hpp"
#include "GlobalNamespace/StaticBeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/NoteCutDirectionExtensions.hpp"
#include "GlobalNamespace/Vector2Extensions.hpp"

#include "AssociatedData.h"
#include "NEHooks.h"
#include "custom-json-data/shared/VList.h"

#include "NEUtils.hpp"
#include "tracks/shared/Json.h"

#include "Constants.hpp"

using namespace GlobalNamespace;
using namespace CustomJSONData;
using namespace NoodleExtensions;

#include <cmath>

void BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceAllNotesAndSlidersDidFinishTimeSliceTranspile(
    BeatmapObjectsInTimeRowProcessor* self,
    GlobalNamespace::BeatmapObjectsInTimeRowProcessor::TimeSliceContainer_1<::GlobalNamespace::BeatmapDataItem*>*
        allObjectsTimeSlice,
    float nextTimeSliceTime) {

  auto notesInColumnsReusableProcessingListOfLists = self->_notesInColumnsReusableProcessingListOfLists;
  for (auto const& l : notesInColumnsReusableProcessingListOfLists) {
    l->Clear();
  }

  auto items = ListW<GlobalNamespace::BeatmapDataItem*>(allObjectsTimeSlice->_items);

  auto enumerable = NoodleExtensions::of_type<NoteData*>(items);
  auto enumerable2 = NoodleExtensions::of_type<SliderData*>(items);
  auto enumerable3 = NoodleExtensions::of_type<BeatmapObjectsInTimeRowProcessor::SliderTailData*>(items);
  for (auto noteData : enumerable) {

    // TRANSPILE HERE
    // CLAMP
    auto list = ListW<GlobalNamespace::NoteData*>(self->_notesInColumnsReusableProcessingListOfLists[std::clamp(noteData->lineIndex, 0, 3)]);
    // TRANSPILE HERE

    bool flag = false;

    for (int j = 0; j < list.size(); j++) {
      // TODO: Find why null
      if (!list[j]) continue;

      if (list[j]->noteLineLayer > noteData->noteLineLayer) {
        list.insert_at(j, noteData);
        flag = true;
        break;
      }
    }
    if (!flag) {
      list.push_back(noteData);
    }
  }
  for (auto const& notesInColumnsReusableProcessingListOfList : self->_notesInColumnsReusableProcessingListOfLists) {
    auto list2 = ListW<GlobalNamespace::NoteData*>(notesInColumnsReusableProcessingListOfList);
    for (int l = 0; l < list2.size(); l++) {
      if (!list2[l]) continue;
      
      list2[l]->SetBeforeJumpNoteLineLayer((NoteLineLayer)l);
    }
  }
  /// TRANSPILE HERE
  return;
  ///
}

MAKE_HOOK_MATCH(
    BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceAllNotesAndSlidersDidFinishTimeSlice,
    &BeatmapObjectsInTimeRowProcessor::HandleCurrentTimeSliceAllNotesAndSlidersDidFinishTimeSlice, void,
    BeatmapObjectsInTimeRowProcessor* self,
    GlobalNamespace::BeatmapObjectsInTimeRowProcessor::TimeSliceContainer_1<::GlobalNamespace::BeatmapDataItem*>*
        allObjectsTimeSlice,
    float nextTimeSliceTime) {
  if (!Hooks::isNoodleHookEnabled())
    return BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceAllNotesAndSlidersDidFinishTimeSlice(
        self, allObjectsTimeSlice, nextTimeSliceTime);

  auto items = allObjectsTimeSlice->items;

  std::vector<CustomNoteData*> customNotes = NoodleExtensions::of_type<CustomNoteData*>(ListW<GlobalNamespace::BeatmapDataItem*>(items));

  if (customNotes.empty())
    return BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceAllNotesAndSlidersDidFinishTimeSliceTranspile(
        self, allObjectsTimeSlice, nextTimeSliceTime);

  float offset = self->_numberOfLines / 2.0f;

  std::unordered_map<float, std::vector<CustomNoteData*>> notesInColumn;
  for (auto noteData : customNotes) {
    if (!noteData) continue;
    float lineIndex = noteData->lineIndex;
    float lineLayer = noteData->noteLineLayer.value__;

    if (noteData->customData->value) {
      rapidjson::Value const& customData = *noteData->customData->value;

      auto pos = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::V2_POSITION.data());
      if (!pos.first) pos = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::NOTE_OFFSET.data());
      if (pos.first) lineIndex = pos.first.value() + offset;

      lineLayer = pos.second.value_or(lineLayer);
    }

    std::vector<CustomNoteData*>& list = notesInColumn[lineIndex];

    bool flag = false;
    for (int k = 0; k < list.size(); k++) {
      float listLineLayer = list[k]->noteLineLayer.value__;
      auto kNote = list[k];

      if (kNote->customData->value) {
        rapidjson::Value const& customData = *kNote->customData->value;

        auto pos = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::V2_POSITION.data());
        if (!pos.first) pos = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::NOTE_OFFSET.data());

        listLineLayer = pos.second.value_or(listLineLayer);
      }

      if (listLineLayer > lineLayer) {
        list.insert(list.begin() + k, noteData);
        flag = true;
        break;
      }
    }

    if (!flag) {
      list.push_back(noteData);
    }

    BeatmapObjectAssociatedData& ad = getAD(noteData->customData);
    if (noteData->customData->value) {
      rapidjson::Value const& customData = *noteData->customData->value;

      auto pair = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::V2_FLIP);

      if (!pair.first) {
        pair = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::FLIP);
      }

      auto [x, y] = pair;

      ad.flipX = x;
      if (ad.flipX) {
        *ad.flipX += offset;
      }
      ad.flipY = y;
    }

    if (!ad.flipY) {
      ad.flipX = lineIndex;
      ad.flipY = 0;
    }
  }

  for (auto& [_, list] : notesInColumn) {
    for (int m = 0; m < list.size(); m++) {
      BeatmapObjectAssociatedData& ad = getAD(list[m]->customData);
      ad.startNoteLineLayer = (float)m;
    }
  }

  auto customSliders = NoodleExtensions::of_type<CustomSliderData*>(ListW<GlobalNamespace::BeatmapDataItem*>(items));
  auto customTails = NoodleExtensions::of_type<BeatmapObjectsInTimeRowProcessor::SliderTailData*>(
      ListW<GlobalNamespace::BeatmapDataItem*>(items));
  for (auto slider : customSliders) {
    NEJSON::OptPair headPos = NEJSON::OptPair(std::nullopt, std::nullopt);

    if (slider->customData->value) {
      rapidjson::Value const& sliderCustomData = *slider->customData->value;
      headPos = NEJSON::ReadOptionalPair(sliderCustomData, NoodleExtensions::Constants::NOTE_OFFSET.data());
    }

    float headX = headPos.first.value_or(slider->headLineIndex - offset) + offset;
    float headY = headPos.second.value_or(slider->headLineLayer.value__);

    for (auto note : customNotes) {
      NEJSON::OptPair notePos = NEJSON::OptPair(std::nullopt, std::nullopt);

      if (note->customData->value) {
        rapidjson::Value const& noteCustomData = *note->customData->value;

        notePos = NEJSON::ReadOptionalPair(noteCustomData, NoodleExtensions::Constants::V2_POSITION.data());
        if (!notePos.first) {
          notePos = NEJSON::ReadOptionalPair(noteCustomData, NoodleExtensions::Constants::NOTE_OFFSET.data());
        }
      }

      float noteX = notePos.first.value_or(note->lineIndex - offset) + offset;
      float noteY = notePos.second.value_or(note->noteLineLayer.value__);

      if (Approximately(headX, noteX) && Approximately(headY, noteY)) {
        slider->SetHasHeadNote(true);

        float startNoteLineLayer = note->beforeJumpNoteLineLayer.value__;
        if (note->customData->value) {
          startNoteLineLayer = getAD(note->customData).startNoteLineLayer;
        }
        if (slider->customData->value) {
          BeatmapObjectAssociatedData& ad = getAD(slider->customData);
          ad.startNoteLineLayer = startNoteLineLayer;
        } else {
          slider->SetHeadBeforeJumpLineLayer(startNoteLineLayer);
        }

        if (slider->sliderType == CustomSliderData::Type::Burst) {
          note->ChangeToBurstSliderHead();

          // PC logic, I've no idea why it's there

          // if (noteData.cutDirection != sliderData.tailCutDirection)
          // {
          //     continue;
          // }

          // Vector2 line = SpawnDataManager.Get2DNoteOffset(noteX, ____numberOfLines, noteY) -
          //               SpawnDataManager.Get2DNoteOffset(tailX, ____numberOfLines, tailY);
          // float num = noteData.cutDirection.Direction().SignedAngleToLine(line);
          // if (!(Mathf.Abs(num) <= 40f))
          // {
          //     continue;
          // }

          // noteData.SetCutDirectionAngleOffset(num);
          // sliderData.SetCutDirectionAngleOffset(num, num);
        } else {
          note->MarkAsSliderHead();
        }
      }
    }

    for (auto tailSlider : customSliders) {
      if (slider == tailSlider || slider->sliderType != CustomSliderData::Type::Normal ||
          tailSlider->sliderType != CustomSliderData::Type::Burst)
        continue;

      NEJSON::OptPair tailPos = NEJSON::OptPair(std::nullopt, std::nullopt);

      if (tailSlider->customData->value) {
        rapidjson::Value const& tailSliderCustomData = *tailSlider->customData->value;

        tailPos = NEJSON::ReadOptionalPair(tailSliderCustomData, NoodleExtensions::Constants::TAIL_NOTE_OFFSET.data());
      }
      float tailX = tailPos.first.value_or(tailSlider->tailLineIndex - offset) + offset;
      float tailY = tailPos.second.value_or(tailSlider->tailLineLayer.value__);

      if (Approximately(tailX, headX) && Approximately(tailY, headY)) {
        slider->SetHasHeadNote(true);
        float startNoteLineLayer = tailSlider->tailBeforeJumpLineLayer.value__;
        if (slider->customData->value) {
          BeatmapObjectAssociatedData& ad = getAD(slider->customData);
          ad.tailStartNoteLineLayer = startNoteLineLayer;
        } else {
          slider->SetTailBeforeJumpLineLayer(startNoteLineLayer);
        }
      }
    }

    for (auto tailSliderData : customTails) {
      auto tailSlider = reinterpret_cast<CustomSliderData*>(tailSliderData->slider);
      if (!tailSlider || slider->sliderType != CustomSliderData::Type::Normal ||
          tailSlider->sliderType != CustomSliderData::Type::Burst)
        continue;

      NEJSON::OptPair tailPos = NEJSON::OptPair(std::nullopt, std::nullopt);

      if (tailSlider->customData->value) {
        rapidjson::Value const& tailSliderCustomData = *tailSlider->customData->value;

        tailPos = NEJSON::ReadOptionalPair(tailSliderCustomData, NoodleExtensions::Constants::TAIL_NOTE_OFFSET.data());
      }
      float tailX = tailPos.first.value_or(tailSlider->tailLineIndex - offset) + offset;
      float tailY = tailPos.second.value_or(tailSlider->tailLineLayer.value__);

      if (Approximately(tailX, headX) && Approximately(tailY, headY)) {
        slider->SetHasHeadNote(true);
        float startNoteLineLayer = tailSlider->tailBeforeJumpLineLayer.value__;
        if (slider->customData->value) {
          BeatmapObjectAssociatedData& ad = getAD(slider->customData);
          ad.tailStartNoteLineLayer = startNoteLineLayer;
        } else {
          slider->SetTailBeforeJumpLineLayer(startNoteLineLayer);
        }
      }
    }
  }

  for (auto customTail : customTails) {
    auto slider = reinterpret_cast<CustomSliderData*>(customTail->slider);

    if (!slider) continue;

    NEJSON::OptPair tailPos = NEJSON::OptPair(std::nullopt, std::nullopt);
    if (slider->customData->value) {
      rapidjson::Value const& sliderCustomData = *slider->customData->value;

      tailPos = NEJSON::ReadOptionalPair(sliderCustomData, NoodleExtensions::Constants::TAIL_NOTE_OFFSET.data());
    }
    float tailX = tailPos.first.value_or(slider->tailLineIndex - offset) + offset;
    float tailY = tailPos.second.value_or(slider->tailLineLayer.value__);

    for (auto note : customNotes) {
      NEJSON::OptPair notePos = NEJSON::OptPair(std::nullopt, std::nullopt);

      if (note->customData->value) {
        rapidjson::Value const& noteCustomData = *note->customData->value;

        notePos = NEJSON::ReadOptionalPair(noteCustomData, NoodleExtensions::Constants::V2_POSITION.data());
        if (!notePos.first)
          notePos = NEJSON::ReadOptionalPair(noteCustomData, NoodleExtensions::Constants::NOTE_OFFSET.data());
      }

      float noteX = notePos.first.value_or(note->lineIndex - offset) + offset;
      float noteY = notePos.second.value_or(note->noteLineLayer.value__);

      if (Approximately(tailX, noteX) && Approximately(tailY, noteY)) {
        slider->SetHasTailNote(true);
        note->MarkAsSliderTail();
        float startNoteLineLayer = note->beforeJumpNoteLineLayer.value__;
        if (slider->customData->value) {
          BeatmapObjectAssociatedData& ad = getAD(slider->customData);
          ad.tailStartNoteLineLayer = startNoteLineLayer;
        } else {
          slider->SetTailBeforeJumpLineLayer(startNoteLineLayer);
        }
      }
    }
  }

  return BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceAllNotesAndSlidersDidFinishTimeSliceTranspile(
      self, allObjectsTimeSlice, nextTimeSliceTime);
}

MAKE_HOOK_MATCH(BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceColorNotesDidFinishTimeSlice,
                &BeatmapObjectsInTimeRowProcessor::HandleCurrentTimeSliceColorNotesDidFinishTimeSlice, void,
                BeatmapObjectsInTimeRowProcessor* self,
                GlobalNamespace::BeatmapObjectsInTimeRowProcessor::TimeSliceContainer_1<::GlobalNamespace::NoteData*>*
                    currentTimeSlice,
                float nextTimeSliceTime) {
  if (!Hooks::isNoodleHookEnabled())
    return BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceColorNotesDidFinishTimeSlice(self, currentTimeSlice,
                                                                                               nextTimeSliceTime);

  // TODO: Implement this properly
  
  auto items = ListW<GlobalNamespace::NoteData*>(currentTimeSlice->items);

  auto colorNotesData = NoodleExtensions::of_type<CustomNoteData*>(items);

  int const customNoteCount = colorNotesData.size();

  float offset = self->_numberOfLines / 2.0f;

  if (customNoteCount == 2) {
    std::array<float, 2> lineIndexes{}, lineLayers{};

    for (int i = 0; i < customNoteCount; i++) {
      CustomNoteData* noteData = colorNotesData[i];

      float lineIndex = noteData->lineIndex;
      float lineLayer = noteData->noteLineLayer.value__;
      if (noteData->customData->value) {
        rapidjson::Value const& customData = *noteData->customData->value;
        auto pos = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::V2_POSITION.data());
        if (!pos.first) pos = NEJSON::ReadOptionalPair(customData, NoodleExtensions::Constants::NOTE_OFFSET.data());
        if (pos.first) lineIndex = pos.first.value() + offset;

        lineLayer = pos.second.value_or(lineLayer);
      }

      lineIndexes[i] = lineIndex;
      lineLayers[i] = lineLayer;
    }

    auto firstNote = colorNotesData[0];
    auto secondNote = colorNotesData[1];

    if (firstNote->colorType != secondNote->colorType &&
        ((firstNote->colorType == ColorType::ColorA && lineIndexes[0] > lineIndexes[1]) ||
         (firstNote->colorType == ColorType::ColorB && lineIndexes[0] < lineIndexes[1]))) {
      for (int i = 0; i < customNoteCount; i++) {
        // god aero I hate the mess of code you've made

        auto noteData = colorNotesData[i];

        NEVector::Vector2 flipVec;

        flipVec.x = lineIndexes[1 - i];

        float flipYSide = (lineIndexes[i] > lineIndexes[1 - i]) ? 1 : -1;
        if ((lineIndexes[i] > lineIndexes[1 - i] && lineLayers[i] < lineLayers[1 - i]) ||
            (lineIndexes[i] < lineIndexes[1 - i] && lineLayers[i] > lineLayers[1 - i])) {
          flipYSide *= -1.0f;
        }

        flipVec.y = flipYSide;

        auto& noteAD = getAD(noteData->customData);
        noteAD.flipX = flipVec.x;
        noteAD.flipY = flipVec.y;
      }
    }
  }

  BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceColorNotesDidFinishTimeSlice(self, currentTimeSlice,
                                                                                      nextTimeSliceTime);
}

void InstallBeatmapObjectsInTimeRowProcessorHooks() {
  INSTALL_HOOK(NELogger::Logger, BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceColorNotesDidFinishTimeSlice);

  INSTALL_HOOK(NELogger::Logger, BeatmapObjectsInTimeRowProcessor_HandleCurrentTimeSliceAllNotesAndSlidersDidFinishTimeSlice)
}

NEInstallHooks(InstallBeatmapObjectsInTimeRowProcessorHooks);