# Remix text layers (M15 T15.1)

Inventory of every text, bar and rectangle that the port draws through the `remix_hud_text` overlay
(`port/remix/remix_hud_text.{h,cpp}`), with the Smash Remix call it stands for, the room that call draws in and the
cases where the overlay order differs from Remix on screen. Read-only audit from the asm and the decomp (2026-11-17);
the cases were not reproduced in game, except the 12CB cursor one, which the owner reported. 88 call sites
(a line that puts several slots counts once), 39 with a visible wrong order.

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
- **The overlay** draws on ImGui's foreground list over the finished game image: every rectangle, then every bar, then
  every label by slot. It is not clipped to the 10..310 x 10..230 sprite scissor, a put lives 2 ImGui frames, and
  nothing is drawn while the port menu is open.

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
  Roulette). Every Remix string is in front of the cursor, so the overlay matches.
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

## Inventory

### 12-Character Battle character select

| Port call | Text | asm | Room, group | Wrong order |
|---|---|---|---|---|
| `gmremixtwelvecbhud.c:60` | "Stocks Remaining" header (160,126) centred, scale 0.875 | TwelveCharBattle.asm:5093 | 0x1E; 8 | fixed: layer (T15.2) |
| `gmremixtwelvecbhud.c:65` | stocks remaining number of P1 (x 124) and P2 (x 192), y 140, red/blue | TwelveCharBattle.asm:5094-5095 (Render.draw_number) | 0x1E; 8 (GROUP_ALWAYS) | fixed: layer (T15.2) |
| `gmremixtwelvecbhud.c:83` | "Character Set" header (160,160), only while the battle is not started | TwelveCharBattle.asm:5098 | 0x1E; 9 | fixed: layer (T15.2) |
| `gmremixtwelvecbhud.c:94` | character set name of each player (Default/Japanese/Polygon/Remix/Custom) at x 124 / 192 | TwelveCharBattle.asm:5099-5100 (Render.draw_string_pointer) | 0x1E; 9 | fixed: layer (T15.2) |
| `gmremixtwelvecbhud.c:109` | "Best Character" header (160,160), once the battle has started | TwelveCharBattle.asm:5109 | 0x1E; 10 | fixed: layer (T15.2) |
| `gmremixtwelvecbhud.c:118` | best character name of each player (x 124 / 192, y 174, max width 72) | TwelveCharBattle.asm:5110-5118 | 0x1E; 10 | fixed: layer (T15.2) |
| `gmremixtwelvecbhud.c:123` | TKO count of the best character of each player (x 124 / 192, y 187) | TwelveCharBattle.asm:5120-5121 (Render.draw_number) | 0x1E; 10 | fixed: layer (T15.2) |
| `mnremixtwelvecbcustomlegend.c:175` | Custom set legend texts: ": Set All" (41 or 247.5, 217), ": Presets" (105 or 173, 187) | TwelveCharBattle.asm:4730-4744 (Set All), 4774-4788 (Presets) | 0x1C; 0x16 | yes |
| `mnremixtwelvecbcustomlegend.c:181` | three 2x2 marks on the legend D-pad icons; y 188 yellow, 208 yellow, 216 | TwelveCharBattle.asm:4911, 4915, 4919 (Render.draw_rectangle) | 0x1C; 0x16 | yes |
| `mnremixtwelvecbstockmanual.c:121` | left blinking yellow 1x1 mark on the D-pad icon of the Manual stock controls | CharacterSelect.asm:5852-5861 (Render.draw_rectangle 1x1 YELLOW) | 0x38; 0x13 | yes |
| `mnremixtwelvecbstockmanual.c:123` | right blinking yellow 1x1 mark on the D-pad icon (digits_x - 20, 196) | CharacterSelect.asm:5864-5870 (Render.draw_rectangle 1x1 YELLOW) | 0x38; 0x13 | yes |

Wrong order cases (the `gmremixtwelvecbhud.c` ones describe the overlay; since T15.2 those texts are drawn by layers in
room 0x1E and the hand and the tokens pass over them):

- `gmremixtwelvecbhud.c:60`: A cursor hand (or the token it carries, or a dropped token) moved across the middle of the screen at y 126-140 is drawn over the header in Remix; in the port the header is drawn over the hand/token.
- `gmremixtwelvecbhud.c:65`: Cursor hand or token passing over (124 or 192, 140): Remix draws it over the number, the port draws the number over it.
- `gmremixtwelvecbhud.c:83`: Known case: the cursor hand over "Character Set" is covered by the text in the port (the player moves the hand here to press the set arrows at y 175).
- `gmremixtwelvecbhud.c:94`: The hand pressing the set arrows (y 175.5, x 91.5/153/159/221) sits over the set name; Remix shows the hand on top, the port shows the name on top of the hand.
- `gmremixtwelvecbhud.c:109`: Cursor hand or token crossing the middle column at y 160-175 is hidden behind the header in the port.
- `gmremixtwelvecbhud.c:118`: Cursor hand or token over the name: text on top in the port, hand on top in Remix.
- `gmremixtwelvecbhud.c:123`: The hand going to the Reset button (136,206) passes over the TKO numbers; the port draws the numbers over the hand.
- `mnremixtwelvecbcustomlegend.c:175`: The other player's hand or token (or a dropped token) crossing the legend at y 187-230 is drawn over it in Remix and under it in the port. The holder's own hand is over the portraits while the legend shows.
- `mnremixtwelvecbcustomlegend.c:181`: A hand or token of the other player over the D-pad icons shows the 2x2 marks on top of it in the port.
- `mnremixtwelvecbstockmanual.c:121`: A hand placed over the D-pad icon shows a 1 pixel yellow dot on top of it in the port; in Remix the hand covers the dot. One pixel, 11 frames of every 20.
- `mnremixtwelvecbstockmanual.c:123`: Same as the left mark: a 1 pixel dot drawn over a hand that sits on the D-pad icon.

### VS, Training, 1P and Bonus character selects

| Port call | Text | asm | Room, group | Wrong order |
|---|---|---|---|---|
| `mnplayersvs.c:1287` | stock number "N" at the right edge of the panel card of a player (y 191, scale 0.90625) | CharacterSelect.asm:5704-5718 | 0x38; 0x13 | yes |
| `mnplayersvs.c:1333` | stock number under the panel for Stock Mode Last / Manual | CharacterSelect.asm:5704-5718 | 0x38; 0x13 | yes |
| `mnremixbonusmodes.c:457` | "STAGE:" right aligned at (208,175), Bonus 1/2 with the Remix choice | Bonus.asm:113 | 0x22; 0x19 | yes |
| `mnremixbonusmodes.c:462` | ": Set Stage" (78.5,217) beside the L button, while a token is held over a fighter | Bonus.asm:129 | 0x22; 0x1A | yes |
| `mnremixbonusmodes.c:486` | bar behind the "Option" title: 144x5 at (175,136), colour 576088FF | Bonus.asm:26 (Render.draw_rectangle) | 0x22; 0x19 | yes |
| `mnremixbonusmodes.c:488` | "MODE:" right aligned at (208,156) | Bonus.asm:112 | 0x22; 0x19 | yes |
| `mnremixbonusmodes.c:490` | mode name "NORMAL" / "REMIX" centred at (249,156) | Bonus.asm:133 (Render.draw_string_pointer mode_pointer) | 0x22; 0x19 | yes |
| `mnremixcsshighscores.c:50` | "High Scores Disabled" (192,217), colour FF8080FF, scale 0.8125 | CharacterSelect.asm:4767 | 0x22; 0x1B | yes |
| `mnremixcssindicators.c:254` | 2x2 yellow mark on the variant D-pad icon of a panel, showing which D-pad direction the | CharacterSelect.asm:4655-4667 | 0x38, Render.create_room_ at; 0x13 | yes |
| `mnremixcsspanelmenutext.c:106` | panel menu item label (panel x + 4, panel y + 18 + 24*row), colour D0D0D0FF, scale 0.875 | CharacterSelectDebugMenu.asm:787-825 | the dl link of the panel card; the GObj link | yes |
| `mnremixcsspanelmenutext.c:110` | panel menu item value, centred on the panel (y + 30 + 24*row), white | CharacterSelectDebugMenu.asm:844-907 | dl link of the panel card: 28; GObj link of | yes |
| `mnremixcsspanelmenutext.c:142` | page number of the panel menu (x + W - 10.5, y + 4), only with more than one page | CharacterSelectDebugMenu.asm:1095-1121 | dl link of the panel card: 28; GObj link of | yes |
| `mnremixenemylabel.c:74` | "Enemy Player" (108,130), scale 0.5, blinking | SinglePlayerEnemy.asm:306 | 0x1E; 8 | yes |
| `mnremixenemylabel.c:76` | "Press Z" (108,140), scale 0.5, blinking | SinglePlayerEnemy.asm:309 | 0x1E; 8 | yes |

Wrong order cases:

- `mnplayersvs.c:1287`: A cursor hand (with or without a token) over the lower right of a panel card at y 191: Remix draws the hand over the stock number, the port draws the number over the hand. The player puts the hand exactly there to press the Manual stock arrows.
- `mnplayersvs.c:1333`: Hand over the number at the lower right of a panel (y 191), e.g. while pressing the Manual arrows: number drawn over the hand in the port.
- `mnremixbonusmodes.c:457`: Hand going to the STAGE arrows/icons passes over "STAGE:": text over the hand in the port.
- `mnremixbonusmodes.c:462`: Only if the hand with the token is carried down to (78-140, 217-228) while still counted as over a fighter; unlikely, since the text shows while the token hovers a portrait.
- `mnremixbonusmodes.c:486`: The port bar is drawn over the "Option" sprite and its outline (game objects in link 34) instead of behind them, and over a cursor hand or token passing at y 136-141, x 175-319. The first part is visible all the time on this screen (the bar crosses the Option title).
- `mnremixbonusmodes.c:488`: The hand pressing the MODE arrows (y 157) sits over "MODE:" / the mode name: text over the hand in the port.
- `mnremixbonusmodes.c:490`: Hand between the two MODE arrows (x 212-285, y 157) is covered by the mode name in the port.
- `mnremixcsshighscores.c:50`: The cursor hand or the token moved to the bottom right (x 192-300, y 217-230) is covered by the red text in the port; Remix draws both over it.
- `mnremixcssindicators.c:254`: The mark only shows while a token is held; if that hand (or another player's) is brought down over the panel's D-pad icon (y about 177-190) the 2x2 dot is drawn over hand and token in the port. Tiny.
- `mnremixcsspanelmenutext.c:106`: The menu is operated with the cursor: the hand pressing the item arrows or the page arrows lies over the labels and values and is covered by them in the port. A token dropped or dragged over the open menu is covered too.
- `mnremixcsspanelmenutext.c:110`: The hand pressing an item arrow (x + 2 or x + W - 5, y + 32 + 24*row) overlaps the value text; the port draws the value over the hand. A long value squeezed to W - 24 can also touch the arrows, which Remix draws over the text and the port under it.
- `mnremixcsspanelmenutext.c:142`: The hand pressing a page arrow sits on the page number between the two arrows; the number is drawn over the hand in the port.
- `mnremixenemylabel.c:74`: Cursor hand (or held token) moved over (108-170, 130-138): hand on top in Remix, text on top in the port. The port sprites of the same label (1P..4P) are game objects in link 34 (ENEMY_LABEL_DL_LINK), i.e. a different layer than the asm room 0x1E as well.
- `mnremixenemylabel.c:76`: Cursor hand over (108-150, 140-148): text over the hand in the port.

### Stage select

| Port call | Text | asm | Room, group | Wrong order |
|---|---|---|---|---|
| `mnremixstagepager.c:61` | "Page" label of the page strip | Stages.asm:2847 (Render.draw_string, setup_) | room 4; 3 | no |
| `mnremixstagepager.c:62` | page number next to "Page" | Stages.asm:2848 | room 4; 3 | no |
| `mnremixstagepanel.c:154` | "Hazards ( ):" / "Hazards:" label and its On/Off value | Stages.asm:2878-2879 (draw_string + draw_string_pointer) | room 2; 0xD | no |
| `mnremixstagepanel.c:158` | "Movement ( ):" / "Movement:" label and its On/Off value | Stages.asm:2880-2881 | room 2; 0xE | no |
| `mnremixstagepanel.c:171` | "Layout ( ):" label and the variant type | Stages.asm:2882-2883 | room 2; 0xC | no |
| `mnremixstagepanel.c:175` | "Roulette ( )" label over the RANDOM square | Stages.asm:2889 | room 2; 0xF | no |
| `mnremixstagepanel.c:191` | stage name centred at (230, 209) | Stages.asm:2851 | room 4; 3 | no |

No wrong order case.

### VS results and Match Stats

| Port call | Text | asm | Room, group | Wrong order |
|---|---|---|---|---|
| `gmremixtwelvecbhud.c:182` | "Game NN" under the results header (32,46) | TwelveCharBattle.asm:1946 | 0x1F; 0x16 | yes |
| `mnremixvsstatspages.c:249` | right aligned value (or "-") per active port of a row | VsStats.asm:432-457 | room 0x1F; page group | no |
| `mnremixvsstatspages.c:267` | striped background (0x40407040) of every second value row | VsStats.asm:394-401 | room 0x1F; page group | no |
| `mnremixvsstatspages.c:285` | row labels | VsStats.asm:405-419 (draw_line_ label) | room 0x1F; page group | no |
| `mnremixvsstatspages.c:294` | section and sub headers | VsStats.asm:405-419 (draw_line_ label, value address 0) | room 0x1F; page group | no |
| `mnremixvsstatspages.c:302` | white underline of a section header (callers 370, 386, 418, 425, 432, 437) | VsStats.asm:288-300 (draw_underline_, room at line 294) | room 0x1F; page group | no |
| `mnremixvsstatspages.c:326` | 1P..4P column headers | VsStats.asm:616-630 (draw_port_headers_, Render.draw_string_) | room 0x1F; 0x0E | no |
| `mnremixvsstatspages.c:328` | white 26x1 underline under each port header | VsStats.asm:633-643 (Render.draw_rectangle_) | room 0x1F; 0x0E | no |
| `mnremixvsstatsview.c:121` | ": Back" hint (B button) | VsStats.asm:958 | room 0x1F; 0x0E | no |
| `mnremixvsstatsview.c:123` | ": Next Page" hint (R button) | VsStats.asm:962 | room 0x1F; 0x0E | no |
| `mnremixvsstatsview.c:168` | same prompt while the stats page is open, drawn at alpha 0x4F | VsStats.asm:940 (object), 955 | room 0x1F; 0x0D | no |
| `mnremixvsstatsview.c:171` | "Press (A) for Match Stats" prompt, page closed | VsStats.asm:940 | room 0x1F; 0x0D | no |
| `mnvsresults.c:1261` | custom player tag name (black copy + coloured name) over the tag of each fighter on the | PlayerTag.asm:77-80 (results hook 0x80133E60), 229-262 | The temporary string uses; GObj link 18 | yes |
| `mnvsresults.c:3762` | "Game NN" under the mode header of the 12-Character Battle results | TwelveCharBattle.asm:1932-1947 | room 0x1F; 0x16 | yes |

Wrong order cases:

- `gmremixtwelvecbhud.c:182`: Mostly a lifetime difference: the asm creates "Game NN" with the results label at tic 120 (tic 1 only on No Contest), the port puts it from tic 1, so it shows 119 tics early, alone over the entry wipe.
- `mnvsresults.c:1261`: From tic 180 the tag sprite is darkened by the tint and the results rows are drawn over it, but the overlay name stays at full brightness and covers the row digits/arrows that cross it (tags sit at x 35..258, y 50..103, the same area as the rows).
- `mnvsresults.c:3762`: The port puts the text every frame from tic 1 (mnVSResultsFuncRun), so for the first 119 frames "Game NN" is shown alone, over the scene-entry transition wipe and before the mode header it belongs to; Remix shows it only together with the header.

### Title and Item Switch

| Port call | Text | asm | Room, group | Wrong order |
|---|---|---|---|---|
| `mnremixitemswitchlegend.c:52` | ": All On/Off" legend text next to the L button | Item.asm:969; called | room 3; 12 | no |
| `mnremixtitleversion.c:11` | "Smash Remix v2.0.1" at (160, 218), colour 0x888800FF | Boot.asm:160; called from | room 1; 3 | yes |

Wrong order cases:

- `mnremixtitleversion.c:11`: After Start/A on the title (or the demo timeout) the game image goes black for about 3 frames while the overlay keeps the yellow version text, because mnTitleFuncRun puts it before the sMNTitleIsProceedScene branch; in Remix the text is covered by the black fill. Possible second case, unverified: the opening slash passing over the text.

### Battle HUD

| Port call | Text | asm | Room, group | Wrong order |
|---|---|---|---|---|
| `ftremixinputdisplay.c:31` | Input Display of each port whose panel value is not Disabled: controller outline with | InputDisplay.asm 180 | 0x17; 0x0B | yes |
| `gmremixkothhud.c:108` | King of the Hill score meter of each active port: coloured fill bar (40x5) plus the white | KingOfTheHill.asm 1640-1657 | 0x17; 0xB | yes |
| `gmremixmusictitle.c:45` | translucent banner rectangle (1, 156, 320x28, 0x34343494) | BGM.asm:1033; hook at | room 0x17; 0xB | yes |
| `gmremixmusictitle.c:48` | track line "note title note" at (160, 158), scale 1.0 | BGM.asm:1076-1086 (Render.draw_string_, blur on) | room 0x17; 0xB | no |
| `gmremixmusictitle.c:50` | game line at (160, 172), scale 0.75 | BGM.asm:1095-1105 (Render.draw_string_, blur on) | room 0x17; 0xB | no |
| `ifremixplayertag.c:59` | Black 1.25x under-copy of the custom player tag name, centred over the fighter's tag arrow | PlayerTag.asm 112-161 | The room argument 0x17 is; 0xB | yes |
| `ifremixplayertag.c:62` | Custom player tag name in the tag colour, over the black copy | PlayerTag.asm 168-205 | Glyphs are SObjs of the; 0xB | yes |
| `sc1premixhrcrun.c:226` | Home-Run Contest distance number, right aligned at; slides to the | SinglePlayerModes.asm 2361 | 0x17; 0x0B | yes |
| `sc1premixhrcrun.c:228` | 'FT.' unit label at (282,47), scale 0.625, moving with the counter | SinglePlayerModes.asm 2365 | 0x17; 0x0B | yes |
| `sc1premixmultiman.c:188` | Multi-Man / Cruel KO counter number at (50,18), scale 0.9 (y 212 with the Dragon King HUD) | SinglePlayerModes.asm 2314 | 0x17; 0x0B | yes |

Wrong order cases:

- `ftremixinputdisplay.c:31`: The sample of a port stays active until the scene changes (remix_input_display_clear is only called from scmanager.c:1293), and DrawInputDisplays has no pause, Disable HUD or port-menu test. So the controllers stay on screen (frozen) during the pause, where Remix hides them with group 0xB and shows the pause decals and legends alone; they stay with Disable HUD = All; they are drawn over the port menu (file header: 'above the game image and the menus'; remix_hud_text skips drawing while the menu is open, remix_hud_text.cpp:225-227); and they are over GO! / GAME SET letters and the scene fade-in. Also over the remix_hud_text labels or under them depending only on GuiWindow registration order (port.cpp:988-989).
- `gmremixkothhud.c:108`: An item spawn arrow (or a stolen stock icon in a team stock match) passing over the meter row (y 188-193, or 16-21 with the Dragon King HUD) is drawn over the bar in Remix and under it in the port. Small and rare. Not an order case but same root: (1) the port stops putting the bar when game_status leaves Go, so the fill disappears at GAME SET / TIME UP while its native frame sprite stays, whereas the asm rectangles live until the scene ends; (2) with Disable HUD = All (Camera.asm 915-934 calls 0x80113F74 after GO, hiding every group 0xB object) Remix hides the meters, the port hides only the native frames (gmremixcamerapause.c:18-26) and keeps the overlay bars. The custom tag names (overlay labels) also draw over the bars in the port while in Remix the meters are over the tag object.
- `gmremixmusictitle.c:45`: Not seen in game: the banner rectangle is made after the HUD objects of scene start, so Remix greys what sits at y 156..184 under it, including the glyphs of a custom player tag; the overlay draws every rectangle under every label, so the tag name stays bright over the banner.
- `ifremixplayertag.c:59`: Results screen: from tic 180 (tic 30 on No Contest) the tint darkens the tag and, in Remix, the name with it; the port name stays at full brightness. In battle the name can cross the fixed HUD objects made later in link 23 (countdown, GO!, GAME SET, the music title banner).
- `sc1premixhrcrun.c:226`: Fade-in at match start: '0.0' is visible over the black fade for 12 frames. Visibility: hidden in Remix during the pause and with Disable HUD = All (group 0xB), still put by the port. The centred counter ends at about y 67-89, above the COMPLETE! letters (y 101), so those do not cross it.
- `sc1premixhrcrun.c:228`: Same as the distance number: visible over the 12-frame fade-in, and not hidden during the pause or with Disable HUD = All.
- `sc1premixmultiman.c:188`: For the first 12 frames of the match the black fade-in covers the whole HUD in Remix; the port shows the '0' at full brightness over the black screen (the native icon and X fade in). Worse, as visibility: group 0xB is hidden by the vanilla pause (ifCommonInterfaceSetGObjFlagsAll(GOBJ_FLAG_HIDDEN), ifcommon.c:3439) and by Disable HUD = All after GO / after unpause (Camera.asm 890-905 and 915-934), so Remix hides the counter with the rest of the HUD; the port hides the icon and X sprites but keeps putting the number, which is left alone on the pause screen.

### Pause

| Port call | Text | asm | Room, group | Wrong order |
|---|---|---|---|---|
| `ifremixgentlemensreset.c:144` | 'Game reset requested by NP.' centred at (160,126) | TwelveCharBattle.asm 3584 | 0x18; 0xE | no |
| `ifremixgentlemensreset.c:146` | 'If accepted, stock counts will be reset.' centred at (160,144) | TwelveCharBattle.asm 3585 | 0x18; 0xE | no |
| `ifremixgentlemensreset.c:148` | 'Accept:' at (40,164), followed by the A+B+Z+R combo sprites (native) | TwelveCharBattle.asm 3586 | 0x18; 0xE | no |
| `ifremixgentlemensreset.c:150` | 'Decline:' at (188,164), followed by the START button sprite (native) | TwelveCharBattle.asm 3608 | 0x18; 0xE | no |
| `ifremixpauselegend.c:243` | Live track title of the pause music legend at (52,74), max width 228 | Pause.asm 295-300 (Render.draw_string(0x18, 0xF, ...) + | 0x18; 0xF | no |
| `ifremixpauselegend.c:245` | Live game title of the pause music legend at (52,90), scale 0.75 | Pause.asm 305-312 | 0x18; 0xF | no |
| `ifremixpauselegend.c:267` | Pause legend text rows: music note, ': Next Music Track', ': Random Music Track' | Pause.asm 279; 322, 324, 327, 329 | 0x18; 0xF (music) and | no |
| `ifremixpauselegend.c:271` | Two yellow 2x2 squares marking the D-pad direction on the music legend rows, at (51,113) | Pause.asm 315 and 318 | 0x18; 0xF | no |

No wrong order case.

### Training

| Port call | Text | asm | Room, group | Wrong order |
|---|---|---|---|---|
| `sc1ptrainingremix.c:301` | "Reset Count:" label (x 99.5, y 25) | Training.asm 1617 (Render.draw_string reset_string) | 0x17; 0x15 | no |
| `sc1ptrainingremix.c:303` | Reset Count value (right aligned at x 221, y 25) | Training.asm 1618 (Render.draw_number reset_counter) | 0x17; 0x15 | no |
| `sc1ptrainingremixaction.c:181` | Show Action: action name of the fighter of each port (x 20, y 55 + 26*port, scale 0.75) | Training.asm 1547 | room passed in a2: 0x17; 0x17 | yes |
| `sc1ptrainingremixaction.c:185` | Show Action: frame count of the action of each port (x 20, y 66 + 26*port, scale 0.75) | Training.asm 1515 | same as the action name: 0x17; 0x17 | yes |
| `sc1ptrainingremixaction.c:208` | "Show" (first word of the L legend, x 20, y 27.25, scale 0.875) | Training.asm 1636; L glyph | 0x17; 0x15 | no |
| `sc1ptrainingremixaction.c:211` | "Action" (second word of the L legend, x 20, y 38.5, scale 0.875) | Training.asm 1637 (Render.draw_string action_legend_2) | 0x17; 0x15 | no |
| `sc1ptrainingremixexit.c:118` | "(Hold" of the "(Hold --)" Exit hint (x 169, y 166, scale 0.875) | Training.asm 1597; A glyph | 0x17; 0xF | no |
| `sc1ptrainingremixexit.c:120` | ")" closing the "(Hold --)" hint, right aligned at the end of the measured full string | Training.asm 1597 (same string as the opening part) | 0x17; 0xF | no |
| `sc1ptrainingremixexit.c:141` | Hold to Exit progress bar (x 73, y 179, 2 px high, width grows to 64) | Training.asm 1600-1613 | 0x17; 0xF | no |
| `sc1ptrainingremixlegend.c:120` | "Press; the Z glyph is a game sprite | Training.asm 1583; Z glyph 1584 | 0x17; 0xE | no |
| `sc1ptrainingremixlegend.c:131` | Yellow 2x2 square marking the D-pad direction of each legend line | Training.asm 1625-1628 (Render.draw_rectangle x4) | 0x17; 0x19 | no |
| `sc1ptrainingremixlegend.c:134` | D-pad legend texts "Toggle Pause", "Quick Reset", "Frame Advance", "Model Display" | Training.asm 1629-1632 | 0x17; 0x19 | no |
| `sc1ptrainingremixmenudraw.c:159` | Z menu row value shown as a number (right aligned at x 252) | Menu.asm 395-428 | 0x17; 0x16 | no |
| `sc1ptrainingremixmenudraw.c:163` | Z menu row value shown as text | Menu.asm 412-428; max | 0x17; 0x16 | no |
| `sc1ptrainingremixmenudraw.c:181` | Z menu row label (x 76, y 50 + 14*row, squeezed to 150 px) | Menu.asm 354-377; max width | 0x17; 0x16 | no |
| `sc1ptrainingremixmenudraw.c:189` | "/" between the R and Z glyphs of the pagination legend (x 288, y 16) | Training.asm 1641 (Render.draw_string pagination_legend_1 + 4) | 0x17; 0x16 | no |
| `sc1ptrainingremixmenudraw.c:192` | "Next/" of the pagination legend (x 272, y 27.25) | Training.asm 1643 (Render.draw_string pagination_legend_1) | 0x17; 0x16 | no |
| `sc1ptrainingremixmenudraw.c:195` | "Prev" of the pagination legend (x 272, y 38.5) | Training.asm 1644 (Render.draw_string pagination_legend_2) | 0x17; 0x16 | no |
| `sc1ptrainingremixmenudraw.c:205` | Z menu cursor square (4x4 red, x 70, row y + 5) | Menu.asm 662-679 (Render.draw_rectangle_ at 676), moved by | 0x17; 0x16 | no |
| `sc1ptrainingremixmenudraw.c:208` | Z menu selection underline (182 x 1 red, x 70, row y + 11) | Menu.asm 681-695 (Render.draw_rectangle_ at 693) | 0x17; 0x16 | no |
| `sc1ptrainingremixmenudraw.c:222` | Clears the cursor square (zero-size rectangle) on the frame the Z menu closes | none found as a draw call: the asm hides group 0x16 | n/a; 0x16 | no |
| `sc1ptrainingremixmenudraw.c:223` | Clears the selection underline (zero-size rectangle) on the frame the Z menu closes | none found as a draw call: the asm hides group 0x16 | n/a; 0x16 | no |

Wrong order cases:

- `sc1ptrainingremixaction.c:181`: Pause with Show Action on and the vanilla menu up (Z menu closed): a long action name starting at x 20 runs past x 68 at y 55/81/107/133 and in the port is drawn over the blue menu fill, the frame, the vanilla option sprites and the "Press Z for Custom Menu" prompt (y 50, same band as the port 1 row at y 55); Remix draws it under all of them, tinted by the translucent fill. The Z-menu-open case is already hidden by the workaround. Also at scene start (every Reset or Quick Reset with Show Action on): Remix draws the row under the 12-frame black fade-in, the port draws it at full white over the fade. When the Z menu opens the rows linger for up to 2 frames over the menu, because the skip does not hide the slots.
- `sc1ptrainingremixaction.c:185`: Same state as the name row, but the number is at most a few digits at x 20 (ends well before x 66), so it only crosses the left magnify bubble of an off-screen fighter frozen at the left edge during pause; practically never over the menu. Hidden together with the name while the Z menu is open. Also at scene start (every Reset or Quick Reset with Show Action on): Remix draws the row under the 12-frame black fade-in, the port draws it at full white over the fade. When the Z menu opens the rows linger for up to 2 frames over the menu, because the skip does not hide the slots.

## Other findings of the audit

Not layer order, found while reading; each is checked and fixed under T15.3 with the screen it belongs to.

- **"Game NN" on the 12CB results** is put from tic 1; the asm creates it with the results label at tic 120.
- **12CB Custom legend**: the asm hides the legend and its marks while the panel menu of the token holder is open
  (`set_panel_display_`, `_custom_portrait_indicators`); the port keeps putting them.
- **Training action rows**: the port hides them only with the Z menu open; with the vanilla pause menu open a long
  action name crosses the menu (the asm moves the rows to room 0x39, under it). The existing `deviations.md` row covers
  the Z menu case only.
- **Item Switch legend sprites** are made on display link 4, the camera drawn first; the asm uses room 3, the
  front-most.
- **Stage select pager sprites** use display link 2 where the asm uses room 4; no visible effect found.
- **Input Display** (`ftremixinputdisplay.c`) is a second ImGui path over the game image; the asm draws it as sprites
  in room 0x17 after the Training objects.
- **Music title banner**: the asm rectangle is clipped by the interface camera viewport, the overlay one is not.

## Input for T15.2

- The asm string is a GObj with `lbCommonDrawSObjAttr` and one SObj per glyph, made from the Sprite already in the
  font file (0xA15, the `reloc_misc/file_2581` the port decodes for ImGui today); the port already draws Remix sprites
  this way (`ifRemixSpriteMakeFromFile`).
- Chosen direction: keep the `remix_hud_text` API and slots; each owner binds its slot range to a layer, a GObj in the
  display link of the asm whose display function draws the rectangles, bars and glyphs with the game's sprite
  functions. The put calls do not change, and slots with no layer stay on the overlay, so the screens move one family
  at a time.
- To settle while implementing: larger display list buffers for the results and the character selects (about 11
  commands per glyph, up to 80 labels on a Match Stats page); a sprite camera for the links that have none (22, 0x37,
  0x38, 0x39); text near the border is clipped by the 10..310 x 10..230 sprite scissor, as on the console; the life of
  a put counted in game tics instead of ImGui frames.

## Draw path (T15.2)

How a slot gets drawn by the game instead of the overlay. Built and tested on one screen, the 12-Character Battle
character select; the other screens move in T15.3.

- **Layer.** `ifRemixTextLayerMake(gobj_link, dl_link, dl_priority)` (`decomp/src/if/ifremixtextlayer.{c,h}`) makes one
  GObj in the group and the room of the asm. Its display function draws the slots bound to it with the game renderer:
  rectangles, then bars (fill and white flash), then labels in slot order, the order of the overlay.
- **Binding.** The owner calls `ifRemixTextLayerBindLabels`, `ifRemixTextLayerBindRects` or `ifRemixTextLayerBindBars`
  with a slot range, once, at the end of the scene setup (as `Render.setup_` runs after it), and keeps its
  `remix_hud_text` puts as they are. A bound slot is skipped by the overlay; a slot with no layer stays on it. A slot
  has one layer and a scene has at most 8 (`REMIX_HUD_LAYERS_MAX`); a refused bind goes to the log and the slots stay
  on the overlay. `remix_hud_text_clear` forgets layers and routes at every scene change.
- **Glyphs.** The font file 0xA15 is loaded into the scene heap by `ifRemixSpriteGetFile`, as the other Remix sprite
  files. Each glyph is the Sprite of the file (16 x 16, IA 8-bit, one bitmap, attribute 0x0201; checked on the 96
  glyphs of the `.o2r`) drawn through `lbCommonPrepSObjAttr` and `lbCommonPrepSObjDraw` from an SObj that lives for the
  call, with the position, scale and colour of the glyph. No object is made or freed per frame.
- **Layout.** `remix_hud_layer_place_glyphs` (`port/remix/remix_hud_layer_rules.{c,h}`, unit tests in
  `tests/remix/test_hud_layer_rules.c`) gives x, y and the two scales of every glyph from the arguments of the put,
  with the advance, fit and aligned width rules of `remix_font_layout`. The route table and the life of a put are in
  the same file; `port/remix/remix_hud_layer.h` is the interface between the slot store and the layer object.
- **Rectangles and bars** are filled as `Render.draw_rectangle_` with alpha: primitive colour, `G_RM_CLD_SURF`, whole
  pixels (the corner and the size truncated one by one, cut to the 320 x 240 frame; the overlay keeps fractions), and
  `lbCommonClearExternSpriteParams` after them.
- **Life of a put.** A bound slot is stamped with the game tic count (`dSYTaskmanUpdateCount`) and is drawn while the
  stamp is at most one frame interval old, so it shows in the frame that follows the put and is gone in the next one
  when the owner stops putting it. `remix_hud_text_hide` still hides at once.
- **Testing.** `SSB64_REMIX_TEXT_OVERLAY=1` makes no layer, so every slot is drawn by the overlay; the same input run
  with and without it gives the two images to compare.

Measured on the 12CB character select, window 1100 x 734, Stretch, Center 4:3 and widescreen off: every text sits on
the same window pixel in both paths (no shift at 3.3 window pixels per N64 pixel). The look differs as the asm does
from the overlay: the game filters the glyph and multiplies its colour by the intensity of the IA texel, so the edges
are softer and darker than the overlay's, which used the alpha alone with no filter.

Limits found, for T15.3:

- **Display list buffer.** A glyph costs about 13 `Gfx`. The 12CB character select peaks at 1987 of the 2750 `Gfx` of
  `dMNPlayersVSTaskmanSetup` with its about 50 glyphs, so that buffer was not raised. A screen with more text needs
  the room counted first (four open panel menus are about 240 glyphs, a Match Stats page up to 80 labels); an overflow
  only writes `DLBuffer OVERFLOW` to the log.
- **File table.** `ifRemixSpriteGetFile` holds 8 files per scene; the font takes one (the 12CB select uses 1 of 8).
- **Sprite camera.** The display link must be drawn by a camera whose display function is `lbCommonDrawSprite`; it
  sets the sprite state and the scissor the glyphs need. A link without one (22 in the battle, and the
  Remix rooms 0x37, 0x38 and 0x39 wherever the scene has no camera for them) needs a camera first, as
  `mnRemixVSStatsSceneMakeCamera` makes one.
- **Clipping.** The glyphs are cut at the camera viewport, at most 10..310 x 10..230, by the sprite code itself; a
  rectangle is cut only by the RDP scissor the camera leaves set (not measured), never by the sprite scissor.
- **Font metrics.** A layer needs the width table of the Remix RAM image and the font file; when either is missing
  the make returns NULL and the text stays on the overlay.
- **Port menu.** The overlay hides its text while the port menu is open; layer text is part of the game image and
  stays under the menu.
