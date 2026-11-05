# Breachpoint

A first-person live-fire training demo for COMP3421. The current level is a
flat concrete courtyard at dusk, with standing pillars, industrial perimeter
buildings and four floodlights. Armed tactical targets return fire after
sustained line of sight. Blue-clothed civilians must be spared.

This is the personal demo copy. The official group repository remains on
[CSE GitLab](https://gitlab.cse.unsw.edu.au/coursework/COMP3421/26T3/groups/H16C_C/group-assignment).
The larger level will be added later.

## Build and run

```sh
cmake -S . -B build
cmake --build build -j 6
./build/Craft
```

Or run `./run.sh` from any working directory. CMake downloads the starter's
dependencies on the first build. Original setup notes and references are in
[docs/StarterNotes.md](docs/StarterNotes.md).

## Controls

| Input | Action |
| ----- | ------ |
| Enter | Leave the title screen |
| Tab | Begin or resume; release the mouse to pause |
| WASD and mouse | Walk and aim |
| Left click | Fire once, with a 0.25-second cooldown |
| R | Reset the round, targets, camera and effects |
| 1 / 2 / 3 | Restart on easy / normal / hard |
| N | Toggle grainy night vision |
| P | Switch Cook-Torrance and Phong lighting |
| H / F3 | Show or hide settings, controls and diagnostics |
| B | Run the fixed 240-frame benchmark |
| Esc | Quit |

The gameplay HUD shows health, time, hostiles, civilian strikes, shot count
and night-vision status. Exposure warnings show when an enemy has sight;
damage flashes at the screen edges and target hits show a hit marker.
Settings and diagnostics are hidden by default.

Press H, then release the mouse with Tab to use the rendering controls. Roughness and metallic
sliders change the gun material. Render preferences persist across a round
restart; the benchmark resets them to a fixed configuration.

## Round rules

- Three armed hostiles and two civilians occupy the courtyard.
- One head or helmet hit, or two body hits, neutralises a hostile.
- Targets fall after being neutralised. Fallen targets no longer block shots.
- Clear sight starts an enemy exposure timer. After the delay, damage pulses
  reduce player health and trigger a flash at that enemy's gun.
- A pillar or wall breaks sight and resets the enemy's exposure timer.
- Win by neutralising all hostiles. Lose at zero health, timeout, or two
  separate civilian casualties.

| Difficulty | Time limit | Exposure delay | Damage pulse |
| ---------- | ---------- | -------------- | ------------ |
| Easy | 120 s | 3 s | 5 health every 1 s |
| Normal | 90 s | 2 s | 8 health every 1 s |
| Hard | 60 s | 1.25 s | 12 health every 0.7 s |

Enemies stand in place. Counterfire uses visibility checks and timed damage,
with no projectile simulation or pathfinding.

## Rendering and checks

The courtyard uses procedural concrete, brick and painted materials. Four
spotlights have cached 1024-pixel shadow maps for the static world. Target
gear includes a helmet and vest, and each hostile carries a basic rifle.
Muzzle flashes and fading particles use one batched point draw. HDR rendering
is tone-mapped before the optional night-vision pass; the HUD stays readable.
Cook-Torrance uses GGX, Smith masking and Schlick Fresnel.

```sh
ctest --test-dir build --output-on-failure
CRAFT_BENCHMARK=1 ./build/Craft
```

Set `CRAFT_SCREENSHOT=/absolute/path/preview.png` to export a frame. During a
CLI benchmark it is saved after measurement, so file writing is excluded.

The benchmark fixes the camera, round and rendering state, clears transient
effects, and measures 240 scene frames after 60 warm-up frames. It includes
scene presentation but excludes the HUD and swap. Cached shadow-map creation
happens before measurement. Its workload is `training-courtyard`.

See [docs/Demo.md](docs/Demo.md) for the walkthrough and transfer steps.
