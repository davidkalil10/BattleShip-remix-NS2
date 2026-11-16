# Dedede inhale on Kirby, Dedede absorb, Dragon King ball joint, respawn scripts of Dedede and Dark Samus (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, lot f2: Dedede, Dragon King, Dark Samus, Ebisumaru, Falco).

## Symptoms

- Kirby with the Dedede power started Dedede's inhale and then fell back to his own: after catching a fighter he swallowed and
  copied it (6% and the green copy effect) instead of holding it, walking and spitting it for 16%. A whiffed inhale ended in
  Kirby's own SpecialNEnd.
- Dedede's spit after eating a projectile kept the color animation and the effect of the inhale and skipped the slope
  routine.
- The Dragon King's neutral special ball floated above his head instead of sitting on the punching fist. Kirby with the
  Dragon King power crashed on the neutral special (`gmCollisionGetFighterPartsWorldPosition` with a null joint).
- A Dedede KO'd with a stored down special charge came back with it (a Gordo on the first short down special), and still
  counted the minions thrown before the KO. A Dark Samus KO'd with a charged shot came back fully charged.

## Root causes and fixes

1. `ftRemixDededeMapKirbyInhaleStatus` (`decomp/src/ft/ftremix/dedede/ftremixdededensp.c`, called at the top of
   `ftMainSetStatus`) returned at once for any fighter other than Dedede. Each of the 27 patches of `DededeSpecial.asm`
   (859-2435) has a `_kirby_hat_check` branch: a fighter that is not Dedede and whose hat id byte (0x0980) is 0x20 or more gets
   Kirby's `DEDEDE_NSP_*` action. The port now does the same for Kirby and J Kirby. The ids are the pure rules of
   `ftremixdededeinhalerules.c` (unit test `tests/remix/test_dedede_inhale_rules.c`).
2. `ftRemixDededeNSPAbsorbInitial` called `ftMainSetStatus` with the 0x0006 that the asm stores as a fourth argument. The asm
   (`DededeSpecial.asm:758-763`) calls 0x80163018, `ftKirbySpecialNEatSetStatusParam`, which takes only the action: it uses
   `FTSTATUS_PRESERVE_SLOPECONTOUR` and runs `proc_slope`. The port now calls that routine.
3. `ftRemixDragonKingNSPSetBallPosition` read `fp->joints[0xC]`. The asm reads FTStruct 0x0928 (`DragonKingSpecial.asm:619,688`),
   which is `joints[16]`: part 0xC of the comment plus the four joints before the common parts. Kirby has no joint 12, hence
   the crash.
4. The `ftManagerInitFighter` switch stands for the `initial_script` table (`Character.asm:1580-1590`) and had no case for
   Dedede (`Dedede.asm:698-703` clears 0x0ADC, 0x0AE0 and 0x0AE4) nor for Dark Samus, who inherits the Samus row
   (`Character.asm:265`). `ftManagerInitFighter` runs again at every respawn (`ftCommonRebirthDownSetStatus`), where the
   `bzero` of the alloc does not. Dedede now runs `ftRemixDededeInitialScript` (charge level and the two field minion slots);
   Dark Samus shares the Samus case.

## Evidence

Before and after on the same inputs (session scratchpad, not committed): Kirby's status log went from 364, 365, 272, 273, 275,
277 to 364, 365, 366, 367, 368, 381, 368, 369; the absorb spit flags from 0x6 to 0x40; the Dedede charge word read after a
respawn from 2 to 0 and the field slots from one live pointer to none; Dark Samus's `charge_level` at the respawn from 7 to 0.

## Same class elsewhere

Other Remix fighters reach a vanilla case of that switch through their parent or an `initial_script` patch and have no case
either. By the roster parents and the `table_patch_start(initial_script, ...)` lines: the Samus row for J Samus, E Samus,
Mewtwo, Sheik, Marina, Peppy and their polygons; the Mario row for Dr. Mario, Wario, J Mario, Mad Piano, Marth, Sonic, Super
Sonic, Goemon, Ebisumaru, Crash, Peach, Roy, Lanky and their polygons; the Captain row for Ganondorf, J Falcon, Banjo, Dragon
King; the Luigi row for J Luigi and Dr. Luigi; the Donkey row for J DK; the Purin row for J Puff and E Puff; the Link row for
the Link clones (they have a case that clears the boomerang only). Each needs its own check of what the port keeps in the
words that row clears. Ebisumaru was checked in game: his up special flag (0x0ADC) is already 0 at the respawn.
