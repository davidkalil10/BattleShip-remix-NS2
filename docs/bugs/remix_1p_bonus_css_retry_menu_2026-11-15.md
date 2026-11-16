# 1P screens: Bonus CSS STAGE row, HRC total, L retry, menu wrap and title card timing (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, lot m4, items M4-1 to M4-7; M4-8 is a kept deviation, `docs/remix/deviations.md` 5.6).

## Symptoms

- Bonus CSS opened on Bonus 3, header cycled to Bonus 1 or 2 with the REMIX choice: no STAGE row, no stage arrows, no stock icons and
  no "L: Set Stage", although the start was manual and the run used the hidden stage index. Opened on Bonus 1/2 and cycled to Bonus 3:
  the row stayed, its arrows could be pressed and an L release still changed the stage.
- A press up to 8 px left of the STAGE left arrow did nothing, and one up to 8 px right of it moved the stage.
- The Home-Run Contest TOTAL was a plain 8-digit number, ten times the Remix value (`00013023` for 1302.3 ft).
- Holding L together with another button (L + A, L + Z) at GAME SET or STAGE CLEAR restarted a practice run, a Bonus 1/2 practice, and
  a Bonus 3, Multi-Man, Cruel or HRC run.
- In a practice 1P run, L held at the GAME SET of a Bonus stage did nothing there: the retry only came about 90 frames later, on the
  STAGE CLEAR screen, and only if L was still held.
- On the 1P menu, a wrap of the cursor (down from the last button, up from the first) waited 8 frames longer before the repeat.
- On the 1P title card "VS" and the opponent name played one frame late.

## Root causes

- `mnRemixBonusModesToggleMake` built the STAGE row only when the screen opened on Bonus 1/2, and the per-frame update, the arrow press
  and the L release only tested that the row existed. `Bonus.asm` `setup_` (11-17) makes the row for Bonus 1, 2 and 3 and hides it on
  Bonus 3; `handle_header_toggle_` (233-257), `arrow_routine_` (302-311), `check_input_` (337-340) and `handle_arrow_press_`
  `_stage_check` (514-518) read `singleplayer_mode_flag` each time.
- The STAGE arrows reused the MODE arrow padding (left arrow: 0 left, 8 right). `Bonus.asm` 520-532 gives both STAGE arrows 8 px on
  the left and 0 on the right.
- `draw_hrc_high_score_decimal_` (`SinglePlayerMenus.asm` 681-732) is patched in twice: after the record line (0x80133B74) and at the
  end of the total routine (0x80133F44). The port called it for the record only.
- `Practice_1P.asm` 184-187 and 248-251 call `Joypad.check_buttons_all_` with "match any" false, which compares the held buttons of
  each controller with the mask (`Joypad.asm` `check_buttons_`: `bne t0, a0, _end`). The port ORed the four controllers and tested the
  L bit.
- `hold_l_to_retry_` sends a BONUS screen whose previous screen is not a Bonus CSS to `_check_practice` (143-153); `ifcommon.c` asked
  the practice retry only in the 1P Game scene.
- `fix_cursor_wrap_` (`SinglePlayerMenus.asm` 249-284) only widens the vanilla compares, so the 8 extra frames stay on "landed on the
  last button going down" and "landed on the first going up"; `mn1PRemixMoveOption` added them on either end in both directions.
- `get_name_delay_` (`SinglePlayer.asm` 1960-1973) replaces `lw t3, 0(t1)` and `addiu t4, t3, 1`, so the wait is the table value;
  `sc1pintro.c` kept the `+ 1`.

## Fix

- `mnremixbonusmodes.c`: the STAGE row is always made with the Option panel; `mnRemixBonusModesIsStageRowShown` (rule
  `mnRemixBonusModesStageRowIsShown`: header on Bonus 1/2 and REMIX) gates the row, its arrows, the icons and the arrow press, and
  "L: Set Stage" and the L release take the Bonus 1/2 test from the header. `mnRemixBonusModesStageArrowHit` has the STAGE padding.
  `mnRemixBonusModesMakeTotal` adds the decimal point for the feet total.
- `gmremixpracticerules.c`: `gmRemixPracticeIsRetryHeld` is an exact match of one controller; `gmremixpractice.c` and
  `sc1premixretry.c` ask each controller. `gmRemixPracticeTryRetryByGameSet` (rule `gmRemixPracticeIsGameSetRetryScene`) takes the 1P
  Game battle and a Bonus stage that did not come from a Bonus CSS.
- `sc1premixmoderules.c` `sc1PRemixModeMenuAddsEdgeWait`, used by `mn1PRemixMoveOption`.
- `sc1pintro.c`: the Remix wait is `sc1PRemixIntroNameDelay` with no `+ 1`.

## Evidence

- STAGE row, 1P menu to Bonus 3, MODE to REMIX, header to Bonus 1, STAGE right arrow, token over a fighter, L: before no row and
  `sc1PBonusRemixSetStageIndex` 0 times; after the row, arrows and icons show, the icons change, "L: Set Stage" shows and the function
  runs twice. Bonus 1 with REMIX, header to Bonus 3, same presses: 2 calls and the row visible before, 0 calls and no row after.
- STAGE left arrow, press point 6 px left of it: 0 calls before, 1 after; 5 px right of it: 1 before, 0 after.
- HRC CSS with records 1234.5 and 67.8 ft: TOTAL `00013023` before, `0001302.3` after.
- Forced FAILURE with L + A held: Bonus 1 practice, Multi-Man and a practice 1P stage restarted before and do not after; STAGE CLEAR
  with L + Z the same; L alone still restarts in all four.
- Practice run, Bonus 1 stage, FAILURE with L held: before, `sc1PRemixRetryTryByGameSet` ran 91 frames and the retry came from the
  STAGE CLEAR scene; after, the stage restarts on the first GAME SET frame and STAGE CLEAR never opens. Without the Practice option the
  run goes on to STAGE CLEAR.
- 1P menu, stick held down then up: the wait after a wrap was 19 frames before and is 11 after; landing on the last button going down
  and on the first going up stays 19.
- Title card, Mario: "VS" on tic 52 and the opponent name on tic 112 before, 51 and 111 after (standard 1P and Remix 1P).

Unit tests: `test_1p_bonus_modes_rules.c`, `test_practice_rules.c`, `test_1p_retry_rules.c`, `test_1p_mode_rules.c`.
