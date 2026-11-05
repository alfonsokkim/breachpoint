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
`ShotHit::normal` is the outward entry-face normal. A zero-distance hit starts
inside or on a box and does not emit impact dust.

## Shot particles

Accepted shots emit six warm muzzle sparks. A resolved hit also emits twelve
grey target particles or brown block particles at the hit position, offset
0.025 units along the entry normal. Misses produce no impact burst. The wall
demo therefore emits dust on the nearer wall, never on the hidden target.

`ShotParticles` uses elapsed seconds for motion, gravity and fading. Each
particle lasts approximately 0.3 to 0.8 seconds, and the system keeps at most
128 live particles. A bounded segment query removes particles before they
cross solid geometry. Expired particles are removed and resets clear them.

The renderer batches all live particles and the flash into one point draw.
The maximum is 129 points including the flash, with no extra triangles. The
benchmark clears and disables both effects, using the workload name
`voxel-hut-character-and-weapon` to distinguish this scene from earlier runs.

## Check it

Each accepted shot creates a 0.12-second muzzle flash and a warm point light.
The simple weapon follows the camera. The flash sits just beyond its barrel;
its position is clipped against the real solid boxes near walls. Shooting
still uses the centre camera ray, so the weapon offset does not change aim.
The world shader uses ambient, diffuse and specular terms with world-space
positions and transformed normals. The barrel is shinier than the wood.
This is basic lighting without shadow maps.

The idle scene is 1,360 triangles and nine draw calls. The active flash adds
one point sprite and one draw call. Point sprites use depth testing, no depth
writes and alpha blending. The point-size range is clamped to the GPU limit.
The benchmark resets and disables the flash; its workload includes the gun.

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
   particles, and disables firing and the demo switch.
7. Shoot a target or wall and watch the dust fade. After a pause the panel
   should return to zero effect points and nine scene draw calls.

Target health, deaths, civilian labels/penalties, timer and difficulty remain
gameplay integration work. Jason can route `ShotHit::kind` and its part index
into target health or civilian rules; the current demo does neither. T is a
demo reset control, not enemy movement or behaviour.
