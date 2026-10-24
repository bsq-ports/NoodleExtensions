#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/hooking.hpp"

#include "GlobalNamespace/DisappearingArrowControllerBase_1.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/CutoutEffect.hpp"

#include "UnityEngine/Vector3.hpp"

#include "Zenject/MonoMemoryPool_1.hpp"
#include "Zenject/MemoryPoolBase_1.hpp"
#include "Zenject/MemoryPoolSettings.hpp"

#include "System/Collections/Generic/Stack_1.hpp"

#include "NEHooks.h"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace Zenject;

template <>
struct ::i2c::metadata_getter<&Zenject::MemoryPoolBase_1<Il2CppObject*>::Despawn> {
  static MethodInfo const* method_info() {
    return i2c::find_method(i2c::class_of<Zenject::MemoryPoolBase_1<Il2CppObject*>*>(), {"Despawn", 1});
  }
};

// template<>
// struct
// ::i2c::metadata_getter<&System::Collections::Generic::Stack_1<ObstacleController*>::Push>
// {
//     static const MethodInfo* get() {
//         auto methodInfo =
//         il2cpp_utils::FindMethod(i2c::class_of<System::Collections::Generic::Stack_1<ObstacleController*>*>(), "Push",
//         std::vector<Il2CppClass*>{}, il2cpp_utils::TypesFrom({i2c::class_of<ObstacleController*>()}));
//
//     }
// };

///* Void Despawn(CutScoreBuffer) */
// void MemoryPoolBase_1_CutScoreBuffer__Despawn
//              (MemoryPoolBase_1_CutScoreBuffer_ *this,CutScoreBuffer *item,MethodInfo *method)
//{
// byte bVar1;
// int iVar2;
// Object *p1;
// if ((DAT_033f3b90 & 1) == 0) {
//   thunk_FUN_00f7f4c0(0x5234);
//   DAT_033f3b90 = 1;
// }
// if (this->_inactiveItems != (Stack_1_CutScoreBuffer_ *)0x0) {
//   bVar1 = (**(code **)method->klass->rgctx_data[10])(this->_inactiveItems,item);
//   p1 = (Object *)Object_GetType(this,0);
//   Assert_1_That_2((bool)(~bVar1 & 1),StringLiteral_Tried_to_return_an_item_to_pool_,p1,
//                   (MethodInfo *)0x0);
//   this->_activeCount = this->_activeCount + -1;
//   if (this->_inactiveItems != (Stack_1_CutScoreBuffer_ *)0x0) {
//     (**(code **)method->klass->rgctx_data[3])(this->_inactiveItems,item);
//     (*(this->klass->vtable).OnDespawned.methodPtr)
//               (this,item,(this->klass->vtable).OnDespawned.method);
//     if (this->_inactiveItems != (Stack_1_CutScoreBuffer_ *)0x0) {
//       iVar2 = (**(code **)method->klass->rgctx_data[6])();
//       if (this->_settings != (MemoryPoolSettings *)0x0) {
//         if (this->_settings->MaxSize < iVar2) {
//                   /* WARNING: Could not recover jumptable at 0x01f0c4a8. Too many branches */
//                   /* WARNING: Treating indirect jump as call */
//           (**(code **)method->klass->rgctx_data[0xc])(this);
//           return;
//         }
//         return;
//       }
//     }
//   }
// }
//                   /* WARNING: Subroutine does not return */
// FUN_00fc4d04();
// }

// I love generics and il2cpp
MAKE_HOOK(MemoryPoolBase_Despawn, (nullptr), void, MemoryPoolBase_1<Il2CppObject*>* self, Il2CppObject* item,
          MethodInfo* methodInfo) {
  static auto* ObstacleControllerKlass = i2c::class_of<ObstacleController*>();

  if (!item || (item->klass != ObstacleControllerKlass
                //    /* && !i2c::functions::class_is_assignable_from(ObstacleControllerKlass, item->klass) */ //
                //    TODO: Is this future proofing even needed?
                )) {
    MemoryPoolBase_Despawn(self, item, methodInfo);
    return;
  }

  if (self) {
    self->_activeCount--;
    if (self->_inactiveItems) {
      self->_inactiveItems->Push(item);
    }
    self->OnDespawned(item);

    if (self->_inactiveItems && self->_settings) {
      if (self->_inactiveItems->_size > self->_settings->MaxSize) {
        self->Resize(self->_settings->MaxSize);
      }
    }
  }
}

// MAKE_HOOK_MATCH(MemoryPoolBase_Despawn,
//                 &MemoryPoolBase_1<Il2CppObject*>::Despawn, void,
//                 MemoryPoolBase_1<Il2CppObject*> *self,
//                             Il2CppObject* item) {
//
// }

void InstallMemoryPoolBaseHooks() {
  auto mInfo =
      i2c::metadata_getter<&Zenject::MemoryPoolBase_1<Il2CppObject*>::Despawn>::method_info();
  INSTALL_HOOK(NELogger::Logger, MemoryPoolBase_Despawn, (void*)(mInfo->methodPointer));
}

NEInstallHooks(InstallMemoryPoolBaseHooks);