#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/DisappearingArrowControllerBase_1.hpp"
#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/CutoutEffect.hpp"
#include "GlobalNamespace/CutoutAnimateEffect.hpp"
#include "UnityEngine/Vector3.hpp"

#include "Animation/AnimationHelper.h"
#include "Animation/ParentObject.h"
#include "AssociatedData.h"
#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace TrackParenting;

MAKE_HOOK_MATCH(CutoutEffect_SetCutout,
                static_cast<void (GlobalNamespace::CutoutEffect::*)(float, UnityEngine::Vector3)>(
                    &GlobalNamespace::CutoutEffect::SetCutout),
                void, CutoutEffect* self, float cutout, UnityEngine::Vector3 cutoutOffset) {
  // Do not run SetCutout if the new value is the same as old.
  if (std::abs(cutout - self->_cutout) <= 0.005) return;

  CutoutEffect_SetCutout(self, cutout, cutoutOffset);
}

// MAKE_HOOK_MATCH(CutoutAnimateEffect_Start, &GlobalNamespace::CutoutAnimateEffect::Start, void,
//                 CutoutAnimateEffect* self) {}

void InstallCutoutEffectHooks() {
  INSTALL_HOOK(NELogger::Logger, CutoutEffect_SetCutout);
  //    INSTALL_HOOK(NELogger::Logger, CutoutAnimateEffect_Start)
}

NEInstallHooks(InstallCutoutEffectHooks);