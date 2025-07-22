#pragma once
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/IJumpOffsetYProvider.hpp"
#include "GlobalNamespace/VariableMovementDataProvider.hpp"
#include "NEObjectPool.hpp"
#include "Zenject/DiContainer.hpp"
#include "sombrero/shared/ColorUtils.hpp"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include <memory>

#pragma once

namespace GlobalNamespace {
class ConditionalMaterialSwitcher;
class BaseNoteVisuals;
class NoteControllerBase;
class GameNoteController;
class MirroredGameNoteController;
class CutoutEffect;

template <typename T> class DisappearingArrowControllerBase_1;

class ObstacleDissolve;
class ObstacleControllerBase;
class CutoutAnimateEffect;
} // namespace GlobalNamespace

class NECaches {
public:
  static float noteJumpMovementSpeed;
  static float noteJumpStartBeatOffset;
  static float beatsPerMinute;
  static float numberOfLines;
  static SafePtr<Zenject::DiContainer> GameplayCoreContainer;
  static SafePtr<GlobalNamespace::IJumpOffsetYProvider> JumpOffsetYProvider;
  static SafePtr<GlobalNamespace::VariableMovementDataProvider> VariableMovementDataProvider;
  static SafePtr<GlobalNamespace::BeatmapObjectSpawnController::InitData> InitData;
  static SafePtr<GlobalNamespace::BeatmapObjectSpawnController> beatmapObjectSpawnController;
  static SafePtr<CustomJSONData::CustomBeatmapData> customBeatmapData;
  static GlobalNamespace::BeatmapObjectSpawnMovementData::NoteJumpValueType noteJumpValueType;
  static std::shared_ptr<NoodleExtensions::Pool::NoodleMovementDataProviderPool> noodleMovementDataProviderPool;
  static float noteJumpValue;

  static bool hasLocalSpaceTrail;
  static bool hasPlayerTransfrom;

  static bool LeftHandedMode;

  struct NoteCache {
    constexpr NoteCache(NoteCache&&) = default;
    // NoteCache(NoteCache const&) = default;
    constexpr NoteCache() = default;

    ArrayW<GlobalNamespace::ConditionalMaterialSwitcher*> conditionalMaterialSwitchers;
    GlobalNamespace::BaseNoteVisuals* baseNoteVisuals{};
    GlobalNamespace::CutoutEffect* cutoutEffect{};

    GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::GameNoteController*>*
        disappearingArrowController{};
    GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::MirroredGameNoteController*>*
        mirroredDisappearingArrowController{};
    bool dissolveEnabled;
  };

  struct ObstacleCache {
    constexpr ObstacleCache(ObstacleCache&&) = default;
    // ObstacleCache(ObstacleCache const&) = default;
    constexpr ObstacleCache() = default;

    ArrayW<GlobalNamespace::ConditionalMaterialSwitcher*> conditionalMaterialSwitchers;
    GlobalNamespace::ObstacleDissolve* obstacleDissolve{};
    GlobalNamespace::CutoutAnimateEffect* cutoutAnimateEffect{};
    std::optional<Sombrero::FastColor> color;
    void* cachedData; // just an id
    bool dissolveEnabled;
  };

  static inline std::unordered_map<GlobalNamespace::NoteControllerBase*, NECaches::NoteCache> noteCache;
  static inline std::unordered_map<GlobalNamespace::ObstacleControllerBase*, NECaches::ObstacleCache> obstacleCache;

  static inline NECaches::NoteCache& getNoteCache(GlobalNamespace::NoteControllerBase* note) {
    return noteCache[note];
  }

  static inline NECaches::ObstacleCache& getObstacleCache(GlobalNamespace::ObstacleControllerBase* obstacle) {
    return obstacleCache[obstacle];
  }

  static void ClearObstacleCaches();
  static void ClearNoteCaches();

  consteval static auto get_noteLinesDistanceFast() {
    return 0.6f;
  }

  static GlobalNamespace::CutoutEffect* GetCutout(GlobalNamespace::NoteControllerBase* nc,
                                                  NECaches::NoteCache& noteCache);

  static GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::GameNoteController*>*
  GetDisappearingArrowController(GlobalNamespace::GameNoteController* nc, NECaches::NoteCache& noteCache);
  static GlobalNamespace::DisappearingArrowControllerBase_1<GlobalNamespace::MirroredGameNoteController*>*
  GetDisappearingArrowController(GlobalNamespace::MirroredGameNoteController* nc, NECaches::NoteCache& noteCache);
};
