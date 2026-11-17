# Remix text layers

How the Remix HUD text, rectangles and bars of the port are drawn: by game objects in the display link (the asm "room")
the Smash Remix call draws in, so every object the asm draws in front of a string (cursor hands, tokens, panels, tints,
the scene fade) passes over it. The put API is `port/remix/remix_hud_text.{h,cpp}`; the layer objects are
`decomp/src/if/ifremixtextlayer.{c,h}`; the slot routing and glyph layout rules are `port/remix/remix_hud_layer*.{c,h}`
with unit tests in `tests/remix`. Every screen that puts text has its layers (the inventory at the end lists the Remix call
behind each put); there is no other draw path, so a slot with no layer is not drawn.

## Rules

- **A room is a display link.** `Render.create_display_object_` (`Render.asm` 960-995) makes the object with
  `gcMakeGObjSPAfter` (group = GObj link) and `gcAddGObjDisplay` (room = display link). `Render.create_room_`
  (1007-1075) makes a sprite camera for that link with the z-index as camera priority.
- **Higher priority draws first, so under.** Cameras and the objects inside a link are kept in descending priority
  (`gcDLLinkGObjTail`, `objman.c` 603-612); equal priorities draw in creation order, the newer on top.
- **Remix objects are the last of their link.** They use the default order `0x80000000` and `Render.setup_` runs
  after the whole scene setup (`Render.asm` 2112-2120), so a Remix string is over every vanilla object of the same
  link made at setup, and under what the scene makes later (countdown, GO!, GAME SET).
- **Remix creates only four rooms**: 0x37 (FPS, z 1, viewport 10,10..50,30), 0x38 (CSS indicators,
  `CharacterSelect.asm` 4590-4598 and `TwelveCharBattle.asm` 5069-5077, z 0x15 in VS and Training, 0x0E in 1P),
  0x39 (Training action rows under the menus, `Training.asm` 1653, z 0x20). Every other room is a vanilla display
  link with the vanilla camera of the scene.

## Draw path

- **Store.** `remix_hud_text_put`, `_put_fit`, `_put_fit_glyph_scale`, `_putf`, `remix_hud_rect_put` and `remix_hud_bar_put` keep
  the last put of a slot (arguments of the asm `Render.draw_string_` / `Render.draw_rectangle_`). Nothing is drawn by the
  port: the ImGui overlay of the HUD text, its window, its font atlas and `SSB64_REMIX_TEXT_OVERLAY` no longer exist, and
  neither does the ImGui Input Display.
- **Layer.** `ifRemixTextLayerMake(gobj_link, dl_link, dl_priority)` makes one GObj in the group and the room of the asm. Its
  display function draws the slots bound to it with the game renderer: rectangles, then bars (fill and white flash), then
  labels in slot order.
- **Binding.** The owner calls `ifRemixTextLayerBindLabels`, `ifRemixTextLayerBindRects` or `ifRemixTextLayerBindBars` with a
  slot range, once, at the end of the scene setup (as `Render.setup_` runs after it), and keeps its `remix_hud_text` puts. A
  slot has one layer and a scene has at most 8 (`REMIX_HUD_LAYERS_MAX`); a refused bind or a layer that cannot be made
  (font file or width table missing, no layer left) goes to the log and its slots are not drawn. `remix_hud_text_clear` forgets
  layers and routes at every scene change.
- **Glyphs.** The font file 0xA15 is loaded into the scene heap by `ifRemixSpriteGetFile`, as the other Remix sprite files.
  Each glyph is the Sprite of the file (16 x 16, IA 8-bit, one bitmap, attribute 0x0201) drawn through
  `lbCommonPrepSObjAttr` and `lbCommonPrepSObjDraw` from an SObj that lives for the call, with the position, scale and colour of
  the glyph. No object is made or freed per frame. The game filters the glyph and multiplies its colour by the IA intensity,
  as the asm does.
- **Layout.** `remix_hud_layer_place_glyphs` (`port/remix/remix_hud_layer_rules.{c,h}`, `tests/remix/test_hud_layer_rules.c`)
  gives x, y and the two scales of every glyph from the arguments of the put, with the advance, fit and aligned width rules of
  `remix_font_layout`. The width and height tables come from the Remix RAM image (`remix_font.h`). The route table and the life
  of a put are in the same file; `port/remix/remix_hud_layer.h` is the interface between the slot store and the layer object.
- **Rectangles and bars** are filled as `Render.draw_rectangle_` with alpha: primitive colour, `G_RM_CLD_SURF`, whole pixels
  (the corner and the size truncated one by one, cut to the 320 x 240 frame), and `lbCommonClearExternSpriteParams` after them.
- **Life of a put.** A slot is stamped with the game tic count (`dSYTaskmanUpdateCount`) and is drawn while the stamp is at
  most one frame interval old, so it shows in the frame that follows the put and is gone in the next one when the owner stops
  putting it. `remix_hud_text_hide` hides at once.
- **Visibility.** Layers join the group of the asm, so the pause, Disable HUD = All and the Match Stats page hide them with the
  objects they belong to. Layer text is part of the game image: it follows the picture frame (widescreen with 2D stretched,
  centred 4:3, widescreen off) like the sprites, and stays under the port menu.

Limits:

- **Display list buffer.** A glyph costs about 13 `Gfx`. Buffers of the screens with much text are raised under `REGION_REMIX`
  (below); an overflow only writes `DLBuffer OVERFLOW` to the log, so a new screen with many labels counts its glyphs first.
- **File table.** `ifRemixSpriteGetFile` holds 8 files per scene; the font takes one.
- **Sprite camera.** The display link must be drawn by a camera whose display function is `lbCommonDrawSprite`; it sets the
  sprite state and the scissor the glyphs need. A link without one (22 in the battle, and the Remix rooms 0x37, 0x38 and 0x39
  wherever the scene has no camera for them) needs a camera first, as `mnRemixVSStatsSceneMakeCamera` makes one.
- **Clipping.** The glyphs are cut at the camera viewport, at most 10..310 x 10..230, by the sprite code itself; a rectangle is cut
  only by the RDP scissor the camera leaves set, never by the sprite scissor.
- **Layers per scene.** At most 8; the busiest scenes (12CB character select, Training) use 6.
- **Order.** Display order is unsigned: `GOBJ_PRIORITY_DEFAULT` (`S32_MIN`) is 0x80000000, the highest `u32`, so a default
  object is drawn first in its link and an object with a smaller order draws later, over it. Objects made later in the same
  link at the default order draw over a layer.

## Layers per screen

Draw order, first (under) to last (over); the number is the camera priority.

- **VS, 12CB and Training character select** (`mnplayersvs.c`, `mnplayers1ptraining.c`): wallpaper 26 (80), portrait
  wallpaper 36 (75), portrait flash 37 (73), portraits 27 (70), panel cards 28 = room 0x1C (50; panel menu, 12CB
  Custom legend, stock arrows), team 34 (45), handicap 35 (43), doors 29 (40), **30 = room 0x1E (35; every 12CB
  text)**, fighters (30), dropped tokens 33 (25), room 0x38 (21; variant and stock indicators), **cursor hands and
  held tokens 32 (20)**, Ready banner 38 (10). Every Remix text of these screens is under the cursor.
- **1P character select** (`mnplayers1pgame.c`): wallpaper 26 (80), 34 = room 0x22 (70; "High Scores Disabled"),
  portrait wallpaper 32 (60), flash 33 (50), portraits 27 (40), panel 28 = room 0x1C (30), fighters (20), dropped
  token 31 (15), room 0x38 (14), 30 = room 0x1E (13; "Enemy Player", then the cursor and held token), Ready 35 (10).
- **Bonus character select** (`mnplayers1pbonus.c`): 34 = room 0x22 (70) holds the Option bar, MODE and STAGE lines and
  "High Scores Disabled"; the cursor is in a later camera.
- **Stage select** (`mnmaps.c`): wallpaper 0 (80), preview wallpaper 7 (70), preview 3 (65), icons 1 (60), cursor 5
  (50), plaque 6 (40), labels 4 (30; page strip, stage name), name and emblem 2 (20; Hazards, Movement, Layout,
  Roulette). Every Remix string is in front of the cursor.
- **VS results** (`mnvsresults.c`): wallpaper 26 (80), tints 34 (70) and 35 (55), emblem 33 (60), fighters (50),
  player tags 27 (30), winner text 29 (20), tint 30 (17; black, from tic 180), header 31 = room 0x1F (15; results
  rows, "Game NN", the Match Stats page), entry wipe 32 (10).
- **Title** (`mntitle.c`): sprites 0|1 (60; version string in room 1), slash 2 (40), black fill made on proceed (0).
- **Item Switch** (`mnvsitemswitch.c`): decal 4 (60), labels 1 (40), cursor 3 = room 3 (30; the L legend).
- **Battle** (`gmcamera.c`): wallpaper 0 (80), battle camera 1-20, player arrows 8 (35), magnifier 9 (30), screen
  flash 22 (20; not a sprite camera), interface 23 then 24 (20), effects 25 (15), scene fade (10; black, 12 frames),
  room 0x37 (1). Link 23 = room 0x17: HUD, then the Remix setup objects (KOTH, Multi-Man and HRC counters, Input
  Display), then what the match makes (countdown, music title banner, GO!, GAME SET). Link 24 = room 0x18: pause
  decals, pause legend, Gentlemen's Reset.
- **Training battle** (`sc1ptrainingmode.c`): as the battle, plus room 0x39 (32) between the player arrows and the
  magnifier, used by the action rows while a menu is open; the vanilla menu fill and the Z menu panel (room 0x16) are
  in link 22, the menu sprites, frame and every other Training text in link 23.

## Layers as built

Every layer is made once per scene, after the screen made its own objects, and binds the slots of its owner.

### Character selects

Every layer is made once per scene, after the screen made its own objects, and binds the slots of its owner. A layer is in
the display link (room) of the asm with the default order unless noted; objects of the same link made later at the default
order (the arrows of the panel menu, the legend sprites) draw over it.

| Screen | Layer | Room (display link), group | Slots | Made in |
|---|---|---|---|---|
| VS, Training, 1P, Bonus 1 and 2 | panel menu: labels, values, page number of every panel | panel card room 28 (0x1C), group 22 | labels 56..83 | `mnRemixPanelMenuStart` |
| VS, 12CB, Training, 1P, Bonus | indicators: stock numbers, D-pad variant marks, 12CB Manual stock marks | room 0x38 (camera priority 21 in VS and Training, 14 in 1P and Bonus, as `css_settings`), group 0x13 | labels 47..50, rects 30..37 | `mnRemixCssTextLayersMake` |
| 12CB | Custom legend texts and marks | room 28, group 22, order 0x40000000 (the asm makes it while the token is held, after the sprites) | labels 100..107, rects 40..45 | `mnRemixTwelveCBCustomLegendMakeLayer` |
| 1P | "Enemy Player", "Press Z" | room 0x1E (30, the cursor hands' link; the hands have order 6..0, so they draw after the default order), group 8 | labels 120..121 | `mnRemixEnemyLabelMake` |
| 1P, Bonus | "High Scores Disabled" | room 0x22 (34), group 0x1B | label 96 | `mnRemixCssHighScoresMakeLayer` |
| Bonus 1 and 2 (the MODE row) | the Option bar, made before the Option title and outline sprites so it draws behind them | room 0x22 (34), group 0x19 | rect 46 | `mnRemixBonusModesToggleMake` |
| Bonus 1 and 2 | MODE, STAGE, mode name, ": Set Stage" | room 0x22 (34), group 0x19 (the asm has ": Set Stage" in 0x1A) | labels 97..100 | `mnRemixBonusModesToggleMake` |

A scene has at most 6 of these (12CB: panel menu, indicators, legend and the three HUD layers).

Notes found while moving them:

- **Display order is unsigned.** `GOBJ_PRIORITY_DEFAULT` (`S32_MIN`) is 0x80000000, the highest `u32`, so a default object is
  drawn first in its link and an object with a smaller order draws later, over it. The 1P cursor hands (6, 4, 2, 0 in link
  30) and the asm arrows of the panel menu (0x40000000, `Render.display_order_room`) rely on it.
- **Room 0x38 had no camera** in the character selects: `mnRemixCssTextMakeIndicatorCamera` makes a sprite camera for it.
  The D-pad icon, variant icons and flag sprites of the indicators stay in display link 28 (they were made there before
  M15), so on the VS select the fighter model still draws over them; only the marks and the stock numbers moved to 0x38.
- **12CB legend sprites** moved from display link 26 to 28 (asm room 0x1C) so the buttons, the arrows and the layer share
  the camera.
- **Display list buffers** (`REGION_REMIX` only): VS and 12CB 2750 + 4096 `Gfx`, Training 2750 + 2048, Bonus 2375 + 1536, 1P
  2375 + 2048 + 1024. Four open panel menus on the VS select (`css_panelmenu_visibility.txt` for four ports) and the 12CB and
  Bonus 3 screens ran with no `DLBuffer OVERFLOW`.
- **Layer text covers nothing it should not**: with the hand over "Shield", the stock number, "Press Z", "High Scores
  Disabled" or the legend texts, the hand is over the text.

### Results

| Screen | Layer | Room (display link), group | Slots | Made in |
|---|---|---|---|---|
| VS results, 12CB | "Game NN" | room 0x1F (31), group 0x16 (hidden with the header by the Match Stats page) | label 40 | `gmRemixTwelveCBHudMakeResultsLayer`, called by `mnVSResultsMakeLabel` |
| VS results | custom tag names (black copies, then names) | room 27 (camera 30), group 18, the room of the tag sprites; made after the four tags | labels 5..12 | `ifRemixPlayerTagMakeNameLayer`, called at the end of `mnVSResultsInitFightersAll` |
| Match Stats | "Press A for Match Stats" prompt | display link 36, group 0x0D, after the A button sprite, under the page background | label 0 | `mnRemixVSStatsSceneMakeLayers` |
| Match Stats | B and R hints, 1P..4P headers, rows, values, stripes, underlines | display link 39, group 0x0E (hidden with the page), over the background, the B and R buttons and the stock icons | labels 1..2 and 48..127, rects 1..47 | `mnRemixVSStatsSceneMakeLayers` |

- The Match Stats camera (priority 12, made by `mnRemixVSStatsSceneMake` at the start of the scene) draws links 36 (A button and prompt),
  37 (the dimming rectangle), 38 (B, R and the stock icons) and 39 (page text) in that order, the order of the room 0x1F objects of
  `VsStats.asm` setup_. The A button moved from 37 to 36, so the background dims it as in the asm.
- The prompt is put at full white; the real rectangle over its layer dims it (about 31 percent), as in the asm.
- The layers are made after `lbRelocInitSetup` in `mnVSResultsFuncStart`, not with the scene objects: the font file is loaded into
  the scene heap and a token made before the reloc setup is stale (SIGSEGV at tic 60).
- The tag names are under the results tints (darkened like the tag arrow from tic 180) and under the page background (dimmed), as in the asm.
- A page puts at most 73 labels (page 1 with combo stats and four ports) of the 80 slots 48..127.
- **Display list buffer** of the results scene (`REGION_REMIX` only): 2500 + 6144 `Gfx` (8644). Measured peak of the frame with
  four ports, the combo meter on, four 8-glyph tag names (W and M, the widest) and pages 1 and 2 opened in turn: 5075 `Gfx`, so
  about 41 percent of the buffer is free (about 270 glyphs); no `DLBuffer OVERFLOW`.
- Layers in the scene: 4 at most (Game NN, tag names, prompt, pages); the limit is 8.

### Battle HUD

All in group 0x0B (the interface link, hidden with the HUD by the pause and by Disable HUD = All) and display link 23 (room 0x17,
drawn by the interface camera, so no new camera was needed). Each is made after the objects the asm makes before it and before what
the match makes later (countdown, GO!, GAME SET, item arrows, the scene fade), which is what the rows below sort against.

| Screen | Layer | Slots | Made in |
|---|---|---|---|
| Battle (VS, 1P, Training, Bonus stage, demo), a fighter with a custom Player Tag | tag names (black copies, then names) over the tag arrows | labels 5..12 | `ifRemixPlayerTagMakeBattleLayer`, called at the end of `ifCommonPlayerTagMakeInterface`; nothing is made when no active fighter has a custom tag |
| KOTH | meter bars and full-bar flash, after the frame sprites | bars 0..3 | `gmRemixKothHudSetup` |
| Match start banner | rectangle, track line, game line, made with the GO lights over the HUD | rect 2, labels 71..72 | `gmRemixMusicTitleShowAtCountdown` |
| 1P Multi-Man and Cruel | KO counter, after the whole scene setup | label 28 | `sc1PRemixMultimanMakeTextLayer`, end of `sc1PGameFuncStart` |
| 1P Home-Run Contest | distance and "FT." | labels 29..30 | `sc1PRemixHrcRunMakeTextLayer`, end of `sc1PGameFuncStart` |

- Smashketball, Tag Team, Tug of War and Stamina put no Remix text (slots 14..18 are reserved and unused), so they have no layer.
- The tag layer is made before the KOTH one, so the meters are over the names, as in Remix.
- Consequences seen in game: the Multi-Man and HRC counters are dimmed by the 12-frame scene fade, as the asm objects are; the layer text is the softer, darker glyph already accepted; the banner text and rectangle are drawn by the interface
  camera.
- A scene has at most 3 of these layers (tags, KOTH or counter, banner); the buffers did not need raising (no `DLBuffer OVERFLOW`).
- The Input Display is made of sprites, see the next section.

### Title, item switch, stage select, pause, Training and Input Display

Measured (Gfx head peaks, no `DLBuffer OVERFLOW`): Training 4733 of 7680 with the Z menu open, Show Action and two Input Display
blocks (buffer not raised); stage select 1899 of 3661 with all four lines. A Training Reset or Quick Reset calls `gmRemixModeSceneClear`
(`sc1PTrainingRemixStartScene`) because it re-runs `scManagerFuncUpdate` without `scManagerRunLoop`; after five resets the layers, Reset Count, Z menu,
Show Action rows and Input Display still draw. `ifRemixInputDisplayClear` runs from `gmRemixModeSceneClear`.

| Screen | Layer | Room (display link), group | Slots | Made in |
|---|---|---|---|---|
| Title | "Smash Remix v2.0.1" | room 1, group 3 (the logo camera, priority 60; the black fill of the proceed step is a later camera, so it covers the text) | label 3 | `mnRemixTitleVersionMakeLayer`, end of `mnTitleFuncStart` |
| Item Switch | ": All On/Off"; the L, R and arrow sprites moved here | room 3 (cursor camera, priority 30), group 12 | label 4 | `mnRemixItemSwitchLegendMakeLayer` |
| Stage select | "Page" and the number | room 4, group 3 | labels 126..127 | `mnRemixStagePagerMakeLayer` |
| Stage select | stage name | room 4, group 3 | label 125 | `mnRemixStagePanelMakeLayers` |
| Stage select | Hazards, Movement, Layout, Roulette lines | room 2, group 0x0D (the buttons are sprites of the same room) | labels 118..124 | `mnRemixStagePanelMakeLayers` |
| Pause (every battle) | music title and legend texts, squares, Gentlemen's Reset request | room 0x18 (24), group 0x0E, order 0x40000000 (draws after the pause menu objects, which are made again at every pause) | labels 51..70 and 19..22, rects 0..1 | `ifRemixPauseLegendMakeLayer`, called from `ifCommonPlayerTagMakeInterface` |
| Training | reset count, Show Action legend, exit hint, D-pad legend, Z menu rows, cursor, underline, exit bar | room 0x17 (23), group 0x1D (never hidden: the texts are put only while they show) | labels 25, 27, 73..104, 113..116; rects 24..30 | `sc1PTrainingRemixTextLayersMake`, end of the Training setup |
| Training | Show Action rows | room 0x39 (57), a new camera of priority 32 (between the player arrows 35 and the magnifier 30), group 0x1D | labels 105..112 | `sc1PTrainingRemixTextLayersMake` |
| Battle with Input Display on | sprites of the block of each port | group 0x0B, room 0x17 | none (sprites) | `ifRemixInputDisplayMake`, end of `ifCommonPlayerDamageInitInterface` |

- The Training Z menu panel (link 22) and the vanilla pause menu are drawn after camera 32, so the rows are tinted and covered by them; a
  long action name no longer runs over the menu. The `sc1ptrainingremixaction.c` skip and the `deviations.md` row are removed.
- The pause layer is made once per battle scene with order 0x40000000 and not per pause: a layer id is only freed at the scene change.
  A scene has now at most 6 layers in the battle (tags, KOTH or counter, banner, pause, and in Training the two Training ones).
- **Input Display** (InputDisplay.asm `create_input_display_objects_` and `update_input_display_`): L, R, A, B, Z, the four C arrows, the
  D-pad body and its four arms, the joystick gate and the joystick, 16 sprites per port in the order of the asm, anchored to the X
  of the series logo of the port's percent HUD (`ifCommonPlayerDamageGetLogoX`). Placement (Training forced to the bottom, the
  Bonus, Multi-Man, All-Star, 1P and Dragon King offsets) and the stick and gate alpha with the 6 frame cooldown are the pure rules
  of `if/ifremixinputdisplayrules.c` (tests `tests/remix/test_inputdisplay_rules.c`). The colours are the asm ones (button colour at
  alpha 0x4F released, full alpha pressed; arms invisible until pressed). The sprites are in group 0x0B, so the pause and Disable HUD =
  All hide them (checked in game), and they are under the GO! and GAME SET objects made later. The block exists only when the
  panel value was not Disabled at the start of the match, as in the asm.
- Not done: the VS timer is not lowered when port 4's Input Display is at the top (`adjust_timer_position_`, a separate patch of the
  timer; see `patch_audit_scopes.md`).
- **Display list buffers**: nothing was raised; the stage select got +1536 `Gfx` under REGION_REMIX (about 80 glyphs).

## Differences from Remix

- The VS timer is not lowered when the Input Display of port 4 is at the top (`adjust_timer_position_`, a separate patch of the
  timer; see `patch_audit_scopes.md`).
- On the Bonus 1 and 2 select, ": Set Stage" shares the layer of the MODE row (group 0x19) instead of the asm group 0x1A.
- On the character selects the D-pad icon, variant icons and flag sprites of the indicators stay in display link 28 (made there
  before the layers existed), so on the VS select the fighter model still draws over them; only the marks and the stock numbers
  are in room 0x38.
- The squeeze of a string wider than its limit and the layout rules follow `Render.apply_max_width_`; the one row in
  `deviations.md` about it stays.

## Fixes beyond the order

- "Game NN" on the 12CB results is put only from the results label (tic 120, tic 1 on No Contest), as the asm creates it.
- The 12CB Custom legend and its marks are hidden while the panel menu of the token holder is open.
- The Training Show Action rows are drawn under the Z menu panel and the vanilla pause menu (room 0x39, its own camera).
- The Item Switch legend sprites moved from display link 4 to 3 and the stage select pager sprites from 2 to 4 (the asm rooms).
- The Input Display is made of game sprites (`ifRemixInputDisplayMake`); the pause, Disable HUD = All and a scene change hide
  or clear it.
- The match-start banner is clipped by the interface camera viewport, as the asm rectangle is.
- The KOTH bars stay put after the match leaves Go (`gmRemixKothProcUpdate` runs the HUD update every frame; the game rules stop).
- The title version text is covered by the black fill of the proceed step.

## Inventory of the Remix calls

### 12-Character Battle character select

| Port call | Text | asm | Room, group |
|---|---|---|---|
| `gmremixtwelvecbhud.c` | "Stocks Remaining" header (160,126) centred, scale 0.875 | TwelveCharBattle.asm:5093 | 0x1E; 8 |
| `gmremixtwelvecbhud.c` | stocks remaining number of P1 (x 124) and P2 (x 192), y 140, red/blue | TwelveCharBattle.asm:5094-5095 (Render.draw_number) | 0x1E; 8 (GROUP_ALWAYS) |
| `gmremixtwelvecbhud.c` | "Character Set" header (160,160), only while the battle is not started | TwelveCharBattle.asm:5098 | 0x1E; 9 |
| `gmremixtwelvecbhud.c` | character set name of each player (Default/Japanese/Polygon/Remix/Custom) at x 124 / 192 | TwelveCharBattle.asm:5099-5100 (Render.draw_string_pointer) | 0x1E; 9 |
| `gmremixtwelvecbhud.c` | "Best Character" header (160,160), once the battle has started | TwelveCharBattle.asm:5109 | 0x1E; 10 |
| `gmremixtwelvecbhud.c` | best character name of each player (x 124 / 192, y 174, max width 72) | TwelveCharBattle.asm:5110-5118 | 0x1E; 10 |
| `gmremixtwelvecbhud.c` | TKO count of the best character of each player (x 124 / 192, y 187) | TwelveCharBattle.asm:5120-5121 (Render.draw_number) | 0x1E; 10 |
| `mnremixtwelvecbcustomlegend.c` | Custom set legend texts: ": Set All" (41 or 247.5, 217), ": Presets" (105 or 173, 187) | TwelveCharBattle.asm:4730-4744 (Set All), 4774-4788 (Presets) | 0x1C; 0x16 |
| `mnremixtwelvecbcustomlegend.c` | three 2x2 marks on the legend D-pad icons; y 188 yellow, 208 yellow, 216 | TwelveCharBattle.asm:4911, 4915, 4919 (Render.draw_rectangle) | 0x1C; 0x16 |
| `mnremixtwelvecbstockmanual.c` | left blinking yellow 1x1 mark on the D-pad icon of the Manual stock controls | CharacterSelect.asm:5852-5861 (Render.draw_rectangle 1x1 YELLOW) | 0x38; 0x13 |
| `mnremixtwelvecbstockmanual.c` | right blinking yellow 1x1 mark on the D-pad icon (digits_x - 20, 196) | CharacterSelect.asm:5864-5870 (Render.draw_rectangle 1x1 YELLOW) | 0x38; 0x13 |

### VS, Training, 1P and Bonus character selects

| Port call | Text | asm | Room, group |
|---|---|---|---|
| `mnplayersvs.c` | stock number "N" at the right edge of the panel card of a player (y 191, scale 0.90625) | CharacterSelect.asm:5704-5718 | 0x38; 0x13 |
| `mnplayersvs.c` | stock number under the panel for Stock Mode Last / Manual | CharacterSelect.asm:5704-5718 | 0x38; 0x13 |
| `mnremixbonusmodes.c` | "STAGE:" right aligned at (208,175), Bonus 1/2 with the Remix choice | Bonus.asm:113 | 0x22; 0x19 |
| `mnremixbonusmodes.c` | ": Set Stage" (78.5,217) beside the L button, while a token is held over a fighter | Bonus.asm:129 | 0x22; 0x1A |
| `mnremixbonusmodes.c` | bar behind the "Option" title: 144x5 at (175,136), colour 576088FF | Bonus.asm:26 (Render.draw_rectangle) | 0x22; 0x19 |
| `mnremixbonusmodes.c` | "MODE:" right aligned at (208,156) | Bonus.asm:112 | 0x22; 0x19 |
| `mnremixbonusmodes.c` | mode name "NORMAL" / "REMIX" centred at (249,156) | Bonus.asm:133 (Render.draw_string_pointer mode_pointer) | 0x22; 0x19 |
| `mnremixcsshighscores.c` | "High Scores Disabled" (192,217), colour FF8080FF, scale 0.8125 | CharacterSelect.asm:4767 | 0x22; 0x1B |
| `mnremixcssindicators.c` | 2x2 yellow mark on the variant D-pad icon of a panel, showing which D-pad direction the | CharacterSelect.asm:4655-4667 | 0x38, Render.create_room_ at; 0x13 |
| `mnremixcsspanelmenutext.c` | panel menu item label (panel x + 4, panel y + 18 + 24*row), colour D0D0D0FF, scale 0.875 | CharacterSelectDebugMenu.asm:787-825 | the dl link of the panel card; the GObj link |
| `mnremixcsspanelmenutext.c` | panel menu item value, centred on the panel (y + 30 + 24*row), white | CharacterSelectDebugMenu.asm:844-907 | dl link of the panel card: 28; GObj link of |
| `mnremixcsspanelmenutext.c` | page number of the panel menu (x + W - 10.5, y + 4), only with more than one page | CharacterSelectDebugMenu.asm:1095-1121 | dl link of the panel card: 28; GObj link of |
| `mnremixenemylabel.c` | "Enemy Player" (108,130), scale 0.5, blinking | SinglePlayerEnemy.asm:306 | 0x1E; 8 |
| `mnremixenemylabel.c` | "Press Z" (108,140), scale 0.5, blinking | SinglePlayerEnemy.asm:309 | 0x1E; 8 |

### Stage select

| Port call | Text | asm | Room, group |
|---|---|---|---|
| `mnremixstagepager.c` | "Page" label of the page strip | Stages.asm:2847 (Render.draw_string, setup_) | room 4; 3 |
| `mnremixstagepager.c` | page number next to "Page" | Stages.asm:2848 | room 4; 3 |
| `mnremixstagepanel.c` | "Hazards ( ):" / "Hazards:" label and its On/Off value | Stages.asm:2878-2879 (draw_string + draw_string_pointer) | room 2; 0xD |
| `mnremixstagepanel.c` | "Movement ( ):" / "Movement:" label and its On/Off value | Stages.asm:2880-2881 | room 2; 0xE |
| `mnremixstagepanel.c` | "Layout ( ):" label and the variant type | Stages.asm:2882-2883 | room 2; 0xC |
| `mnremixstagepanel.c` | "Roulette ( )" label over the RANDOM square | Stages.asm:2889 | room 2; 0xF |
| `mnremixstagepanel.c` | stage name centred at (230, 209) | Stages.asm:2851 | room 4; 3 |

### VS results and Match Stats

| Port call | Text | asm | Room, group |
|---|---|---|---|
| `gmremixtwelvecbhud.c` | "Game NN" under the results header (32,46) | TwelveCharBattle.asm:1946 | 0x1F; 0x16 |
| `mnremixvsstatspages.c` | right aligned value (or "-") per active port of a row | VsStats.asm:432-457 | room 0x1F; page group |
| `mnremixvsstatspages.c` | striped background (0x40407040) of every second value row | VsStats.asm:394-401 | room 0x1F; page group |
| `mnremixvsstatspages.c` | row labels | VsStats.asm:405-419 (draw_line_ label) | room 0x1F; page group |
| `mnremixvsstatspages.c` | section and sub headers | VsStats.asm:405-419 (draw_line_ label, value address 0) | room 0x1F; page group |
| `mnremixvsstatspages.c` | white underline of a section header (callers 370, 386, 418, 425, 432, 437) | VsStats.asm:288-300 (draw_underline_, room at line 294) | room 0x1F; page group |
| `mnremixvsstatspages.c` | 1P..4P column headers | VsStats.asm:616-630 (draw_port_headers_, Render.draw_string_) | room 0x1F; 0x0E |
| `mnremixvsstatspages.c` | white 26x1 underline under each port header | VsStats.asm:633-643 (Render.draw_rectangle_) | room 0x1F; 0x0E |
| `mnremixvsstatsview.c` | ": Back" hint (B button) | VsStats.asm:958 | room 0x1F; 0x0E |
| `mnremixvsstatsview.c` | ": Next Page" hint (R button) | VsStats.asm:962 | room 0x1F; 0x0E |
| `mnremixvsstatsview.c` | same prompt while the stats page is open, drawn at alpha 0x4F | VsStats.asm:940 (object), 955 | room 0x1F; 0x0D |
| `mnremixvsstatsview.c` | "Press (A) for Match Stats" prompt, page closed | VsStats.asm:940 | room 0x1F; 0x0D |
| `mnvsresults.c` | custom player tag name (black copy + coloured name) over the tag of each fighter on the | PlayerTag.asm:77-80 (results hook 0x80133E60), 229-262 | The temporary string uses; GObj link 18 |
| `mnvsresults.c` | "Game NN" under the mode header of the 12-Character Battle results | TwelveCharBattle.asm:1932-1947 | room 0x1F; 0x16 |

### Title and Item Switch

| Port call | Text | asm | Room, group |
|---|---|---|---|
| `mnremixitemswitchlegend.c` | ": All On/Off" legend text next to the L button | Item.asm:969; called | room 3; 12 |
| `mnremixtitleversion.c` | "Smash Remix v2.0.1" at (160, 218), colour 0x888800FF | Boot.asm:160; called from | room 1; 3 |

### Battle HUD

| Port call | Text | asm | Room, group |
|---|---|---|---|
| `ftremixinputdisplay.c`, `ifremixinputdisplay.c` | Input Display of each port whose panel value is not Disabled: L, R, A, B, Z, the C arrows, D-pad body and arms, joystick gate and joystick as 16 sprites | InputDisplay.asm 180 | 0x17; 0x0B |
| `gmremixkothhud.c` | King of the Hill score meter of each active port: coloured fill bar (40x5) plus the white | KingOfTheHill.asm 1640-1657 | 0x17; 0xB |
| `gmremixmusictitle.c` | translucent banner rectangle (1, 156, 320x28, 0x34343494) | BGM.asm:1033; hook at | room 0x17; 0xB |
| `gmremixmusictitle.c` | track line "note title note" at (160, 158), scale 1.0 | BGM.asm:1076-1086 (Render.draw_string_, blur on) | room 0x17; 0xB |
| `gmremixmusictitle.c` | game line at (160, 172), scale 0.75 | BGM.asm:1095-1105 (Render.draw_string_, blur on) | room 0x17; 0xB |
| `ifremixplayertag.c` | Black 1.25x under-copy of the custom player tag name, centred over the fighter's tag arrow | PlayerTag.asm 112-161 | The room argument 0x17 is; 0xB |
| `ifremixplayertag.c` | Custom player tag name in the tag colour, over the black copy | PlayerTag.asm 168-205 | Glyphs are SObjs of the; 0xB |
| `sc1premixhrcrun.c` | Home-Run Contest distance number, right aligned at; slides to the | SinglePlayerModes.asm 2361 | 0x17; 0x0B |
| `sc1premixhrcrun.c` | 'FT.' unit label at (282,47), scale 0.625, moving with the counter | SinglePlayerModes.asm 2365 | 0x17; 0x0B |
| `sc1premixmultiman.c` | Multi-Man / Cruel KO counter number at (50,18), scale 0.9 (y 212 with the Dragon King HUD) | SinglePlayerModes.asm 2314 | 0x17; 0x0B |

### Pause

| Port call | Text | asm | Room, group |
|---|---|---|---|
| `ifremixgentlemensreset.c` | 'Game reset requested by NP.' centred at (160,126) | TwelveCharBattle.asm 3584 | 0x18; 0xE |
| `ifremixgentlemensreset.c` | 'If accepted, stock counts will be reset.' centred at (160,144) | TwelveCharBattle.asm 3585 | 0x18; 0xE |
| `ifremixgentlemensreset.c` | 'Accept:' at (40,164), followed by the A+B+Z+R combo sprites (native) | TwelveCharBattle.asm 3586 | 0x18; 0xE |
| `ifremixgentlemensreset.c` | 'Decline:' at (188,164), followed by the START button sprite (native) | TwelveCharBattle.asm 3608 | 0x18; 0xE |
| `ifremixpauselegend.c` | Live track title of the pause music legend at (52,74), max width 228 | Pause.asm 295-300 (Render.draw_string(0x18, 0xF, ...) + | 0x18; 0xF |
| `ifremixpauselegend.c` | Live game title of the pause music legend at (52,90), scale 0.75 | Pause.asm 305-312 | 0x18; 0xF |
| `ifremixpauselegend.c` | Pause legend text rows: music note, ': Next Music Track', ': Random Music Track' | Pause.asm 279; 322, 324, 327, 329 | 0x18; 0xF (music) and |
| `ifremixpauselegend.c` | Two yellow 2x2 squares marking the D-pad direction on the music legend rows, at (51,113) | Pause.asm 315 and 318 | 0x18; 0xF |

### Training

| Port call | Text | asm | Room, group |
|---|---|---|---|
| `sc1ptrainingremix.c` | "Reset Count:" label (x 99.5, y 25) | Training.asm 1617 (Render.draw_string reset_string) | 0x17; 0x15 |
| `sc1ptrainingremix.c` | Reset Count value (right aligned at x 221, y 25) | Training.asm 1618 (Render.draw_number reset_counter) | 0x17; 0x15 |
| `sc1ptrainingremixaction.c` | Show Action: action name of the fighter of each port (x 20, y 55 + 26*port, scale 0.75) | Training.asm 1547 | room passed in a2: 0x17; 0x17 |
| `sc1ptrainingremixaction.c` | Show Action: frame count of the action of each port (x 20, y 66 + 26*port, scale 0.75) | Training.asm 1515 | same as the action name: 0x17; 0x17 |
| `sc1ptrainingremixaction.c` | "Show" (first word of the L legend, x 20, y 27.25, scale 0.875) | Training.asm 1636; L glyph | 0x17; 0x15 |
| `sc1ptrainingremixaction.c` | "Action" (second word of the L legend, x 20, y 38.5, scale 0.875) | Training.asm 1637 (Render.draw_string action_legend_2) | 0x17; 0x15 |
| `sc1ptrainingremixexit.c` | "(Hold" of the "(Hold --)" Exit hint (x 169, y 166, scale 0.875) | Training.asm 1597; A glyph | 0x17; 0xF |
| `sc1ptrainingremixexit.c` | ")" closing the "(Hold --)" hint, right aligned at the end of the measured full string | Training.asm 1597 (same string as the opening part) | 0x17; 0xF |
| `sc1ptrainingremixexit.c` | Hold to Exit progress bar (x 73, y 179, 2 px high, width grows to 64) | Training.asm 1600-1613 | 0x17; 0xF |
| `sc1ptrainingremixlegend.c` | "Press; the Z glyph is a game sprite | Training.asm 1583; Z glyph 1584 | 0x17; 0xE |
| `sc1ptrainingremixlegend.c` | Yellow 2x2 square marking the D-pad direction of each legend line | Training.asm 1625-1628 (Render.draw_rectangle x4) | 0x17; 0x19 |
| `sc1ptrainingremixlegend.c` | D-pad legend texts "Toggle Pause", "Quick Reset", "Frame Advance", "Model Display" | Training.asm 1629-1632 | 0x17; 0x19 |
| `sc1ptrainingremixmenudraw.c` | Z menu row value shown as a number (right aligned at x 252) | Menu.asm 395-428 | 0x17; 0x16 |
| `sc1ptrainingremixmenudraw.c` | Z menu row value shown as text | Menu.asm 412-428; max | 0x17; 0x16 |
| `sc1ptrainingremixmenudraw.c` | Z menu row label (x 76, y 50 + 14*row, squeezed to 150 px) | Menu.asm 354-377; max width | 0x17; 0x16 |
| `sc1ptrainingremixmenudraw.c` | "/" between the R and Z glyphs of the pagination legend (x 288, y 16) | Training.asm 1641 (Render.draw_string pagination_legend_1 + 4) | 0x17; 0x16 |
| `sc1ptrainingremixmenudraw.c` | "Next/" of the pagination legend (x 272, y 27.25) | Training.asm 1643 (Render.draw_string pagination_legend_1) | 0x17; 0x16 |
| `sc1ptrainingremixmenudraw.c` | "Prev" of the pagination legend (x 272, y 38.5) | Training.asm 1644 (Render.draw_string pagination_legend_2) | 0x17; 0x16 |
| `sc1ptrainingremixmenudraw.c` | Z menu cursor square (4x4 red, x 70, row y + 5) | Menu.asm 662-679 (Render.draw_rectangle_ at 676), moved by | 0x17; 0x16 |
| `sc1ptrainingremixmenudraw.c` | Z menu selection underline (182 x 1 red, x 70, row y + 11) | Menu.asm 681-695 (Render.draw_rectangle_ at 693) | 0x17; 0x16 |
| `sc1ptrainingremixmenudraw.c` | Clears the cursor square (zero-size rectangle) on the frame the Z menu closes | none found as a draw call: the asm hides group 0x16 | n/a; 0x16 |
| `sc1ptrainingremixmenudraw.c` | Clears the selection underline (zero-size rectangle) on the frame the Z menu closes | none found as a draw call: the asm hides group 0x16 | n/a; 0x16 |
