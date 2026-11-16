# Remix stage loader: wallpaper types, Crash bonus 1-up and announcer setup (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, lot h4, findings H4-1, H4-3 and H4-4).

## Symptoms

- 13 Remix stages showed the common wallpaper (it pans with the camera) instead of their own background type: Dragon King, Dragon
  King Remix, N64, N64 Remix, Frosty Village and Sector Z DL/Omega/Remix take the Sector Z wallpaper (zooms with the camera
  distance), Yoshi's Island DL/Omega the static one, Flat Zone, Flat Zone II and GB Land the Bonus 3 fill (no wallpaper sprite).
- On Crash's Break the Targets and Board the Platforms, standing on the 1-up's box did nothing: in Smash Remix the 1-up shrinks
  away, FGM 0x5C3 plays and the music becomes Crash Bash Loading.
- The Pokemon announcer only existed in VS and the 1P game: Training, the bonus stages, How to Play, the VS demo and the VS sudden
  death had no announcer even when it was active (for example Training on Pokemon Stadium with the default mode).

## Root cause

- `Stages.asm` `bg_type` (2774-2789) replaces the vanilla wallpaper switch with a table filled by `set_bg_type` (5065-5275);
  `grRemixWallpaperRulesKind` returned the common kind for every added stage with a function.
- `id_fix_2_` (Stages.asm 632-642) runs a nonzero `function_table` entry of an added stage before the bonus ground: for Crash BTT
  and BTP that is `Hazards.crash_btt_setup` / `crash_btp_setup` (Hazards.asm 9254-9348), which create the 1-up watcher and then
  jump to the bonus ground setup (0x80105700). `grRemixMakeGround` returned on the BTT/BTP class first.
- `id_fix_2_` (Stages.asm 621-627) calls `PokemonAnnouncer.announcer_setup_` inside `grMainSetupMakeGround`, which every scene that
  builds a ground runs; the port called `ifRemixAnnouncerSetup` from the VS and 1P scene starts only.

## Fix

- `grremixwallpaperrules.c` holds the `set_bg_type` table and a Sector kind; `grRemixWallpaperMake` calls `grWallpaperMakeSector`.
- `grremixcrashbonus.c` (`grRemixCrashBonusMake`, rules in `grremixcrashbonusrules.c`) ports the setup and `crash_bt*_main`; the
  BTT/BTP branch of `grRemixMakeGround` calls it before the bonus ground. The setup's clear of `0x801313F0 + 0x60` (the
  under-water flags of the water hazards) has no port counterpart: the port keeps those flags in `grremixwater.c`.
- `grMainSetupMakeGround` calls `ifRemixAnnouncerSetup` first; the VS and 1P calls are gone.

## Verification

`gdb_count.sh` runs (lot h4 report): the wallpaper maker on stages 50, 53, 115, 136 and 152 went from `grWallpaperMakeCommon` to
Bonus3, Sector, static, Bonus3 and Sector (stage 65 stays common); the Crash 1-up DObj sits over line 9 / 0x16, and a fighter on
that line shrinks it, plays FGM 1475 and BGM 367; `ifRemixAnnouncerRun` went from 0 to every frame in Training on Pokemon Stadium,
which played the general clip 0x49A.
