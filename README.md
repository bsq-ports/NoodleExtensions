# Noodle Extensions

A port of the Noodle Extensions mod made originally for the PC version of Beat Saber to the Oculus Quest platform.

## 1.40.8 To-Do

### This list is not final and will continue to grow

- [x] Fix note jump animation (notes simply pop into existence rather than flying into view)
- [x] Fix notes being invisible on pretty much all modcharts. Perhaps notes that are on a track?
- [x] Fix audio being drowned out for seemingly no reason. Common in Doppelganger. Perhaps an issue with walls?
- [x] Fix notes looking towards (0, 0, 0). Maybe calculated in `src/hooks/NoteJump.cpp`
- [x] Fix start beat offset
- [x] Implement `disableBadCut*` modifiers
- [x] Fix crashes when restarting a modchart
- [x] Fix crash when Replay tries to load a modchart replay
- [ ] Remove wall adjustments. Not only do they look different, but they were first added to get around poorly acting distortion effects, which is a solved problem with GraphicsTweaks
- [x] Investigate jump distance / speed
- [x] Implement saber trail parenting to player track
- [x] Fix saber trail culling
- [x] Fix parenting inconsistencies (likely order based e.g "Make a Wish")
- [x] Fix obstacle spawn, duration and speed e.g `Try This` the arrows are in the lane before moved. 
- [x] Fix obstacle rotation e.g flower in `Try`.
- [x] Fix V3 custom event animating offset position
- [x] Fix notes being double spawned
- [x] Fix CJD loading arcs and chains
- [x] Fix fake notes
- [x] Fix CJD VNJS
- [x] Fix Chroma V2 position and localPosition missing kNoteLinesDistance
- [ ] Beat Leader is generating invalid, corrupted replays and refrains from uploading them
- [ ] Fix some walls not moving and or spawning e.g. `Dark Heart Surgery`

## 1.40.8 Optional To-Do

- [x] Fix saber animations
- [ ] The texture on some scaled obstacles will bleed outside the obstacle
- [ ] Obstacle frames dissolve but not the fill
- [ ] Fix Chroma not overriding player's override environment and colors
- [ ] Fix lighting in maps such as Paradigm, Change of Scene, and BSSDHUYR Megamix 2023

### [Recorded](https://drive.google.com/drive/folders/1XNpSEJ44uwEr9L9W3qukeGNnQX24wF0U?usp=drive_link) Maps
Dates are (year/month/day)
- [ ] [1015](https://beatsaver.com/maps/1f304) (25/12/23)
  - [x] Crashes
  - [ ] 0:00 Obstacles are missing their frame outline
  - [x] 1:35 Weird environment geometry under player (Also 1.37)
  - [ ] 1:51 Obstacle dissolve doesn't work
- [ ] [Analys](https://beatsaver.com/maps/d00c) (25/11/25)
  - [ ] 0:50 The obstacles next to the player may not be positioned correctly
  - [ ] 0:56 Track does not rotate smoothly
  - [ ] 1:34 Circle of arrows despawn or does not follow player (Also 1.37)
- [ ] [As the World Caves In](https://beatsaver.com/maps/210e3) (25/12/4)
  - [ ] [Awaiting analysis]
- [ ] [BSSDHUYR Megamix 2023](https://beatsaver.com/maps/39736) (25/12/23)
  - [x] Crashes
  - [ ] Lighting is broken
  - [ ] 1:50 Unintentional hyper walls
  - [ ] 2:40 Floor obstacles don't move along the track
  - [x] 3:20 Notes with base provider animated color are black (Also 1.37)
  - [ ] 5:12 The "2023" wall text doesn't appear in the mirror
  - [ ] 5:27 The "4" wall text doesn't glow the same as the rest of the wall text (Also 1.37)
- [x] [Centipede](https://beatsaver.com/maps/293ab) (25/12/4)
  - [x] So many notes missing
- [ ] [Change of Scene](https://beatsaver.com/maps/3f19a) (25/12/4)
  - [x] Player is slightly elevated above the track
  - [x] 0:00 Title shows as CHANGE OF SCE--:
  - [x] 1:49 Track doesn't follow player
  - [ ] [Awaiting analysis]
- [ ] [C18H27NO3 (Mawntee)](https://beatsaver.com/maps/17d7e) (25/11/26)
  - [ ] [Awaiting analysis]
- [ ] [Disaster](https://beatsaver.com/maps/1dc64) (25/12/4)
  - [ ] [Awaiting analysis]
- [ ] [Doppelganger](https://beatsaver.com/maps/d53c) (25/12/23)
  - [x] Audio being drowned out
  - [x] Obstacles are scaled weirdly
  - [x] Exaggerated note movement
  - [x] Note dissolve isn't removed
  - [x] 1:23 Obstacle is positioned weirdly
  - [ ] 1:37 Obstacle scales while dissolving (currently just disabled)
  - [x] 2:08 Obstacles are at the wrong Y position
  - [x] 3:04 Obstacle timing issues
  - [x] 3:04 Track teleports with no smoothing
- [ ] [Echo](https://beatsaver.com/maps/269a7) (25/12/4)
  - [x] Environment cubes that make up the screen are missing
  - [ ] Environment cubes that make up the screen are slightly smaller than intended
  - [ ] Notes are being reflected in a mirror
  - [ ] Environment spike things are positioned incorrectly or missing
  - [x] 2:20 Rotated tracks are missing
- [ ] [Glorious Octagon of Destiny](https://beatsaver.com/maps/26ad0) (25/11/26)
  - [x] 0:19 "VIVA" text comes in from top and bottom rather than spinning in from sides
  - [ ] 0:25 Obstacle field is missing (Also 1.37)
  - [x] 0:39 Obstacles are missing
  - [ ] 0:47 Note arrows don't disappear on cut (Also 1.37)
  - [x] 0:52 Unintentional hyper walls
  - [x] 1:16 The two tracks are positioned on top of each other
  - [x] 1:30 Unintentional fast walls
  - [x] 1:44 Track doesn't rotate with player
  - [ ] 1:47 Wall text doesn't move
  - [x] 2:46 Some walls are unintentionally hyper, some aren't
  - [x] 3:05 Wall text is centered and doesn't scroll
  - [x] 3:14 Notes are missing
  - [x] 3:14 Obstacles are missing
  - [ ] 4:00 Many small white cubes float in the air
  - [x] 4:10 Notes and Super Hexagon obstacles are missing
  - [x] 4:31 "LEVEL FAILED" text is either further away or smaller than normal
  - [x] 4:31 Funny wall in first quadrant of recording
  - [x] 4:31 Notes are missing
  - [x] 4:52 Wall text is missing
  - [x] 5:21 Unintentional hyper walls
  - [x] 5:21 The "printing" obstacle doesn't move
  - [x] 5:33 Wall sign is far away
  - [ ] 5:33 Wall sign is difficult to see
  - [x] 5:42 Notes are missing
  - [x] 5:57 Wall arrows are on top of each other
  - [x] 5:57 Wall arrows don't duplicate (Also 1.37)
  - [ ] 6:09 Wall arrows don't fully disappear (Also 1.37)
  - [x] 6:26 Notes are positioned next to player
  - [x] 6:51 Unintentional hyper walls in Papyrus section
  - [ ] 7:27 Wall text missing letters (Also 1.37)
- [ ] [Heliov](https://beatsaver.com/maps/27ade) (25/12/23)
  - [ ] Lighting is broken
  - [ ] Fog is missing
  - [ ] 0:00 Saber trail disappears and reappears several times
  - [ ] 3:53 This environment scene can't be seen at all
  - [ ] 4:12 Bottom part of environment seems cut off
  - [ ] 4:15 Clouds are missing
- [x] [IGDWUTSWHWHMTC (QueenChief)](https://beatsaver.com/maps/20bc7) (25/11/26)
- [ ] [Lucy, The God Of Time](https://beatsaver.com/maps/15b16) (25/12/23)
  - [ ] Some walls aren't glowy enough
- [x] [Make a Wish](https://beatsaver.com/maps/1a32d) (25/12/4)
  - [x] 0:40 Unintentional hyper walls
- [ ] [Midnight Lady](https://beatsaver.com/maps/da60) (25/12/4)
  - [x] 0:46 Saxaphones have certain walls that rotate in opposite directions
  - [ ] [Awaiting analysis]
- [ ] [Noodle Animation Stuff](https://beatsaver.com/maps/1a53c) (25/12/4)
  - [ ] [Awaiting analysis]
- [ ] [NULCTRL Meiso Flip](https://beatsaver.com/maps/ed2a) (25/11/25)
  - [ ] All the white glowing walls are slightly transparent instead of solid white
  - [ ] 0:39 Walls are supposed to cover the wall text
  - [ ] 1:09 Arrows animation begins with max offset instead of slowly increasing offset
  - [ ] 1:18 Arrows are abnormally bright
  - 1:38 Note for later fixes: These walls don't have bleeding textures
- [ ] [Over Again](https://beatsaver.com/maps/2a093) (25/12/4)
  - [ ] [Awaiting analysis]
- [ ] [Paradigm](https://beatsaver.com/maps/3bbb0) (25/11/25)
  - [x] Crashes
  - [ ] Lighting is broken
  - [ ] Fog is missing (but maybe a PC only feature?)
  - [ ] 1:16 Sabers appear to fly in from above instead of from the front
  - [ ] 4:37 Head follows the sabers
  - [ ] 6:49 Something is obscuring light rays below the horizon
- [ ] [PAUSE](https://beatsaver.com/maps/17e36) (25/11/25)
  - [x] Notes are missing during the pause effect
  - [ ] 1:18 Unintentional hyper walls
- [ ] [Pictured as Perfect](https://beatsaver.com/maps/3f6e7) (25/12/4)
  - [ ] [Awaiting analysis]
- [x] [Real or Lie (Pixelguy)](https://beatsaver.com/maps/126e4) (25/11/26)
- [ ] [Signager](https://beatsaver.com/maps/12498) (26/11/25)
  - [ ] [Awaiting analysis]
- [ ] [Six Forty Seven (Reddek)](https://beatsaver.com/maps/2c89c) (25/12/4)
  - [x] All of the wall decorations are positioned very very wrong
  - [ ] [Awaiting analysis]
- [ ] [Six Forty Seven (SuperMemer417, nasafrasa)](https://beatsaver.com/maps/2a2bd) (25/11/26)
  - [ ] Environment pieces aren't being removed (Probably fixed, just needs a replay)
- [ ] [Takeoff](https://beatsaver.com/maps/2c1ff) (25/12/4)
  - [ ] Crashes
- [ ] [Team Grimoire (Salty)](https://beatsaver.com/maps/4b476) (26/1/28)
  - [ ] [Awaiting analysis]
  - [ ] [Very broken]
- [ ] [Try](https://beatsaver.com/maps/14d64) (25/12/4)
  - [x] 0:00 Flower is messed up
  - [ ] [Awaiting analysis]
- [ ] [Try This](https://beatsaver.com/maps/decf) (25/11/26)
  - [x] Arrow walls disappear early
  - [ ] 0:31 Unintentional hyper wall
- [x] [UUUUUUUUUUUUUUUUUUUUh](https://beatsaver.com/maps/217cf) (25/12/23)
  - [x] 2:00 Platform isn't moved down
- [ ] [Up & Down](https://beatsaver.com/maps/11cf8) (25/11/25)
  - [ ] Environment pieces aren't being removed
  - [x] 1:00 Animated notes spawn in late
- [ ] [Up & Down (Remastered)](https://beatsaver.com/maps/2c2f4) (25/12/23)
  - [x] Large column environment pieces are missing (26/1/28)
  - [x] 0:03 Track doesn't rotate with the player
  - [x] 0:33 Decorative obstacles are missing
  - [ ] 0:33 Only the left decorative obstacle is missing (26/1/28)
  - [x] 0:33 Half the notes are missing
  - [x] 1:05 Notes are missing
  - [ ] 1:05 Notes don't change color
  - [x] 1:54 Tease notes are missing
- [ ] [Wavetapper](https://beatsaver.com/maps/26660) (25/12/23)
  - [ ] White background is missing


## FAQ (Frequently Asked Questions)
- Why are the wall colors not the same as PC (desaturated, transparent etc.)?
  - Due to the Quest not having distortion on walls, Noodle Extensions will attempt to change the walls materials to solid if these conditions are met:
    - Wall color alpha (controlled by Chroma) is greater than or equal 0
    - Dissolve animation is being applied
- Bombs are not coloring/are always black
  - A bug specific to Quest causes bombs to lose their color/default to black when dissolve animation is being applied. It is not certain if this bug will be fixed or worked around.
- Why does Noodle Extensions disable my Qosmetics notes/walls?
  - Qosmetics notes/walls drastically reduce performance especially in Noodle maps and also ruin the artistic experience. 
- Why do you suggest disabling Mapping Extensions? It works fine for me
  - While it _could_ work, we didn't extensively test the impact of performance or stability using Mapping Extensions. It's at your discretion if you choose to use Mapping Extensions simultaneously with Noodle.
  - While some maps do "require" both Noodle Extensions and Mapping Extensions, this is not a supported scenario in either PC or Quest and should be discouraged.
- Where are Tracks/CustomJSONData QMod downloads?
  - Noodle Extensions and Chroma will download these dependencies automatically
- Noodle Extensions isn't loading/working and PinkCore says I don't have the mod installed
  - Try to reinstall Noodle Extensions and update Chroma to at least version 2.5.7 or newer.
- I found a map that doesn't work!!!!11!!11!/Noodle is missing a feature
  - You most likely downloaded the wrong map as most Noodle features are supported (no exceptions except the TODO)
  - In the case you are absolutely certain you found a bug/missing feature, report it in GitHub issues along with a log and steps to reproduce the issue. Footage of said map is also recommended being provided.

## Building from Source

You'll need [qpm_rust](https://github.com/RedBrumbler/QuestPackageManager-Rust), Powershell, Ninja, and CMake in `PATH`.

You'll also need the latest Android NDK. Create a file called `ndkpath.txt` containing the path to the directly where you extracted the ndk to.

```sh
qpm-rust restore
qpm-rust cache legacy-fix
pwsh ./build.ps1
pwsh ./createqmod.ps1 noodle_extensions
```

## What still needs to be done?
- Left handed mode
- Beatmap note/obstacle/bomb count (includes fakes)
- Some maps such as Centipede by totally balloon have fake notes registered as real notes
- Make Noodle only run on Noodle maps

