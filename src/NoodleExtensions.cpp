#include "Animation/Events.h"
#include "NEHooks.h"
#include "NECaches.h"
#include "NELogger.h"
#include "Zenject/DiContainer.hpp"
#include "custom-json-data/shared/CJDLogger.h"
#include "songcore/shared/SongCore.hpp"

#include "scotland2/shared/loader.hpp"
#include "scotland2/shared/modloader.h"

float NECaches::noteJumpStartBeatOffset;
float NECaches::numberOfLines;
float NECaches::beatsPerMinute;
float NECaches::noteJumpValue;
GlobalNamespace::BeatmapObjectSpawnMovementData::NoteJumpValueType NECaches::noteJumpValueType;
bool NECaches::hasLocalSpaceTrail;
bool NECaches::hasPlayerTransfrom;
bool NECaches::LeftHandedMode;
SafePtr<Zenject::DiContainer> NECaches::GameplayCoreContainer;
SafePtr<GlobalNamespace::IJumpOffsetYProvider> NECaches::JumpOffsetYProvider;
SafePtr<GlobalNamespace::VariableMovementDataProvider> NECaches::VariableMovementDataProvider;
SafePtr<GlobalNamespace::BeatmapObjectSpawnController::InitData> NECaches::InitData;
SafePtr<GlobalNamespace::BeatmapObjectSpawnController>  NECaches::beatmapObjectSpawnController;
std::shared_ptr<NoodleExtensions::Pool::NoodleMovementDataProviderPool> NECaches::noodleMovementDataProviderPool;

void InstallAndRegisterAll() {
  // Force load to ensure order
  auto cjdModInfo = CustomJSONData::modInfo.to_c();
  auto tracksModInfo = CModInfo{ .id = "Tracks" };
  auto chromaModInfo = CModInfo{ .id = "Chroma" };

  modloader_require_mod(&cjdModInfo, CMatchType::MatchType_IdOnly);
  modloader_require_mod(&tracksModInfo, CMatchType::MatchType_IdOnly);
  modloader_require_mod(&chromaModInfo, CMatchType::MatchType_IdOnly);

  Hooks::InstallHooks();
  NEEvents::AddEventCallbacks();
  SongCore::API::Capabilities::RegisterCapability(NoodleExtensions::U8_REQUIREMENTNAME);

  // Modloader::requireMod("MappingExtensions");
  //     if (!Modloader::getMods().contains("MappingExtensions")) return;

  SongCore::API::LevelSelect::GetLevelWasSelectedEvent() +=
      [](SongCore::API::LevelSelect::LevelWasSelectedEventArgs const& event) {
        if (!event.isCustom) return;

        if (!event.customLevelDetails) return;
        auto const& requirements = event.customLevelDetails->difficultyDetails.requirements;

        bool meRequirement = std::any_of(requirements.begin(), requirements.end(),
                                         [](auto const& s) { return s == NoodleExtensions::U8_ME_REQUIREMENTNAME; });
        bool neRequirement = std::any_of(requirements.begin(), requirements.end(),
                                         [](auto const& s) { return s == NoodleExtensions::U8_REQUIREMENTNAME; });

        for (auto const& r : requirements) {
          NELogger::Logger.debug("Installed on map {}", r.c_str());
        }

        if (meRequirement && neRequirement) {
          NELogger::Logger.debug("Remove install");
          SongCore::API::PlayButton::DisablePlayButton(NoodleExtensions::U8_REQUIREMENTNAME,
                                                       "Map uses both NE and ME. This is not supported!");

        } else {
          NELogger::Logger.debug("Register install");
          SongCore::API::PlayButton::EnablePlayButton(NoodleExtensions::U8_REQUIREMENTNAME);
        }
      };
}
