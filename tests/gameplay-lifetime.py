#!/usr/bin/env python3
"""Exercise the production scene-lifetime guard with repeated gameplay/scene transitions."""
from pathlib import Path
import json, subprocess, tempfile
root=Path(__file__).resolve().parents[1]
source=r'''#include "GameplaySceneLifetime.hpp"
#include <cassert>
int main() {
  GameplaySceneLifetime lifetime;
  assert(!lifetime.Unloaded(0));
  lifetime.Loaded(90,"BTSEnvironment");
  assert(!lifetime.Unloaded(90));
  int releases=0;
  for (int map=1; map<=100; ++map) {
    lifetime.Loaded(map,"GameCore");
    lifetime.Loaded(1000+map,"Environment");
    assert(!lifetime.Unloaded(1000+map));
    // Empty transition/menu scene loads cannot release in-flight gameplay.
    lifetime.Loaded(2000+map,"EmptyTransition");
    lifetime.Loaded(3000+map,"MainMenu");
    assert(!lifetime.Unloaded(2000+map));
    if (lifetime.Unloaded(map)) ++releases;
    assert(!lifetime.Unloaded(map));
  }
  assert(releases==100);
  // If a new gameplay scene is installed before the old one unloads,
  // that old unload must not clear the newly installed gameplay references.
  lifetime.Loaded(500,"GameCore");
  lifetime.Loaded(501,"GameCore");
  assert(!lifetime.Unloaded(500));
  assert(lifetime.Unloaded(501));
  lifetime.Loaded(600,"GameCore");
  const bool unloadedOld=lifetime.Unloaded(600);
  // Original sceneUnloaded callback synchronously starts a replacement scene.
  lifetime.Loaded(601,"GameCore");
  bool releasedReplacement=unloadedOld && !lifetime.HasGameplay();
  assert(!releasedReplacement);
  assert(lifetime.Unloaded(601));
  assert(!lifetime.HasGameplay());
}
'''
with tempfile.TemporaryDirectory(prefix="gameplay-lifetime-") as tmp:
 path=Path(tmp); (path/"test.cpp").write_text(source)
 subprocess.run(["c++","-std=c++20","-Wall","-Wextra","-Werror","-I",str(root/"include"),str(path/"test.cpp"),"-o",str(path/"test")],check=True)
 subprocess.run([str(path/"test")],check=True)
print(json.dumps({"production_helper":"include/GameplaySceneLifetime.hpp","gameplay_cycles":100,"additive_unloads_do_not_release":True,"duplicate_unloads_do_not_release":True,"replaced_scene_guard":True,"reentrant_original_callback_guard":True,"passed":True}))
