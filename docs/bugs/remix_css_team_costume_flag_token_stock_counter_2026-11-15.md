# Character select: team costume, J/E flag, token hit test and the Stock Mode counter (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, lot s2, items S2-1 to S2-4).

## Symptoms

- VS team battle: C right or C left on a selected fighter cycled its costume, so a red-team Mario stopped being red and the match
  started with that costume. A Sonic of a team other than yellow changed costume too.
- 1P, Training and Bonus character select: the J or E flag of a selected regional fighter was 8 units left of its Remix position.
- Every character select: a held token picked the portrait (and the bonus and Random bookends) 3 units early to the left and up,
  and 3 units late to the right and down. A token at x 26 to 28 already showed the first portrait of the row, and the last row
  stopped answering 3 units above its bottom edge.
- 12-Character Battle character select: the same test was 13 units off in x and 12 in y. A token drawn over the right half of
  a portrait showed the fighter of the next column, a token above the grid picked the first row, and the bottom 12 units of the
  last row and the right 13 units of the last column picked nothing.
- VS character select with the panel option Stock on Last or Manual: nothing under the panel. The Remix shows the stock icon, an
  "x" and the stock count there, and in Manual two blinking arrows that change the count with A.
- Stock on Manual with a count never edited gave 4 stocks whatever the rules said; the Remix uses the stock count of the rules.
- King of the Hill and Smashketball kept the Stock option of the panels; the Remix sets it back to Default when their character
  select opens.
- 12-Character Battle: the stock icon of the "icon x N" indicator was never drawn (only "x N").

## Root causes

- `Costumes.asm` `select_` 879-885: in a VS team battle the delay slot of `bne at, a2, _store_costume` (the Sonic check) runs
  `lw v0, 0x004C(s0)` for every fighter, so the costume written back is the current one; only the shade is stored, and Sonic's
  C left/right flips the Classic model (yellow team: costume 5 or 1, 888-897). `mnPlayersVSUpdateCostume` wrote the cycled
  costume.
- `CharacterSelect.asm` 5384-5398 (`update_variant_indicators_`, `_draw_flag`): `bne t5, t6, pc() + 0x10` skips the `-8` when
  the player struct is not the VS one. `mnRemixCssMakeFlag` added `-8` on every screen.
- `get_character_id_` (488-493) replaces the two `mfc1` of the vanilla routine and returns from it, so the vanilla `+13 / +12`
  never run: the asm tests the raw token position against `START_X - TOKEN_OFFSET_X` = 29 and `START_Y - TOKEN_OFFSET_Y` = 35.
  The port kept `+13 / +12` against rectangles that start at 39 and 44, which is `x - 26` and `y - 32`.
- The 12-Character Battle branch of the same routine (509-521) adds `START - TOKEN_OFFSET` back before
  `TwelveCharBattle.get_character_id_`, which compares the raw position with `LEFT_GRID_START_X` 19, `RIGHT_GRID_START_X` 155
  and `START_Y` 25 (1024-1064). `mnRemixTwelveCBLayoutSlotAtPoint` has those constants, and its two callers passed `+13 / +12`.
- `setup_` (4771-4794) registers `update_stock_icon_and_count_indicators_` on the VS screen too; its VS branch (5625-5652) reads
  `stock_count_table` or `previous_stock_count_table`. The port ran the indicator only when the 12-Character Battle was active
  and left the Manual count to the port menu.
- `StockMode.asm` 17-18 starts `stock_count_table` at -1; the CSS (5632-5637) and `set_initial_stock_count_` read that as the
  count of the rules. The port option `StockCount` had the range 0..99 and the default 3.
- `KingOfTheHill.asm` 220-224 and `Smashketball.asm` 164-168 (`before_css_setup_`) clear `stockmode_table`. The port had no
  such reset, so the new counter showed up in those two modes.
- The stock sprite of a fighter file is stored hidden and needs the port's sprite fixup; the 12CB indicator copied it raw, so
  it stayed hidden (and drew as a black box with the hidden bit cleared).

## Fixes

- `mnRemixCssCostumeAfterButton` (`mnremixcssrules.c`): the costume after a C button is the current one in a team battle.
  `mnPlayersVSUpdateCostume` applies it before the Classic Sonic toggle.
- `mnRemixCssFlagOffsetX` (`mnremixcssrules.c`): -8 on the VS screen, 0 elsewhere; `mnRemixCssIndicatorsSetLayout` (called by
  the 1P, Training and Bonus screens) marks the screen as not VS.
- `MNREMIX_CSS_TOKEN_HIT_OFFSET_X/Y` (10, 9, `mnremixcsstokenrules.h`) replace the `+13 / +12` at every token hit test of
  `mnplayersvs.c`, `mnplayers1pgame.c`, `mnplayers1ptraining.c` and `mnplayers1pbonus.c`.
- 12-Character Battle: `mnPlayersVSTwelveCBGetHoveredSlot` passes the raw token position, and
  `mnPlayersVSGetTwelveCBPuckFighterKind` goes through it.
- `mnPlayersVSStockModeFrameUpdate` / `mnPlayersVSUpdateStockModeIndicator` (`mnplayersvs.c`) draw the indicator under each of
  the four VS panels in Last or Manual with the code of the 12CB one (`mnPlayersVSDrawStockIndicator`);
  `mnPlayersVSCheckStockModeArrowPress` steps the Manual count on a press or a held A. Pure rules
  `gmRemixStockModeRulesCssCount` and `gmRemixStockModeRulesStepManualCount` (`gmremixstockmoderules.c`).
  `mnremixtwelvecbstockmanual.c` now serves four panels and its blink advances once per frame (`mnRemixTwelveCBStockManualTick`).
- `StockCount` goes from -1 to 99 with the default -1 (`remix_player_panel.cpp`); `gmRemixStockModeRulesManualCount` reads a
  negative count as the match count. The CSS writes the count back once it shows it, as the asm does.
- `gmRemixStockModeClearModes` (`gmremixstockmode.c`), called by `gmRemixModeBeforeCSS` for King of the Hill and Smashketball,
  sets the Stock option of the four ports to Default.
- `mnPlayersVSTwelveCBPlaceStockIcon` prepares the sprite with `ifCommonPlayerStockPrepareSprite`, as the battle HUD does.

## Tests

Unit: `test_css_rules.c` (team costume, flag offset), `test_css_token_rules.c` (bookend edges at the raw asm positions),
`test_stockmode_rules.c` (CSS count, Manual step and wrap, unset Manual count), `test_twelvecb_layout.c` (hit test extents).

In game (VS CSS scene 16, 1P 17, Training 18, Bonus 19): a team battle with C right, C right, C left, C up keeps the red Mario
and starts the match with it, Sonic only switches model; the flag of a J fighter on the four screens; a held token at raw x 28,
at y 33 and at y 105; `SSB64_REMIX_PANEL="StockMode.1=2;StockMode.2=1"` shows "icon x 4" under both panels, the arrows blink on
P1, two presses of the right arrow give 1 then 2 and the match starts with 2 stocks for P1. With `SSB64_CSS_STOCKS=9` and
`StockMode.1=2` the panel shows "x 10", the right arrow gives 1 and an untouched count starts the match with 10 stocks.
`SSB64_REMIX_MODE=koth` and `smashketball` with `StockMode.1=2` show no counter. 12-Character Battle
(`SSB64_REMIX_MODE=12cb`): a held token of P1 at raw 43,40 shows Luigi (Mario before), at 11,40 nothing (Luigi before), at
51,104 Fox (nothing before), at 51,48 Mario (Captain Falcon before), at 51,20 nothing (Mario before); P2 at 263,40 shows Link
(nothing before).
