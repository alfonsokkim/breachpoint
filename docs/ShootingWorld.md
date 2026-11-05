# Shooting on the voxel map

This branch builds on the camera/map integration in MR !4. Each part of the
shooting demo is added in its own commit so it can be reviewed separately.

## Current controls

Enter starts the scene. Tab captures/releases the mouse. WASD moves and the
mouse aims. Left click fires while captured; holding it does not repeat fire.
The minimum firing interval is 0.25 seconds and the range is 50 world units.
While captured, T switches between the clear and wall-blocked demonstrations,
resetting the camera, target position, shot count and feedback.

The spawn faces Steve's head. A target hit briefly makes the crosshair green;
a solid hit makes it amber; a miss makes it red. The panel records the part or
block index and hit distance. This reports hits without applying health or
death behaviour.

## Part bounds and shot queries

`Character::getHitBounds()` returns six world-space boxes in model-part order:
head, torso, left arm, right arm, left leg, right leg. The UI numbers these
from 1, while the returned vector and `ShotHit::index` use zero-based indices.

Each box is calculated from the same root and part transforms used to render
the mesh. `transformBounds` transforms all eight corners, so rotation and
negative or nonuniform scales still produce ordered enclosing bounds. It is
for affine model matrices, not perspective projection matrices.

For example, Steve's head is 8 x 8 x 8 skin pixels. With the model scaled by
1/16 and placed at feet `(11.5, 5, 15.5)`, its bounds are
`(11.25, 6.5, 15.25)` to `(11.75, 7, 15.75)`. The spawn ray hits that head at
3.75 units. The box vector is cached rather than rebuilt every frame.
`Character::setFeetPosition` updates the root transform and cached bounds
together without recreating the mesh buffers.

`resolveShot` compares the nearest character-part hit with the nearest solid
voxel hit. A nearer solid blocks the target, and solids win equal-distance
ties. `ShotHit::kind` identifies which vector owns the returned index.

## Check it

1. Start, capture the mouse and click without moving the aim. The panel should
   report target part 1 at 3.75 units.
2. Hold the button. Only one shot should be added.
3. Press T and click. Steve is now inside the hut. The shot hits a real wall
   block at about 2.13 units, before the torso. The panel reports that a target
   was behind the nearer wall. This uses a target-only ray query to confirm
   the obstruction rather than assuming it from the demo label.
4. Press T and click again. The exposed head is hittable again at 3.75 units.
5. Release the mouse. Clicking the controls must not fire or switch the demo.
6. The benchmark resets the outside target and spawn, clears shot feedback and
   disables firing and the demo switch.

Target health, deaths, civilian labels/penalties, timer and difficulty remain
gameplay integration work. Firing effects are the next increments on this
branch.
