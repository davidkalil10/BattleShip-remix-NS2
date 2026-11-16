# King of the Hill win countdown froze and the Smashketball chant named the wrong fighter (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.1, lot B, items B1, B6 and B7).

## Symptoms

- King of the Hill: when the king left the hill, or someone contested it, during the 3.5 s win countdown, the countdown stayed
  where it was and went on when the same king came back, so a match could end after less than 3.5 s of uninterrupted holding.
  A countdown frozen at 30, 90 or 150 played the "three", "two" or "one" announcer voice on every frame while the hill stayed empty
  or contested (1067 "three" voices in one run).
- Smashketball: on a third basket in a row the crowd chanted another fighter's name, or nothing, and from then on it chanted again
  on every basket of the streak.

## Root cause

- `gmRemixKothTrackScore` (`gmremixkothking.c`) returned before the scoring loop when there was no king. In the asm
  (`KingOfTheHill.asm` 1292-1381, `track_score_`) the loop runs every frame, and `_score_next` clears the `winner_times` of every
  team that is not the king, so a frame with king -1 or -2 clears all of them.
- `gmRemixSmashketballScoreOwner` (`gmremixsmashketballscore.c`) passed the port to `ftPublicRemixForceCall`; the asm passes
  `FTStruct.player_num` (+0x18, `Smashketball.asm` 897-899), which is what `ftPublicTryStartCall` looks the fighter up by. The
  player_num is a creation counter that changes at every respawn, so it rarely equals the port.
- The forced chant skipped the same-player check in `ftPublicTryStartCall` (`ftpublic.c`). The asm hook `force_chant_`
  (`Smashketball.asm` 1136-1142) jumps back to 0x80164B14, which is that check (ROM disassembly: `lw t9, -0x3054(t9)` loads
  `sFTPublicCallPlayerNum`, then `bnel t8, t9`), so only the damage and wait checks are skipped.

## Fix

- The scoring loop runs every frame; the kept-timer rule is `gmRemixKothRulesWinnerTimeKept` (`gmremixkothrules.c`).
- The chant gets `owner_fp->player_num`, and the same-player check applies to forced chants too.

## Verification

- `m14b_b1_before` / `m14b_b1_after` (Peach's Castle, `SSB64_KOTH_LINE=1 SSB64_KOTH_GOAL=1`, the hill moved under gdb to an empty
  line when a win timer reached 30): before, the timer stayed at 30 for 500+ frames and the "three" voice (FGM 470) played 1067
  times; after, the timer is 0 the next frame and the voice played twice in the whole match.
- `m14b_b7_before_*`, `m14b_b6_after_*`, `m14b_b7_after_*` (Smashketball, teams, all CPUs, ports 1, 2 and 4, streaks forced to 2 under gdb): before, 9 of the 10
  forced chants looked up a fighter other than the owner (2) or none (7); after, all 10 resolve to the owner and chant. With the last
  called player set to the owner, the before binary still chanted (3 times) and the after binary never does.
