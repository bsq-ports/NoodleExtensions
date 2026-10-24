#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/SaberTrailRenderer.hpp"
#include "UnityEngine/Bounds.hpp"
#include "UnityEngine/Vector3.hpp"

#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

MAKE_HOOK_MATCH(SaberTrailRenderer_UpdateMesh, &SaberTrailRenderer::UpdateMesh, void, SaberTrailRenderer* self,
                TrailElementCollection* trailElementCollection, Color color) {
  SaberTrailRenderer_UpdateMesh(self, trailElementCollection, color);
  if (!Hooks::isNoodleHookEnabled()) return;

  self->_bounds = Bounds(Vector3::getStaticF_zeroVector(), Vector3::getStaticF_positiveInfinityVector());
}

void InstallSaberCullingFixHook() {
  INSTALL_HOOK(NELogger::Logger, SaberTrailRenderer_UpdateMesh);
}

NEInstallHooks(InstallSaberCullingFixHook);