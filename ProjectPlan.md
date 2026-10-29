# Breachpoint

COMP3421 Group Project Plan

## Group members

| Member            | zID      | Planned workload |
| ----------------- | -------- | ---------------- |
| Alfonso Kim       | z5587687 | 25 to 30 hours   |
| Jason Luo (Jubby) | z5589683 | 25 to 30 hours   |

## Game concept and gameplay

Breachpoint is a first-person tactical training game: clear a dark voxel house room by room, identifying hostile targets and avoiding civilians.

Neutralise all hostiles before time expires. Lose at zero health, timeout, or two civilian hits. Clear enemy sight starts an exposure timer, then repeated damage and barrel flashes. Lost sight resets exposure. HUD: health, time, hostiles remaining, civilian strikes.

### Controls

| Input          | Action                                          |
| -------------- | ----------------------------------------------- |
| Enter / Tab    | Start / capture or release the mouse            |
| WASD and mouse | Walk on the horizontal plane and look           |
| Left click     | Fire                                            |
| L              | Toggle the weapon flashlight                    |
| N / F          | Toggle night vision / hip-fire laser (optional) |
| Esc            | Quit                                            |

### Levels

| Configuration           | Design                                                                                                    |
| ----------------------- | --------------------------------------------------------------------------------------------------------- |
| Level 1: Training range | Single floor, open sightlines, obvious hostiles, and a longer time limit.                                 |
| Level 2: Night house    | Two floors, tight rooms, civilians mixed with hostiles, delayed threat reveals, and a shorter time limit. |

Tab releases the mouse for menus. Restart and level switching reset the camera, health, time, targets, strikes, exposure, effects, and outcome.

## Visual and technical design

| Area                     | Planned implementation                                                                                                                                                                                                             |
| ------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| World and camera         | A 32 x 32 x 32 voxel grid, first-person camera, textured block meshes, and player collision. Diagnostics show camera pose, faces, triangles, and scene draw calls.                                                                 |
| Lighting and materials   | GLSL ambient, diffuse, and specular lighting with dim room lights, a soft weapon spotlight, and a brief muzzle-flash light. Matte walls and cardboard will contrast with shiny metal.                                              |
| Hierarchy and animation  | Weapon hierarchy: body, slide, sights and flashlight; animate slide and recoil. Target hierarchy: base, torso, head, arms and optional gun; animate falls and a barrel flash on each enemy damage pulse.                           |
| Shooting and counterfire | Shared world-space box queries select the nearest target part and check solid-voxel occlusion. Bounded enemy-to-player sight checks drive exposure and repeated damage, with gun-barrel flashes but no projectiles or pathfinding. |
| Optimisation             | Compare all-face and hidden-face-culled meshes with the same scene, camera, materials, effects, and frozen game state. Use 60 warm-up and 240 measured frames at 1920 x 1080; report mean ms/frame and geometry counts.            |
| Challenge tasks          | After the core game: time-based particles with motion and lifetimes for sparks and impact dust; Cook-Torrance BRDF with interactive roughness and metallic controls.                                                               |
| Optional extension       | Optional after core: grainy night vision and hip-fire laser. Dot ends at first wall/target; beam visible in night vision.                                                                                                          |

## Responsibilities and integration

| Work area              | Alfonso Kim                                                                | Jubby                                                                                | Integration                                                                         |
| ---------------------- | -------------------------------------------------------------------------- | ------------------------------------------------------------------------------------ | ----------------------------------------------------------------------------------- |
| World and gameplay     | Camera, nearest-hit and line-of-sight queries, diagnostics, and benchmark. | Voxel storage, house builder, level layouts, chunk mesh, and player collision.       | Jubby supplies solid boxes and collision corrections; nearest wall/target hit wins. |
| Models and interaction | Weapon model, firing, recoil, and player/enemy muzzle-flash rendering.     | Target models, exposure timer, repeated damage, and hit animation.                   | Jubby's damage pulse triggers enemy flash/light; lost sight resets exposure.        |
| Rendering              | Phong lighting, flashlight/laser, muzzle flashes and particles.            | Textures, BRDF controls and night vision.                                            | Check consistent coordinates, materials, effects, and damage feedback.              |
| Game state and UI      | Supply visibility queries and effect triggers to game logic.               | Health, rules, HUD, menus, level switching, counterfire reset, and full reset logic. | Reset camera, effects and game state; review branches before merging.               |
| Delivery               | Optimisation results and VLAB checks.                                      | Level and gameplay testing.                                                          | Record asset licences, prepare report evidence, and rehearse the presentation.      |

## Milestones

Lab discussion: 29 October 2026. Internal targets, subject to tutor feedback:

| Date        | Milestone           | Working result                                                                                                                                                                                      |
| ----------- | ------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 1 November  | Minimum playable    | One 32 x 32 x 32 level: walking, collision, wall-blocked hits, timer and health/hostile HUD.                                                                                                        |
| 8 November  | Complete version    | Two levels, complete reset and switching, win and failure states, line-of-sight counterfire with damage and muzzle flash, both hierarchies, lighting, particles, BRDF, and optimisation comparison. |
| 14 November | Polished submission | Balanced gameplay, consistent visuals, reproducible benchmark results, VLAB or CSE build/run check, report evidence, and presentation rehearsal.                                                    |

Revised plan due: 23:59 on 1 November 2026, after tutor feedback. Final project due: 23:59 on 15 November 2026. Live presentation: Week 10 lab.
