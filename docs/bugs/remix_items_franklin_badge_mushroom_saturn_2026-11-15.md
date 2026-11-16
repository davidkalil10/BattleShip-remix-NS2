# Items: Franklin Badge reflect after another reflector and in a rematch, mushroom swap timer and rumble, Mr. Saturn flags (2026-11-15)

Found by the M14 equivalence audit (ROADMAP T14.3, lot i2: `items/FranklinBadge.asm`, `items/SuperMushroom.asm`,
`items/MrSaturn.asm`, `items/Lightning.asm` against `decomp/src/it/itremix/`).

## Symptoms and causes

| Item | Symptom | Cause |
|---|---|---|
| Franklin Badge | After a move with its own reflect or absorb hitbox (Fox or Falco down special, Ness, Perfect Shield, the Dedede and Marina absorbs) the badge stopped reflecting for the rest of its 24 seconds; the immunity stayed. | `FranklinBadge.asm` 136-166 (`handle_active_franklin_badge_`) writes the badge hitbox and the reflect flag back as soon as the hitbox at 0x850 is not the badge's and the 0x18C halfword is not 0x0880, not 0x0C80 (rewritten to 0x0880) and has the 0x0400 bit off. The port only set the flag while the hitbox was still the badge's. |
| Franklin Badge | A Fox reflector held while the badge ran out stopped reflecting. | `FranklinBadge.asm` 184-190 skips the flag clear when t2 = 1, which the delay slots set while another reflector owns the hitbox. The port always cleared `is_reflect`. |
| Franklin Badge | In a rematch (Salty Runback, or a new match with the same ports) a badge gave immunity for the whole match and never reflected, expired or restored the colour. | The port adds its countdown process once per fighter GObj and remembered that GObj across scenes; `itRemixFranklinBadgeClearAll` cleared only `is_active`. The next scene gives the same port the same GObj address, so no process was added. The asm registers a routine on every first pickup (line 79). |
| Franklin Badge | A reflection played no sound and no green circle, and the Remix custom-reflect rules (Samus bomb, Thunder, cannonball reflectable; speed simply inverted) did not apply. | The reflect hitbox had type 4, after the asm comment "custom value of 4"; `Reflect.reflect_type.CUSTOM` is 3, so `ftRemixReflectIsCustom` was false and `reflect_initial_` never ran. Found while testing the first row. |
| Super / Poison Mushroom | A tiny (poisoned) fighter who took a Super Mushroom stayed tiny 48 more frames and snapped to normal with no grow animation or sound; a giant fighter who took a Poison Mushroom started to shrink 9 frames late. | `SuperMushroom.asm` 284-288: the `bnel` on the new item id adds SHRINK - GROW (-9) when the new item is not Poison, that is when the active effect is the Poison one (542, the frame before its transition 2 at 543; 551 for an active Super effect). The port added it when the new item was Poison. |
| Super / Poison Mushroom | The 60-frame rumble of a grow or shrink stopped at the first status change (one motor pulse instead of twelve). | `SuperMushroom.asm` 573-582 stores 0x17 over the rumble id of the script that `Global.rumble_` leaves in v0, so the `gmRumbleStopRumbleID(player, 7)` of `ftMainSetStatus` does not find it. The port called `ftParamMakeRumble(fp, 7, 60)` only. |
| Mr. Saturn | `is_damage_all` stayed FALSE, so the attack-against-attack checks of `itprocess.c` (835-837, 948-950) skipped the items and projectiles of the thrower and of his team. | `MrSaturn.asm` 200 calls 0x8017279C (`itMainClearOwnerStats`) at spawn; the port did not. The flag does not decide whom the Saturn hits (`ftmain.c` 3973 reads only the owner and the team). |
| Mr. Saturn | After a hit a thrown Mr. Saturn despawned up to 17 frames later than in the asm. | `MrSaturn.asm` 541-542 (`falling_initial_`) stores 0x5674 at 0x2D2: `pickup_wait` 0x567 (1383), `is_allow_knockback` 0, `is_unused_item_bool` 1. The port kept the 1400 of the pickup. |
| Mr. Saturn | A thrown Mr. Saturn that hit something, or a dropped one, could vanish when it landed (1 chance in 4). | `MrSaturn.asm` 535-539 (`falling_initial_`, "don't randomly despawn") clears mask 0x02 of 0x2CE, the low bit of `times_thrown`, so the throw or drop does not count; the port left the count at 1 and `itMapCheckDestroyDropped` rolled the despawn. |

## Fix

- `itremixfranklinbadge.c`: `itRemixFranklinBadgeGetFlags` rebuilds the 0x18C halfword, `itRemixFranklinBadgeRulesGetMaintain`
  and `itRemixFranklinBadgeRulesKeepsReflectAtExpiry` (`itremixfranklinbadgerules.c`) decide the restore and the expiry;
  `itRemixFranklinBadgeClearAll` forgets the process owner; the hitbox type is `FTREMIX_REFLECT_TYPE_CUSTOM`.
- `itremixsupermushroom.c`: `itRemixSuperMushroomRulesGetSupersededTimer` (`itremixsupermushroomrules.c`); the rumble
  goes through `gmRumbleRemixSetPlayerRumbleKeepOnStatus` (`gm/gmrumble.c`), which relabels the new script with
  `GMRUMBLE_REMIX_KEEP_ID`.
- `itremixmrsaturn.c`: `itMainClearOwnerStats` in `itRemixMrSaturnSetup`; the 0x5674 fields and the
  `times_thrown` low bit in `itRemixMrSaturnFallSetStatus`.

Unit tests: `tests/remix/test_franklinbadge_rules.c`, `tests/remix/test_supermushroom_rules.c`.

## Kept as deviations

`docs/remix/deviations.md`, 5.5: a badge that runs out on the frame it takes its hitbox back (the asm leaves the
reflect flag set), the Lightning badge check by position in the fighter list instead of by port, and the End event of
the relabelled rumble script (past the tables on the N64).
