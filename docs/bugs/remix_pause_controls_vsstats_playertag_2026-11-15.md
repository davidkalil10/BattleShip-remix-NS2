# Pause controls outside the default pause, Match Stats details, custom tag name, team costumes from the VS menu (2026-11-15)

Found by the M14 equivalence audit (lot T14.3-s3, screens).

## Symptoms

1. In a pause where the camera controls are off (the fighter out of the zoom range or KO'd, a Bonus stage, Camera Mode Fixed),
   the D-pad song change, the R legend and the D-pad Left HUD toggle did nothing. In a 12-Character Battle the Gentlemen's Reset
   request text was not drawn in such a pause either.
2. On Time Twister, a song picked in the pause was replaced by the song of the next background at the following warp.
3. The Match Stats pages had no white line under the 1P..4P column headers.
4. The custom Player Tag name stayed fully opaque over a cloaked or invisible fighter, whose arrow fades.
5. "Highest Taken" ignored damage that does not go through `ftParamUpdateDamage`: the initial damage of the panel, poison.
6. The striped rows of the Match Stats started the same way on every results screen; in the Remix they alternate from one screen
   to the next when the row count of a screen is odd (always with the Combo Meter on).
7. Leaving the VS menu for Smashketball (or King of the Hill, Tug of War or the default VS with teams on) opened the CSS with the
   fighters already chosen in their free-for-all costume on a team panel.
8. The custom Player Tag name had no black copy under it.

## Root causes

1. `Pause.asm` `extra_controls_` hooks 0x80114320, before the pause kind test, and its camera-allowed tests are commented out
   (:174, :244); only the camera legend group 0x10 depends on the kind (:351-354). The port called `gmRemixBgmPauseUpdate`,
   `ifRemixPauseLegendUpdate` and `ifRemixGentlemensResetUpdate` inside `if (kind == nIFPauseKindDefault)`.
2. `Pause.asm`:562-563 clears `Hazards.time_twister_bgm_change` on every manual pick, and `Hazards.asm`:7932/8005 skip the swap
   when it is 0. The port never cleared its flag.
3. `VsStats.asm`:633-643 (`draw_port_headers_`) draws a 26x1 rectangle at Y 26 for each active port.
4. `PlayerTag.asm`:287,296 (249,258 on the results) copies the alpha of the indicator image to every character each frame; the
   port passed a fixed 0xFF.
5. `VsStats.asm`:528-538 samples `Global.vs.pN + 0x4C` (`stock_damage_all`) in `run_collect_`, every frame of the match.
6. `VsStats.asm`:30-31 `stripe_on` is a global byte flipped by `draw_line_` and never reset.
7. `VsRemixMenu.asm`:438-467 (`save_global_settings_`) writes Stock Team into the rule value and calls
   `mnVSModeSetCostumesAndShades` when the match will be in teams.
8. `PlayerTag.asm`:112-161 builds the name twice: a black copy (X offset 7 against 8, Y offset -1.65, character scale 1.25),
   then the coloured one.

## Fixes

1. `ifcommon.c` `ifCommonBattlePauseUpdateInterface`: the three calls run before the kind test; `ifRemixPauseLegendUpdate`
   takes `is_camera_control` and keeps the camera legend hidden outside the default pause.
2. `grremixtimetwister.c` `grRemixTimeTwisterCancelBgmChange`, called by `gmRemixBgmPauseUpdate` on every pick.
3. `mnremixvsstatspages.c` `mnRemixVSStatsPagesDrawPortHeader` puts the rectangle.
4. `ifcommon.c` and `mnvsresults.c` pass the sprite alpha; `ifRemixPlayerTagDrawName` uses it for both layers.
5. `mnremixvsstats.c` `mnRemixVSStatsCollectMidmatch`, called every frame by `scVSBattleFuncUpdate`; the call in
   `ftParamUpdateDamage` is gone.
6. `mnremixvsstatspages.c` `mnRemixVSStatsPagesStartScreen` (from `mnRemixVSStatsViewSetup`) carries the stripe state between
   results screens; rules `mnRemixVSStatsRulesPageOneRows`, `mnRemixVSStatsRulesScreenRows`,
   `mnRemixVSStatsRulesStripeAfterRows` (unit tests in `tests/remix/test_vsstats_rules.c`).
7. `mnvsmode.c` `mnVSModeSyncTeamCostumes` after `mnVSModeSaveSettings` on VS Start, VS Options and a Remix mode; rule
   `mnRemixVSModeExitSyncsTeamCostumes` (unit tests in `tests/remix/test_css_vsmodes_rules.c`), teams from
   `gmRemixModeGetTeams`.
8. `remix_hud_text_put_fit_glyph_scale` (glyphs enlarged in place) and the black copy in `ifRemixPlayerTagDrawName`, on text
   slots 5..8 under the names on 9..12. The name is hidden while the Match Stats page is shown (see `docs/remix/deviations.md`).

## Verification

- VS on Dream Land with Camera Mode Fixed, and Break the Targets through the 1P path: pause, R shows the music legend (no
  camera rows), D-pad Down changes the song (`[bgm] play` in the log), D-pad Left hides the pause HUD. Before: none of the three.
- Time Twister: pause after the first warp, D-pad Right: no `[bgm] play` at the second warp (before: track 359 at tic 5005).
- VS with `SSB64_REMIX_PANEL="Damage.2=120"`: "Highest Taken" of 2P is 120 (before: 0); the headers have their underline.
- Two matches in one run: page 1 of the second results screen starts with the 1P row striped (before: same rows as the first).
- VS CSS picks, back to the menu, Remix Modes > Smashketball: Fox on the red team wears its red team costume (before: default).
- Custom tag names in a match and on the results: black copy under the name; with Visibility Cloaked the name is invisible like
  the arrow (before: opaque).
