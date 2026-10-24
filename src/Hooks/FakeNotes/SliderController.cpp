#include "NELogger.h"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/SliderController.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

MAKE_HOOK_MATCH(SetSaberAttraction, &SliderController::SetSaberAttraction, void, SliderController* self, bool saberAttraction) {
  if (!Hooks::isNoodleHookEnabled()) return SetSaberAttraction(self, saberAttraction);
  if(FakeNoteHelper::GetAttractableArc(self->_sliderData) && saberAttraction) {
    SetSaberAttraction(self, saberAttraction);
  }
}

void InstallFakeSliderControllerHooks() {
  INSTALL_HOOK(NELogger::Logger, SetSaberAttraction);
}
NEInstallHooks(InstallFakeSliderControllerHooks);