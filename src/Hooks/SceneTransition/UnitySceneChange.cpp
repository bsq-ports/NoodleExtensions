#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "NEHooks.h"

#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"

using namespace UnityEngine;

MAKE_HOOK_MATCH(SceneManager_Internal_ActiveSceneChanged, &UnityEngine::SceneManagement::SceneManager::Internal_ActiveSceneChanged,
                void, UnityEngine::SceneManagement::Scene previousActiveScene, UnityEngine::SceneManagement::Scene newActiveScene) {
  
  if (previousActiveScene.IsValid() && previousActiveScene.name == "GameCore") {
    Hooks::setNoodleHookEnabled(false);
  }

  SceneManager_Internal_ActiveSceneChanged(previousActiveScene, newActiveScene);
}

void SceneManager_Internal() {
  INSTALL_HOOK(NELogger::Logger, SceneManager_Internal_ActiveSceneChanged);
}

NEInstallHooks(SceneManager_Internal)