#include "beatsaber-hook/shared/callback.hpp"
#pragma once

#include "UnityEngine/Color.hpp"
#include "conditional-dependencies/shared/main.hpp"
#include "utils.hpp"

#include "GlobalNamespace/ObstacleController.hpp"

#include <optional>

namespace Chroma {

/// TODO: NOT TESTED, USE WITH CAUTION
/// Uses conditional dependency to call the method safely or return
/// nullopt if it isn't found or if the original method returned nullopt
class ObstacleAPI {
public:
  /// TODO: Unsure of whether this returns nullopt if Chroma sets the color or not.
  /// Gets the obstacle color or null if either Chroma is not setting the color or method was not found
  static std::optional<Sombrero::FastColor>
  getObstacleControllerColorSafe(GlobalNamespace::ObstacleController* oc) noexcept {
    static auto function =
        CondDeps::Find<OptColor, GlobalNamespace::ObstacleController*>(CHROMA_ID, "getObstacleControllerColorSafe");

    if (function) {
      // Returns the color struct
      auto optColor = function.value()(oc);

      if (!optColor.isSet) return std::nullopt;

      return optColor.getColor();
    }

    return std::nullopt;
  }

  /// Sets the obstacle color if the method was found.
  static void setObstacleColorSafe(GlobalNamespace::ObstacleController* nc, Sombrero::FastColor color0) noexcept {
    static auto function = CondDeps::Find<void, GlobalNamespace::ObstacleController*, Sombrero::FastColor>(
        CHROMA_ID, "setObstacleColorSafe");

    if (function) {
      function.value()(nc, color0);
    }
  }

  /// Sets all the obstacle color if the method was found.
  static void setAllObstacleColorSafe(Sombrero::FastColor color0) noexcept {
    static auto function = CondDeps::Find<void, Sombrero::FastColor>(CHROMA_ID, "setAllObstacleColorSafe");

    if (function) {
      function.value()(color0);
    }
  }

  /// Sets if the obstacle is colorable. If this is set to true, Chroma will NOT color the obstacle and
  /// instead the work should be done by the mod handling coloring obstacles usually a custom obstacles mod
  static void setObstacleColorable(bool colorable) {
    static auto function = CondDeps::Find<void, bool>(CHROMA_ID, "setObstacleColorable");

    if (function) {
      function.value()(colorable);
    }
  }

  /// Checks if the obstacle is colorable. If this is set to true, Chroma will NOT color the obstacle and
  /// instead the work should be done by the mod handling coloring obstacles usually a custom obstacle mod
  static std::optional<bool> isObstacleColorable() {
    static auto function = CondDeps::Find<bool>(CHROMA_ID, "isObstacleColorable");

    if (function) {
      return function.value()();
    }

    return std::nullopt;
  }

  /// This retrieves the callback used for Saber Color changed.
  ///
  /// \return
  using ObstacleCallback = unordered_event_callback<GlobalNamespace::ObstacleControllerBase*, Sombrero::FastColor const&>;
  static std::optional<std::reference_wrapper<ObstacleCallback>> getObstacleChangedColorCallbackSafe() {
    static auto function = CondDeps::Find<ObstacleCallback*>(CHROMA_ID, "getObstacleChangedColorCallbackSafe");

    /// Oh boi what have I done
    if (function) {
      ObstacleCallback& callback = *function.value()();
      return std::make_optional(std::ref(callback));
    }

    return std::nullopt;
  }
};
} // namespace Chroma