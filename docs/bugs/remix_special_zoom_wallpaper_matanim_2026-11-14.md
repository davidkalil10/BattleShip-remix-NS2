# Remix: Special Zoom background stayed flat purple, and the zoom changed the wrong magnifier byte

Code: `decomp/src/gm/gmremixspecialzoomwallpaper.c` (`ftRemixSpecialZoomWallpaperShow`),
`decomp/src/gm/gmremixspecialzoom.c` (`ftRemixSpecialZoomApply`, `ftRemixSpecialZoomCameraZoomIn`,
`ftRemixSpecialZoomEnd`, `ftRemixSpecialZoomSlowMoCameraStep`).

## Symptom

Issue #8 (PS5, v2.0.1-port.12): with Special Zoom set to ANY KO the zoom "is glitchy and has a purple background".
Reproduced on Linux in Training: the background behind the zoomed fighters is a flat purple plane for the whole zoom.

An audit of the feature against `SpecialZoom.asm` found three more differences.

## Causes and fixes

1. **Background animation.** `create_gfx` is a clone of the vanilla `mvOpeningRunMakeCrash`
   (`mv/mvopening/mvopeningrun.c`): it makes the object from file 0x4B (`MVOpeningRunCrash`) and calls 0x8000BE28
   with offset 0x3700, which is `gcAddMatAnimJointAll`; 0x3700 is the material animation table of that file. The
   port called `gcAddAnimJointAll` with the same pointer, so the texture stayed on its first frame (the purple
   palette). The address is identified by the cloned routine, the type of the table and its place between
   `gcAddAnimJointAll` (0x8000BD8C) and `gcAddAnimAll` (0x8000BED8); the repository has no symbol map naming it.
2. **Magnifier.** The asm saves, clears and restores `gPlayerCommonInterface.is_ifmagnify_display` (byte 0x1580,
   asm 834-839, 897-900, 1108). The port used the next byte, `magnify_mode`. The magnifier bubble was not hidden
   during the zoom, and a pause taken during a zoom could leave it off until the match ended. The three sites now
   use `is_magnify_display`.
3. **Widescreen.** The plane is authored for 4:3 and the port's widescreen compresses clip-space X, so the stage
   showed at both sides during the zoom. The plane's X scale is divided by `port_widescreen_clip_x_scale()`
   (`docs/remix/deviations.md` 5.10).
4. **Training camera.** In Training `objects_update` branches straight to `_update_camera` (asm 486-520); the port
   applied the Camera Mode and Time Twister block there too. The block now only applies outside Training.

## Verification

`SSB64_REMIX_TOGGLES="Special Zoom (BETA)=2" SSB64_TRAIN_FKINDS=0,8 SSB64_VS_STAGE=6
SSB64_REMIX_TRAINING="Percent2=300" scene.sh 54 <input: jump to the platform, forward smash>`, screenshots over the
zoom:

- before: flat purple from the hit to the end of the zoom, with the stage visible at both sides;
- after: purple, then the ray burst, then almost white before the stage comes back, across the whole wide frame.
  No "stale token" or "UNHANDLED" line in the log;
- Flash Guard on: the zoom runs with no background;
- pause during the zoom in Training: the menu opens over the running background and the match resumes;
- two zooms in a row (Training quick reset with D-pad left after the first, then the same smash): the second zoom
  plays the whole background sequence like the first;
- quick reset during a zoom: the zoom ends, the scene resets with no leftover background, and a new zoom plays;
- Training with Camera Mode FIXED: the camera keeps the fixed wide view during the zoom (the camera function runs
  every frame in Training, as at asm 493-494), so the fighters stay small over the background.

Magnifier, in a VS match (`SSB64_REMIX_PANEL="Damage.2=300"`, forward smash, START during the zoom, START again 74
frames later) under gdb with a watchpoint on both bytes: `ftRemixSpecialZoomApply` clears `is_magnify_display`,
`ftRemixSpecialZoomEnd` sets it again, and the zoom no longer writes `magnify_mode`. The screenshot taken during the
pause shows the pause frame over the zoom.

## Not covered

- Stamina: the asm turns the zoom off on every screen when the VS rule is Stamina; the port only does it in a
  Stamina battle (`docs/remix/deviations.md` 5.10).
