# Camera and world integration

Review the new commits in this order:

1. `hook up the camera to jasons map`: combines A2's camera with Jason's
   existing chunk and character. Spawn is `(11.5, 6.6, 19.5)`, looking towards
   the hut. The chunk shader takes the camera's combined view/projection matrix.
2. `use the maps blocks for world queries`: the chunk caches one unit AABB per
   solid voxel while building its mesh. The camera's aim query uses that list.
3. `stop the player walking through walls`: applies a swept player body to
   camera movement, with floor-edge protection, tests and these notes.

The older A2 commit remains in the branch history. Jason's map commits are
preserved as parents of the integration. A3 shooting is on a separate branch.

## Collision

The player's body is 0.5 units wide and 1.8 units tall, with eyes 1.6 units above
the feet. On this platform, feet are at y=5 and eyes at y=6.6. The body fits
through the one-block-wide, two-block-tall doorway without touching the roof.

`movePlayer` resolves X and Z separately, stopping at the nearest wall face.
This allows sliding along a wall and prevents crossing it in a large movement
step. Touching a floor or wall is allowed; overlapping a solid is not.

Each foot corner must stay on connected floor surfaces at the existing feet
height. Ground checks follow touching block-top intervals, so movement crosses
voxel seams but stops at a gap or platform edge. Movement remains horizontal.
Gravity, jumping, slopes, stairs and collision with Steve are outside this step.

The boxes come directly from the chunk's blocks, not a second handwritten map.
`getSolidBounds()` returns a const reference valid until the mesh is rebuilt.
The current chunk model matrix is identity, so cell boxes are world-space boxes.

## Try it

Build with CMake, run `./build/Craft`, press Enter and then Tab. Use WASD and
mouse look. Tab releases the mouse; Esc quits.

- Walk into the hut's wall and try a diagonal approach. You should stop or slide.
- Approach the opening from outside. You should be able to enter and leave.
- Walk towards every edge of the platform. Your body should stay supported.
- Look at the walls and floor. The panel should show a solid-block hit distance.
- Release the mouse and use the benchmark button. It should reset to the same
  spawn and freeze movement until the run completes.

The tests cover raised floors, walls in both directions, large movements,
diagonal sliding, doorway clearance, edges, gaps, ordering and camera correction.
VLAB/CSE build and live input checks are still required there. The benchmark
workload is `voxel-hut-and-character`, not an optimisation comparison.
