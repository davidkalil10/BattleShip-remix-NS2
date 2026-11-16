# Stamina in Tag Team and Tug of War, Tag Team costumes and stolen stocks (2026-11-15)

Found by the M14 equivalence audit (lot T14.3-m1, VS modes).

## Symptoms

1. With the VS menu Rule left on Stamina, picking Tag Team or Tug of War on the Remix Modes page opened a CSS with the H.P.
   picker ("STOCK 200") instead of the stock count. In the match every fighter had one stock, the HUD showed HP, knockback was
   fixed at 20% and a fighter was KO'd when its HP ran out. In the Remix both modes are plain Timed Stock matches.
2. In a Tag Team team battle, the first fighter of a green-team port wore the yellow team costume and a yellow-team port the green
   one (red and blue were right).
3. In a Tag Team team battle, taking a stock from a teammate on Stock Mode Manual with fewer stocks than the match gave the thief a
   fighter past that teammate's CSS queue (a port-menu slot or Random), not the teammate's last queued character.

## Root causes

1. `TagTeam.asm` 380-385 and `TugOfWar.asm` 59-63 (`before_css_setup_`) write `Global.vs.game_mode = 3`, and every Stamina hook
   tests `game_mode == 5`; `VsRemixMenu.asm` restores the menu value when the CSS closes. The port encodes Stamina as
   `gGMRemixStaminaRule` plus Timed Stock, and `gmRemixModeBeforeCSS` forced the rule to Timed Stock without clearing the flag,
   which `mnVSModeSaveSettings` had set from the menu Rule.
2. `gmRemixTagTeamApplyCssFirstSlot` passed `players[].color` to `ftParamGetCostumeTeamID`, which takes a team id. In a team
   battle the colour is the team's port colour (`mnRemixTeamsGetColor`: green 3, yellow 2), so green and yellow swapped.
3. `gmRemixTagTeamStealStock` took the slot `match stocks - stolen count`. The asm (`TagTeam.asm` 1765-1773) uses
   `current_slot - stolen count`, and `current_slot` ends the CSS at the per-port stock count of `css.get_stock_count_`, which is
   the Manual count when it is below the setting.

## Fixes

1. `gmremixmodecss.c`: `gmRemixModeBeforeCSS` clears `gGMRemixStaminaRule`; the vanilla settings saved before a mode keep the flag
   and `gmRemixModeLeaveCSS` restores it, so the VS menu shows Stamina again on the way back.
2. `gmremixtagteam.c`: slot 0 takes the team costume of `players[].team`.
3. `gmremixtagteam.c` / `gmremixtagteamrules.c`: the stolen slot is the victim's initial stock count plus the Stock Mode Last skip,
   minus the stolen count (`gmRemixTagTeamRulesStealSlot`, unit tests in `tests/remix/test_tagteam_rules.c`).

## Verification

- VS menu, Rule Stamina, Remix Modes > Tag Team and > Tug of War: the CSS shows the stock picker; a four-player Tug of War match
  shows stock icons and 0%; going back from the CSS shows Rule Stamina again.
- Tag Team team battle with ports 3 and 4 on green and yellow (CSS team costumes 2 and 5): the match keeps 2 and 5 (before: 5 and 2).
- Tag Team, `SSB64_VS_TEAMS=1`, 4 stocks, P1 Manual 1 stock, P2 Manual 2 stocks: P1 steals slot 1 of P2 (before: slot 3).
