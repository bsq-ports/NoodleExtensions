#!/usr/bin/env python3
"""Run the actual cleanup body with strong-root stand-ins and an omission negative control."""
from pathlib import Path
import json, subprocess, tempfile
root=Path(__file__).resolve().parents[1]
s=(root/'src/NoodleExtensions.cpp').read_text()
start=s.index('void NECaches::ClearGameplayReferences() {')
end=s.index('\nvoid InstallAndRegisterAll()',start)
body=s[start:end]
prefix=r'''#include <memory>
#include <cassert>
#include <unordered_map>
struct Logger {template<class... T> void info(const char*, T...) {}};
namespace NELogger { inline struct Logger Logger; }
struct Node {};
struct NECaches {
 inline static std::shared_ptr<Node> GameplayCoreContainer, JumpOffsetYProvider,
   VariableMovementDataProvider, InitData, beatmapObjectSpawnController, noodleMovementDataProviderPool;
 inline static std::unordered_map<int,int> notes, walls, sliders, saberMovements;
 inline static std::shared_ptr<Node> saberOrigin;
 inline static bool hasLocalSpaceTrail=true,hasPlayerTransfrom=true;
 static void ClearNoteCaches() {notes.clear();}
 static void ClearObstacleCaches() {walls.clear();}
 static void ClearSliderCaches() {sliders.clear();}
 static void ClearSaberMovementCaches() {saberMovements.clear();saberOrigin=nullptr;}
 static void ClearGameplayReferences();
};
'''
suffix=r'''
int main() {
 for(int cycle=0;cycle<100;++cycle) {
   auto map=std::make_shared<Node>(); std::weak_ptr<Node> weak=map;
   NECaches::GameplayCoreContainer=map;NECaches::JumpOffsetYProvider=map;
   NECaches::VariableMovementDataProvider=map;NECaches::InitData=map;
   NECaches::beatmapObjectSpawnController=map;NECaches::noodleMovementDataProviderPool=map;
   NECaches::saberOrigin=map;map.reset();
   NECaches::notes[cycle]=cycle;NECaches::walls[cycle]=cycle;NECaches::sliders[cycle]=cycle;
   NECaches::saberMovements[cycle]=cycle;
   assert(!weak.expired());NECaches::ClearGameplayReferences();assert(weak.expired());
   assert(NECaches::notes.empty()&&NECaches::walls.empty()&&NECaches::sliders.empty()&&NECaches::saberMovements.empty());
   assert(!NECaches::hasLocalSpaceTrail&&!NECaches::hasPlayerTransfrom);
   NECaches::ClearGameplayReferences();
 }
}
'''
with tempfile.TemporaryDirectory(prefix='noodle-roots-') as tmp:
 p=Path(tmp)
 def run(name,method):
  (p/(name+'.cpp')).write_text(prefix+method+suffix)
  subprocess.run(['c++','-std=c++20','-I',str(root/'include'),str(p/(name+'.cpp')),'-o',str(p/name)],check=True)
  return subprocess.run([str(p/name)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL).returncode
 assert run('production',body)==0
 assert run('negative-control',body.replace('  GameplayCoreContainer = nullptr;','  /* deliberately retained for negative control */'))!=0
print(json.dumps({'production_cleanup_body':True,'strong_roots_release_cycles':100,'idempotence':True,'retained_container_negative_control_failed_as_expected':True,'passed':True}))
