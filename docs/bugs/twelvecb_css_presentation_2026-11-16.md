# 12CB CSS presentation (M14 T14.6 lot g04)

Source: `TwelveCharBattle.asm` `leave_css_setup_`, `update_character_set_`, `portrait_change_`, `update_variant_indicators_`, `handle_custom_presses_`.

- **Leaving the CSS with B reset the battle.** `gmRemixModeLeaveCSS` called `gmRemixTwelveCBReset`; the asm keeps the config until RESET. Fixed: the call is gone, re-entering shows the defeated portraits, Best Character and RESET again.
- **Set arrows kept the old fighter.** The panel fighter was not looked up again in the new set. Fixed in `mnPlayersVSTwelveCBRefreshFighterAfterSetChange`.
- **Same character, other portrait.** Moving the token between two portraits of one character did not refresh the panel (defeated pose, stocks). Fixed with `mnPlayersVSTwelveCBCheckPortraitChanged` and a pose-change rebuild in `mnPlayersVSUpdateFighter`.
- **J and E flag of the panel** was never drawn in 12CB (the indicator frame update returned early). Fixed in `mnPlayersVSRemixTwelveCBFlagFrameUpdate`; the 12CB CSS entry now also calls `mnRemixCssIndicatorsReset` (a stale GObj crashed the second CSS visit).
- **Set arrow zones** now use the asm inclusive bounds (right zone 33 wide).

Open: the Random "?" entry of the Z/R cycle (owner decision, see `docs/remix/modes/twelvecb.md`).
