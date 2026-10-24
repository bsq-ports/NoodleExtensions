#include "NELogger.h"
#include "VariableMovementHelper.hpp"
#include "beatsaber-hook/shared/hooking.hpp"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/safeptr.hpp"
#include "beatsaber-hook/shared/arrayw.hpp"
#include "beatsaber-hook/shared/listw.hpp"
#include "beatsaber-hook/shared/stringw.hpp"

#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "GlobalNamespace/SaberTypeExtensions.hpp"
#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/NoteBasicCutInfoHelper.hpp"

#include "NEConfig.h"
#include "NEUtils.hpp"
#include "Animation/AnimationHelper.h"
#include "Animation/ParentObject.h"
#include "tracks/shared/TimeSourceHelper.h"
#include "AssociatedData.h"
#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace UnityEngine;

MAKE_HOOK_MATCH(GameNoteController_HandleCut, &GameNoteController::HandleCut, void, GameNoteController* self,
                Saber* saber, Vector3 cutPoint, Quaternion orientation,
                Vector3 cutDirVec, bool allowBadCut) {
  if (!Hooks::isNoodleHookEnabled()) return GameNoteController_HandleCut(self, saber, cutPoint, orientation, cutDirVec, allowBadCut);

  auto customNoteData = i2c::try_cast<CustomJSONData::CustomNoteData*>(self->_noteData);
  if (customNoteData && customNoteData->customData->value) {
    BeatmapObjectAssociatedData& ad = getAD(customNoteData->customData);
    
    bool disableBadCutDirection = ad.objectData.disableBadCutDirection;
    bool disableBadCutSaberType = ad.objectData.disableBadCutSaberType;
    bool disableBadCutSpeed = ad.objectData.disableBadCutSpeed;

    // little opt: dont run it if it's not needed
    if (disableBadCutDirection || disableBadCutSaberType || disableBadCutSpeed) {
      bool directionOK, speedOK, saberTypeOK;
      float cutDirDeviation, cutDirAngle;
      NoteBasicCutInfoHelper::GetBasicCutInfo(self->_noteTransform, self->_noteData->colorType, self->_noteData->cutDirection, saber->saberType, saber->bladeSpeedForLogic,
                                              cutDirVec, self->_cutAngleTolerance, directionOK, speedOK, saberTypeOK, cutDirDeviation, cutDirAngle);

      if((disableBadCutDirection && !directionOK) || (disableBadCutSpeed && !speedOK) || (disableBadCutSaberType && !saberTypeOK)) {
        return;
      }
    }
  }

  GameNoteController_HandleCut(self, saber, cutPoint, orientation, cutDirVec, allowBadCut);
}

// postfix '_BadCutsModifier' because FakeNotes use that too
void InstallGameNoteControllerHooks_BadCutsModifier() {
  INSTALL_HOOK(NELogger::Logger, GameNoteController_HandleCut);
}

NEInstallHooks(InstallGameNoteControllerHooks_BadCutsModifier);