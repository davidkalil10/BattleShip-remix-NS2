# Training respawn percent, pause and D-pad shortcuts, custom spawn; 12-Character Battle pause and winner lock (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, lot m2, items M2-1 to M2-9).

## Symptoms

- A KO respawn came back with the initial damage instead of 0%: the Training menu percent with Reset Sets Percent on, and the
  panel Damage value in VS, 1P and Bonus matches.
- Training: after a freeze with D-pad up, setting D-pad Controls to Reset Only or Off left the match frozen with no way to unfreeze
  it; the four D-pad shortcuts did nothing while paused; the camera kept easing while the match was frozen; a D-pad right tap on
  a lag tic of the vanilla Speed option was lost, and with Game Speed at 2x or 3x it ran 2 or 3 frames.
- Training: only the menu owner's controller could pause and unpause.
- Training: a Custom spawn (and a "Port n" Spawn choice) survived the exit and placed the fighter at the old stage's coordinates
  on the next stage.
- 12-Character Battle: START on controller 3 or 4 paused the match ("3P PAUSE") and nobody could unpause it; after a complete
  battle the winner could take another portrait on the CSS.

## Root cause

- `ftRemixPanelGetInitialDamage` was called from `ftManagerInitFighter`, which the respawn (`ftCommonRebirthDownSetStatus`) also
  calls. `css/Damage.asm` `char_init_` and `Training.asm` `init_character_` (416-449) test the saved return address 0x800D86B4 and
  only act on the call made by `ftManagerMakeFighter`.
- `sc1PTrainingRemixPollDpad` returned before reading anything unless the game status was Go, and returned for Reset Only and Off
  without clearing the freeze; the asm hook (`advance_frame_`, 0x8018DA40) sits after the Go/Pause switch and clears the freeze
  (1189-1201). The freeze and the frame advance were applied at once instead of being latched until a frame that is not a lag tic
  (1243-1292), the frame advance still asked `Speed.handle_rate_training`, and a frozen frame took the lag-tic branch that runs
  the camera (the asm returns to 0x8018DA50, past it).
- `sc1PTrainingModeCheckEnterMenu` / `CheckLeaveMenu` read only `gSCManagerSceneData.player` (`check_enter_menu_` 626-669 and
  `check_leave_menu_` 673-715 loop over the four controllers and lend the menu to the one that pressed).
- `sc1PTrainingRemixStartScene` never reset the spawns on entry from the stage select (`load_from_sss_` 841-853).
- `ifCommonBattleGoUpdateInterface` polled the four ports in a 12-Character Battle (`gentlemens_reset_` `_pause_press_check`,
  `TwelveCharBattle.asm` 3712-3718, lowers the bound to 2), and `gmRemixTwelveCBGetWinnerPortraitOf` required the status STARTED
  where the asm only tests "not started" (2897-2910, 1317-1322).

## Fix

- The Remix initial damage is read in `ftManagerMakeFighter` and passed in a copy of the description; `ftManagerInitFighter` is
  vanilla again.
- New pure rules `sc1ptrainingframerules.c` (`sc1PTrainingFrameRulesReadDpad`, `sc1PTrainingFrameRulesResolve`); the scene
  (`sc1PTrainingModeUpdateAll`) runs a frame, one forced frame, the camera only, or nothing. The quick reset restores the BGM volume.
- New `sc1ptrainingremixpause.c`: the pressing port takes the menu, the owner gets it back on leave, on a reset and on the exit;
  the item spawn point and the Close-up view fall back to the owner's fighter.
- `sc1PTrainingRemixSpawnResetAll` on entry from the stage select.
- `gmRemixTwelveCBGetPausePortCount`, and the winner lock for every status but NOT_STARTED.
- Kept better than the asm (`docs/remix/deviations.md` 5.7): unfreezing under a slow Game Speed, the menu hand-over only when the
  pause opens, the Close-up view from a port with no fighter.

## Verification

Unit tests: `test_training_frame_rules.c`, `test_training_port_rules.c` (pressing port), `test_twelvecb_rules.c` (winner lock,
pause ports). In game, before and after (Dream Land, Mario and Kirby):

- Respawn: Training with Percent 88 and VS with panel Damage 50, P1 walks off the stage: 88% / 50% on the halo before, 0% after.
- D-pad Controls set to Off or Reset Only 100 frames after a freeze: 296 of 900 frames ran before, 793 after.
- Paused: D-pad left plays FGM 0xA2 and reloads the scene (BGM volume 0x3C00 then 0x7800), D-pad up and down freeze the match and
  turn the model display on; nothing happened before.
- Frozen for 400 frames: the camera call ran 401 times before, 0 after.
- Eight D-pad right taps while frozen: Speed 1/2 ran 4 frames before and 8 after; Game Speed 2x ran 16 before and 8 after.
- Controller 2 pauses, moves the cursor, unpauses and resets; controller 4 (no fighter) spawns a Maxim Tomato over the owner and
  picks the Close-up view.
- Custom spawn saved at x 2278 on Dream Land, exit, Peach's Castle: Mario appeared at x 2278 before, on his spawn point after.
- 12-Character Battle: controller 3 START showed "3P PAUSE" before and nothing after; with the battle COMPLETE the winner's A on
  another portrait selected it before (select and announcer sounds) and plays the denied sound (FGM 165) after.
