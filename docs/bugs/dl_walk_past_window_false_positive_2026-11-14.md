# A match shows only the backdrop: a list in the heap after a registered range taken for a runaway walk

Code: `libultraship/src/fast/interpreter.cpp` (`SteppedOffRegisteredRange`, `WalkBoundsCheck`, used by `gfx_step`),
`port/port_dl_ranges.cpp` (`port_dl_range_lookup`, `kWalkPastWindow`), test `tests/dl_ranges/`.

## Symptom

Seen once on Linux in 5 runs of the v2.0.1-port.10 AppImage (Crash vs Kirby, Final Destination): stage, fighters and
HUD missing for the whole match, backdrop drawn. The engine log repeats, with the same address on every frame:

```
gfx_step: cmd 0x55b69ca923f0 walked past registered DL range — stopping entire walk
```

The walker code is the same in port.10, port.12 and HEAD (same libultraship commit, no change in
`port/port_dl_ranges.*`). A Windows report on v2.0.1-port.12 describes the same picture ("characters are entirely
missing, it just shows the skybox and nothing else"), without the engine log; see "Not covered".

## Cause

`port_dl_range_lookup` answers `PORT_DL_WALKED_PAST` for any address up to 64 KiB (`kWalkPastWindow`) after the end of
a registered range (the scene arena, the reloc ranges), and `gfx_step` stopped the whole walk of the frame on that
answer. The window is general heap. The widened copies of packed display lists (`portNormalizeDisplayListPointer`)
are `std::vector`s there and are not registered; static lists and mod buffers are outside the ranges too. When the
allocator places one inside the window, the first command of a legitimate list reads as a runaway walk, and because
the copy is cached for the scene the frame is cut at the same point every frame. What was drawn before it (the
backdrop) stays.

The placement depends on the allocator, so the bug is intermittent on Linux, where the copies usually sit megabytes
after the arena.

## Fix

A walk that runs off a range lands on the commands right after its end, so `gfx_step` now stops only there
(`SteppedOffRegisteredRange`: walked past, less than three commands after the range end, and not inside a widened
copy). Three, because the walker steps over `G_TEXRECT` and `G_TEXRECTFLIP` in one go and they take three slots. The
rule itself is `Fast::DLWalkSteppedOffRange` in `fast/dl_walk_bounds.h`. The lookup returns the walked-past span
starting at the end of the range, which gives the walker that distance and lets it cache the answer like the others.
The G_DL push check keeps the whole 64 KiB window: it sees computed addresses before they are followed.

## Verification

- Unit test of the lookup spans and of the walk-off rule: `cmake --build <build> --target run_dl_ranges_tests`.
- False positive, temporary `kWalkPastWindow = (size_t)1 << 46` (every address after a range is in the window),
  `SSB64_START_SCENE=22 SSB64_VS_FKINDS=72,8 SSB64_VS_STAGE=16`: before the fix the screenshot at frame 500 has the
  backdrop only and the log 21 "walked past" lines; the dump shows the rejected command as the widened copy
  (`callee=scene_arena+...`, `norm=<heap address>`). After the fix a four-fighter match is drawn in full, no warning,
  `dl walk 0.7 ms`. Run again with the final three-command rule: full match, no warning.
- Real walk off a range, temporary registration of only the first 0x33F0 bytes of the arena: the root list crosses
  that end and the walk is stopped with the warning, as before.
- Normal build: Crash vs Kirby with neutral specials, a four-fighter match and the CSS flow, no "walked past" or
  "rejecting DL push" line.

## Not covered

- The Windows report is not confirmed as this bug: it needs `logs\BattleShip-Remix.log` with the "walked past" line.
  Large allocations on Windows start 64 KiB apart, which makes a list right after the arena less likely there.
- The push check still rejects a host pointer to a legitimate list inside the window (only that sub-list is lost).
- The "walked past registered DL range" stutter listed for the Switch in `docs/remix/ROADMAP.md` may be the same
  family; not retested.
