#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/DisappearingArrowControllerBase_1.hpp"
#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/CutoutEffect.hpp"

#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

template <>
struct ::i2c::metadata_getter<
    &DisappearingArrowControllerBase_1<GameNoteController*>::SetArrowTransparency> {

  // make the compiler happy
  constexpr static std::size_t size = 0xc4;
  constexpr static std::size_t addrs = 0x26b6608;

  static MethodInfo const* method_info() {
    return i2c::find_method(
        i2c::class_of<GlobalNamespace::DisappearingArrowControllerBase_1<GameNoteController*>*>(), {"SetArrowTransparency", 1});
  }
};

MAKE_HOOK_MATCH(DisappearingArrowControllerBase_SetArrowTransparency,
                &DisappearingArrowControllerBase_1<GameNoteController*>::SetArrowTransparency, void,
                DisappearingArrowControllerBase_1<GameNoteController*>* self, float arrowTransparency) {
  DisappearingArrowControllerBase_SetArrowTransparency(self, arrowTransparency);
  if (!Hooks::isNoodleHookEnabled()) return;
  if (!self->_arrowCutoutEffect) return;
  self->_arrowCutoutEffect->SetCutout(1.0f - arrowTransparency);
}

void InstallDisappearingArrowControllerHooks() {
  INSTALL_HOOK(NELogger::Logger, DisappearingArrowControllerBase_SetArrowTransparency);
}

NEInstallHooks(InstallDisappearingArrowControllerHooks);
