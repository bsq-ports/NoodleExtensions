#include "beatsaber-hook/shared/hooking.hpp"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/arrayw.hpp"

#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectManager.hpp"
#include "GlobalNamespace/IAudioTimeSource.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"
#include "GlobalNamespace/IReadonlyBeatmapData.hpp"

#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace UnityEngine::SceneManagement;
//
// MAKE_HOOK_MATCH(BeatmapObjectSpawnController_Start2, &BeatmapObjectSpawnController::Start, void,
// BeatmapObjectSpawnController *self) {
//    auto *beatmapObjectManager = reinterpret_cast<BeatmapObjectManager *>(self->beatmapObjectSpawner);
//    auto *coreSetup = reinterpret_cast<BeatmapObjectCallbackController *>(self->beatmapObjectCallbackController);
//    IAudioTimeSource* IAudioTimeSource = coreSetup->audioTimeSource;
//    IReadonlyBeatmapData* beatmapData = coreSetup->beatmapData;
//
//    std::vector<std::string> ObjectsToKill = {
//        "Window",
//        "TrackMirror",
//        "TrackConstruction",
//        "ConstructionGlowLine",
//        "Spectrograms",
//        "Mirror",
//        "Frame",
//        "Construction",
//        "SaberBurnMarksParticles"
//    };
//
//    Il2CppString *il2cppStringBig = StringW("Big");
//
//    for (std::string objectName : ObjectsToKill) {
//        auto gameObjects = Object::FindObjectsOfType<GameObject*>();
//        Il2CppString* il2cppObjectName = StringW(objectName);
//        if (objectName == "TrackLaneRing" || objectName == "BigTrackLaneRing") {
//            for (int i = 0; i < gameObjects->Length(); i++) {
//                auto* gameObject = gameObjects->values[i];
//                if (gameObject->get_name()->Contains(il2cppObjectName)) {
//                    if (objectName == "TrackLaneRing" && gameObject->get_name()->Contains(il2cppStringBig)) {
//                        continue;
//                    }
//
//                    gameObject->SetActive(false);
//                }
//            }
//        } else {
//            for (int i = 0; i < gameObjects->Length(); i++) {
//                auto* gameObject = gameObjects->values[i];
//                if (gameObject->get_name()->Contains(il2cppObjectName)) {
//                    // TODO: Make sure we aren't removing the environment in menus
//                    gameObject->SetActive(false);
//                }
//            }
//        }
//    }
//
//    BeatmapObjectSpawnController_Start2(self);
//}

// void InstallBeatmapObjectSpawnControllerHooks() {
//     INSTALL_HOOK(NELogger::Logger, BeatmapObjectSpawnController_Start2);
// }
//  NEInstallHooks(InstallBeatmapObjectSpawnControllerHooks);