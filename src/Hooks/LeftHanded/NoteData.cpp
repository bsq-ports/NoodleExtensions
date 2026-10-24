#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/NoteData.hpp"

#include "AssociatedData.h"

#include "NEHooks.h"
#include "custom-json-data/shared/CustomBeatmapData.h"

using namespace GlobalNamespace;

// MAKE_HOOK_MATCH(NoteData_Mirror, &NoteData::Mirror,
//                 void, NoteData *self, int lineCount) {
//     static auto CustomNoteDataKlass = i2c::class_of<CustomJSONData::CustomNoteData*>();
//     if (self->klass != CustomNoteDataKlass)
//     {
//         return NoteData_Mirror(self, lineCount);
//     }
//
//     auto customNoteData = reinterpret_cast<CustomJSONData::CustomNoteData*>(self);
//
//     auto& ad = getAD(customNoteData->customData);
//
//
//
//     if (ad.objectData.startX) {
//         ad.objectData.startX = ((ad.objectData.startX.value() + 0.5f) * -1) - 0.5f;
//     }
//
//     if (ad.flip) {
//         ad.flip->x = ((ad.flip->x + 0.5f) * -1) - 0.5f;
//     }
//
//     if (ad.objectData.localRotation) {
//         ad.objectData.localRotation->y *= -1;
//         ad.objectData.localRotation->z *= -1;
//     }
//
//     if (ad.objectData.rotation) {
//         ad.objectData.rotation->y *= -1;
//
//         // TODO: If this breaks, I blame Aero for having single float as rotation values
//         ad.objectData.rotation->z *= -1;
//     }
//
//     // TODO: If this breaks, I blame Aero for having single float as rotation values
//     if (ad.objectData.cutDirection) {
//         ad.objectData.cutDirection = NEVector::Quaternion::Euler(360 -
//         ad.objectData.cutDirection->get_eulerAngles().y);
//     }
//
//     return NoteData_Mirror(self, lineCount);
// }
//
// void InstallNoteDataHooks(Logger &logger) {
//     INSTALL_HOOK(NELogger::Logger, NoteData_Mirror);
// }
//
// NEInstallHooks(InstallNoteDataHooks);