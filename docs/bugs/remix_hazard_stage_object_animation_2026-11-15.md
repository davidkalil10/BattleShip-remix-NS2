# Pirate Land cannon and Rainbow Road Chain Chomp did not animate (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.1, lot A, items A1 and A4).

## Symptoms

- Pirate Land (stage 0x9E): the cannon on the ship showed no barrel. The barrel never moved to -128, 0 or +128, the 1.125 scale
  pulse of a shot and the cannon animation were missing, and the cannonballs left from a point where nothing was drawn.
- Rainbow Road (stage 0xA3): a Chain Chomp spawn never showed a charging chomp. The model stayed on the first frame of its
  animation at the spawn point (off camera), and its hitbox weapon, which follows the root joint, stayed there with it for the
  whole 0x546 frames.

## Root cause

Both stage objects are built by the asm with `Render.STAGE_OBJECT_INIT_` (0x8000F590, `gcSetupCustomDObjs`) with `tk1 = 0x1C`
(`nGCMatrixKindTraRotRpyRSca`), and both register `gcPlayAnimAll` (0x8000DF34) as a per-frame process
(`Render.REGISTER_OBJECT_ROUTINE_`, kind 1, priority 5).

- `grremixpiratecannon.c` called `grModelSetupGroundDObjs` with a static `DObjTransformTypes` array that nothing filled, so every
  kind was `nGCMatrixKindNull`: no DObj of the cannon got a matrix, and translate, rotate and scale never reached the draw
  (Hazards.asm 3770-3780).
- `grremixrainbow.c` ran `gcPlayAnimAll` once at the spawn and never again (Hazards.asm 5737-5743). The same file also drew the
  chomp on display link 6 with `gcDrawDObjTreeForGObj`, where the asm uses link 0x11 with `grDisplayLayer0PriProcDisplay`
  (0x80104D90, no z-buffer; Hazards.asm 5683, 5707-5715).

## Fix

- Pirate Land: `gcSetupCustomDObjs(..., nGCMatrixKindTraRotRpyRSca, nGCMatrixKindNull, nGCMatrixKindNull)`, as the asm.
- Rainbow Road: `gcAddGObjProcess(chomp_gobj, gcPlayAnimAll, nGCProcessKindFunc, 5)`; the display is added at creation on link
  0x11 with `grDisplayLayer0PriProcDisplay` and the GObj starts hidden (asm GObj 0x7C = 1), and every spawn clears the flag.

## Evidence

- gdb on the cannon root DObj: `xobjs_num` 0 on the root and both children before, 1 after (`m14a_pirategdb_*`). The
  screenshots show the barrel on the ship after the fix and not before.
- Rainbow Road with the spawn roll forced: before, no chomp in any shot from frame 1500 to 2100; after, the chomp comes down the
  track, hits Mario (75%) and knocks him out (`m14a_rainbowgdb_*`).

## Lesson

A static `DObjTransformTypes` array passed to `grModelSetupGroundDObjs` must be filled, or the call draws nothing. When a stage
object of `Hazards.asm` uses `STAGE_OBJECT_INIT_`, port it with `gcSetupCustomDObjs` and the same kinds, and check every
`REGISTER_OBJECT_ROUTINE_` of the setup for a process the port must add.
