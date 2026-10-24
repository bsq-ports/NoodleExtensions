#include "beatsaber-hook/shared/hooking.hpp"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"

#include "NEHooks.h"

#include "custom-types/shared/coroutine.hpp"

#include "sombrero/shared/Vector2Utils.hpp"
#include "sombrero/shared/Vector3Utils.hpp"

#include "cpp-semver/shared/cpp-semver.hpp"

#include "GlobalNamespace/MainFlowCoordinator.hpp"

#include "UnityEngine/WaitForSecondsRealtime.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/UI/LayoutElement.hpp"
#include "UnityEngine/Canvas.hpp"

#include "HMUI/Screen.hpp"
#include "System/Collections/IEnumerator.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace UnityEngine::UI;
using namespace Sombrero;

static const std::string QosID = "Qosmetics";
static const std::string QosVersionRange = ">1.3.2";

static const std::string SongLoaderID = "SongLoader";
static const std::string SongLoaderVersionRange = ">=0.7.2";

static const std::string PinkCoreID = "pinkcore";
static const std::string PinkCoreVersionRange = ">1.6.2";

static const std::string ChromaID = "chroma";
static const std::string ChromaVersionRange = ">=2.5.7";

static const std::string MappingExtensionsID = "MappingExtensions";

static std::vector<modloader::ModInfo> outdatedMods;
static bool MEInstalled = false;

/*static void AddToDependencyIfOutdated(std::string const& modName, std::string const& versionRange,
                                      std::unordered_map<std::string, const Mod> const& modList,
                                      std::vector<const modloader::ModInfo>& outdatedMods) {
  auto modLoaded = false;//Modloader::requireMod(modName);

  if (modLoaded) {
    auto modInfo = modList.find(modName);
    if (modInfo != modList.end()) {

      if (!semver::satisfies(modInfo->second.info.version, versionRange)) {
        outdatedMods.emplace_back(modInfo->second.info);
      }
    }
  }
}*/

custom_types::Helpers::Coroutine openDialogLater() {

  /*co_yield reinterpret_cast<System::Collections::IEnumerator*>(CRASH_UNLESS(WaitForSecondsRealtime::New_ctor(0.5f)));

  using namespace QuestUI;

  // Create canvas
  auto screen = BeatSaberUI::CreateCanvas();

  // Required for Modal
  screen->AddComponent<HMUI::Screen*>();
  auto canvasTransform = reinterpret_cast<UnityEngine::RectTransform*>(screen->get_transform());

  // Position in front of the user
  canvasTransform->set_localPosition({ 0, 1.5, 2.0f });

  // Create modal
  auto modalTransform = BeatSaberUI::CreateModal(
      screen->get_transform(), [screen](auto modal) { Object::Destroy(screen); }, false);

  // Size of modal
  FastVector2 sizeDelta(55.0f, 40.0f);
  reinterpret_cast<UnityEngine::RectTransform*>(modalTransform->get_transform())->set_sizeDelta(sizeDelta);

  // Layout elements.
  // Horizontal parent for centering
  auto horizontal = BeatSaberUI::CreateHorizontalLayoutGroup(modalTransform->get_transform());
  auto vertical = BeatSaberUI::CreateVerticalLayoutGroup(horizontal->get_transform());

  // Which layout to use as parent
  auto layout = vertical;

  auto* layoutelem = layout->get_gameObject()->AddComponent<LayoutElement*>();
  layoutelem->set_preferredWidth(sizeDelta.x);
  layoutelem->set_preferredHeight(sizeDelta.y);

  // Align and size children properly
  layout->set_childAlignment(UnityEngine::TextAnchor::MiddleCenter);

  layout->set_childControlHeight(true);
  layout->set_childForceExpandHeight(true);
  layout->set_childControlWidth(true);
  layout->set_childForceExpandWidth(true);

  // Texts
  if (MEInstalled) {
    auto warningMEText = BeatSaberUI::CreateText(layout->get_transform(), "Mapping extensions is installed. REMOVE IT");
    warningMEText->set_enableWordWrapping(true);
    warningMEText->set_alignment(TMPro::TextAlignmentOptions::Center);
    warningMEText->set_fontSize(warningMEText->get_fontSize() * 1.125f);
  }

  if (!outdatedMods.empty()) {
    auto warningText = BeatSaberUI::CreateText(
        layout->get_transform(),
        "There seems to be some incompatible mods with Noodle that have updates. Please update the following mods:");
    warningText->set_enableWordWrapping(true);
    warningText->set_alignment(TMPro::TextAlignmentOptions::Center);
    warningText->set_fontSize(warningText->get_fontSize() * 0.85f);

    auto smallLayout = layout; // BeatSaberUI::CreateVerticalLayoutGroup(layout->get_transform());

    for (auto const& mod : outdatedMods) {
      std::string modInfo(mod.id + ":" + mod.version);
      auto modText = BeatSaberUI::CreateText(smallLayout->get_transform(), modInfo);
      modText->set_enableWordWrapping(true);
      modText->set_alignment(TMPro::TextAlignmentOptions::Center);
      modText->set_fontSize(modText->get_fontSize() * 0.7f);
    }
  }

  modalTransform->Show(true, true, nullptr);*/

  co_return;
}

MAKE_HOOK_MATCH(MainFlowCoordinator_DidActivate, &GlobalNamespace::MainFlowCoordinator::DidActivate, void,
                GlobalNamespace::MainFlowCoordinator* self, bool firstActivation, bool addedToHierarchy,
                bool screenSystemEnabling) {
  MainFlowCoordinator_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);

  static bool dialogOpened = false;
  if (!dialogOpened) {
    dialogOpened = true;
    self->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(openDialogLater()));
  }
}

void InstallOutdatedModsHooks() {
  //auto const modList = Modloader::getMods();

  //AddToDependencyIfOutdated(QosID, QosVersionRange, modList, outdatedMods);
  //AddToDependencyIfOutdated(SongLoaderID, SongLoaderVersionRange, modList, outdatedMods);
  //AddToDependencyIfOutdated(PinkCoreID, PinkCoreVersionRange, modList, outdatedMods);
  //AddToDependencyIfOutdated(ChromaID, ChromaVersionRange, modList, outdatedMods);

  //if (modList.contains(MappingExtensionsID)) {
  //  MEInstalled = true;
  //}

  if (outdatedMods.empty() && !MEInstalled) return;

  //QuestUI::Init();
  INSTALL_HOOK(NELogger::Logger, MainFlowCoordinator_DidActivate);
}

// NEInstallHooks(InstallOutdatedModsHooks);
