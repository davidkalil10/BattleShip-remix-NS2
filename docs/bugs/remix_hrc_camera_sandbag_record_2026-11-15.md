# Home-Run Contest: camera pan, a Sandbag that fought back and a record saved on FAILURE (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, lot m3, items M3-1, M3-4 and M3-5).

## Symptoms

- The camera locked onto a flying Sandbag almost at once, so the Sandbag sat near the centre of the screen and the platform left the view
  in about 20 frames. In Smash Remix the camera trails well behind.
- After about 14% of damage the Sandbag turned, walked, jumped and air-jumped by itself. A weak hit that should have pushed it off the
  platform and past the start line (4.0 ft, COMPLETE) ended in FAILURE because the Sandbag jumped back.
- An attempt that ended in FAILURE could store a distance as the fighter's record.

## Root causes

- `HRC.asm` 100-103 (`main_`) pans the camera by `(sandbag x - camera x) / 8.0` each frame: the constant is `lui at, 0x4100`, and the
  comment beside it says 3. The port copied the comment (`SC1PREMIX_HRC_CAMERA_PAN_DIVISOR 3.0F`). The warp test reads the panned X,
  so the 100 ft warp and the sign change also came at other frames; the distance itself does not depend on the camera.
- `SinglePlayerModes.asm` 7081-7097 (`rest_area_and_hrc_spawning`) makes the HRC opponent player kind 5, the kind of the How to Play
  fighters, which reads a key script and here has none: no input at all. The port left it `nFTPlayerKindCom` with the `nFTComputerTraitLink`
  trait of the first 1P stage, which stands still only below 14% (`ftComputerProcessTrait`).
- The asm writes `HRC_HIGH_SCORE_TABLE` from `_time_save` (`SinglePlayerModes.asm` 1301), a hook inside the Complete announce
  (0x80114D64). `HRC.asm` 340-349 (`_failure`) calls the plain end message. The port called `sc1PRemixHrcRecordSave` before it chose
  between the two, and the distance keeps its last value when the Sandbag comes back behind the start line.

## Fix

- `sc1premixhrcrules.c`: the pan divisor is 8.0.
- `sc1premixhrc.c` `sc1PRemixHrcFighterKind`, called where `sc1pgame.c` fills `desc.pkind`: the Sandbag fighter is
  `nFTPlayerKindGameKey`. The battle-state slot stays a CPU so the player count and the damage displays keep the Sandbag
  (`docs/remix/deviations.md` 5.7).
- `sc1premixhrcrun.c` `sc1PRemixHrcRunEndAttempt`: the record is saved only in the COMPLETE branch.

## Evidence

Scene 52 with `SSB64_REMIX_1P_MODE=hrc`, Mario, a bat smash at frame 215 (`sc1PRemixHrcTrack` printed under gdb): the camera step over the
gap to the Sandbag was 0.3333 on every frame before and is 0.1250 after, with the same 1582.7 ft. Two forward smashes with no bat (17%):
before, the Sandbag went through Turn, JumpAerialB, FallAerial and WalkMiddle and the attempt was a FAILURE; after, it only falls and lands
(Fall, LandingLight, Wait), and the attempt is COMPLETE at 4.0 ft. With the distance forced to 123.4 ft behind the start line, the FAILURE
logged `hrc: record saved ... distance=1234` before and calls `sc1PRemixHrcRecordSave` zero times after.
