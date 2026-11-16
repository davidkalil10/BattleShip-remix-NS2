# Wario body slam clang, Wolf and Slippy reflect speed, J Link Spin Attack landing lag, Slippy Firefox threshold (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, lot f5: Sandbag, Sheik, Slippy, Wario, Wolf, Young Link).

## Symptoms

- Wario's Body Slam (and Kirby's copy of it) lost the clang it is meant to win: while the shoulder is invincible (frames 11 to
  22 of the slam, the 14% hitbox), a fighter attack that met it cancelled the slam's hitbox and kept its own. A Fox Firefox
  crossed the slam and neither fighter was hit.
- A projectile reflected by Wolf or by Slippy came back at the speed it arrived with.
- J Link landed from the special fall of his aerial Spin Attack in 13 frames, as US Link does, instead of 11.
- Slippy's Firefox went straight up on a light stick tilt and did not turn to the stick side.

## Root causes and fixes

1. `ftMainUpdateAttackStatFighter` (`decomp/src/ft/ftmain.c`) tested the wrong fighter in both Body Slam checks. The asm macro
   `body_slam_clang_patch` (`Wario/Wario.asm:483-506`, hooks 0x800E294C and 0x800E29EC) calls `check_body_slam_invincible_`
   with the `a0` the game loaded just before each hook: `this_fp` at the first (`lw a0, 0x40(sp)`), `other_fp` at the second
   (`lw a0, 0x38(sp)`), which is the fighter whose hitbox that block cancels. The port passed `other_fp` in the first block and
   `this_fp` in the second, so it protected the slammer's opponent. The two arguments are now the asm's.
2. `wolf_reflect_speed` (`Wolf/Wolf.asm:390-431`, hook 0x80166DDC in the `!is_static_damage` branch of the reflect block of
   `wpProcessProcHitCollisions`) was listed as ported but had no code. `decomp/src/wp/wpprocess.c` now multiplies
   `wp->physics.vel_air.x` by `ftRemixVariantTuneRulesReflectSpeedMul` of the reflecting fighter: 2.0 for Wolf, 0.40625
   (0x3ED00000) for Slippy, 1.0 for the others.
3. `up_special_landing_` (`YoungLink/YoungLink.asm:259-290`) gives Young Link 0.33 and J Link 0.75; the port had only the Young
   Link value, so J Link kept the 0.65 of the US build. `ftRemixYoungLinkUpSpecialLandingLag` now returns
   `ftRemixVariantTuneRulesSpinAttackLandingLag` for the other fighters (0.75 for J Link).
4. `up_special_angle_patch_2_` (`Slippy/Slippy.asm:441-466`, hook 0x8015C63C) replaces the `|stick_y|` term of the
   `|x| + |y| >= 45` test of `ftFoxSpecialAirHiSetStatusFromGround` by 300, so Slippy always passes it. The port used the forced
   value only in the `atan2`. The test in `decomp/src/ft/ftchar/ftfox/ftfoxspecialhi.c` now uses `ftFoxSpecialHiGetStickY` too.

The two multipliers and the landing lag are pure rules of `ftremixvarianttunerules.c` (unit test
`tests/remix/test_varianttune_rules.c`).

## Evidence

Before and after on the same inputs (session scratchpad, not committed), Final Destination, values read under gdb:

- Wario's slam meets Fox's Firefox with the shoulder invincible (slam 14%, Firefox 16%), Wario on port 1 and on port 2 (both
  argument orders of the routine), three timings each: the hit marked as clanged went from Wario's to Fox's in the six runs,
  and Fox now takes the 14%.
- Mario's fireball (horizontal speed 30.59) into a held reflector: it left Wolf at 30.59 before and 61.18 after, Slippy at
  30.59 before and 12.43 after; Fox stays at 30.59.
- J Link's aerial Spin Attack into the special fall: the landing lag argument went from 0.65 to 0.75 and the landing status
  from 13 to 11 frames; Young Link (0.33, 25 frames) and Link (0.65, 13 frames) did not change.
- Slippy's grounded Firefox with the stick at (-20, 0): 90 degrees facing right before; 86.19 degrees facing left after
  (`atan2(300, 20)`), horizontal speed -7.65. Fox with the same input still goes straight up.
