# Peach's Float in the double jump, Mad Piano's book count, Shadow Ball announcer call and air transition (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, batch f4: Mewtwo, Peach, Peppy, Mad Piano, Roy).

## Symptoms

- Peach and N Peach holding a C button during the double jump did not start Float until the JumpAerial animation ended and
  she entered Fall: 60 frames after the double jump, also with the stick held down. In the Remix a held-down stick starts
  Float at once, and otherwise Float starts after frame 20 of JumpAerialF/B once she is no longer rising.
- Mad Piano with N bonus ammo (absorbed projectiles) fired N books; the Remix fires N + 1 (the normal book plus one per
  ammo), and the book with the widest spread (1 + 0.25 * N) never appeared.
- With the Pokemon announcer active, starting a Shadow Ball charge (Mewtwo, or Kirby with his power) played no "It's
  building energy" call (FGM 0x4A6).
- A grounded Shadow Ball shot that lost the floor clamped the carried x speed to the fighter's maximum air speed.

## Root cause

- `PeachSpecial.asm` `check_float_` patches four interrupts: Jump (0x8013F688), JumpAerial (0x8013FB54), Fall (0x8013F9C8)
  and Pass (0x80141D88). The port called `ftRemixPeachJumpAerialCheckOrFloat` from three of them;
  `ftCommonJumpAerialProcInterrupt` (`ft/ftcommon/ftcommonjumpaerial.c`) still called the vanilla check, so the frame-20
  gate of `ftRemixPeachFloatDoubleJumpGate` was dead code.
- `fireball.asm` `Fireball.Book.create_` makes a book and then loops with `bnel t8, t9, _loop` (the increment sits in the
  delay slot of the branch-likely), so the body runs for t8 = 0..bonus_ammo. `wpRemixPianoBookCount` returned `bonus_ammo`.
- `PokemonAnnouncer.asm` `charging_announcement_` patches 0x80168E08 inside the Charge Shot stage setting (0x80168DDC),
  which `MewtwoSpecial.asm` `attach_shadow_ball_` calls. The port's own stage setting,
  `ftRemixMewtwoShadowBallMakeWeapon`, had no announcer call.
- `MewtwoSpecial.asm` `air_shoot_transition_` copies only the first 8 instructions of 0x8015DA04, which end before the
  status change; the `ftPhysicsClampAirVelXMax` call (0x800D8EB8) that follows it in the vanilla routine is not copied.
  The port kept the clamp.

## Fix

- `ftCommonJumpAerialProcInterrupt` calls `ftRemixPeachJumpAerialCheckOrFloat` under `REGION_REMIX`.
- `wpRemixPianoBookRulesCount` (`wp/wpmario/wpmarioremixfireballrules.c`, unit test `tests/remix/test_pianobook_rules.c`)
  returns `bonus_ammo + 1`, and 1 with no ammo.
- `ftRemixMewtwoShadowBallMakeWeapon` plays `nIFRemixAnnouncerCallCharge` when the announcer is active.
- `ftRemixMewtwoSpecialAirNShootTransition` no longer clamps. With the stock attributes this changes nothing in play: the
  ground recoil is at most 26 and friction lowers it before the edge (20.4 measured for a full charge), under the maximum
  air speed of Kirby (28) and of Mewtwo (36). It only shows when something else pushes the speed above that maximum.
