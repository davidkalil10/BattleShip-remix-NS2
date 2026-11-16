# CPU AI: Magic Cloud inputs, Sheik and Peppy charge, level 10 Metal targets and Smashketball aim (2026-11-15)

Found by the M14 equivalence audit of `AI.asm` and the per-fighter AI files (ROADMAP T14.3, lot a1). Four hooks existed in
the port but did not do what the asm does.

## 1. Goemon and Ebisumaru on the Magic Cloud got no CPU inputs

- Symptom: a Goemon CPU riding the Magic Cloud did not hold the stick up on most frames and almost never pressed Z over
  the stage; an Ebisumaru CPU almost never pressed A for the nair once above the floor (8 of 82 frames above a floor).
- Root cause: `recovery_stick_y_fix` (`AI.asm` 1401-1460) replaces the loop condition of `ftComputerUpdateInputs`
  (0x8013253C), so it runs after every command except END (END branches to 0x80132554). The port called
  `ftRemixAiCloudRecoveryInput` after the loop, which is only reached when a script ends on a wait. The walk and recover
  objectives send the script `MoveAuto, END` every frame, and its END returned before the call.
- Fix: the call sits at the end of the loop body in `decomp/src/ft/ftcomputer.c`.

## 2. Sheik and Peppy CPUs stopped deciding while they charged far from the target

- Symptom: with the target 1200 units or more away, a Sheik or Peppy CPU that had started its needle or revolver charge got
  no attack or far-range decision until the charge ended.
- Root cause: the begin and ground charge actions of `improve_remix_specials` (`AI.asm` 967-1023) leave through
  0x80136BF8, which is `move v0, zero`: `ftComputerCheckTryChargeSpecialN` returns FALSE there, like the vanilla Samus and
  Donkey Kong charge actions. The port returned TRUE, and the caller (`func_ovl3_8013837C`) turns TRUE into "decide nothing"
  (-1) instead of 1.
- Fix: `ftRemixAiSpecialsNeedleCharge` returns TRUE only when it presses B (`ftRemixAiSpecialsNeedleChargeReturnsTrue`).

## 3. A level 10 Metal Mario or Metal Luigi ignored airborne opponents

- Symptom: the CPU stood on the stage while its opponent was in the air.
- Root cause: the level 10 branch of `detect_offstage_opponents` (`AI.asm` 4868-4898) jumps to 0x80132AB8, the distance
  code, which lies after the Metal test of `ftComputerCheckFindTarget` (0x80132AA0-0x80132AB4). The port skipped only the
  on-stage group.
- Fix: the Metal test is skipped too when `ftRemixAiMovementTargetsOffstage` is TRUE.

## 4. Level 10 CPUs aimed at the opponent in Smashketball, not at the ball

- Symptom: a level 10 CPU chose and timed its attacks against the target fighter; lower levels used the ball.
- Root cause: `ai_ball_attack_range` (`Smashketball.asm` 1210-1259) writes the ball into the target stack slots of
  `ftComputerCheckDetectTarget`, and the level 10 `fix_movement_prediction` (`AI.asm` 5317-5391) reads those slots.
  `ftRemixAiPredictAttackPosition` read the target fighter instead.
- Fix: `ftRemixAiPredictionGetTargetMotion` (`decomp/src/ft/ftremix/ftremixaiprediction.c`) takes the ball numbers from
  `ftRemixAiModesGetBallTarget` when a ball is in play. As in the asm, the grounded test and the floor distance stay the
  target fighter's.

## Kept different from the asm

`docs/remix/deviations.md` 5.9: the level 10 combo weight keeps the opponent's own fall numbers in Smashketball (the asm
uses the ball's by side effect), and a Metal Mario below level 10 tests every opponent (the asm's `metal_luigi_ai`
overwrites `s7` and tests only the first).
