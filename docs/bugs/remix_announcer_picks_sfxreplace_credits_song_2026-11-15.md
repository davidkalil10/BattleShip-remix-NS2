# Pokemon announcer picks, Flat Zone sounds in the Remix 1P modes, credits song from the menu (2026-11-15)

Found by the M14 equivalence audit (lot T14.3-s4, audio logic that is not a toggle: `PokemonAnnouncer.asm`, `SFXReplace.asm`,
`BGM.asm`).

## Symptoms

1. A KO above 150% within 600 frames of another one played a regular KO clip (0x469-0x46C) instead of a long KO clip
   (0x476, 0x477).
2. "It's a Head on Collision" (0x4B2) played at the first clang of a match and never again.
3. "The battle is coming right down to the wire" (0x4A0) played once per match, and already when every player still had two
   stocks.
4. In a team stock battle, a defeated player always got both team KO comments (0x4B9, then 0x4BA two seconds later), and both
   again after their cooldown.
5. With six general comments cooling down, about one comment window in ten stayed silent.
6. The announcer kept counting during GAME SET and could play a general comment or a pending sleep comment over "There goes the
   battle".
7. On Flat Zone and Flat Zone II in All-Star, Multi-Man, Cruel Multi-Man, Bonus 3 and the Home-Run Contest the fighters kept
   their normal step and landing sounds.
8. Credits opened from the port menu (Remix > Other Screens) after a Remix 1P or All-Star run played the fighter's ending song.

## Root causes

The announcer entry is 16 bytes: id, FGM id, cooldown (word), running timer (word at +8), `used` (halfword at +0xC), flag. The
asm reads some of these fields with the wrong width, and on a big-endian CPU that gives the high part, which is always 0. The port
had implemented what the asm comments say instead of what the code does.

1. `PokemonAnnouncer.asm` 2124-2136 (`_long_ko`): `lhu t0, 0x0008(t1)` reads the high half of the timer, 0 for a 0x258 cooldown,
   so the branch to `_regular_ko` is never taken. The port tested the whole timer.
2. `PokemonAnnouncer.asm` 1666-1668 (`clang_announcement_`): `lbu t1, 0x000C(a2)` reads the high byte of `used` (0x0001), so the
   "used in match" test never blocks. The port had a once-per-match table flag for Clang.
3. `PokemonAnnouncer.asm` 2165-2183 (`_general`): 0x801317CC is `sIFCommonPlayerStocksNum`, the stock number of the HUD
   (`stock_count + 1`), and the asm wants it under 2; the port compared `stock_count` with 2. The `used` test is again an `lbu`
   of the high byte, so the close-match clip (cooldown 0) replaces every general comment while the condition holds.
4. `PokemonAnnouncer.asm` 1810-1813 (`team_ko_announcement_`): the asm skips the call when the `used` halfword of the first team
   KO call is set (it reads that entry twice). The port had no such test.
5. `PokemonAnnouncer.asm` 2185-2199 (`_unspecific_loop`): the asm draws until a clip has no cooldown; the port stopped after 8
   draws.
6. `PokemonAnnouncer.asm` 2252-2257 (`announcer_setup_`): `Render.CREATE_OBJECT_` gets group 1 and `announcer_constant_` as the
   object routine. `ifCommonBattleInterfaceProcUpdate` pauses every GObj at GAME SET and resumes only links 8 and 11; the port had
   put the announcer on link 11 (Interface) with a process.
7. `SFXReplace.asm` 82-84 accepts the screen 0x77 (`Global.screen.REMIX_MODES`), which `SinglePlayerModes.asm` 864-886
   (`_initial_screen_set`) stores for every Remix 1P mode but Remix 1P. The port runs those modes on the 1P game scene, so its
   comparison with 0x77 was dead code.
8. `BGM.asm` 1361-1365 (`remix_1P_ending_song`) plays the standard song when the previous screen is the settings screen (0x39).
   The port only looked at the stored 1P mode, which stays Remix 1P or All-Star until the Mode Select clears it.

## Fixes

1. `if/ifremixannouncer.c` `ifRemixAnnouncerPickLongKO`: always a long KO clip.
2. `if/ifremixannouncertables.c`: Clang is a plain table (1800-frame cooldown, flag 1).
3. `ifRemixAnnouncerIsCloseMatch`: every player of the match on the last stock (`ifRemixAnnouncerRulesIsLastStock`), ports with
   no player skipped; `ifRemixAnnouncerPickGeneral` no longer tests `used`.
4. Team KO is the table whose first entry ends the call (`IFRemixAnnouncerTable.is_spent_by_first`,
   `ifRemixAnnouncerRulesIsSpent`).
5. `ifRemixAnnouncerRulesPickFree`: draws until a free clip, after a check that one exists.
6. `ifRemixAnnouncerSetup`: GObj id 0x3F2 on link 1 with `ifRemixAnnouncerRun` as its run function.
7. `ft/ftremix/ftremixsfxreplace.c`: the screen test accepts the 1P game scene of a mode of
   `sc1PRemixModeRunsOnRemixModesScreen` (`sc1premixmoderules.c`).
8. `sc/sc1pmode/sc1premixending.c`: credits whose previous scene is the credits scene (how `portSCManagerRequestScene` enters a
   scene from the port menu) keep the standard song (`sc1PRemixEndingUsesFighterSong(mode, is_opened_from_menu)`).

Kept better than the asm (`docs/remix/deviations.md` 5.7): `announcer_setup_` clears `used` with `sb` (high byte only), so in the
asm the quick KO clips and the team KO comments are spent until power-off; the port resets them every match. The asm close-match
test reads stale HUD numbers for ports with no player; the port skips those ports.

## Verification

Pokemon Stadium (stage 45), `SSB64_AUDIO_FGM_LOG=1`; ids 1142/1143 long KO, 1129-1132 regular KO, 1202 clang, 1184 close match,
1177-1182 and 1214/1215 general, 1209/1210 team KO, 1115 There goes the battle, 1538/1539 Game & Watch step and landing.

- Six KOs at 200%, 360 frames apart: before 1142 1143 1129 ..., after only 1142 and 1143.
- Two Marios clanging forward smashes every 250 frames for 4900 frames (63 clang calls): before 1202 once, after three times.
- Two idle players, 2 stocks, 5100 frames: before 1184, after a general clip. 1 stock, 10000 frames: before 1184 then a general
  clip, after 1184 twice.
- Team stock battle, P1 defeated: before 1209 then 1210, after 1209 only.
- 300 forced comment windows with six clips cooling down: before 276 of 310 played, after every window played.
- GAME SET: before the announcer routine ran 134 more times and a forced general comment played after 1115; after 0 runs and
  no clip.
- All-Star with the first match on Flat Zone: before steps 110-130 and landing 77, after 1538 x25 and 1539 x11.
- Credits from the port menu with the stored mode Remix 1P and Banjo: before track 124, after track 39; the credits after a
  Remix 1P run with Banjo still play 124.
- Unit tests: `tests/remix/test_announcer_rules.c`, `test_1p_mode_rules.c`, `test_1p_leftover_rules.c`.
