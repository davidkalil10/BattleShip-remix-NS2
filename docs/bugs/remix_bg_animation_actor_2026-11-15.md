# Remix stages showed no background sprites (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, lot h2, finding H2-1).

## Symptoms

- The flying background sprites of the vanilla ground actor (Peach's Castle birds, Sector Z Arwings, Yoshi's Island and Congo
  Jungle sprites) never appeared on the Remix stages that have them in Smash Remix: the Dream Land, Omega and Remix variants of the
  vanilla stages, and the custom sprites of Smashville, Spiral Mountain, Zebes Landing, Green Hill Zone, Edo Town, Scuttle Town,
  Big Boo's Haunt and others (27 stages).
- On Big Boo's Haunt the Boos never flew; the Boo laugh ran on a countdown the port kept apart in `grremixbbh.c`.

## Root cause

Stages.asm `bg_animation_` (2713-2770) replaces the vanilla `stage_id < 9` test of `efGroundMakeAppearActor` (0x80116AE4)
with a lookup in a table of bg_info pointers: slots 0..7 point at the vanilla `dEFGroundDatas`, and every `add_bg_animation`
call (Stages.asm 5078-5265) writes the bg_info of an added stage (sprite_info_arrays, STAGE_FILE_OFFSET, the sprites of
`stages/<stage>.bin`). The port still gated the actor on `gkind <= nGRKindBattleEnd`, and the audit row claimed the table was
zero for added stages.

## Fix

- `ef/efremixgroundrules.c` holds the 27 bg_info rows, transcribed from the stage asm files and the 0x40-byte sprites of the
  `.bin` files (only the fields that differ: altitudes, z, scale, update routines, offsets).
- `ef/efremixground.c` `efRemixGroundGetData` returns the vanilla row for ids 0..7, builds an `EFGroundData` from the Remix row
  for an added stage, NULL otherwise; `efGroundMakeAppearActor` uses it under `PORT && REGION_REMIX`.
- `grremixbbh.c` reads the real actor countdown (`sEFGroundActor.make_wait`, RAM 0x80131AD8) as `boo_sound_` does.
- The asm direction 0xF of the Sector Z and Yoshi's Island variants is kept as the vanilla -1 / -3 (`deviations.md` 5.7).

## Verification

`gdb_count.sh` with the actor countdown forced to 300 frames: before the fix Smashville (76), Sector Z DL (113) and Big Boo's
Haunt (192) never reached `efGroundSetupRandomWeights`; after it every listed stage makes and updates sprites (Sector Z DL
Arwings and the Big Boo's Haunt Boos are on screen), the Boo laugh plays once per Boo (3 Boos, 3 laughs), and stages with no
`add_bg_animation` (Mushroom Kingdom, Mad Monster Mansion) still make no actor.
