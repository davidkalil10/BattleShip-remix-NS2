# Items: Waddle Doo beam never fired and did not form a chain, Turnip stayed 13 seconds on the ground (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, lot i3: `items/WaddleDoo.asm`, `items/Turnip.asm` against
`decomp/src/it/itremix/`).

## Symptoms and causes

| Item | Symptom | Cause |
|---|---|---|
| Waddle Doo | The beam did nothing: its four stars appeared, never moved, never turned their hitbox on and never expired, so every beam left four weapons alive for the rest of the match. | `ITRemixWaddleDooBeamVars` is laid over `WPStruct.weapon_vars`, and its first two fields (`startup_timer`, `draw_toggle`) shared the bytes of `weapon_vars.star.lifetime`, which the delegated `itStarRodWeaponStarProcUpdate` counts down. Writing the timer after the duration gave a lifetime of 0x1000A; the first decrement after the start-up turned the timer into -1 and the start-up branch ran forever. On the N64 the start-up timer is at 0x29A, just before the weapon vars (0x29C). Found while taking the before measurement of the next row. |
| Waddle Doo | With the row above fixed, the four stars collapsed into one column 256 units in front of the Doo, so the beam reached 256 units instead of about 1024 and missed a fighter the Doo had just targeted (its detection range is 1096). | `WaddleDoo.asm` 159-163, 260 and 304-305 (`waddle_doo_attack_main`, `star_stage_setting`): the object written at 0x2A0 is a2, the Doo for the first star only and the last created star (0x350) for the next ones. `beam_main` (725-788) then puts each star at parent + (lr * 256, sin(angle) * 256), which makes a chain. The port gave every star the Doo as parent. |
| Waddle Doo | On the first active frame of each star the hitbox swept from the spawn point (the Doo or the previous star) to the star: a 256-unit capsule that the Remix does not have. | `WaddleDoo.asm` 374 and 393 store the start position at 0x154/0x158 of the weapon struct, `attack_coll.attack_pos[0].pos_curr`; the port wrote `coll_data.pos_prev`. |
| Turnip | A Turnip that hit something, or that Peach dropped, stayed on the ground for 13 seconds and could be picked up all that time; in the Remix it blinks as soon as it lands and is gone 48 frames later. | `Turnip.asm` 288-289 (`falling_initial_`) stores GROUND_TIMER (0x030C) with `sh` at 0x2D2. That halfword is `pickup_wait:12` followed by `is_allow_knockback`, `is_unused_item_bool` and `is_static_damage`, so the timer is 0x030 = 48 and the first two flags are set. The port wrote 0x030C (780) to `pickup_wait`. |

## Fix

- `itremixwaddledoo.c`: `ITRemixWaddleDooBeamVars` starts with the star lifetime (a static assert ties it to
  `wpStarRodWeaponVarsStar.lifetime`); `itRemixWaddleDooAttackCreateStars` passes the Doo or the last star as the parent
  down to `itRemixWaddleDooBeamInit`, which also writes `attack_pos[0].pos_curr`;
  `itRemixWaddleDooBeamGetParentKind` reads the kind of an item or weapon parent in `itRemixWaddleDooBeamProcUpdate`.
- `itremixturnip.c`: `itRemixTurnipDroppedProc` takes the four fields from
  `itRemixTurnipRulesSplitGroundTimer` (`itremixturniprules.c`).

Unit test: `tests/remix/test_turnip_rules.c`.

## Evidence

Final Destination, the second fighter idle. Dedede holds the down special 90 frames (a Waddle Doo): before, 0 hits and
17536 star updates in 1500 frames with the timer below zero; after, the stars sit at 256, 512, 768 and 1024 units from
the Doo, the first beam hits three times and each star's first sweep starts on the star. Peach throws a Turnip at
Kirby: `pickup_wait` at landing 780 before and 48 after, destroyed 780 and 48 frames after landing.
