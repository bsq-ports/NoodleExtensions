#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"

#include "GlobalNamespace/MirroredNoteController_1.hpp"
#include "GlobalNamespace/MirroredGameNoteController.hpp"
#include "GlobalNamespace/INoteMirrorable.hpp"
#include "GlobalNamespace/IGameNoteMirrorable.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/BaseNoteVisuals.hpp"
#include "GlobalNamespace/CutoutAnimateEffect.hpp"
#include "GlobalNamespace/CutoutEffect.hpp"
#include "GlobalNamespace/DisappearingArrowControllerBase_1.hpp"
#include "GlobalNamespace/ConditionalMaterialSwitcher.hpp"
#include "GlobalNamespace/BoolSO.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Renderer.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "Animation/ParentObject.h"
#include "NECaches.h"
#include "NEConfig.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

using MirroredNote = MirroredNoteController_1<INoteMirrorable*>;
using MirroredGameNote = MirroredNoteController_1<IGameNoteMirrorable*>;

template <>
struct ::il2cpp_utils::il2cpp_type_check::MetadataGetter<
    &MirroredNoteController_1<Il2CppObject*>::UpdatePositionAndRotation> {
  static MethodInfo const* get() {
    return il2cpp_utils::FindMethodUnsafe(classof(MirroredNoteController_1<Il2CppObject*>*),
                                          "UpdatePositionAndRotation", 0);
  }
};

static constexpr void AddToTrack(CustomJSONData::CustomNoteData* noteData, GameObject* gameObject) {
  if (noteData->customData && noteData->customData->value) {
    auto const& tracks = TracksAD::getAD(noteData->customData).tracks;


    if (!tracks.empty()) {
      for (auto& track : tracks) {
        track.RegisterGameObject(gameObject);
      }
    }
  }
}

static bool initMirror = false;

static constexpr bool CheckSkip(Transform* noteTransform, Transform* followedNoteTransform) {
  auto const position = followedNoteTransform->get_position();
  auto noteTransformGameObject = noteTransform->get_gameObject();

  if (position.y < 0) {
    noteTransformGameObject->SetActive(false);

    return false;
  }

  noteTransformGameObject->SetActive(true);
  return true;
}

static void UpdateMirror(Transform* mirroredNoteTransform, Transform* followedObjectTransform,
                         NoteControllerBase* mirroredNoteController, NoteControllerBase* followedNote,
                         auto&& setMirrorHide) {

  auto const followedObjectTransformLocalScale = followedObjectTransform->get_localScale();

  mirroredNoteTransform->set_localScale(followedObjectTransformLocalScale);

  auto& mirrorCache = NECaches::getNoteCache(mirroredNoteController);
  auto& followedNoteCache = NECaches::getNoteCache(followedNote);

  auto followedCutoutEffect = NECaches::GetCutout(followedNote, followedNoteCache);
  auto followedDisappearingArrowController =
      NECaches::GetDisappearingArrowController((GameNoteController*)followedNote, followedNoteCache);

  ArrayW<ConditionalMaterialSwitcher*>& materialSwitchers = mirrorCache.conditionalMaterialSwitchers;

  if (!getNEConfig().enableMirrorNoteDissolve.GetValue() && followedCutoutEffect) {
    // rekt

    setMirrorHide(followedNoteCache.cutoutEffect->_cutout < 0.8);
    if (followedNoteCache.cutoutEffect->_cutout < 0.8) return;
  }

  if (initMirror && materialSwitchers) {
    for (auto* materialSwitcher : materialSwitchers) {
      materialSwitcher->_renderer->set_sharedMaterial(materialSwitcher->_material0);
    }
    mirrorCache.dissolveEnabled = false;
  }

  if (!materialSwitchers) {
    materialSwitchers = mirroredNoteController->GetComponentsInChildren<ConditionalMaterialSwitcher*>();
  }

  auto followedCutoutEffectCutout = followedCutoutEffect ? followedCutoutEffect->_cutout : 0;
  auto followedDisappearingArrowControllerCutout =
      followedDisappearingArrowController ? followedDisappearingArrowController->_arrowCutoutEffect->_cutout : 0;

  bool noteDissolveConfig = getNEConfig().enableNoteDissolve.GetValue();
  bool isDissolving = followedCutoutEffectCutout > 0 || followedDisappearingArrowControllerCutout > 0;

  if (materialSwitchers && mirrorCache.dissolveEnabled != isDissolving && noteDissolveConfig) {
    for (auto* materialSwitcher : materialSwitchers) {
      materialSwitcher->_renderer->set_sharedMaterial(isDissolving ? materialSwitcher->_material1
                                                                  : materialSwitcher->_material0);
    }
    mirrorCache.dissolveEnabled = isDissolving;
  }

  if (followedCutoutEffect) {
    auto cutoutEffect = NECaches::GetCutout(mirroredNoteController, mirrorCache);
    if (cutoutEffect) cutoutEffect->SetCutout(followedCutoutEffect->_cutout);
  }

  static auto MirrorKlass = classof(GlobalNamespace::MirroredGameNoteController*);
  static auto GameKlass = classof(GameNoteController*);

  if (il2cpp_functions::class_is_assignable_from(GameKlass, followedNote->klass) &&
      il2cpp_functions::class_is_assignable_from(MirrorKlass, mirroredNoteController->klass)) {
    auto disappearingArrowController =
        NECaches::GetDisappearingArrowController((MirroredGameNoteController*)mirroredNoteController, mirrorCache);

    if (disappearingArrowController && followedDisappearingArrowController)
      // SetArrowTransparency does 1 - x, so we resolve for x
      // 1 - x = y
      // -x = y - 1
      // x = -y + 1
      disappearingArrowController->SetArrowTransparency(
          (followedDisappearingArrowController->_arrowCutoutEffect->_cutout - 1) * -1);
  }
}

#define MIRROR_HOOK(name, generic)                                                                                     \
  MAKE_HOOK_FIND_INSTANCE(name##_Mirror, classof(MirroredNoteController_1<generic*>*), "Mirror", void,                 \
                          MirroredNoteController_1<generic*>* self, generic* noteController) {                         \
    initMirror = true;                                                                                                 \
    name##_Mirror(self, noteController);                                                                               \
    initMirror = false;                                                                                                \
    if (!Hooks::isNoodleHookEnabled()) return;                                                                         \
    auto* followedNote = reinterpret_cast<GameNoteController*>(self->followedNote);                                    \
    auto customNoteData = il2cpp_utils::try_cast<CustomJSONData::CustomNoteData>(followedNote->noteData);              \
                                                                                                                       \
    if (customNoteData) {                                                                                              \
      BeatmapObjectAssociatedData& ad = getAD(customNoteData.value()->customData);                                     \
      if (!ad.mirror) {                                                                                                \
        self->Hide(true);                                                                                              \
        return;                                                                                                        \
      }                                                                                                                \
      AddToTrack(customNoteData.value(), self->get_gameObject());                                                      \
    }                                                                                                                  \
  }

MIRROR_HOOK(MirroredNote, INoteMirrorable)
MIRROR_HOOK(MirroredGameNote, IGameNoteMirrorable)

// I love generics and il2cpp
// MirroredNoteController_1_System_Object__UpdatePositionAndRotation
MAKE_HOOK(Mirror_UpdatePositionAndRotation, nullptr, void, MirroredNoteController_1<Il2CppObject*>* self,
          MethodInfo* methodInfo) {
  if (!Hooks::isNoodleHookEnabled()) return Mirror_UpdatePositionAndRotation(self, methodInfo);
  if (!CheckSkip(self->_noteTransform, self->_followedNoteTransform)) {
    return;
  }
  Mirror_UpdatePositionAndRotation(self, methodInfo);
  UpdateMirror(self->_noteTransform, self->_followedNoteTransform, self,
               il2cpp_utils::cast<NoteControllerBase>(self->followedNote), [self](bool hide) { self->Hide(hide); });
}

void InstallMirroredNoteControllerHooks() {
  INSTALL_HOOK(NELogger::Logger, MirroredNote_Mirror);
  INSTALL_HOOK(NELogger::Logger, MirroredGameNote_Mirror);

  auto mInfo = il2cpp_utils::il2cpp_type_check::MetadataGetter<
      &MirroredNoteController_1<Il2CppObject*>::UpdatePositionAndRotation>::get();
  INSTALL_HOOK_DIRECT(NELogger::Logger, Mirror_UpdatePositionAndRotation, (void*)(mInfo->methodPointer));
}

NEInstallHooks(InstallMirroredNoteControllerHooks);