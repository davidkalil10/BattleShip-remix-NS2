# Items: Flashbang and Conker grenade spin axis, Flashbang owner flag, Cloaking Device opacity and sparkle, Deku Nut flags, Basketball hitlag and fire flash (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, lot i1: `items/Flashbang.asm`, `items/CloakingDevice.asm`,
`items/DekuNut.asm`, `items/Basketball.asm` against `decomp/src/it/itremix/`).

## Symptoms and causes

| Item | Symptom | Cause |
|---|---|---|
| Flashbang (Peppy down special) and Conker's grenade | The grenade tumbled toward and away from the camera instead of rolling in the screen plane, and the fuse smoke left the pin at an angle that never changed. | `Flashbang.asm` 308-311 and 367 (`ConkerSpecial.asm` 1365-1367 and 1424) spin and read DObj 0x38, which is `rotate.vec.f.z` (`rotate` starts at 0x28: XObj pointer, angle, then x, y, z). The port used `rotate.vec.f.x`. |
| Flashbang | Peppy's own attacks went through his grenade, and A next to it picked it up (status LightGet) where the asm bats it. | `Flashbang.asm` 171-173 ORs 0x0080 into the halfword at ITStruct 0x2CE: bit 7 of byte 0x2CF, `is_damage_all`. The port set bit 7 of byte 0x2CE, `is_allow_pickup`. The same sequence in `ConkerSpecial.asm` 1232-1234 was already ported as `is_damage_all`. |
| Cloaking Device | A cloaked fighter sent flying stayed almost invisible. | `CloakingDevice.asm` 132-141 adds the length of FTStruct 0x8C/0x90 to the opacity: `coll_data.pos_diff`, the distance moved this frame. The port read `physics.vel_air`, which has no knockback (`vel_damage_air`). Walking and jumping gave the same value in both. |
| Cloaking Device | A cloaked Samus or Donkey Kong with a full charge kept spawning the charge sparkle effect every 12 frames. | `CloakingDevice.asm` 100-104: while the colanim loop slot (0xA30) holds 0x8012CB34, the loop body of `dGMColScriptsFighterCommonSpecialNCharge`, the handler writes 2 to the loop count (0xA34) every frame. The loop never ends, so the script never reaches its `Goto` and never runs its `Effect` command again. The colour flash of the loop goes on. The port had no such step. |
| Deku Nut | The nut's hurtbox ignored the attacks of the fighter who threw it: the thrower could not blow up his own nut. | `DekuNut.asm` 100 and 108-112: `spawn_deku_nut_` calls `explode_subroutine_` (team 4, `is_damage_all`), as the Bob-omb calls `itMainClearOwnerStats`. The port only called it at the explosion. The flag only matters for the item as a target (`itprocess.c`); a thrown nut never hits its owner in either version (`ftmain.c` tests the owner without the flag). |
| Deku Nut | A nut hit registered with knockback about 77 instead of the 2500 maximum (hit effects and sounds chosen from the knockback; the stun itself does not use it). | `DekuNut.asm` 140-141 and 202-203 store 0x100 with `sh` at ITStruct 0x140, the 32-bit `attack_coll.knockback_scale`: the high half, so the scale is 0x0100 0064 with the attribute value 100 in the low half. The port stored 256. |
| Basketball | About half of the bounces froze the ball one frame less. | `Basketball.asm` 1290-1296 converts speed * 0.03 with `cvt.w.s`, which rounds to nearest; the port truncated. |
| Basketball | A hit on a flaming ball did not restart its fire colour script. | `Basketball.asm` 868-882 calls `itMainCheckSetColAnimID` with `GFXRoutine.id.BALL_FIRE` on a hit while on fire and with 5 otherwise; the port skipped the call while on fire. |

## Fix

- `itremixflashbang.c`, `itremixconkergrenade.c`: spin and smoke angle on `rotate.vec.f.z`; the Flashbang sets
  `is_damage_all` and leaves `is_allow_pickup` clear.
- `itremixcloakingdevice.c`: opacity from `coll_data.pos_diff`; `itRemixCloakingDeviceHoldChargeSparkle` keeps the loop
  count of the fully charged script at 2 while the cloak runs.
- `itremixdekunut.c`: `itRemixDekuNutSetDamageAll` at spawn; the knockback scale through
  `itRemixDekuNutRulesKnockbackScale` (`itremixdekunutrules.c`).
- `itremixbasketball.c`: `itRemixBasketballRulesBounceHitlag` (`itremixbasketballrules.c`, `lrintf`); the hit colanim is
  `BALL_FIRE` while on fire.

Unit tests: `tests/remix/test_dekunut_rules.c`, `tests/remix/test_basketball_bounce_rules.c`.

## Kept as deviations

Two differences of the same audit are asm bugs and stay as the port has them (`docs/remix/deviations.md`, 5.5): a second
Cloaking Device pickup while cloaked (the asm plays the decloak sound when the first handler expires, in the middle of the
cloak) and the Cloaked visibility state (the asm ORs the state id 1 where it means 0xFFFFFF00).
