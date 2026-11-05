# Courtyard demo

The demo is one flat, single-storey training ground. Concrete pillars provide
cover in an open courtyard, with low industrial buildings around the edge.
The lighting is dusk with four warm floodlights. All geometry and surface
patterns are built in the project; no Rainbow Six map or game assets are used.

## Quick walkthrough

1. Start the game and press Enter. Point out the courtyard, concrete slab
   joints, brick walls, floodlight fixtures and pillar shadows.
2. Press 1 for easy mode, then Tab. The timer starts only when captured.
3. Shoot the centre hostile's head without moving the aim. The remaining
   count drops from three to two, and the target falls.
4. Stay exposed briefly to show health decreasing and enemy barrel flashes.
5. Move behind a pillar to break sight. Counterfire stops and exposure resets.
6. Shoot a pillar to show the wall receiving the impact instead of a target
   behind it. Civilian targets also intercept bullets aimed through them.
7. Press Tab to pause. H opens the settings panel. Press N to show night vision and P to compare lighting.
   The gun's roughness and metallic sliders provide a material comparison.
8. Press R to return to a fresh round. Press B for the fixed benchmark.

Win and all three failure states have distinct HUD messages. R remains
available after an outcome. A larger map and multiple levels are later work.

## Bring the personal changes into the official repo

After GitHub access is granted, run these commands from a clean official
GitLab checkout. They preserve the existing commit authors and include the
camera/shooting foundation as well as the courtyard changes.

```sh
git remote add alfonso git@github.com:alfonsokkim/breachpoint.git
git fetch origin
git fetch alfonso
git switch -c courtyard-demo origin/main
git merge alfonso/main
cmake -S . -B build
cmake --build build -j 6
ctest --test-dir build --output-on-failure
git push -u origin courtyard-demo
glab mr create --source-branch courtyard-demo --target-branch main
```

If the `alfonso` remote already exists, use `git remote set-url alfonso` with
the same GitHub URL instead of adding it again. Review any merge conflicts
before proceeding. The official GitLab repository has not been updated by
this personal demo work.

## Verification on 5 November

The Mac build passes all 73 test cases and 421 assertions. Tests cover course
reachability, nearest-hit and wall occlusion, civilian interception, target
health, win/failure rules, exposure resets and frame-size-independent timers.
Separate code reviews are clean after fixing startup culling, elapsed-time
handling and shot ordering at the time limit.

Live checks reached a full course clear and player death, recorded a civilian
penalty, and checked pause, restart, difficulty, night vision and the B-key
benchmark. The CLI benchmark completed 240 samples at 1920 x 1080, with a mean
of 0.7541 ms/frame for this scene. This is a baseline, not an optimisation
comparison. The screenshot export was also opened and checked.

The gameplay HUD replaces the permanent control sidebar. Health, time,
remaining hostiles and civilian strikes stay visible, with an exposure meter,
damage feedback, hit markers and ready/pause/outcome prompts. H or F3 toggles
the settings panel. The updated HUD was checked at 1280 x 720 and 1920 x 1080,
including a completed round. The 73 tests still pass and a separate HUD review
found no actionable issues.

## Scope

Current: custom courtyard, tactical targets and civilian models, health,
head/body hits, falling targets, exposure-based counterfire, difficulty,
timer, pause, reset, win/loss HUD, floodlights, static world shadows,
Cook-Torrance controls and night vision. The existing collision, shot queries,
muzzle lighting and particles remain connected.

The static shadow maps cover courtyard geometry. Moving targets do not cast
dynamic shadows. One damage-flash light is selected when multiple enemies fire
together, while every active enemy still renders its muzzle flash. The current
demo has no moving enemies, audio, stairs or second level. VLAB runtime checks
remain pending.
