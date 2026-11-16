# Throw angle of the random knockback, Stamina armor on the lethal hit, env colour override priority (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, batch s1: the shared fighter systems that are not toggles).

## Symptoms

- With the panel option Knockback on Random, a throw used the angle rolled for the port and the current action of the
  thrown fighter. A player with the option on threw at the vanilla angle unless the victim also had it on, and a victim
  with the option on was thrown at a random angle by everyone, also by players with the option off.
- In Stamina, the hit that takes the last HP still lost knockback to the status armor of the victim (moves with armor,
  moveset command 0xD1) and to the 2/3 of a crouching fighter, so the KO launch came out weaker or not at all.
- A fighter with an env colour override (Visibility Cloaked, None or Dark, Cloaking Device, moveset command 0xD9) kept
  the override colour while a colour animation set its colour 2 or while the fighter drew with its fog colour.

## Root cause

- `Knockback.asm` `get_angle_throw_` (0x8014B16C) reads the port (0x0D) and the action (0x24) from `s0`, and at that point
  of `ftCommonThrownReleaseThrownUpdateStats` `s0` is the thrower (`capture_fp`; `Rage.asm` uses the same register for
  the attacker percent). The port passed `this_fp`, the thrown fighter.
- `Stamina.asm` `stamina_armor_fix_` (called first by `Size.asm` `passive_armor_`, hook 0x800E6324) pops the return
  address on the lethal hit and returns straight to 0x800E632C with `f2 = f12 = 0`: the status armor (0x7E8) and the
  passive armor (0x7E4) are both 0, and the reload of `f0` from 0x7E0 at 0x800E6328 is skipped, so `f0` still holds the
  knockback read at 0x800E62D0, before the crouch discount. The port only returned 0 for the passive armor.
- `CharEnvColor.asm` `override_env_color_` patches 0x800FC9DC, inside `mpCollisionSetLightColorGetAlpha`, and 0x80390548,
  its menu counterpart. The fighter draw only calls them when neither `colanim.is_use_color2` nor `is_use_fogcolor` is
  set. The port tested the override before both.

## Fix

- `ftCommonThrownReleaseThrownUpdateStats` calls `ftRemixKnockbackAngle(capture_fp, ...)`.
- `ftMainProcParams` keeps the knockback read before the crouch discount and calls `ftRemixStaminaKnockbackAfterArmor`
  (`ft/ftremix/ftremixstaminaguard.c`; rule `gmRemixStaminaRulesKnockbackAfterArmor`, unit test
  `tests/remix/test_stamina_rules.c`). `ftRemixSizePassiveArmor` no longer tests the Stamina rule.
- `ftDisplayMainProcDisplay` takes the Remix override only when `ftRemixEnvColorRulesReplacesLight` says so
  (`ft/ftremix/ftremixenvcolorrules.c`, unit test `tests/remix/test_envcolor_rules.c`). The mod event
  `FighterEnvColorQueryEvent` keeps its priority over every colour.

In vanilla data the colour 2 command is only in Master Hand's `dGMColScriptsFighterBossYubideppou3` and the fog colour is
only set by the boss code, so the third fix shows with a forced colour animation, not in a normal match.

## Verification

- Throw: Mario forward-throws an idle Kirby on Final Destination, gdb on `ftCommonDamageInitDamageVars`. Option on for
  the thrower: angle 45 (vanilla) before, 121 (the thrower's entry for ThrowF 169) after. Option on for the victim only:
  91 (the victim's entry for its thrown status 186) before, 45 after. Option off: 45.
- Stamina with 5 HP, Mario's forward smash (17 damage) on Kirby, knockback of the `[remix-fighter] hit` line: standing
  79.65; crouching 53.10 before, 79.65 after; with 30 of status armor 49.65 before, 79.65 after. With 150 HP (not
  lethal) the three stay 79.65, 53.10 and 49.65.
- Mario under Visibility Dark with colour animation 72 (the Master Hand script) applied every 45 frames: a black
  silhouette before, drawn in colour after; without the colour animation he stays black. Same with Visibility Cloaked.

The same audit found that the trail of Pikachu's Thunder absorbed by Marina or Dedede falls into the reflect damage
scale in the asm; the port keeps its behaviour (`docs/remix/deviations.md` 5.3).
