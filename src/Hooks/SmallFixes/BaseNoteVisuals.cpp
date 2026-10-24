#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/BaseNoteVisuals.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/DisappearingArrowControllerBase_1.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/GameObject.hpp"

#include "custom-json-data/shared/CustomBeatmapData.h"

#include "NEHooks.h"
#include "AssociatedData.h"
#include "NECaches.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

MAKE_HOOK_MATCH(BaseNoteVisuals_Awake, &BaseNoteVisuals::Awake, void, BaseNoteVisuals* self) {
  if (!Hooks::isNoodleHookEnabled()) return BaseNoteVisuals_Awake(self);

  BaseNoteVisuals_Awake(self);

  static auto ICubeNoteTypeProviderKlass = i2c::class_of<GameNoteController*>(); // i2c::class_of<INoteMovementProvider*>();
  static auto CustomNoteDataKlass = i2c::class_of<CustomJSONData::CustomNoteData*>();

  if (self && self->_noteController &&
      i2c::functions::class_is_assignable_from(ICubeNoteTypeProviderKlass, self->_noteController->klass)) {
    NoteController* noteController = static_cast<NoteController*>(self->_noteController.ptr());

    if (!noteController->_noteData) return;

    DisappearingArrowControllerBase_1<GameNoteController*>* disappearingArrowController;

    if (i2c::functions::class_is_assignable_from(CustomNoteDataKlass, noteController->_noteData->klass)) {
      //        if (noteData->customData) {
      auto& ad = NECaches::getNoteCache(noteController);

      disappearingArrowController =
          (DisappearingArrowControllerBase_1<GlobalNamespace::GameNoteController*>*)ad.disappearingArrowController;
      if (!disappearingArrowController) {
        disappearingArrowController =
            self->get_gameObject()->GetComponent<DisappearingArrowControllerBase_1<GameNoteController*>*>();
        ad.disappearingArrowController = disappearingArrowController;
      }
    } else {
      disappearingArrowController =
          self->get_gameObject()->GetComponent<DisappearingArrowControllerBase_1<GameNoteController*>*>();
    }

    disappearingArrowController->SetArrowTransparency(
        1); // i have no fucking idea how this fixes the weird ghost arrow bug
  }
}

void InstallBaseNoteVisualsHooks() {
  INSTALL_HOOK(NELogger::Logger, BaseNoteVisuals_Awake);
}

NEInstallHooks(InstallBaseNoteVisualsHooks);