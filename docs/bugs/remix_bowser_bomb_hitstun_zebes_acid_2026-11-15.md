# Bowser's Stadium bomb hitstun stuck on, Zebes lava levels leaking between matches (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, lot h3, findings H3-1 and H3-5).

## Symptoms

- Bowser's Stadium (0x43): once a fighter was hit by a bomb, every later hit on that fighter (punch, projectile, throw)
  got the bomb's extra hitstun `x 9 / (gravity + 4)`, about 1.8x for a typical gravity, until the bomb respawned (300
  frames) or longer when the new bomb reused the same GObj. Combos were much stronger than in the Remix. Under gdb,
  `h3_bomb_hitby_before` showed 11 of 12 hits of a 3000-frame match with the bomb multiplier after one bomb hit at t=444.
- Planet Zebes and every Zebes clone played after Norfair, Norfair Remix, Zebes DL or Zebes Omega in the same session
  used that stage's lava levels: the acid rose to the previous stage's heights and started at -2400 instead of -3000
  (`h3_zebes_before`: match 2 on gkind 3 printed the Norfair table).

## Root cause

- `grRemixBowserBombsTrackHitObject` (Hazards.asm `track_hit_object_`, hook 0x800E3F14) was called only from the item
  branch of `ftMainUpdateDamageStatItem`. The asm writes `hitlog[0].attacker_gobj` at the top of
  `ftMainProcessHitCollisionStatsMain` for every attacker class, so the next non-bomb hit replaces the bomb.
- Hazards.asm `alt_acid_levels_` writes `dGRZebesAcidAttributes` (0x8012EA60) in place. On N64 that table is battle
  overlay data that the scene manager reloads from ROM for every match; in the port it is an ordinary global and nothing
  wrote the vanilla levels back.

## Fix

- `decomp/src/ft/ftmain.c`: `ftMainProcessHitCollisionStatsMain` records `sFTMainHitLogs[0].attacker_gobj` before its
  loop under `REGION_REMIX`; the item-branch call is gone. `grRemixBowserBombsIsHitByBomb` also ignores a NULL record.
- `decomp/src/gr/grcommon/grzebes.c`: the vanilla levels are saved before the first Remix overwrite and written back on
  every other Zebes-based stage (`grRemixZebesSelectAcidLevels` in `gr/grremix/grremixzebes.c`, unit test
  `tests/remix/test_zebes_acid_rules.c`).

## Same class elsewhere

Any Remix patch that writes vanilla overlay `.data` in place relies on the N64 overlay reload to undo it. A port of such
a write needs an explicit restore when the next scene does not overwrite the value itself.
