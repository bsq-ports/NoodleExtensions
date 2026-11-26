#pragma once
#include <optional>
#include <string_view>

// Scene names may no longer be available during the unload callback. Match the
// handle captured while GameCore was loaded, ignoring additive environments.
class GameplaySceneLifetime {
  std::optional<int> gameplayHandle;
public:
  void Loaded(int handle, std::string_view name) {
    if (name == "GameCore") gameplayHandle = handle;
  }
  bool HasGameplay() const { return gameplayHandle.has_value(); }
  bool Unloaded(int handle) {
    if (!gameplayHandle || *gameplayHandle != handle) return false;
    gameplayHandle.reset();
    return true;
  }
};
