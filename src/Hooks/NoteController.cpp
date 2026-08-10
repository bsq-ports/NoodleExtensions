#include "NELogger.h"
#include "VariableMovementHelper.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"

#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/MirroredGameNoteController.hpp"
#include "GlobalNamespace/NoteFloorMovement.hpp"
#include "GlobalNamespace/NoteJump.hpp"
#include "GlobalNamespace/NoteCutInfo.hpp"
#include "GlobalNamespace/NoteMovement.hpp"
#include "GlobalNamespace/BaseNoteVisuals.hpp"
#include "GlobalNamespace/CutoutAnimateEffect.hpp"
#include "GlobalNamespace/CutoutEffect.hpp"
#include "GlobalNamespace/DisappearingArrowControllerBase_1.hpp"
#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/BurstSliderGameNoteController.hpp"
#include "GlobalNamespace/BombNoteController.hpp"
#include "GlobalNamespace/ConditionalMaterialSwitcher.hpp"
#include "GlobalNamespace/BoxCuttableBySaber.hpp"
#include "UnityEngine/Renderer.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"

#include "NEConfig.h"
#include "NEUtils.hpp"
#include "Animation/AnimationHelper.h"
#include "Animation/ParentObject.h"
#include "tracks/shared/TimeSourceHelper.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "NECaches.h"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "custom-json-data/shared/JsonUtils.h"

#include "sombrero/shared/linq_functional.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace TrackParenting;

BeatmapObjectAssociatedData* noteUpdateAD = nullptr;
TracksAD::TracksVector noteTracks;

std::unordered_map<std::string, std::unordered_set<NoteController*>> linkedNotes;
std::unordered_map<NoteController*, std::unordered_set<NoteController*>*> linkedLinkedNotes;

CutoutEffect* NECaches::GetCutout(GlobalNamespace::NoteControllerBase* nc, NECaches::NoteCache& noteCache) {
  CutoutEffect*& cutoutEffect = noteCache.cutoutEffect;

  if (cutoutEffect) return cutoutEffect;

  noteCache.baseNoteVisuals = noteCache.baseNoteVisuals ?: nc->get_gameObject()->GetComponent<BaseNoteVisuals*>();
  CutoutAnimateEffect* cutoutAnimateEffect = noteCache.baseNoteVisuals->_cutoutAnimateEffect;
  ArrayW<UnityW<CutoutEffect>> cuttoutEffects = cutoutAnimateEffect->_cuttoutEffects;
  for (CutoutEffect* effect : cuttoutEffects) {
    if (effect->get_name() != u"NoteArrow") {
      cutoutEffect = effect;
      break;
    }
  }

  return cutoutEffect;
}

GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::GameNoteController*>*
NECaches::GetDisappearingArrowController(GlobalNamespace::GameNoteController* nc, NECaches::NoteCache& noteCache) {
  auto& disappearingArrowController = noteCache.disappearingArrowController;
  if (!disappearingArrowController) {
    disappearingArrowController =
        nc->get_gameObject()->GetComponent<DisappearingArrowControllerBase_1<GameNoteController*>*>();
  }

  return disappearingArrowController;
}

GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::MirroredGameNoteController*>*
NECaches::GetDisappearingArrowController(GlobalNamespace::MirroredGameNoteController* nc,
                                         NECaches::NoteCache& noteCache) {
  auto& disappearingArrowController = noteCache.mirroredDisappearingArrowController;
  if (!disappearingArrowController) {
    disappearingArrowController =
        nc->get_gameObject()->GetComponent<DisappearingArrowControllerBase_1<MirroredGameNoteController*>*>();
  }

  return disappearingArrowController;
}

float noteTimeAdjust(float original, float jumpDuration) {
  if (noteTracks.empty()) return original;

  auto time = NoodleExtensions::getTimeProp(noteTracks);

  if (time) {
    return *time * jumpDuration;
  }

  return original;
}

void NECaches::ClearNoteCaches() {
  NECaches::noteCache.clear();
  noteUpdateAD = nullptr;
  noteTracks.clear();
  linkedNotes.clear();
  linkedLinkedNotes.clear();
}

MAKE_HOOK_MATCH(NoteController_Init, &NoteController::Init, void, NoteController* self,
                GlobalNamespace::NoteData* noteData, ByRef<GlobalNamespace::NoteSpawnData> noteSpawnData,
                float_t endRotation, float_t uniformScale, bool rotateTowardsPlayer, bool useRandomRotation) {

  NoteController_Init(self, noteData, noteSpawnData, endRotation, uniformScale, rotateTowardsPlayer, useRandomRotation);

  if (!Hooks::isNoodleHookEnabled()) return;

  static auto CustomKlass = classof(CustomJSONData::CustomNoteData*);

  if (noteData->klass != CustomKlass) return;

  auto* customNoteData = reinterpret_cast<CustomJSONData::CustomNoteData*>(noteData);

  Transform* transform = self->get_transform();
  transform->set_localScale(NEVector::Vector3::one()); // This is a fix for animation due to notes being
  // recycled

  if (!customNoteData->customData) return;
  BeatmapObjectAssociatedData& ad = getAD(customNoteData->customData);

  if (!ad.parsed) return;

  auto link = ad.objectData.link;
  if (link) {
    auto& list = linkedNotes[*link];
    list.emplace(self);
    linkedLinkedNotes[self] = &list;
  }

  // TRANSPILERS SUCK!
  auto flipYSide = ad.flipY ? *ad.flipY : customNoteData->flipYSide;

  if (flipYSide > 0.0f) {
    self->_noteMovement->_jump->_yAvoidance = flipYSide * self->_noteMovement->_jump->_yAvoidanceUp;
  } else {
    self->_noteMovement->_jump->_yAvoidance = flipYSide * self->_noteMovement->_jump->_yAvoidanceDown;
  }

  auto& noteCache = NECaches::getNoteCache(self);

  // TODO: reimplement material switching
  ArrayW<ConditionalMaterialSwitcher*>& materialSwitchers = noteCache.conditionalMaterialSwitchers;
  if (!materialSwitchers) {
    materialSwitchers = self->GetComponentsInChildren<ConditionalMaterialSwitcher*>();
  }

  for (auto* materialSwitcher : materialSwitchers) {
    materialSwitcher->_renderer->set_sharedMaterial(materialSwitcher->_material0);
  }
  noteCache.dissolveEnabled = false;

  NoteJump* noteJump = self->_noteMovement->_jump;
  NoteFloorMovement* floorMovement = self->_noteMovement->_floorMovement;

  NEVector::Quaternion localRotation = NEVector::Quaternion::identity();
  if (ad.objectData.rotation || ad.objectData.localRotation) {
    if (ad.objectData.localRotation) {
      localRotation = *ad.objectData.localRotation;
    }

    if (ad.objectData.rotation) {
      NEVector::Quaternion worldRotationQuatnerion = *ad.objectData.rotation;

      NEVector::Quaternion inverseWorldRotation = NEVector::Quaternion::Inverse(worldRotationQuatnerion);
      noteJump->_worldRotation = worldRotationQuatnerion;
      noteJump->_inverseWorldRotation = inverseWorldRotation;
      floorMovement->_worldRotation = worldRotationQuatnerion;
      floorMovement->_inverseWorldRotation = inverseWorldRotation;

      worldRotationQuatnerion = worldRotationQuatnerion * localRotation;
      transform->set_localRotation(worldRotationQuatnerion);
    } else {
      transform->set_localRotation(NEVector::Quaternion(transform->get_localRotation()) * localRotation);
    }
  }

  auto scale = NEVector::Vector3(ad.objectData.scaleX.value_or(1.0f), ad.objectData.scaleY.value_or(1.0f),
                                 ad.objectData.scaleZ.value_or(1.0f));
  transform->set_localScale(scale);
  ad.internalScale = scale;

  Vector3 moveStartPos = noteSpawnData->moveStartOffset;
  Vector3 moveEndPos = noteSpawnData->moveEndOffset;
  Vector3 jumpEndPos = noteSpawnData->jumpEndOffset;
  auto movement = VariableMovementW(self->_noteMovement->_variableMovementDataProvider);
  float jumpGravity = movement.CalculateCurrentNoteJumpGravity(noteSpawnData->gravityBase);
  float halfJumpDuration = movement.halfJumpDuration;

  float zOffset = self->_noteMovement->_zOffset;
  moveStartPos.z += zOffset;
  moveEndPos.z += zOffset;
  jumpEndPos.z += zOffset;

  ad.endRotation = endRotation;
  ad.moveStartPos = moveStartPos;
  ad.moveEndPos = moveEndPos;
  ad.jumpEndPos = jumpEndPos;
  ad.worldRotation = self->get_worldRotation();
  ad.localRotation = localRotation;

  float startVerticalVelocity = jumpGravity * halfJumpDuration;
  float yOffset =
      (startVerticalVelocity * halfJumpDuration) - (jumpGravity * halfJumpDuration * halfJumpDuration * 0.5f);
  ad.noteOffset = Vector3(jumpEndPos.x, moveEndPos.y + yOffset, 0);

  self->Update();
}

MAKE_HOOK_MATCH(NoteController_ManualUpdate, &NoteController::ManualUpdate, void, NoteController* self) {

  if (!Hooks::isNoodleHookEnabled()) return NoteController_ManualUpdate(self);

  noteUpdateAD = nullptr;
  noteTracks.clear();

  static auto CustomKlass = classof(CustomJSONData::CustomNoteData*);

  if (self->_noteData->klass != CustomKlass) return NoteController_ManualUpdate(self);

  auto* customNoteData = reinterpret_cast<CustomJSONData::CustomNoteData*>(self->_noteData);
  if (!customNoteData->customData) {
    noteUpdateAD = nullptr;
    noteTracks.clear();
    return NoteController_ManualUpdate(self);
  }

  // TODO: Cache deserialized animation data
  // if (!customData.HasMember("_animation")) {
  //     NoteController_Update(self);
  //     return;
  // }

  BeatmapObjectAssociatedData& ad = getAD(customNoteData->customData);
  auto const& tracks = TracksAD::getAD(customNoteData->customData).tracks;

  noteUpdateAD = &ad;
  noteTracks = tracks;
  if (noteTracks.empty() && !ad.animationData.parsed) {
    return NoteController_ManualUpdate(self);
  }

  NoteJump* noteJump = self->_noteMovement->_jump;
  NoteFloorMovement* floorMovement = self->_noteMovement->_floorMovement;
  VariableMovementW variableMovementDataProvider = self->_noteMovement->_variableMovementDataProvider;

  auto time = NoodleExtensions::getTimeProp(noteTracks);
  float normalTime;
  if (time) {
    normalTime = time.value();
  } else {
    float jumpDuration = variableMovementDataProvider.jumpDuration;
    float elapsedTime = TimeSourceHelper::getSongTime(noteJump->_audioTimeSyncController) -
                        (customNoteData->_time_k__BackingField - (jumpDuration * 0.5f));
    normalTime = elapsedTime / jumpDuration;
  }

  // auto context = TracksAD::getBeatmapAD(NECaches::customBeatmapData->customData).internal_tracks_context;
  AnimationHelper::ObjectOffset offset = AnimationHelper::GetObjectOffset(ad.animationData, noteTracks, normalTime);

  if (offset.positionOffset.has_value()) {
    auto const& offsetPos = *offset.positionOffset;
    floorMovement->_moveStartOffset = ad.moveStartPos + offsetPos;
    floorMovement->_moveEndOffset = ad.moveEndPos + offsetPos;
    noteJump->_startOffset = ad.moveEndPos + offsetPos;
    noteJump->_endOffset = ad.jumpEndPos + offsetPos;
    noteJump->_startPos = NEVector::Vector3(variableMovementDataProvider.moveEndPosition) + noteJump->_startOffset;
    noteJump->_endPos = NEVector::Vector3(variableMovementDataProvider.jumpEndPosition) + noteJump->_endOffset;
  }

  auto transform = self->get_transform();

  if (offset.scaleOffset.has_value()) {
    transform->set_localScale(*offset.scaleOffset * ad.internalScale);
  }

  if (offset.rotationOffset.has_value() || offset.localRotationOffset.has_value()) {
    NEVector::Quaternion worldRotation = ad.worldRotation;
    NEVector::Quaternion localRotation = ad.localRotation;

    NEVector::Quaternion worldRotationQuaternion = worldRotation;
    if (offset.rotationOffset.has_value()) {
      worldRotationQuaternion = worldRotationQuaternion * *offset.rotationOffset;
      NEVector::Quaternion inverseWorldRotation = NEVector::Quaternion::Inverse(worldRotationQuaternion);
      noteJump->_worldRotation = worldRotationQuaternion;
      noteJump->_inverseWorldRotation = inverseWorldRotation;
      floorMovement->_worldRotation = worldRotationQuaternion;
      floorMovement->_inverseWorldRotation = inverseWorldRotation;
    }

    worldRotationQuaternion = worldRotationQuaternion * localRotation;

    if (offset.localRotationOffset.has_value()) {
      worldRotationQuaternion = worldRotationQuaternion * *offset.localRotationOffset;
    }

    transform->set_localRotation(worldRotationQuaternion);
  }

  auto& noteCache = NECaches::getNoteCache(self);

  bool noteDissolveConfig = true; // getNEConfig().enableNoteDissolve.GetValue();
  bool hasDissolveOffset = offset.dissolve.has_value() || offset.dissolveArrow.has_value();
  bool isDissolving = offset.dissolve.value_or(0) > 0 || offset.dissolveArrow.value_or(0) > 0;
  if (hasDissolveOffset && noteCache.dissolveEnabled != isDissolving && noteDissolveConfig) {
    // TODO: reimplement material switching
    ArrayW<ConditionalMaterialSwitcher*> materialSwitchers = noteCache.conditionalMaterialSwitchers;
    for (auto* materialSwitcher : materialSwitchers) {
      materialSwitcher->_renderer->set_sharedMaterial(isDissolving ? materialSwitcher->_material1
                                                                   : materialSwitcher->_material0);
    }
    noteCache.dissolveEnabled = isDissolving;
  }

  if (offset.dissolve.has_value()) {
    CutoutEffect* cutoutEffect = NECaches::GetCutout(self, noteCache);
    CRASH_UNLESS(cutoutEffect);

    if (noteDissolveConfig) {
      cutoutEffect->SetCutout(1 - *offset.dissolve);
    } else {
      cutoutEffect->SetCutout(*offset.dissolve >= 0 ? 0 : 1);
    }
  }

  static auto* gameNoteControllerClass = classof(GameNoteController*);
  static auto* bombNoteControllerClass = classof(BombNoteController*);

  if (il2cpp_functions::class_is_assignable_from(gameNoteControllerClass, self->klass)) {
    if (offset.dissolveArrow.has_value() && self->_noteData->colorType != ColorType::None) {
      auto disappearingArrowController = NECaches::GetDisappearingArrowController((GameNoteController*)self, noteCache);
      if (noteDissolveConfig) {
        disappearingArrowController->SetArrowTransparency(*offset.dissolveArrow);
      } else {
        disappearingArrowController->SetArrowTransparency(*offset.dissolveArrow >= 0 ? 1 : 0);
      }
    }
  }

  if (il2cpp_functions::class_is_assignable_from(gameNoteControllerClass, self->klass) ||
      il2cpp_functions::class_is_assignable_from(bombNoteControllerClass, self->klass)) {
    if (offset.cuttable.has_value()) {
      bool enabled = *offset.cuttable >= 1;

      if (self->klass == gameNoteControllerClass) {
        auto* gameNoteController = reinterpret_cast<GameNoteController*>(self);
        ArrayW<UnityW<BoxCuttableBySaber>> bigCuttables = gameNoteController->_bigCuttableBySaberList;
        for (auto bigCuttable : bigCuttables) {
          if (bigCuttable->canBeCut != enabled) {
            bigCuttable->set_canBeCut(enabled);
          }
        }
        ArrayW<UnityW<BoxCuttableBySaber>> smallCuttables = gameNoteController->_smallCuttableBySaberList;
        for (auto smallCuttable : smallCuttables) {
          if (smallCuttable->canBeCut != enabled) {
            smallCuttable->set_canBeCut(enabled);
          }
        }
      } else if (self->klass == bombNoteControllerClass) {
        auto* bombNoteController = reinterpret_cast<BombNoteController*>(self);
        CuttableBySaber* cuttable = bombNoteController->_cuttableBySaber;
        if (cuttable->get_canBeCut() != enabled) {
          cuttable->set_canBeCut(enabled);
        }
      }
    }
  }

  NoteController_ManualUpdate(self);

  // NoteJump.ManualUpdate will be the last place this is used after it was set in
  // NoteController.ManualUpdate. To make sure it doesn't interfere with future notes, it's set
  // back to null
  noteUpdateAD = nullptr;
  noteTracks.clear();
}

MAKE_HOOK_MATCH(NoteController_SendNoteWasCutEvent_LinkedNotes, &NoteController::SendNoteWasCutEvent, void,
                NoteController* self, ByRef<::GlobalNamespace::NoteCutInfo> noteCutInfo) {

  NoteController_SendNoteWasCutEvent_LinkedNotes(self, noteCutInfo);

  if (!Hooks::isNoodleHookEnabled()) return;

  auto* customNoteData = il2cpp_utils::try_cast<CustomJSONData::CustomNoteData>(self->_noteData).value_or(nullptr);
  if (!customNoteData || !customNoteData->customData) {
    return;
  }

  BeatmapObjectAssociatedData& ad = getAD(customNoteData->customData);

  auto link = ad.objectData.link;
  if (!link) return;

  auto& list = linkedNotes[*link];

  list.erase(self);
  linkedLinkedNotes.erase(self);

  auto cuts = list | Sombrero::Linq::Functional::Select([&](auto const& noteController) {
                return std::pair(
                    noteController,
                    NoteCutInfo(noteController->_noteData, noteCutInfo->speedOK, noteCutInfo->directionOK,
                                noteCutInfo->saberTypeOK, noteCutInfo->wasCutTooSoon, noteCutInfo->saberSpeed,
                                noteCutInfo->saberDir, noteCutInfo->saberType, noteCutInfo->timeDeviation,
                                noteCutInfo->cutDirDeviation, noteCutInfo->cutPoint, noteCutInfo->cutNormal,
                                noteCutInfo->cutDistanceToCenter, noteCutInfo->cutAngle, noteCutInfo->worldRotation,
                                noteCutInfo->inverseWorldRotation, noteCutInfo->noteRotation, noteCutInfo->notePosition,
                                noteCutInfo->saberMovementData));
              }) |
              Sombrero::Linq::Functional::ToVector();

  for (auto const& note : list) {
    linkedLinkedNotes.erase(note);
  }
  list.clear();

  for (auto& [noteController, cutInfo] : cuts) {
    auto ref = ByRef<NoteCutInfo>(cutInfo);
    noteController->SendNoteWasCutEvent(ref);
  }
}
MAKE_HOOK_MATCH(BeatmapObjectManager_Despawn_LinkedNotes,
                static_cast<void (GlobalNamespace::BeatmapObjectManager::*)(::GlobalNamespace::NoteController*)>(
                    &GlobalNamespace::BeatmapObjectManager::Despawn),
                void, BeatmapObjectManager* self, GlobalNamespace::NoteController* noteController) {

  BeatmapObjectManager_Despawn_LinkedNotes(self, noteController);

  if (!Hooks::isNoodleHookEnabled()) return;

  auto linkedLinkedIt = linkedLinkedNotes.find(noteController);
  if (linkedLinkedIt != linkedLinkedNotes.end()) {
    linkedLinkedIt->second->erase(noteController);
    linkedLinkedNotes.erase(linkedLinkedIt);
  }
}

MAKE_HOOK_MATCH(BurstSliderGameNoteController_ManualUpdate, &BurstSliderGameNoteController::ManualUpdate, void, BurstSliderGameNoteController* self) {
  // TRANSPILE
  static auto const* ManualUpdate = il2cpp_utils::il2cpp_type_check::MetadataGetter<&NoteController::ManualUpdate>::methodInfo();
  il2cpp_utils::RunMethodRethrow<void, false>(self, ManualUpdate);
  self->SetBigCuttableColliderSize();
}

void InstallNoteControllerHooks() {
  INSTALL_HOOK(NELogger::Logger, NoteController_Init);
  INSTALL_HOOK(NELogger::Logger, NoteController_ManualUpdate);

  INSTALL_HOOK(NELogger::Logger, NoteController_SendNoteWasCutEvent_LinkedNotes);
  INSTALL_HOOK(NELogger::Logger, BeatmapObjectManager_Despawn_LinkedNotes);

  INSTALL_HOOK(NELogger::Logger, BurstSliderGameNoteController_ManualUpdate);
}

NEInstallHooks(InstallNoteControllerHooks);