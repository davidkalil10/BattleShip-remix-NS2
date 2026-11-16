# Fighters: Lanky grape spawn point and Kirby's copied grape, Marina midair jumps and Cargo shake gem (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, lot f3: `Ganondorf/`, `Goemon/`, `Lanky/`, `Lucas/`, `Marina/`
against `decomp/src/ft/ftremix/<fighter>` and the shared jump code).

## Symptoms and causes

| Fighter | Symptom | Cause |
|---|---|---|
| Lanky | The grape appeared at the Grape Shooter joint, about 316 world units closer to Lanky than on the N64. | `LankySpecial.asm` 102-118 (`main_`) writes (0, -200, 0), or (300, 0, 0) for Kirby and J Kirby (`lui at, 0x4396`; the asm comment says 200), into the vector it passes to 0x800EDF24. `gmCollisionGetFighterPartsWorldPosition` multiplies that vector by the joint matrix: it is a local offset. The port passed (0, 0, 0), with a comment saying the function overwrites the vector. |
| Kirby, J Kirby (Lanky power) | The neutral special did nothing: Kirby played an action with no grape, on the first press and on the re-fire. | `LankySpecial.asm` 15-21, 44-50 and 151-160 set `Kirby.Action.LANKY_NSP_Ground` / `LANKY_NSP_Air`, which are 0x1C5 / 0x1C6 (0x12F + the index 150 / 151 in `Kirby.asm`). The port used 0xE7 / 0xE8, the parent-action column of `Character.add_new_action` (`Kirby.asm` 487-488), which are other Kirby statuses. |
| Marina, N Marina | Every midair jump was as high as the first (y speed 70.5); the five midair jumps could be chained two frames apart. | `Marina.asm` 802-828 (`jump_height_patch_`, 0x8013FE58) replaces the stick term 80 of the jump height with `jump_height_table[jumps_used]` (80, 70, 60, 50, 40), and 830-873 (`jump_timing_patch_`, 0x80140310) refuses a midair jump out of JumpAerialF/B until the jump moveset sets temp variable 2. Neither patch was ported; `patch_audit_scopes.md` listed both as ported. |
| Marina | The Cargo shake gem popped out on the first frame of the shake, before the damage, and also when the carried fighter could not take damage. | `MarinaSpecial.asm` 76-96 (`shake_main_`) reaches the gem block only after it applied the moveset damage to a vulnerable victim. The port ran the gem spawn every frame, and its position vector was not zeroed before the joint transform. |

## Fix

- `ftremixlankynsp.c`: `ftRemixLankyNSPCreateGrape` passes the asm offsets; the Kirby status ids are 0x1C5 / 0x1C6.
- `marina/ftremixmarinarules.c` (new, pure rules): `ftRemixMarinaJumpStickRange`, `ftRemixMarinaJumpIsLocked`,
  `ftRemixMarinaShakeDamageApplies`, `ftRemixMarinaShakeSpawnsGem`.
- `marina/ftremixmarina.c`: `ftRemixMarinaJumpAerialStickRange` and `ftRemixMarinaJumpAerialIsLocked`, called from
  `ftCommonJumpAerialSetStatus` and `ftCommonJumpAerialCheckInterruptCommon` (`ftcommonjumpaerial.c`).
- `ftremixmarinansp.c`: `ftRemixMarinaCargoShakeProcUpdate` spawns the gem only on the frame the damage was applied; the
  gem position starts at (0, 0, 0).

Two more findings of the lot are asm bugs and stay as the port has them (`docs/remix/deviations.md`, Fighters): the Ryo
burst when the Ryo has no linked effect object, and Marina's grounded DSP absorb action.

Unit test: `tests/remix/test_marina_rules.c`.

## Evidence (Final Destination, gdb probes on `wpManagerMakeWeapon`, the end of `ftCommonJumpAerialSetStatus`, `ftParamUpdateDamage`, `itManagerMakeItemSetupCommon`)

| Case | Before | After |
|---|---|---|
| Lanky grape, Lanky at x -1800 | spawn at x -1173.4, y 232.1 | x -857.1, y 233.2 |
| Kirby with the Lanky power, B then B taps | status 231 (0xE7), no weapon | status 453 (0x1C5) on the ground and 454 (0x1C6) in the air, four grapes (kind 0x25) per special, the first at x -1454.0 with Kirby at -1800 |
| Marina, one midair jump every 25 frames | y speed 70.5 five times | 70.5, 63.75, 57.0, 50.25, 43.5 |
| Marina, jump tapped every 2 frames | midair jumps at tics 45, 47, 49, 51, 53 | 45, 61, 77, 93, 109 |
| Marina Cargo shake started at tic 265 | gem at tic 266, damage at tic 280 | gem and damage at tic 280; no gem and no damage with the carried Kirby forced invincible |
