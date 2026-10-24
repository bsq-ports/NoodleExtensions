#pragma once
#include "config-utils/shared/config-utils.hpp"

#include "beatsaber-hook/shared/stringw.hpp"

enum struct MaterialBehaviour { SMART_COLOR = 0, SEMI_BASIC = 1, BASIC = 2 };

inline std::vector<std::string> getMaterialBehaviourValues() {
  return { { "Smart Color", "SemiBasic", "Basic" } };
}

DECLARE_CONFIG(NEConfig) {
  CONFIG_VALUE(enableNoteDissolve, bool, "Enable note dissolve", true);
  CONFIG_VALUE(enableMirrorNoteDissolve, bool, "Enable mirror note dissolve", true,
              "If enabled, allows note mirrors to dissolve. When disabled, hides the notes if dissolved");
  CONFIG_VALUE(enableObstacleDissolve, bool, "Enable obstacle dissolve", true);
  CONFIG_VALUE(qosmeticsModelDisable, bool, "Disable Qosmetics models on NE maps", true,
              "If enabled, NE will disable qosmetics walls and notes to improve performance");
  CONFIG_VALUE(materialBehaviour, int, "Obstacle material behaviour", 0);
};