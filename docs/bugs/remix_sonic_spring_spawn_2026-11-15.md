# Sonic's up special spring vanished on its first frame (2026-11-15)

Found while testing the M14 equivalence audit (ROADMAP T14.1, lot C, item C3 needs a fighter to land on the spring).

## Symptoms

- Sonic's up special (ground or air) launched Sonic but no spring stayed behind: nobody could bounce on it, and the "spring
  bounce" rules of every character (Fox reflector, Kirby Final Cutter, Ness PSI magnet...) never ran.
- Under gdb the spring was made at the right X and a Y of about 1.8e14 (`m14c_c3_dbg`: `SPAWN x=-1800.0 y=179188166295552.0`),
  then destroyed the same frame by the blast-zone test of the weapon process.

## Root cause

`ftRemixSonicUSPCreateSpring` (`decomp/src/ft/ftremix/sonic/ftremixsonicusp.c`) passed an uninitialized `Vec3f` to
`gmCollisionGetFighterPartsWorldPosition`. That function (0x800EDF24) transforms the vector in place: the input is the offset
in the joint's space. The asm (`SonicSpecial.asm` 1312-1316, `_create_spring`) writes 0 to x, y and z before the call. The port
took whatever the stack held, so the spawn position depended on the build and the previous call.

## Fix

`spawn_pos` starts at `{ 0, 0, 0 }`. After the fix the spring is made at the body joint minus 228 (`SPAWN y=127.7`), settles,
bounces Sonic and other fighters, and expires after 240 frames.

## Same class elsewhere

Every other Remix caller of `gmCollisionGetFighterPartsWorldPosition` was checked by reading: they set the vector first, except
`ftRemixMarinaCargoShakeSpawnGem` (`marina/ftremixmarinansp.c`), which passes an uninitialized `pos`; it is left for its own
check against `MarinaSpecial.asm`.
