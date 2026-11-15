# Smash Remix 2.0.1 in the PC port: roadmap

This is the **only plan** of the Remix work. Whoever works on the Remix (agent or human) follows this file, without
opening work outside it: new work first gets a milestone here. The Remix screens stay 4:3: widescreen is **out of scope**
for the Remix work (the port's own optional Widescreen setting only widens battle scenes). The reference documentation
is indexed in [`README.md`](README.md).

## How to use (work protocol)

1. At the start of each session, read **Current position** and any open milestone.
2. Work only on the first open item of the current milestone. A blocked item gets `BLOCKED: <reason>` and the next one is
   done. New work (a bug batch, an audit) opens a new milestone after the last one, with its tasks, before any code
   changes.
3. An item is ticked `[x]` only after it passes its acceptance test: the build, the unit tests, and an in-game smoke test
   (skill `remix-smoke-tests`) that the lead runs on the build that will be committed. A fix with pure rules also gets a
   unit test in `tests/remix/`, on top of the smoke test, never instead of it. A fix that touches several screens, modes
   or call sites is smoke-tested on each of them. Note the short evidence next to the item (run name, counts, what the
   screenshot shows), and set its state in `patch_audit.md` when it is a ledger item.
4. End of a milestone or phase = **checkpoint**: bug write-up in `docs/bugs/` when one applies (with its index row),
   commits in the decomp and in the main repository **without attribution trailers**, update of **Current position**.
   It is also a checkpoint when the user writes "checkpoint".
5. Workers (Agent) run in parallel, one package or batch each (disjoint files), with a brief written for it (not kept in
   the repository). They write code, research and their own before/after runs; the lead reruns the smoke tests, signs
   off and commits. A batch closes with its own commit.
6. A significant bug fixed becomes `docs/bugs/<topic>_<YYYY-MM-DD>.md` with a row in `docs/bugs/README.md`.
7. Reply to the user in Portuguese; everything written in the repository is in English.

## Fixed rules

- Toolchain and dependencies only in `.toolchain/` (no brew, system pip, sudo, `/opt`, `/usr/local`, `~/.cache`,
  `~/.cmake`). Python: `.toolchain/host/venv/bin/python -I`. `builtin cd`.
- Build with the `.toolchain/` cmake and ninja and absolute paths: `cmake --build <build dir> --target ssb64 -j <n>`; the
  configure line of each platform and the build directories are in `build_and_roms.md`. A new file in `decomp/src` needs
  the configure step again.
- ROM-derived bytes never go into versioned files. Authorised source: `.toolchain/pkgs/smashremix-src`.
- The asm is the reference, except where it is itself wrong: an asm bug, a missing piece or something the port already
  does better is kept or fixed in the port, never copied, and recorded in `deviations.md` (owner policy, 2026-11-15).
  No fallback to the parent fighter and no containment hacks; a necessary deviation is recorded in `deviations.md` (or
  a `docs/bugs/` write-up when it comes from a bug).
- Code in English, no `//` comments that repeat the code. `#ifdef REGION_REMIX` inside vanilla files.
- References in comments: `torch/` and `libultraship/` are independent repositories, so their comments do not cite docs
  of this repository. `decomp/` may cite `docs/remix/*.md` and `docs/bugs/`. Durable findings go into the topic doc
  (`docs/remix/<topic>.md`), never into per-task notes or `build/`.
- Tests: skill `remix-smoke-tests` (`.claude/skills/remix-smoke-tests/`: `vs.sh`, `scene.sh`, `css.sh`,
  `parallel.sh`, `special_matrix.sh`, `stage_sweep.sh`, `item_cpu_sweep.sh`, `sheet.py`, `lldb_count.sh` on macOS and
  `gdb_count.sh` on Linux, the inputs and how to read results), runs in `build/agent-shots/vs/`; reference emulator
  `scripts/ref-emu/capture.sh` (`notes/i8_reference_emulator.md`). A `CAMetalLayer nextDrawable` crash is a macOS
  flake: rerun before investigating.

## Current position

**M14 is open (equivalence audit of the non-toggle logic, 2026-11-15); M0 to M13 are closed.** The Remix is playable
on every platform the releases ship, and the owner validated the listening pass, the Switch hardware and the updaters
on 2026-11-15. Releases are not tracked here: they follow `AGENTS.md` ("Releases") and `build_and_roms.md`.

| Milestone | State |
|---|---|
| M0 to M10 | closed; their user passes T9.1b and T10.2 validated by the owner (2026-11-15) |
| M11 Finalization | closed 2026-11-06: "Remix playable" (T9.3) reported; ledger `patch_audit.md` 266 `done`, 4 `n/a`, 0 `open`; `parity.md` 277 `ported`, 2 `adapted`, 10 `n/a`, 0 `partial` |
| M12 Platforms and releases | releases from `release-remix.yml` (desktop, Switch, PS5, Android) |
| M13 Toggle parity audit | closed 2026-11-15: 35 findings in 10 of the 11 batches, each fixed with a lead smoke test or kept as a `deviations.md` row |
| M14 Equivalence audit of the non-toggle logic | open: the pilot confirmed 23 differences (T14.1); the rest of the audit is T14.2 |

- **Next step:** T14.1 lot A (hazards).
- **Last checkpoint:** M13 closed (2026-11-15; outer `8332d12`, decomp `3413abab6`).
- **Overall estimate:** 100% by the table at the end of this file, which counts presence (every asm patch hooked); the
  equivalence of the non-toggle logic is what M14 measures.

## Owner validations

The passes only the owner or a console can do; all closed on 2026-11-15.

- [x] T9.1b Listening and visual pass by the owner (the lead cannot hear): stages, victory songs, Sound Test, hit SFX,
  menu graphics; `t9_1b_checklist.md`. Done: validated by the owner (2026-11-15).
- [x] T10.2 Switch hardware validation (`docs/switch_port_2026-10-12.md`, rounds R1 to R6 under **M10**). Done: validated by the owner (2026-11-15),
  after the display-list walk fix of 2026-11-14 (`docs/bugs/dl_walk_past_window_false_positive_2026-11-14.md`), the fix
  for the `gfx_step: walked past registered DL range` stutter. Metal Mario tiny on the Training CSS was never
  reproduced.
- [x] M12 updaters on a real install: the Switch zip update and the macOS in-place update. Done: validated by the owner (2026-11-15).

---

## History and decisions (M0 to M10)

Standing decisions:
- No fallback to the parent fighter and no containment hacks: behaviour equals the Remix. A necessary deviation is documented in `docs/bugs/`.
- Widescreen is out of scope (4:3). ROM-derived bytes never enter versioned files; Remix data is read at runtime from the Remix RAM image (`remix_ram_image`, symbols from `tools/gen_remix_actions.py`).
- The Remix menu framework (`Menu.asm`) is not replicated; the port menu (Remix section) has the same entries and values.
- Fighter ids equal the Remix fkind; registries, tables and the save are sized for 97 fighters.

### M0. Governance
One roadmap and one parity table.
- [x] T0.1 Single roadmap, referenced from `AGENTS.md`; `ROADMAP.md`.
- [x] T0.2 Per-`.asm` parity table; `docs/remix/parity.md` from `tools/gen_remix_parity.py`.

### M1. Stabilise roster, items, stages
All Remix stages load; the 15 ported hazards and the standard items behave in a match.
- [x] T1.1 Mementos (59) loads: relocation-chain fixups stop at the first pending slot; `portRelocBeginChainWalk`; `docs/bugs/reloc_chain_fixup_overwrites_pending_slot_2026-10-14.md`.
- [x] T1.2 Dream Greens (149) crash fixed in `grpupupu.c` (vanilla Dream Land untouched).
- [x] T1.3 Standard items verified in match; `docs/bugs/remix_standard_items_2026-10-14.md`. Psyduck is disabled in Remix 2.0.1 (n/a).
- [x] T1.4 Fifteen hazards verified in match; `gr/grremix`, `it/itremix`.
- [x] T1.4b Subsystem-free hazard `REMIX-TODO`s (KlapTrap, Robot Bee, Cacodemon animations, Pirate cannon, Rainbow chomp, Casino bumper); `gr/grremix`.
- [x] T1.5 Remaining hazards: `grremixtimetwister.c`, `grremixscuttle.c`, ScuttleMinion/BulletBill (items 0x3E/0x3D, `itremixitems.c`), Peach Castle II Banzai factory, Bowser's Stadium bombs, Yoshi clouds, Dream Land wind. Decisions: the 2.0.1 asm switches the Scuttle minion off (`b _end`), mirrored by `GRREMIX_SCUTTLE_MINIONS_ENABLED`; the Banzai Bill is dead code in 2.0.1 (nothing references `peach_castle_II_setup_`), so the factory stays faithful (Robot Bee 0x3B, no laser) and unreachable (`hazards.md`).
- [x] T1.5b Time Twister (210) and Scuttle Town (191) background/warp-ball fades. The source lists it open in M1, but M11 Phase 1 I6 reports it complete and seen on screen; `gr/grremix/grremixstagebg.*`. (State: done in M11.)
- [x] T1.6 Stage select with 85 stages in 10 pages; Training never reads outside the vanilla tables (`mnmaps.c`).
- [x] T1.7 Sweeps (80 fighters x NSP/DSP/USP, stages 41 to 215, 17 items) without crashes beyond the macOS `CAMetalLayer nextDrawable` flake.

### M2. Gameplay subsystems
- [x] T2.1 GFX + GFXRoutine (particle scripts, 50 colanim routines in `dGMColScriptsDescs` with `port_override`); `docs/bugs/remix_gfx_and_gfxroutine_2026-10-16.md`.
- [x] T2.2 CharEnvColor: `ftremixenvcolor.c`, read in `ftdisplaymain.c`. Deviation: the asm swaps the material display list for alpha (`custom_display_lists_*`); the port only uses env-colour alpha, so parts with special render modes (Yoshi tongue, GND shield) may lack the adjustment (reopened by the patch audit as GP-6).
- [x] T2.3 Reflect: custom hitbox (`FTSpecialColl.kind` = id << 16 | 3) with per-id routines; `ftremixreflect.c`. CPU hooks are M7.
- [x] T2.4 Size phases b and c (weights, speeds, jumps, armour, hitboxes, entries); `size.md`.
- [x] T2.4b Match size state (`Size.match_state_table`): `ftRemixSizeGetMatchState/SetMatchState` in `ftremixsize.c` (GIANT 2.25, TINY 0.5); Lightning, Metallic laser and Mushrooms restore the mode size; per-port state from the panel.
- [x] T2.5 PokemonAnnouncer: `if/ifremixannouncer.c`, `ifremixhitannouncer.c`. Naturalness is judged by T9.1b.
- [x] T2.6 Kirby: `ftremixkirbycopy.c` (Remix inhale table, 97 fighters), `ftremixkirbyhats.c`, `ftremixkirbypower.c`; `docs/bugs/remix_kirby_copy_actions_2026-10-16.md`.
- [x] T2.7 Other hit systems (Knockback, Poison, Damage, Surface, Blastzone, Camera, Spawn, CustomGrab, `command.asm`); `vanilla_patches.md`, `hazards.md`.
- [x] T2.7b Neutral spawns, blastzone warp, pause camera, conveyors (`grRemixSurfaceConveyorPush`), stun/flashbang (`ftremixdamagestun.c`), bury/plunge (`ftremixbury.c`).
- [x] T2.8 Piano `extra_part_visibility_`, Marina `prevent_item_despawn_`; `clear_items_in_1p` already covered.
- [x] T2.9 Containments kept as documented substitutes for undefined Remix behaviour: capture-joint fallback (`ftcommoncapturepulled.c`; Bowser's `joint_itemheavy_id` is 31, inherited from Yoshi) and NULL `throw_desc` (`ftcommonthrown2.c`; also moves without `SetThrow`, e.g. Banjo USP). On N64 the null pointer reads RDRAM page zero.
- [x] T2.9b Polygons' TransN guard. Closed in M11 Phase 3: motion rows have flags 0 in the Remix ROM; 0x40000000 comes from `ftPortParentHiddenJointFlags` (`ftport.c`); the guard in `ftmain.c` clears XRotN/TransN/YRotN in entry statuses.
- [x] T2.10 fkind audit: Kirby inhale reads `copy[victim_fp->fkind]` (also `efmanager.c`), Falcon Dive via `Character.falcon_dive_id.table`; the thrown-damage script uses the parent row (an index above 27 is undefined in the Remix).
- [x] T2.11 `REMIX-TODO` outside AI/modes/menus closed (M11 Phase 3). Raw `ITStruct` offsets mapped to named fields; `itremixblueshellrules.*`, `if/ifremixrectangle.*`. Reverted on purpose: Metallic laser and Marina rotation read 0x8019CAA0/0x8018C230 from the RAM image (starts at 0x80400000, returns 0); they stay DEVIATION/TODO with the reason.

### M3. Audio
- [x] T3.1 BGM: 375 sequences, per-fighter victory BGM (`mnremixwinbgm.c`), results load Remix fighter files (`mnvsresults.c`).
- [x] T3.2 SFX/voices: blobs `audio/fgm_tbl`, `fgm_ucd`; Sector Z hit-SFX pan (`lbcommon.c`).
- [x] T3.3 Sound test: `mnsoundtest.c`, `mnremixsoundtest.c` (375 songs, 1094 voices).

### M4. Settings and toggles
- [x] T4.1 Registry of 531 toggles with profiles CE/TE/NE/JP (`remix_toggles.cpp`), CVar `gRemix.Toggle.<n>`, `gmRemixToggle(<name>)`/`gmRemixToggleVS(<name>)` (`gm/gmremixtoggle.h`).
- [x] T4.2 Gameplay toggles, one file per system: about 60 toggles from Hitstun to Neutral Spawns, results toggles `gmremixgameend.c`, Menu Music `gmremixmenumusic.c`, Disable HUD `ifremixhud.c`, Model Display `ftremixhitboxdisplay.c`; `toggles.md`.
- [x] T4.3 Stage/item toggles: `grRemixHazardsAreOn`/`grRemixMovementIsOn`, `itRemixStandardItemIsOn`.

### M5. CSS, menus, screens
- [x] T5.1 VS CSS (`mnplayersvs.c`): 3x10 grid, costumes/shades, D-pad variants, Random Select; zoom crash `docs/bugs/remix_css_camera_zoom_null_battle_state_2026-10-20.md`.
- [x] T5.1b 1P/Training/Bonus CSS sharing `mnplayers/mnremixcss.c`.
- [x] T5.1c Stage select emblem (`mnmaps/mnremixemblem.c`) and variants on C-down (`mnmaps/mnremixstagevariant.c`).
- [x] T5.2 Per-port panel: `port/remix/remix_player_panel.*` (21 options, `SSB64_REMIX_PANEL`), read through `gmRemixPanel(OPTION, port)` in `gm/gmremixpanel.*`; `panel.md`.
- [x] T5.2b/T5.3b Panel and screen leftovers moved to T8.2.
- [x] T5.3 Screens: results no longer white (Metal backend lacked `SetColorWriteMask`; `docs/bugs/metal_redirect_z_color_write_2026-10-20.md`), Remix Modes page (`mnvsmode/mnremixvsmode.c`), Music Title/Input Display overlays, credits (`credits/remix/`), Character Data (`mndata/mnremixchardata.c`), Gallery (`mncommon/mnremixgallery.c`), VS stats (`mnremixvsstats.c`). Settings button, L shortcut and toggle menu are replaced by the port menu.
- [x] T5.4 Containment hacks (`docs/bugs/remix_css_and_parent_fallbacks_2026-10-12.md`): capture-joint and `SetThrow` guards log once; kept because inherited routines read those globals: CSS drop on unloaded file, figatree heap, parent `SetupFilesAllKind`.
- [x] T5.5 Save for 97 fighters (`GMCOMMON_FIGHTERS_SAVE_NUM`, `lbtypes.h`; `lbBackupInitRemixDefaults`): 8 KB to 123 KB; an old save fails the checksum and restarts.

### M6. Modes
Pure-rule modules with unit tests in `tests/remix/` (target `run_remix_mode_tests`).
- [x] T6.0 Mode base: `gm/gmremixmode.c`, `SSB64_REMIX_MODE`, Timed Stock, `port/remix/remix_hud_text`; `modes_base.md`.
- [x] T6.1a Stamina: `gm/gmremixstamina.*`. T6.1b Tug of War: `gm/gmremixtugofwar.*` (probable asm bugs in shield break and team credit checked before copying).
- [x] T6.1c King of the Hill: `gm/gmremixkoth*.c`, `mnremixgoalpicker`. Fix: team score counts once per frame (the asm summed per player).
- [x] T6.1d Tag Team: `gm/gmremixtagteam.c`. Decision: queue not drawn in the CSS (slot 0 from the CSS, slots 1 to 5 from the port menu, Random by default); Phase 5 later drew it in the CSS. The Remix does not avoid repeats in the queue (`_recent_randoms_lookup`, window of 6), so no repeat rule is ported.
- [x] T6.1e Smashketball: `gm/gmremixsmashketball*.c`.
- [x] T6.1f 12-Character Battle rules: `gm/gmremixtwelvecb*.c` (game vector of 48 instead of the asm's 24, which overflows). Visual was a provisional overlay until Phase 2. Fix: the START block once "seen" was a false positive; `mnplayersvs.c` now closes slots 3 and 4 in 12CB.
- [x] T6.2 1P modes (`sc/sc1pmode/*`, `mn/mn1pmode/mnremix1pmode.*`, `SSB64_REMIX_1P_MODE`):
  - Bonus 2 exit `sc1premixbonusexit.*`; Multi-Man/Cruel `sc1premixmultiman.*` (KO record `multiman_kos/cruel_kos`; `_percent_fix`, `crash_prevent` n/a); HRC `sc1premixhrc*.*` (record `LBBackup1PRecord.hrc_distance`); `docs/bugs/remix_1p_hrc_and_path_cp6e_2026-10-27.md`.
  - R1 data from the RAM image `sc1premix1pdata.*`; R2 draw `sc1premix1prules.*` (deviation: the asm loop that never ends returns -1 and the draw picks another fighter); R3 per-stage match (`docs/bugs/remix_1p_size_override_cp6f_2026-10-27.md`; deviation: `_opponent_cpu` compares against an overwritten register, so CPU level is 10 on Very Hard everywhere); R4 title cards `sc1premixintro.*`; R5 Giga Bowser final (`sc1PRemix1PBossHitPoints`: 200/250/300).
  - A1 All-Star: `sc1premixallstar.*` (order, progress, Rest Area, hearts, 24-member final, end and credits).
- [x] T6.3 Training (the Remix has no save state): D-pad freeze/advance/model/quick reset and Reset Count `sc1ptrainingremix.*`, 13 extra items in the pause item row.
- [x] T6.D1, T6.D2, T6.D3, T6.D4: test shortcuts `port/remix/remix_debug.*`; `gmremixmode.c` split; mode-rule unit tests (`remix_mode_tests`); mode files above 300 lines split.
- [x] T6.D5 Review of worker code. Deviations: 127-stock cap in Tug of War (asm wraps at -128); `last_hit` indexed by victim port, not KOer team (`TugOfWar.asm:302`).
- [x] T6.D6 Tag Team icons and stock steal `gm/gmremixtagteamicons.*`; `docs/bugs/remix_tagteam_icons_and_link_boomerang_2026-10-29.md`.

### M7. AI
Finding: the 30 Remix fighters used the parent's attack list (`remix_roster.cpp` copies the descriptor); Remix tables are in the image (`Character.ai_behaviour.table`, `lv10_ai_behaviour.table`).
- [x] T7.1 AI.asm (70 hooks), Improved AI, Single Button Mode CPU hooks, `VanillaLv10Attacks.asm`; `ft/ftremix/ftremixai*.c` with small hooks in `ftcomputer.c`; `docs/bugs/remix_routine_gobjs_on_effect_link_2026-10-29.md`.
  - [x] T7.1a attack tables `ftremixaitables.*`. [x] T7.1b custom input scripts `ftremixaicommands.*` (deviation: TURNAROUND_WAIT waits for Turn or TurnRun; jump table became a `switch`). [x] T7.1c PREVENT/LONG_RANGE `ftremixaiprevent.*`, `ftremixailongrange.*`. [x] T7.1d combat/recovery (`ftremixaiteching.*`, `ftremixaispecials.*`). [x] T7.1e level 10 and DI (`ftremixailv10.*`, `ftremixailevel.*`). [x] T7.1f CPU level to 10 in the CSS (`mnremixcpulevel.*`).
  - AI leftovers (`REMIX-TODO(AI)`) are Phase 6.

### M8. Remaining parity
- [x] T8.1 (absorbed by M11 Phase 4) Ported: AerialFastFall, Skeleton, timeouts (`mnremixtimeouts.*`), tournament rules (`sc/scremixtournament.*`), SFXReplace, Practice overlays, DK Mode (`ftremixdkmode.*`; a port variable because the RAM image is read-only), Sword Trail, MagnifyingGlass (`gmremixmagnify.*`), VsDemo (`scremixvsdemo.*`), Accessibility, Hitbox (`ftremixhitbox.c`), Japan fixes (`ftremixjapanfix_reflect.*`). Cheats not needed (unlock-all and music are port toggles); Walljump is inactive in 2.0.1.
- [x] T8.2 Leftovers of M5/M6/M7, mostly absorbed by M11 Phases 4 and 5 (the source left it unticked). Done here: Training Z menu as port section (`remix_training.*`; deviation: CPU only), `cpu_di_` (`ftremixaidi.c`), result rows, Stock Mode Last (`gmremixstockmode.c`), pause legends (`ifremixpauselegend.*`), handicap rules (`mnRemixHandicap*`), Training Music, per-port Training chain (`sc1ptrainingportrules.*`; deviation: option **Auto** keeps the CSS value).
- [x] T8.3 Visual fidelity of mode texts/HUDs: executed as M11 Phase 2.

### M9. Final desktop verification
- [x] T9.1 Full sweep: the automatable part is done (sweeps, 4-player matches in six modes, Training through the CSS); the regression pass is M11 Phase 8. Defect fixed: Salty Runback/R+L combo needed the held state **equal** to the mask (`Joypad.check_buttons_all_`); `gmremixgameendinputrules.*`.
- [x] T9.1b Listening pass by the user: validated by the owner (2026-11-15) (see **Owner validations**).
- [x] T9.2 Vanilla `REGION_US` builds and plays: CMake of `us`/`jp` skips Remix-only sources (`ft/ftremix`, `gr/grremix`, `it/itremix`, `port/remix/`, keeping `remix_debug`); `REGION_REMIX` guards in `ftport.c`, `port.cpp`.
- [x] T9.3 Report to the user in Portuguese ("Remix playable"), 2026-11-06 after Phase 8; adaptations listed in
  `deviations.md` and the `adapted` rows of `parity.md`.

### M10. Switch
- [x] T10.1 `scripts/switch/build.sh remix` (`BattleShip-Remix.nro`; fix: `<math.h>` before `<ft/fighter.h>` in `ftremixaipostsheik.c`) and `scripts/switch/pack.sh`.
- [x] T10.2 Hardware validation by the user (`docs/switch_port_2026-10-12.md`), validated by the owner (2026-11-15); six rounds of findings:
  - R1 black screen/slow audio: `gfx_vtx_addr_is_unresolved` rejected pointers below 4 GB (`libultraship/src/fast/interpreter.cpp`). R2/R3 title offsets (`mntitle.c`); the odd 1P Game was Classic Co-op, now off by default in `REGION_REMIX`.
  - R4/R5 CSS `menu_zoom` (`tools/gen_remix_roster.py`), entry statuses from `Character.entry_action.table`, default save masks (`scmanager.c`), watchdog/slow-frame logs (`port/port.cpp`).
  - R6 Tag Team crash: `sSCVSBattleStatusBuffer[100]` (`scvsbattlefiles.c`) overflowed with 24-fighter queues; 1024 under `REGION_REMIX` plus a critical log in `lbreloc_bridge.cpp`.
  - Last two: the `gfx_step: walked past registered DL range` stutter (display-list walk fix of 2026-11-14) and Metal Mario tiny in the Training CSS (never reproduced).

---

## M11. Finalization (approved 2026-11-03)

End criterion: "Remix playable" with **all** Remix features present. An adaptation to the port is accepted when the feature
exists for the player and the deviation is documented in `deviations.md`. `n/a` only for the approved items: the Spawned
Fear Practice overlay (no observable effect) and Advanced Hurtbox with the `Hitbox.asm` debug patches (debug toggle only).
The credits speed assist, approved as `n/a` on 2026-11-03, turned out to be ported (`scstaffroll.c` calls
`sc1PRemixAfkIsSpeedAssistActive`); the Character Data emblem and move names left the `n/a` list on 2026-11-05 because the
asm has them. In `parity.md` every Remix file ends as `ported` or `adapted`, never `partial`.

User decisions (2026-11-03): the reference emulator lives in `.toolchain/`, outside git (I8); test packages embed the `.o2r`
(personal use, marked in the file name and script header), the final publishable packages carry no `.o2r` and extract it
from the user's ROM on first run; CI builds `remix` in the matrix of `.github/workflows/release.yml` (superseded: releases come from `release-remix.yml`, see **M12**).

Closed phases:

- [x] **Phase R** (recover interrupted work): Kabel HUD of the HRC, button sprites as native SObj (`menus.md`), versioned package scripts (`scripts/linux/`, `scripts/macos/package-remix-contained.sh`; `build_and_roms.md`), old subagent work re-checked, current `parity.md` (`adapted` counts as done).
- [x] **Phase 1** (infrastructure), all but part of I7: I1 display groups `gmremixgroupdisplay*`; I2 extension sprites per SObj `if/ifremixsprite*`; I3 Kabel text `port/remix/remix_font_layout.*`; I4 runtime BGM `gm/gmremixbgm*`; I5 `tools/n64_addr_to_symbol.py`; I6 stage group 2 and camera `gr/grremix/grremixstagebg.*` (closes T1.5b); I8 reference emulator `.toolchain/ref-emu/` (`scripts/ref-emu/capture.sh`, `notes/i8_reference_emulator.md`). I7 part image/palette swap (`ftremixpartimage*`, `ftremixyoshiegg.c`, `notes/i7_part_image.md`) delivers extra costumes in match and in the CSS cycle and the Yoshi shield/escape egg colour; the thrown, laid and entry egg colour followed in Phase 5b lot 3 (below). Classic Sonic is not I7 (main-file swap); done in Phase 5.
- [x] **Phase 2** (visual of the modes, T8.3): all `remix_hud_text_*` callers use the asm parameters; Match Stats (sprite buttons, Kabel page, stock icons); pause legend; 12CB CSS steps 1 to 19b (`css_reference.md`; Gentlemen's Reset `if/ifremixgentlemensreset.*`, Custom set and StockMode Manual in `mn/mnplayers/mnremixtwelvecb*`); Multi-Man HUD sprites; HRC counter; Training item-name image, D-pad legend and Z menu of 23 entries (`sc1ptrainingremix*`); results labels (`mnVSResultsMakeRemixRow`); KOTH meter; music-title banner (LUS toast and `remix_overlay.*` removed). Deviations: "Press Z" opens the port menu in the prompt (`remix_training_open_menu`); the Dragon King Multi-Man HUD, the Smashketball result row and the 12CB CPU-opponent reset branch were checked by pure rules only. Empty-panel Smash emblem: no Remix patch draws it (nothing to port).
- [x] **Phase 3** (stages and gameplay, T2.9b, T2.11): Size of Ness (`ftremixsizeness.*`) and entries, Banjo USP wave, Goemon cloud, Yoshi `stage_hazard_table`, Rainbow Chomp, Metallic laser, Pirate cannonball (its speed does not exist in the asm; `cannonball_properties_struct` is dead data), Whispy wind box (`grRemixDreamLandGetWindBox`), Z-cancel cruel (`docs/bugs/remix_zcancel_egg_lp64_2026-11-04.md`), D-pad Control `ftremixdpad.c`, Corneria laser, Marina throw_turn, throw announcer, Shield Costume `ftremixshieldcostume.c`, Hitbox display. Zebes/Japes fields proved not portable. Deviations: NKirby keeps the vanilla fthrow (apparent Remix bug, compare in the emulator in Phase 8); Big Boo background sprite (`bg_animation_`) not ported; Kirby Cube pair `n/a`.
- [x] **Phase 4** (CSS, stage select, panel, screens; T8.1/T8.2): title "Smash Remix v2.0.1" (`mnremixtitleversion.*`), credits glyphs and speed assist, Gallery with 40 (`mnremixgalleryrules.*`), item-switch legend, RANDOM ALL, transition logos; stage-select grid of 5 pages with roulette and tournament layout (`mnremixstage*`); panel features (D-pad Map, DpadControl, tag names, Start With, Practice 1P, Skip Start Cheer, Enemy Player/AFK, Link JP sounds, CPU costumes); VS CSS bookends, variant icons and J/E flag (`tools/test_remix_css_offsets.py`), slide-in `mnremixcssslide*`, high-score gating `sc1premixhighscores*`; per-port panel menu in the CSS (`mn/mnplayers/mnremixcsspanelmenu*`, `panel.md`); CSS crash fix `docs/bugs/remix_css_bookend_width_img_2026-11-04.md`. Deviations: Roulette ignores directions and draws among the unlocked stages of the page; tournament Random list not ported (reopened: ST-23); port RNG; no `dont_freeze_stage` or Settings shortcut; the CSS model does not reload on Model/Kirby Hat/Visibility/Size changes (reopened: MN-20). Handicap icons above 9 moved to Phase 6.
- [x] **Phase 5** (modes): CSS mode headers, announcers, GOAL/Time picker, Tag Team queue drawn in the CSS (`css_reference.md`, `mnremixcssmoderules`); Classic Sonic (`CSONIC_MAIN`, `deviations.md`); yellow team (`css_reference.md`); Tag Team/Last/Tug of War/Stamina (`modes/tagteam.md`, `modes/stamina.md`, `modes/tugofwar.md`; `docs/bugs/remix_tugofwar_asm_deviations_2026-11-05.md`); KOTH and Smashketball visuals (`modes/koth.md`, `modes/smashketball.md`); 1P leftovers (`modes/singleplayer.md`: records, Remix BTT/BTP, Bonus 3, Rest Area, Giga Bowser end); result extra row (`gmRemixGameEndRowsRowTics/RowY`); Training Show Action, Hold to Exit. Deviations: 12CB announces the vanilla voice instead of "Player N Wins" (reopened: MD-4); yellow "WINS!" x = 175.0 as the Remix reads it outside the array (the per-fighter x is MD-14); team card shows one Dedede (one-frame pose; checked against the emulator in Phase 5b lot 6); the Training Custom spawn on respawn does not exist in the asm.

- [x] **Phase 1, rest of I7** (model part image or palette swap, `MODEL_PART_IMAGE_INIT_`; `notes/i7_part_image.md`): the
  Yoshi egg colour when the egg is thrown, laid and at the entry. Done in Phase 5b lot 3 (package F2, with CT-9; evidence in the ledger rows GP-11 `pc_gp11_*` and CT-9 `pc_ct9_*`). Costumes
  beyond `num_costumes` in the match and on the CSS, and the shield and escape egg, are done.
- [x] **Phase 5b** gaps found by the patch audit (2026-11-05, plan approved the same day). The ledger is
  [`patch_audit.md`](patch_audit.md): one ID per item (prefix per group), asm location, decomp function, what the player
  misses, hook site, effort; the per-scope map is [`patch_audit_scopes.md`](patch_audit_scopes.md). Every open ledger item
  (251 at the audit) was placed in the lots below; lot 8 and Phase 6 closed the count. The CPU-only scopes got the rows SH-15 to
  SH-27 and ST-27 on 2026-11-05 (Phase 6 step 1; 263 open items since).
  - **Acceptance per item:** read the asm scope, port under `#if defined(PORT) && defined(REGION_REMIX)`
    (`porting_guide.md`), a unit test in `tests/remix/` when the item is a pure rule, the in-game test of its lot, then its
    ledger state `done (<run name, evidence>)` or `n/a (<reason checked in the asm>)`. A visual item is compared with a
    reference-emulator capture.
  - **Acceptance of the phase:** no open ledger item outside the AI ones (Phase 6), `tools/audit_remix_patches.py` re-run,
    `parity.md` consistent with the ledger.
  - **Duplicate IDs close together:** GP-1 = CT-14 (with the rapid-jab part of FT-38 and FT-43), SH-7 = CT-2, SH-2 = CT-8,
    GP-7 = CT-15, GP-8 = CT-21, FT-1 = CT-18, FT-8 = CT-19, MS-14 = CT-3, MS-15 = ST-20, the double-jump part of FT-28 =
    SH-1.

  - [x] **Lot 0, groundwork** (2026-11-05): 46 symbols exported to `port/remix/remix_symbols.h`
    (`tools/gen_remix_actions.py`), `scene.sh`, the env-gated logs `SSB64_REMIX_FIGHTER_LOG`, `SSB64_BGM_LOG`,
    `SSB64_PORT_MENU_OPEN` and `SSB64_REMIX_AI_LOG`, the Character Data reference lap, ledger corrections.
  - [x] **Lot 1, Character Data screen** (2026-11-05): MN-16 to MN-19 (the crash on the lap to Dedede, the emblems of
    the 18 extra fighters, special actions, jab combo and move names), checked against `build/ref-shots/chardata_lap`;
    `docs/bugs/remix_chardata_dedede_crash_2026-11-05.md`; emblem deviation in `deviations.md` 5.6.
  - [x] **Lot 2, match flow and stages** (ST, IT-8, IT-10, OP-37, MS-13, MS-15, MS-17, MS-26): every match on a Remix
    stage ended with FAILURE instead of GAME SET (ST-1); countdown, pause camera, surfaces, barrels and bonus, stage
    options. Fixed on the way: the VS results screen aborted after a four-Remix-fighter match
    (`docs/bugs/remix_vs_results_status_buffer_2026-11-05.md`). Since Phase 5 the BTT/BTP stages run the bonus ground,
    so they are tested through the 1P path (SKILL.md).
  - [x] **Lot 3, fighters** (GP, SH, FT, CT outside AI): eight packages; roster special matrix 94 x 3 clean; a
    verification batch of 184 runs (`pc_*`, `lldb_count.sh`/`lldb_first.sh`). Deviations in `deviations.md` 5.8.
  - [x] **Lot 4, items and Size** (IT-2 to IT-11). `SSB64_REMIX_FORCE_ITEM` rewrites every item made, container drops
    included; container tests use `SSB64_FORCE_ITEM_KIND`.
  - [x] **Lot 5, CSS, menus and screens** (MN, MD-3, MD-12 to MD-17, MD-20, MD-22, CT-1, CT-13, CT-24, OP-29 to OP-33,
    OP-40, OP-41, MS screens; MS-2 `n/a`, the port never shows the N64 logo). Fixed on the way: the Gallery label read
    the common menu file; the port menu was built before the Remix toggles loaded (now after `remix_toggles_init`;
    `SSB64_PORT_MENU_PAGE` opens a page for screenshots); `syAudioGetBGMPlaying` cleared when a song without a loop
    ended; Giant DK crashed the VS results and Metal Mario and the polygons had empty menu motions
    (`docs/bugs/remix_giant_dk_results_menu_motions_2026-11-05.md`). Results sweep `l5_rs_*` (82 Remix winners) clean.
  - [x] **Lot 6, modes** (MD, OP): 1P game, 1P overlays, bonus stage, 1P intro, All-Star, stock modes and 12CB. Test
    notes: the 1P scene shortcut needs `SSB64_SPGAME_FKIND` (fkind 28 otherwise), and an All-Star run starts from the 1P
    CSS (scene 17) with `SSB64_REMIX_1P_MODE=allstar`.
  - Lots 2 to 6 closed their remaining in-game checks in the 2026-11-05/06 verification batches; the ledger rows carry
    each run (a few scenarios that never trigger in game, such as the Peach BTT pipe ST-10, are closed on unit tests
    with the attempts written in the row).

  - [x] **Lot 7, audio.** Done (2026-11-06): coded by two workers and built (7228 unit checks); every item checked in game:
    MS-3, MS-6, MS-8, MS-22, MS-23, then MS-5 (`l7_hurry_mk`, Mushroom Kingdom stage 8), MS-4 (`l7_trap`), MS-16 and MS-25
    (`l7_crash_burrow`). `SSB64_MIDI_LOG=1` logs the MIDI overrides. A1 BGM: MS-3 with MS-6 (alt and random music by C button and scene, FD theme), MS-4, MS-5 (the
    30-second hurry swap by track). A2 FGM and MIDI: MS-8, MS-22 with MS-23 (per-song priority, bend range and volume
    overrides; hook site confirmed first), MS-16 with MS-25 (Crash DSP voices and the announcer lines). Tests:
    `SSB64_BGM_LOG` with `test_bgm_rules` extended, the FGM log and `wavstat.py`, the audio dump against the reference
    RMS per track; each item added to the T9.1b listening list.
  - [x] **Lot 8, `parity.md` from the ledger:** `tools/gen_remix_parity.py` reads the ledger states, so a file with an open
    item is `partial`; `tools/audit_remix_patches.py` re-run, no unhooked patch without a ledger ID. Done (2026-11-06):
    every ledger item is placed on its `.asm` row (by the asm or scope cell, the table rows that patch a CT table, the
    including file for `css/*.asm`, or the section file); the Port column lists the open IDs, and every `partial` row has
    one. Rows: 287 (fighters 98 ported / 54 partial, items 29 / 4, stages 6 / 1, audio 5 / 1, menus and modes 30 / 14,
    AI 8 / 24, infra 4 done and 9 n/a); the partial rows are the 124 items still open (in-game checks of lots 5 to 7,
    CT-7 on every fighter row, and the Phase 6 items). Audit re-run: 1969 patches (hooked 1171, ambiguous 460, unhooked
    323, symbolic 12, no function 3), the same total as the 2026-11-05 audit; each of the 323 unhooked patches has a
    scope-map line, and each one with a GAP or STUB verdict is covered by a ledger item (the 13 that a name match missed
    sit in the line ranges or grouped scopes of MN-4, SH-1, SH-4, SH-6 and MD-13).
- [x] **Phase 6** AI, run beside Phase 5b lots 1 to 7 (user decision, 2026-11-05). One worker at a time owns
  `ft/ftcomputer.c`; the others work in `ft/ftremix/ftremixai*.c`, `ftcommoncatch1.c`, `ftcommondash.c` and
  `mnremixcpulevel.c`, which no Phase 5b package edits; registry edits (`port/fighter_registry.h`) are merged by the lead.
  **Acceptance:** `REMIX-TODO(AI)` at 0, every AI ledger item closed, recovery at level 9 not below the re-measured
  baseline, KOTH and Smashketball CPUs reach the hill and the ball, the reflect test with CPU Falco and Ness. Reference:
  [`ai.md`](ai.md).
  Steps 1 to 3 (2026-11-05): baseline with `ai_recovery.sh 10 4 14000` (712 recovery episodes, 88.6% ok), two
  coded batches (SH-16 to SH-27, ST-27, FT-1 = CT-18, FT-2 to FT-13, FT-36, FT-52, GP-8 = CT-21, CT-17, CT-20, MD-10,
  MD-11) and a re-measure (88.8%); the self-destruct count (`selfko`) became the acceptance metric. Deviations in
  `deviations.md` 5.9.
  Step 4, acceptance (2026-11-06): `REMIX-TODO(AI)` at 0 (the last five became notes checked in the asm). Every AI ledger
     item is `done` with its run: lldb hit counts in CPU-only matches with no vanilla fighter that takes the same path
     (`p6p_*`, `p6q_*`, `p6r_*`, `p6s_*`; `lldb_count.sh` gained `ret:` probes on the pure rules functions, since line
     breakpoints on a `return` land on merged lines in the Release build), with controls at 0 (level 9 for the level 10
     items, Improved AI off). Recovery at level 9: 88.8% against the 88.6% baseline (step 3). Smashketball (`p6_ball_a`,
     `_b`): the CPUs score and the match ends; KOTH (`p6_koth_a`, `_b`): the crown bars fill and a CPU stands on the hill.
     Reflect: CPU Falco, Wolf, J Ness and Lucas press and hold reflect/absorb (`p6p_reflect`). FT-11 on the VS CSS
     (`p6_css_lv10`, `p6_css_hcap`). Found on the way: CPU Peppy crashed on the down-special detonate check (fixed,
     `docs/bugs/remix_peppy_flashbang_slot_lp64_2026-11-06.md`). Notes: FT-9 (jab auto grab) and SH-25 at level 9 cannot
     fire in Remix either (ledger rows); under lldb, stage 92 stops at launch in objc_msgSend (no lldb: runs).
- [x] **Phase 7** open bugs, instrumentation and packages.
  - [x] Bugs (2026-11-06): status buffers: `sc1pgame.c` and `sc1ptrainingmode.c` already use 256 entries under
    `REGION_REMIX`; the 1P intro (100 entries) shows Remix enemies on stages 1 to 10 of Remix 1P with no "Status Buffer is
    full" (`p7_intro_*`). Not reproduced: Poly Young Link on the VS CSS (D-pad down on Young Link shows the model on the
    first pick, `p7_r2_120_0`, `p7_r2_122_0`), Metal Mario size on the Training CSS (`p7_tr_136_DU`, same size as the CPU
    Mario), the rc 139 flake on Tal Tal (12 four-fighter matches on 0x2E and 0xA7, all rc 0, `p7_taltal_*`). The Switch
    stutter (`walked past registered DL range`, stale token 0x57EE) needs the hardware: moved to T10.2.
  - [x] Peach's Castle: blue quads with the roof-tile texture float in the sky when the camera pans left (2026-11-05,
    `b11_pc_wide`, `b11_us_pc`): not a bug. They are layer-0 objects 3 and 4 of the Castle ground data (roof wedges, DLs
    0x2808 and 0x28F8 of file 106, animated between x = +-3000 and +-3900 near the camera bounds). A reference capture with
    P1 walking left (`build/ref-shots/pc_pan_remix`, frame 008) shows the same wedge in the top-left corner; the earlier
    reference `vs1` never panned that far.
  - [x] Giga Bowser 1.70 zoom on the CSS (2026-11-05): not a bug. The reference capture (`build/ref-shots/giga_css2`, D-pad
    up over Bowser on the VS CSS of the real Remix) shows Giga Bowser overflowing the panel at the port's size
    (`b10_giga_css`).
  - [x] `PEACH_TURNIP_GFX` out of bounds (2026-11-05): not a bug. Dump of file 5144 (`SSB64_DUMP_FILE_ID`): slots 919 to 928
    are the turnip's 10 images; the two outside the file (927 and 928, targets 2482 and 2352 in a file of 1176 words) are
    images 8 and 9, and the turnip only uses images 3 to 6 (`stage_setting_` L181-217). Zeroing those slots, as the bridge
    does, is safe.
  - [x] Instrumentation (2026-11-06): spdlog flushed before the `RelocPointerTable` aborts (`port_log` already flushes
    every write), `scene <id> summary: <frames>, <slow>, max <ms>` at every scene change (`p7_instr`), and
    `lbReloc: slow load file_id <id>` for a load above 20 ms.
  - [x] Test packages with the `.o2r` embedded (macOS script and AppImage), marked for personal use (2026-11-06):
    `dist/desktop/BattleShip-Remix-2.0.1-port.07d89c8-macOS-arm64.dmg/.zip` (the bundled binary reaches a VS match, rc 0)
    and `build-linux-release/BattleShip-Remix-2.0.1-port.ab57e0f-linux-x86_64.AppImage` (podman amd64; the Dockerfile now
    zeroes the AppImage magic of linuxdeploy and appimagetool, which broke binfmt under emulation).
  - [x] Final packages without the `.o2r`: superseded by the releases of `release-remix.yml` (**M12**).
- [x] **Phase 8** final verification. Regression: 142 VS stages (`parallel.sh stages`) plus the 33 bonus-class stages through
  the 1P path, special matrix 95 × 3, item sweep, 4 players in every mode
  through the CSS and Training, the vanilla build, unit tests. User passes: T9.1b (listening and visual), T10.2 (Switch).
  Reference comparison: emulator captures (I8) against the port on every mode screen, CSS, stage select, pause and results;
  each difference is fixed or documented as an adaptation. Close: `parity.md` only `ported` / `adapted` / the approved
  `n/a`, T9.3 (report "Remix playable" with the list of adaptations) and the final checkpoint.
  Evidence (2026-11-06): stages `st_*` 142 rc 0, none blank; bonus path `p8b_<3|7>_<fkind>` 160 runs (fkinds 0..11,
  29..96 on BTT and BTP), 2 shots each, no log error; special matrix `m_*` 285 rc 0 (`m_35_usp` was a Metal flake, clean
  on rerun); item sweep `cpu_46..65` 13 Remix items rc 0, 22 to 42 spawns each, no log error; 4 players through the VS
  CSS (`p8f_default`, `p8f_koth`, `p8f_smashketball`, `p8f_tugofwar`), Tag Team, teams and stamina by the VS shortcut
  (`p8f_tagteam_vs` swaps on a KO, `p8f_teams_vs`, `p8f_stamina_vs` 150 HP; the Tag Team CSS refuses START until every
  queue is full, as in `TagTeam.asm`), 12CB full loop `p8f_12cb_flow` (match, results, back to the CSS with the stock
  taken), Training `p8f_training`; `run_remix_mode_tests` 7236 checks, 0 failed; `build-us` `ssb64` builds. Reference:
  `build/ref-shots/` `css_menu`, `koth`, `tagteam`, `smashketball`, `tugofwar`, `pause`, `stats`, `r12` against
  `p8f_*`, `p8r_vsmenu`, `p8r_pause`: same layout and texts (the reference pause shows the legend opened with R).
  Sound Test: `build/ref-shots/p8_data` against `p8r_sound`, same layout, legend and cursor. Stage select, title, 1P
  intro cards, Character Data and the Giga Bowser CSS were compared in lots 1, 5 and 6. Congratulations:
  `build/ref-shots/p8_scene_37` (reached with a local mupen64plus cheat on the title scene: `D00A4AD0 0001`,
  `810465D2 0001` (`screen_interrupt`), `800A4AD0 0037`) against `p8r_congra`, same picture and text. Credits: the same
  cheat with scene 0x38 presents no frame after the jump (tried 600 s and 1800 s; the only shot is the boot notice), so
  there is no reference capture; the port credits
  (`p8r_credits`) roll without error.
  `parity.md`: 277 `ported`, 2 `adapted`, 10 `n/a`, 0 `partial`.

### M12. Platforms and releases (after M11)

Not part of the Remix port itself; recorded here so the plan matches what ships. Reference: `build_and_roms.md`, `AGENTS.md` ("Releases", "Asset extraction fingerprint", "Logs").

- [x] Releases: a `v*` tag on `feature/remix-port` runs `.github/workflows/release-remix.yml` (macOS `.dmg`, Linux AppImage, Windows zip, Switch zip and bare `.nro`, macOS app zip, Android arm64 APK, PS5 title zip, `SHA256SUMS.txt`) and opens a draft whose body is `.github/release-notes.md`. The inherited upstream workflows are disabled.
- [x] PS5 native title `PPSA99264` (`scripts/ps5/`, `port/ps5/`), with its zip updater.
- [x] Android arm64 APK (`dev.lucasliet.battleship.remix`), with its own boot-time updater (`AppUpdater`, `UpdatePrompt`, a `PackageInstaller` session).
- [x] Updater: start-up popup on desktop and Switch with restart after install; Switch swaps the running `.nro`.
- [x] Switch updater installs the full switch zip: it unpacks into `update-pending/`, swaps the `.nro` and applies the other files at the next boot; `.o2r` and `.recipe` stay manual. The bare `.nro` asset stays for one transitional release.
- [x] macOS updater installs the zipped `.app` in place (new `BattleShip-Remix-macos.zip` release asset; the new app is swapped in on restart).
- [x] Windows updater script fixed for the Remix executable name (`update_game.bat` named `BattleShip.exe`).
- [x] Hardware validation of the Switch zip update and of the macOS in-place update on a real install: validated by the owner (2026-11-15).
- [x] Asset recipe: the Torch revision is pinned in `tools/torch_extract_rev.txt`, so unrelated Torch commits do not force a re-extraction; the Switch error screen tells a missing `.recipe` from a mismatch.
- [x] Logs hold only the last run (`ssb64.log`, `logs/BattleShip-Remix.log`, PS5 `boot.txt`).
- [x] Bug reports go through `.github/ISSUE_TEMPLATE/bug_report.yml`.

### M13. Toggle parity audit against the asm (opened 2026-11-14)

Issue #8 showed that a toggle can be `ported` in the ledger and still differ from the asm in ways no smoke test caught:
Charge Smashes ran its logic on the wrong frame and scaled damage on the wrong motion event, Special Zoom called the
joint animation instead of the material animation and wrote the magnifier byte next to the right one
(`docs/bugs/remix_charge_smash_never_starts_2026-11-14.md`, `docs/bugs/remix_special_zoom_wallpaper_matanim_2026-11-14.md`).
M13 repeats that comparison for every other toggle of `toggles.md`: every asm site that reads the toggle against every
port site, looking for a wrong hook site or call frequency, field, constant, condition, vanilla function or value mapping.
The asm is the reference, except where it is itself wrong: an asm bug, a missing piece or something the port already does better is kept or fixed in the port, never copied, and recorded in `deviations.md` (owner policy, 2026-11-15).

- [x] T13.1 Audit, read-only, one auditor per batch and one skeptic per finding (only findings confirmed on both sides
  count). Batches (registry keys):
  1. `skip_results_screen`, `hold_to_pause`, `css_panel_menu`, `practice_overlay`, `cinematic_entry`, `flash_on_z_cancel`
  2. `model_display`, `special_model`, `advanced_hurtbox`, `hold_to_exit_training`, `improved_combo_meter`,
     `tech_chase_combo_meter`, `combo_meter`, `1v1_combo_meter_swap`
  3. `neutral_spawns`, `salty_runback`, `preserve_salty_song`, `stereo_fix`, `variant_random`, `disable_hud`,
     `full_results`, `skip_start_cheer`
  4. `default_cpu_level`, `puff_sing_anim`, `l_random_char`, `dpad_css_control`, `pk_thunder_reflect_crash_fix`,
     `flash_guard`, `screenshake`, `blastzone_gfx`
  5. `hitstun`, `hitlag`, `di`, `japanese_sounds`, `j_stun_sleep`, `momentum_slide`
  6. `shieldstun`, `z_cancel_opts`, `punish_on_failed_z_cancel`, `tripping`, `rage`, `footstool`
  7. `air_dodge`, `jab_lock`, `ledge_jump`, `perfect_shield`, `parry`, `spot_dodge`
  8. `fast_fall_aerials`, `ledge_trump`, `wall_teching`, `item_containers`, `game_speed`, `blastzone_warp`
  9. `single_button_mode`, `item_dropping`, `move_staling`, `stopwatch_behaviour`, `improved_ai`
  10. `play_music`, `random_music`, `menu_music`, `show_music_title`, `bgm_volume`, `fgm_volume`, `pokemon_voices`,
      `announcer_mode`
  11. `sss_layout`, `hazard_mode`, `whispy_mode`, `saffron_poke_rate`, `dragon_king_hud`, `camera_mode`, `yi_clouds`
- [x] T13.2 Fix the confirmed findings, one commit per batch. Every fix needs an in-game smoke test that shows the asm
  behaviour (before and after); a fix with pure rules also gets a unit test in `tests/remix/`, in addition to the smoke
  test, never instead of it. A significant one gets a `docs/bugs/` write-up. Sign-off: the worker's report is not
  enough; the lead reruns the smoke test of every fix on the build that will be committed (skill `remix-smoke-tests`),
  looks at the screenshots and logs, records the evidence under the batch below, and only then commits.
  Evidence per batch (lead sign-off):
  - Batch 2 (`combo_meter`, `special_model_display`): four level-9 CPUs with Combo Meter on show COMBO+2 to COMBO+8
    above the damage HUD in the attacker's colour. Sudden death (VS shortcut, `SSB64_VS_TIME=1`, game rules set to TIME
    from gdb at the first setup): `ifRemixComboMeterSetup` runs again with `is_suddendeath=1` and the records go from
    4 hits to 0. Smashketball overtime (`SSB64_REMIX_MODE=smashketball`, four CPUs): the second setup has
    `gmRemixSmashketballKeepsMatchStats()=1` and the records stay 2,0,2,2. Training (scene 54, D-pad Down, exit to the
    Training CSS and back): hitbox models in the first Training, normal models in the second, and the cfg file only
    changed the window position.
  - Batch 3 (`variant_random`, `disable_hud`, `stereo_fix`): menu rolls draw like `Global.get_random_int_safe_` (game
    RNG advanced 1 to 64 times by the frame counter). 12CB Custom D-pad Down (Custom set forced from gdb): DEFAULT only
    grid fighters, INCLUDE BONUS adds bonus ids (66, 74, 75, 76...), VANILLA only ids up to Ness; the same presses 7
    frames later give other fighters. Tag Team Random queue slots (stocks set to 5 from gdb): VANILLA only ids up to
    Ness, INCLUDE BONUS bonus and variant ids, and two press timings give different queues. Positional sounds: Home-Run
    Contest 7 of 7 and Sector Z Remix 6 of 6 with pan 0x80 (the sound's own pan), Dream Land positional (0x31 to 0x45).
    Training pause shows "Press Z for Custom Menu" with Disable HUD PAUSE and ALL. All runs rc=0, screenshots read.
  - Batch 4 (`puff_sing_anim`, `l_random_char`, `dpad_css_control`): Jigglypuff's Sing (gdb on the effect update) has
    the matrices [Tra, 0x2E] on 176 frames with the ring centred on her; the scale takes 130 values with the toggle on
    and stays 1.0 with it off. VS CSS with L random: after picking Mario, three L presses give Ness, Kirby and Crash;
    a CPU panel keeps its fighter (fkind 73 on all three samples). D-pad cursor while P1 carries P2's CPU token: the
    cursor leaves x=78 only 30 frames after D-pad Right (asm delay), and the token moves from Ness to Kirby. 1P, Bonus
    and Training CSS (gdb on each cursor update, D-pad Right held from the same frame): with the own token held over a
    portrait the cursor first moves 29 updates later than with the token placed (228 against 199 on all three screens).
    All runs rc=0, screenshots read.
  - Batch 5 (`japanese_sounds`): gdb printed the id passed to `ftRemixCommandPlayHitFGM` for five jab and tilt hits.
    Mario with the toggle at 0 sends 0x28/0x20, at 1 sends 0x93/0x8F; J Mario at 0 and 1 sends 0x93/0x92/0x8F, at 2
    sends 0x28/0x26/0x20, as in `Character.asm` `apply_sound_type_`.
  - Batch 6 (`footstool`, `z_cancel`): the footstool with no jump left plays FGM 1200 twice and puts P2 in status 32
    twice (0 with the toggle off). A level-10 CPU with Z-Cancel DISABLED lands its aerials in LandingAirNull (219) four
    times with four laugh FGMs (1316); with Z-Cancel at DEFAULT no laugh and normal landings.
  - Batch 7 (`perfect_shield`, `parry`): the perfect shield check now runs only for hop-capable airborne projectiles
    and items, as the asm patch site. gdb counts with P1 cycling the shield: Mario's fireballs 5 checks and 4 reflects;
    Pikachu's Thunder Jolt (no hop) 0 checks and 0 reflects; forced Green Shells 7 checks and 0 reflects, Eggs 4
    checks, Bob-ombs 0 checks. The direct-hit parry (kept per hitbox, `deviations.md` 5.3) played its sound 12 times
    against jabs. All runs rc=0, screenshots read.
  - Batch 8 (`fast_fall_aerials`, `item_containers`): Mario's nair with stick down lands at battle tic 64 with the
    toggle on and 81 with it off (frame 460: on the ground against still in the air). Crates with ALWAYS EXPLODE play
    0x3B 21 times and 0x3C never (DEFAULT: 25 and 23).
  - Batch 9 (`improved_ai`): four level-9 CPUs, 3 stocks, 14000 frames, gdb hit counts on the committed build.
    `fix_target_pos_x` now runs before the ground/air split: with Improved AI on, Flat Zone (50) made 2850 grounded
    calls of `ftRemixAiRecoveryClampWalkTargetX`, 80 of them with the target past the bounds minus 200 (clamped);
    Onett (73) 4899 grounded calls, none out of bounds (before the fix the call was air-only, 0 grounded calls).
    J Pikachu and E Pikachu (with Mario and Fox, Dream Land) now reach the vanilla Pikachu gate: 3568 calls of
    `ftRemixAiRecoveryKeepsPikachuVanilla` with Improved AI off, all on the vanilla side; 3106 with it on, none on
    the vanilla side (before, only fkind 9 reached it). Results screens normal, no abort or assert in the logs. The
    PK Thunder ledge table indexing (`port * 4` in the asm) is an asm bug, kept per port (`deviations.md` 5.9).
  - Batch 10 (`play_music`, `menu_music`, `bgm_volume`, `music_title`): toggles set at run time from gdb on the VS
    mode menu. Play Music OFF then closing the settings stops the kept track (71 to -1); with music off then on and a
    close, or A on BGM Volume, the menu track restarts (-1 to 71); in a match neither starts a menu track. RANDOM and
    RANDOM ALL with the 64 menu track playing: the VS CSS plays its own track 10 and the VS menu 44 again; the 1P,
    Training and both Bonus CSS play 10. Back from the Data screens RANDOM, RANDOM CLASSICS and RANDOM ALL roll a new
    track (262, 283, 334). SM64 trap: 0 of 150 RANDOM presses after three direct SM64 presses land on the trap
    (8 of 150 before). Music title at the 1P countdown: Meta Crystal and Duel Zone now show their banner until GO.
    All runs rc=0, screenshots read.
  - Batch 11 (`camera_mode`, `hazard_mode`, `dragon_king_hud`): 1P with Camera Mode SCENE, switched to NORMAL from gdb
    at the second camera: stages 2 and 3 get the vanilla routine table back (Default, Inishie, Zebes) and the default
    routine, so the SCENE table no longer leaks into the next camera. VS on Planet Zebes (3) and Mushroom Kingdom (8):
    FIXED gives a still full-stage view and SCENE the wide anim view, against the tracking camera with NORMAL. Rainbow
    Road (163): the Chain Chomp exists with Hazard Mode 0 and not with 1 or 2 (Hazards.asm tests the whole value). A
    VS match left on Dragon King (scene gkind 0x35 set from gdb) played on Dream Land puts the combo meter at y 167,
    above the damage HUD, since the battle-state gkind decides. Hazard Mode outside VS and Training and the Zebes acid
    AI keep the port's screen gate (`deviations.md` 5.7 and 5.9). All runs rc=0, screenshots read.
  - Follow-up checks on the final build:
    - RANDOM token (0x1B set from gdb before START) on the VS, 1P and Training CSS: VANILLA gives ids up to Ness (10,
      4, 10, 7, 11, 10), INCLUDE BONUS gives bonus ids (65), and a 7-frame later START changes the fighter.
    - Combo meter in 1P: set up once, its update ran 2636 times.
    - BGM Volume through its menu widget (`SSB64_PORT_MENU_SET="BGM Volume=60@400"` on the VS mode menu, music
      off at start): with Play Music turned ON from gdb the widget callback restarted track 71; with it still OFF
      nothing played.
    - Settings close through `PortMenu::UpdateElement` (menu opened with `SSB64_PORT_MENU_OPEN`, hidden from gdb):
      the hook ran, Play Music OFF stopped track 71, and Play Music ON restarted it from silence.
    - Camera BONUS on Planet Zebes: the default and Zebes slots hold the Mushroom Kingdom routine, which the camera
      runs.
    - Controls: with Perfect Shielding off, the check returns at entry and fireballs are never reflected. A level-9
      CPU with Improved AI and Z-Cancel DISABLED lands in 219 twice and 215 once, with 3 laughs.
    - Regression: stage sweep (0 to 16 and 41 to 215, 159 stages, no crash, no blank screen), special matrix of 24
      fighters (72 runs), CPU item sweep (5 kinds); all rc=0 with no abort or assert. Ids 17 to 40 are the bonus
      stages, which the VS shortcut cannot load.
- [x] T13.3 Update `toggles.md`, `patch_audit_scopes.md` and `deviations.md` for what changed, and the release notes.
  Done with each batch commit; the release notes have the M13 highlights (Combo Meter, settings checked against Smash
  Remix).

### M14. Equivalence audit of the non-toggle logic (opened 2026-11-15)

M11 checked that every asm patch has a port hook (presence); M13 checked that the hook does what the asm does, but only
for the settings toggles. The rest of the hand-ported logic (fighter specials, stage hazards, modes, items, shared
gameplay patches) never had that equivalence check. A read-only pilot on 2026-11-15 (one auditor per area, one skeptic
per finding, the same method as T13.1) sampled six areas and confirmed 23 differences:

| Area sampled | Routines equal to the asm | Confirmed differences |
|---|---|---|
| Marth (`MarthSpecial.asm`, `Marth.asm` logic) | 27 | 0 |
| Sonic and Super Sonic (`SonicSpecial.asm`, `SSonic.asm`, `Sonic.asm` logic) | 56 | 2 |
| Hazards of Pirate Land, Rainbow Road, Metallic Madness, Spawned Fear (`Hazards.asm`) | 19 | 10 |
| King of the Hill and Smashketball (`KingOfTheHill.asm`, `Smashketball.asm`) | 24 | 8 (4 more already documented) |
| Items Blue Shell, Stopwatch, Waddle Dee, Boomerang | 38 | 2 |
| `Damage.asm` and the shared fighter files | 40 | 1 |

Fighters show few differences; hazards, modes and items about one per two or three routines, the M13 rate. Acceptance
of every fix as in M13 (T13.2): an in-game smoke test that shows the asm behaviour before and after, rerun by the lead
on the build that will be committed; a unit test as well when the fix is a pure rule; a `deviations.md` row when the
asm is the one that is wrong (owner policy, 2026-11-15). One commit per lot.

- [ ] T14.1 Fix the pilot findings (asm paths under `.toolchain/pkgs/smashremix-src/src/`; line numbers of 2026-11-15).
  - Lot A, hazards:
    - [ ] A1 Pirate Land cannon DObjs get no transform XObj (asm 0x1C TraRotRpyRSca): the barrel never moves to
      -128/0/+128, the 1.125 fire pulse and the cannon animation are missing, the parts sit at the parent origin.
      `Hazards.asm:3770-3780` / `grremixpiratecannon.c:81,299-300`.
    - [ ] A2 Pirate Land: no muzzle effect (`0x80100480`, white sparkle) when a cannonball spawns.
      `Hazards.asm:4179-4183` / `grremixpiratecannonball.c:84-105`.
    - [ ] A3 Pirate Land: the cannonball's second transform is 0x48 (billboard with scale) in the asm, 0x47 in the port.
      `Hazards.asm:4326` / `grremixpiratecannonball.c:48-51`.
    - [ ] A4 Rainbow Road: the Chain Chomp has no per-frame `gcPlayAnimAll` process, so it stays on its first animation
      frame at the spawn point instead of charging along the track. `Hazards.asm:5737-5743` /
      `grremixrainbow.c:75-85,131-138`.
    - [ ] A5 Rainbow Road: the Chomp draws on display link 6 with the z-buffer; the asm uses link 0x11 with
      `grDisplayLayer0PriProcDisplay` (no z-buffer, drawn over the track). `Hazards.asm:5683,5707-5715` /
      `grremixrainbow.c:148`.
    - [ ] A6 Rainbow Road start sequence: the asm plays 0x403 five times and 0x404 at 0xAC8 (delay-slot decrement);
      the port plays four and 0x404 at 0xA8C, so one beep is missing and the cycle is 60 frames short (fix `hazards.md`
      too). `Hazards.asm:5953-5965` / `grremixrainbow.c:28-29,106-118`.
    - [ ] A7 Rainbow Road: the Chomp growl window reads GObj+0x0E (`frame_draw_last`, a global 8-bit counter), so it
      reopens every 256 frames for the Chomp's whole life; the port opens it only in its first 15 frames.
      `Hazards.asm:6081-6095` / `grremixrainbowchomp.c:61-75`.
    - [ ] A8 Metallic Madness: `shrink_timer_` clears the laser shrink when the player has an active Super or Poison
      Mushroom; the port lets the 900-tic timer run and then overwrites the mushroom size.
      `Hazards.asm:4746-4760` / `grremixmetallic.c:216-224`.
    - [ ] A9 Spawned Fear: the Cacodemon factory timer goes back to 0 (delay slot) even when a Cacodemon is alive; the
      port keeps counting, so a new one can appear right after the last dies, and a forced frame 3000 with one alive is
      never retried. `Hazards.asm:8641-8646` / `grremixcacodemonfactory.c:63-68`.
    - [ ] A10 Spawned Fear: the Thunderball lacks the 0x47 billboard transform and the `wpMainVelSetModelPitch` call at
      spawn. `Hazards.asm:9235,9032-9039` / `grremixthunderball.c:70-74,153-171`.
  - Lot B, modes:
    - [ ] B1 KOTH: every frame with no king (empty or contested hill) resets the 3.5 s win countdown of every team; the
      port freezes it. `KingOfTheHill.asm:1292-1305,1375-1381` / `gmremixkothking.c:123-127`.
    - [ ] B2 KOTH: the meter frame sprite uses the `Color.damage_icon` tints (FFA0A0, B4B4FF, E6E678, A0FFA0, DCDCDC),
      not the darker bar colours. `KingOfTheHill.asm:1555-1558,1596,1623-1632` / `gmremixkothhud.c:81-83`.
    - [ ] B3 KOTH: the hill re-pick retries invalid lines forever and only counts repeats of the current hill (4
      draws); the port spends one of 3 draws on every invalid line and keeps the old hill.
      `KingOfTheHill.asm:974-990,1005-1037` / `gmremixkoth.c:134-141`, `gmremixkothhill.c:92-100`.
    - [ ] B4 KOTH hidden hill: the asm moves the hill as soon as the hidden marker leaves the blast zone, else on the
      121st hidden frame, and resets `hidden_frames` only there; the port skips the blast-zone check, moves one frame
      early and resets on every change. `KingOfTheHill.asm:1127-1148` / `gmremixkoth.c:164-168`.
    - [ ] B5 KOTH: each full bar has its own flash starting at alpha 0; the port runs one shared phase from match start.
      `KingOfTheHill.asm:1459-1473` / `gmremixkothhud.c:87-95,102,116`.
    - [ ] B6 Smashketball streak chant: the asm passes `FTStruct.player_num`, the port the port index (wrong or no
      chant when the ports used are not contiguous). `Smashketball.asm:897-899` /
      `gmremixsmashketballscore.c:67,75`.
    - [ ] B7 Smashketball forced chant keeps the same-player check: one chant per streak, not one per basket from the
      third on. `Smashketball.asm:1136-1142` / `ftpublic.c:100-107`.
    - [ ] B8 Smashketball CPU ball seek reads the item DObj translate; the port uses the owner's hand position while
      the ball is held (check in game first; low confidence). `Smashketball.asm:1175-1179` /
      `ftremixaimodes.c:37,60`.
  - Lot C, items, fighters and shared patches:
    - [ ] C1 Blue Shell: the rolling shell refreshes the first-place fighter every 8 frames and runs the red shell
      follow routine (`0x8017A3A0`) toward it; the port keeps the throw velocity. `items/BlueShell.asm:873-876,
      1349-1429` / `itremixblueshell.c:437-453,342-352`.
    - [ ] C2 Blue Shell: when the interact counter runs out while rolling, the asm keeps the shell (its destroy branch
      falls into `_end`, which clears v0); the port destroys it. Likely an asm bug: keep the port and add a
      `deviations.md` row, after confirming. `items/BlueShell.asm:1174-1182,1238-1239` /
      `itremixblueshell.c:480-484`.
    - [ ] C3 Sonic spring: the asm turns only Tumble into JumpF (`sltu` against FallSpecial); the port's inclusive range
      also frees a helpless (FallSpecial) fighter. `Sonic/SonicSpecial.asm:1747-1752` / `ftremixsonicusp.c:311-314,
      447-450`.
    - [ ] C4 Super Sonic `end_invisibility_` only toggles the invisibility bit; the port also writes the moveset env
      colour, which hides the player tag for a few frames at the end of the teleport (same pattern in Sheik and Mewtwo:
      check them too). `Sonic/SonicSpecial.asm:3456-3467` / `ftremixssonicusp.c:68-72`.
    - [ ] C5 Bury timer: the asm rounds 0.7 × percent to nearest (`cvt.w.s`), the port truncates (one frame shorter
      about half the time). `Damage.asm:385-397,434-446` / `ftremixbury.c:52-57`.
- [ ] T14.2 Audit the rest, read-only, same method as the pilot (only findings confirmed by the skeptic count), one
  batch per area: the hazards of every other stage (`hazards.md`); the other modes (Tag Team, Stamina, Tug of War, 12CB,
  Training, the 1P modes: Remix 1P, Multi-Man, Cruel, HRC, Bonus 3, All-Star); the other 29 items; the other fighters
  (specials and fighter-specific routines); the remaining shared files that are not toggles. Each batch adds its findings
  as items under T14.3.
- [ ] T14.3 Fix the T14.2 findings, one lot per batch, with the T14.1 acceptance.
- [ ] T14.4 Update `hazards.md`, the mode docs, `deviations.md`, `patch_audit_scopes.md` and the release notes for what
  changed.

---

## Progress estimate (update at every checkpoint)

From the patch audit (2026-11-05): share of the Remix patches that are ported, adapted or `n/a` with a checked reason, per
area (patch counts of the ledger groups; an S-effort item weighs as much as an L one, so this undercounts the work done).

| Area | Weight | Done |
|---|---|---|
| Fighters (roster, specials, projectiles, entries, shared patches, per-fighter tables) | 35% | 100% (every GP, SH, FT, CT item `done` or `n/a`) |
| Items and Size | 10% | 100% (IT) |
| Stages (loader, select, hazards, camera, surfaces) | 20% | 100% (ST) |
| Audio | 10% | 100% (MS audio scopes; listening pass T9.1b done) |
| Settings, menus, modes | 20% | 100% (MN, MD, OP, MS) |
| Final verification | 5% | 100% (Phase 8, T9.3, M13, T9.1b and T10.2 done) |
| **Weighted total** | | **100%** (2026-11-15: the owner's passes closed the last 1%; the 2026-11-05 audit estimate was ~72%) |
