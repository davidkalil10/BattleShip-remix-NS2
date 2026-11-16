# Fighters: Banjo Shock Spring friction, Kirby's copied Crash spin (crash and shield hitlag), Crash eating sound, Conker grenade fuse (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, lot f1: `Banjo/`, `Bowser/`, `GBowser/`, `Conker/`, `Crash/` and
`Crash.asm` against `decomp/src/ft/ftremix/<fighter>` and `decomp/src/it/itremix/itremixconkergrenade.c`).

## Symptoms and causes

| Fighter | Symptom | Cause |
|---|---|---|
| Banjo | The up special attack dash started at 80 units per frame and lost 1.1 every frame (1391 units in 20 frames instead of 1600); the drift of the begin phase decayed after the stick was released. | `BanjoSpecial.asm` `begin_physics_` (1567-1577) and `attack_physics_` (1678-1694) run `bnez v0, _check_begin` after the gravity call. The caller enters `proc_physics` with `lw v0, 0x9E0(s1); jalr v0` (0x800E20AC) and neither gravity routine writes v0, so v0 is the routine's own address and the branch is always taken: the air friction (0x800D9074) of both routines and the air-speed cap (0x800D8FA8) of the attack never run. The port read the branch the other way and ran them every frame. |
| Kirby, J Kirby (Crash power) | The game crashed (SIGSEGV in `gcAddXObjForDObjFixed`, from `efManagerMakeEffect`) as soon as the copied spin started. | `CrashSpecial.asm` 876-880 stores Crash file 7 + 0xCC4 in the effect descriptor's `file_head`, a pointer to the file pointer: the effect file is the pointer held in word 0xCC4 (Kirby's spin file), as word 0xCC0 is for the Dig GFX. The port used file 7 + 0xCC4 itself as the file base, so the DObj, MObj and animation offsets pointed into Crash's file. |
| Kirby, J Kirby (Crash power) | A spin blocked by a shield froze for the shield hitlag (7 frames in the test) before the bounce back. | `CrashSpecial.asm` 919-952 (`shield_hitlag_patch_`) zeroes the hitlag for Kirby and J Kirby with the Crash hat in their blocked actions. The port looks the callback up by the attacker's fkind, and only Crash's descriptor had it. |
| Crash | He played his eating sound (FGM 0x5BF) on a Heart Container. | `Crash.asm` 761-785 patches 0x80145C44, the heal call of the Maxim Tomato case (Dango jumps into it). The Heart Container call is a separate one at 0x80145CAC, which no patch touches. |
| Conker | The grenade exploded 1 or 2 frames late after hitting a fighter or a shield with a speed whose fraction was 0.5 or more. | `ConkerSpecial.asm` 1766 converts the speed with `cvt.w.s`, which rounds to nearest; the port truncated. |

## Fix

- `ftremixbanjousp.c`: `ftRemixBanjoUSPBeginProcPhysics` no longer calls `ftPhysicsApplyAirVelXFriction`;
  `ftRemixBanjoUSPAttackProcPhysics` no longer calls `ftPhysicsCheckClampAirVelXDecMax` and the friction.
- `ftremixcrashnsp.c`: `ftRemixCrashNSPSpinFileHead` resolves word 0xCC4 (`PORT_RESOLVE`) for Kirby and J Kirby.
- `ftremixdeschooks.c`: `ftRemixBindKirbyCopyDescriptorHooks` installs `ftRemixCrashNSPShieldHitlagSkip` on the J Kirby
  descriptor and, through `bindKirbyCopyActions` (`port/remix/remix_roster.cpp`), on Kirby's.
- `ftcommonget.c`: the Heart Container case no longer calls `ftRemixCrashEatSfx`.
- `itremixconkergrenade.c`: the fuse cut is `itRemixConkerGrenadeRulesHurtFuse` (`itremixconkergrenaderules.c`, `lrintf`).

Unit test: `tests/remix/test_conkergrenade_rules.c`.
