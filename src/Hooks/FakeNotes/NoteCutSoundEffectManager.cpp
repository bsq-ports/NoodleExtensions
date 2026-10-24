#include "beatsaber-hook/shared/hooking.hpp"
#include "beatsaber-hook/shared/api.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/types.hpp"
#include "beatsaber-hook/shared/utils.hpp"

#include "GlobalNamespace/NoteCutSoundEffectManager.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "UnityEngine/Time.hpp"

#include "FakeNoteHelper.h"
#include "NEHooks.h"
#include "SharedUpdate.h"
#include "custom-json-data/shared/CustomBeatmapData.h"
#include "custom-types/shared/coroutine.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;

static NoteCutSoundEffectManager* currentSoundEffectManager;
static int lastFrame = -1;
static int cutCount = -1;
static int const maxNotesPerFrame = 30;
static std::vector<NoteController*> hitsoundQueue;

static bool ProcessHitSound(NoteController* noteController) {
  int frameCount = Time::get_frameCount();
  if (frameCount == lastFrame) {
    cutCount++;
  } else {
    lastFrame = frameCount;
    cutCount = 0;
  }

  return cutCount < maxNotesPerFrame;
}

custom_types::Helpers::Coroutine AddNotesLater() {
  while (currentSoundEffectManager) {
    if (hitsoundQueue.empty()) co_yield nullptr;

    int notesRemaining = maxNotesPerFrame - cutCount;

    notesRemaining = std::clamp(notesRemaining, 0, (int)hitsoundQueue.size());

    for (int i = 0; i < notesRemaining; i++) {
      auto noteController = hitsoundQueue.back();
      hitsoundQueue.pop_back();

      currentSoundEffectManager->HandleNoteWasSpawned(noteController);
    }

    cutCount += notesRemaining;

    co_yield nullptr;
  }
  co_return;
}

MAKE_HOOK_MATCH(NoteCutSoundEffectManager_Start, &NoteCutSoundEffectManager::Start, void,
                NoteCutSoundEffectManager* self) {
  if (!Hooks::isNoodleHookEnabled()) return NoteCutSoundEffectManager_Start(self);

  currentSoundEffectManager = self;
  cutCount = 0;
  hitsoundQueue.clear();
  self->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(AddNotesLater()));
  NoteCutSoundEffectManager_Start(self);
}

MAKE_HOOK_MATCH(NoteCutSoundEffectManager_HandleNoteWasSpawned, &NoteCutSoundEffectManager::HandleNoteWasSpawned, void,
                NoteCutSoundEffectManager* self, NoteController* noteController) {
  if (!Hooks::isNoodleHookEnabled()) return NoteCutSoundEffectManager_HandleNoteWasSpawned(self, noteController);

  if (!FakeNoteHelper::GetFakeNote(noteController->_noteData)) {
    if (ProcessHitSound(noteController)) {
      NoteCutSoundEffectManager_HandleNoteWasSpawned(self, noteController);
    } else {
      hitsoundQueue.emplace_back(noteController);
    }
  }
}

void InstallNoteCutSoundEffectManagerHooks() {
  INSTALL_HOOK(NELogger::Logger, NoteCutSoundEffectManager_Start);
  INSTALL_HOOK(NELogger::Logger, NoteCutSoundEffectManager_HandleNoteWasSpawned);
}
NEInstallHooks(InstallNoteCutSoundEffectManagerHooks);