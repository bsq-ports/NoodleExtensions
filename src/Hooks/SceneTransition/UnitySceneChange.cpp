#include <string>

#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "NEHooks.h"
#include "NECaches.h"
#include "GameplaySceneLifetime.hpp"
#include "UnityEngine/SceneManagement/LoadSceneMode.hpp"

#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"

using namespace UnityEngine;

MAKE_HOOK_MATCH(SceneManager_Internal_ActiveSceneChanged, &UnityEngine::SceneManagement::SceneManager::Internal_ActiveSceneChanged,
                void, UnityEngine::SceneManagement::Scene previousActiveScene, UnityEngine::SceneManagement::Scene newActiveScene) {

  if (newActiveScene.IsValid() && newActiveScene.name != "GameCore" && newActiveScene.name != "EmptyTransition") {
    Hooks::setNoodleHookEnabled(false);
  }

  SceneManager_Internal_ActiveSceneChanged(previousActiveScene, newActiveScene);
}

static GameplaySceneLifetime gameplayLifetime;

MAKE_HOOK_MATCH(SceneManager_Internal_SceneLoaded, &UnityEngine::SceneManagement::SceneManager::Internal_SceneLoaded,
                void, UnityEngine::SceneManagement::Scene scene, UnityEngine::SceneManagement::LoadSceneMode mode) {
  if (scene.IsValid()) {
    gameplayLifetime.Loaded(scene.get_handle(), std::string(scene.get_name()));
  }
  SceneManager_Internal_SceneLoaded(scene, mode);
}

MAKE_HOOK_MATCH(SceneManager_Internal_SceneUnloaded, &UnityEngine::SceneManagement::SceneManager::Internal_SceneUnloaded,
                void, UnityEngine::SceneManagement::Scene scene) {
  const bool unloadedGameplay = gameplayLifetime.Unloaded(scene.get_handle());
  // Unity callbacks may still use the outgoing gameplay objects. They can also
  // synchronously load a replacement GameCore, whose references must survive.
  SceneManager_Internal_SceneUnloaded(scene);
  if (unloadedGameplay && !gameplayLifetime.HasGameplay()) NECaches::ClearGameplayReferences();
}

void SceneManager_Internal() {
  INSTALL_HOOK(NELogger::Logger, SceneManager_Internal_ActiveSceneChanged);
  INSTALL_HOOK(NELogger::Logger, SceneManager_Internal_SceneLoaded);
  INSTALL_HOOK(NELogger::Logger, SceneManager_Internal_SceneUnloaded);
}

NEInstallHooks(SceneManager_Internal)
