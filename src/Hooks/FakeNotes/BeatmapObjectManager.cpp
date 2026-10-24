#include "beatsaber-hook/shared/byref.hpp"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/NoteCutCoreEffectsSpawner.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteCutInfo.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

extern std::optional<NoteCutCoreEffectsSpawner*> noteCutCoreEffectsSpawner;

MAKE_HOOK_MATCH(HandleNoteControllerNoteWasMissed, &BeatmapObjectManager::HandleNoteControllerNoteWasMissed, void,
                BeatmapObjectManager* self, NoteController* noteController) {
  if (!Hooks::isNoodleHookEnabled()) return HandleNoteControllerNoteWasMissed(self, noteController);

  bool isFakeNote = FakeNoteHelper::GetFakeNote(noteController->_noteData);
  if (!isFakeNote) {
    HandleNoteControllerNoteWasMissed(self, noteController);
  }
}

// lazy
// static Il2CppType const* NoteCutInfoT(by_ref<NoteCutInfo const> info) {
//   return il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_type<by_ref<NoteCutInfo const>>::get();
//   //    return il2cpp_utils::il2cpp_type_check::il2cpp_arg_type<by_ref<NoteCutInfo const>>::get(info);
// }

// // GlobalNamespace::NoteCutCoreEffectsSpawner::
// void HandleNoteWasCut(NoteCutCoreEffectsSpawner* self, GlobalNamespace::NoteController* noteController,
//                       by_ref<GlobalNamespace::NoteCutInfo const> noteCutInfo) {
//   static auto ___internal__logger =
//       ::Logger::get().WithContext("GlobalNamespace::NoteCutCoreEffectsSpawner::HandleNoteWasCut");
//   static auto* ___internal__method = THROW_UNLESS((::il2cpp_utils::FindMethod(
//       self, "HandleNoteWasCut", std::vector<Il2CppClass*>{},
//       ::std::vector<Il2CppType const*>{ ::il2cpp_utils::ExtractType(noteController), ::NoteCutInfoT(noteCutInfo) })));
//   ::i2c::run_method<void>(self, ___internal__method, noteController, by_ref(noteCutInfo));
// }

MAKE_HOOK_MATCH(BeatmapObjectManager_HandleNoteControllerNoteWasCut,
                &BeatmapObjectManager::HandleNoteControllerNoteWasCut, void, BeatmapObjectManager* self,
                GlobalNamespace::NoteController* noteController, by_ref<GlobalNamespace::NoteCutInfo const> noteCutInfo) {
  if (!Hooks::isNoodleHookEnabled())
    return BeatmapObjectManager_HandleNoteControllerNoteWasCut(self, noteController, noteCutInfo);

  // If not fake note or noteCutCoreEffectsSpawner is null
  bool isFakeNote = FakeNoteHelper::GetFakeNote(noteController->_noteData);
  if (!isFakeNote) {
    BeatmapObjectManager_HandleNoteControllerNoteWasCut(self, noteController, noteCutInfo);
    return;
  }

  if (!noteCutCoreEffectsSpawner) {
    NELogger::Logger.error("noteCutCoreEffectsSpawner is null");
    self->Despawn(noteController);
    return;
  }

  CRASH_UNLESS(*noteCutCoreEffectsSpawner);
  //        NELogger::Logger.debug("note %p %p extract type %p", &noteCutInfo.heldRef,
  //        &i2c::class_of<GlobalNamespace::NoteCutInfo>()->this_arg, NoteCutInfoT(noteCutInfo));
  //        NELogger::Logger.debug("no arg %p arg %p",
  //        il2cpp_utils::il2cpp_type_check::il2cpp_no_arg_type<by_ref<NoteCutInfo const>>::get(),
  //        il2cpp_utils::il2cpp_type_check::il2cpp_arg_type<by_ref<NoteCutInfo const>>::get(noteCutInfo));
  // HandleNoteWasCut(*noteCutCoreEffectsSpawner, noteController, noteCutInfo);
  noteCutCoreEffectsSpawner.value()->HandleNoteWasCut(noteController, noteCutInfo);
  self->Despawn(noteController);
}

void InstallBeatmapObjectManagerHooks() {
  INSTALL_HOOK(NELogger::Logger, HandleNoteControllerNoteWasMissed);
  INSTALL_HOOK(NELogger::Logger, BeatmapObjectManager_HandleNoteControllerNoteWasCut);
}

NEInstallHooks(InstallBeatmapObjectManagerHooks);