## Highlights

- **All platforms: a match no longer shows only the stage backdrop.** Now and then a match was drawn with the backdrop
  and nothing else (no stage, fighters or HUD) for its whole length, depending on where the system placed the game's
  memory. The renderer took a valid list of drawing commands for a runaway one and dropped the rest of every frame.
- **Charge Smashes work** ([#8](https://github.com/lucasliet/BattleShip-remix/issues/8)). With the setting on, holding
  A on a smash attack never started the charge. Now the fighter freezes, shakes and flashes yellow while A is held, the
  charge ends after one second (never with UNLIMITED CHARGE), and the hit does up to 40% more damage, as in Smash
  Remix. The Beam Sword, Fan and Star Rod smashes charge too; the Home-Run Bat does not, as in Smash Remix.
- **Giant and tiny fighters deal 1.3x and 0.7x damage with every attack.** Smash Remix scales the damage of every
  hitbox of a fighter whose size was changed (the size option of the CSS panel, Super and Poison Mushrooms). The port
  only scaled a few attacks, so most hits of a giant or tiny fighter did normal damage; now they all follow the size.
- **Special Zoom: the background behind the zoom is animated**
  ([#8](https://github.com/lucasliet/BattleShip-remix/issues/8)). It stayed a flat purple screen; it now plays the
  impact flash of Smash Remix, from purple to the ray burst to white, and covers the whole picture with widescreen on.
  The magnifying glass is hidden during the zoom and comes back after it, also when the game is paused in the middle
  of a zoom, and in Training the camera keeps following during the slow motion.
- **The Combo Meter shows up.** With the setting on (the default in most profiles), no COMBO counter ever appeared in
  VS or 1P matches, so the 20-hit and Pokemon combo calls never played and the combo rows of the match stats stayed
  at 0. It now sits above the damage of the attacker, also in sudden death.
- **Remix settings checked one by one against Smash Remix.** Every setting was compared with the original code and
  the differences were fixed:
  - Japanese Sounds now covers punches and kicks.
  - Footstool works with no jump left.
  - Fast Fall Aerials lets you fast fall during an aerial.
  - Crates set to always explode no longer play the open sound.
  - Z-Cancel DISABLED also applies to the CPUs.
  - Camera Mode works on Planet Zebes and Mushroom Kingdom and no longer carries into the next match.
  - Rainbow Road's Chain Chomp follows Hazard Mode.
  - Random Select filters the 12-Character Battle and Tag Team random picks.
  - L random rolls again after a pick.
  - Jigglypuff's Sing ring is centred.
  - Perfect Shield reflects only projectiles that would bounce off the shield.
  - Random menu music follows each screen.
  - Several CPU recovery details match the original.

  Where Smash Remix itself has a bug, the port keeps the corrected behaviour.
- **Stages, modes, items and fighters checked against Smash Remix.** The same comparison was run on the rest of the
  ported game logic, and about 150 differences were fixed. Some of them:
  - Pirate Land's cannon moves and fires with its flash; Rainbow Road's Chain Chomp charges down the track.
  - The animated backgrounds of 27 stages play; stages use their own wallpaper kind.
  - Great Bay, Twilight City and Pirate Land water no longer slows a jump out of it.
  - King of the Hill: the win countdown resets while the hill is empty or contested, each full bar flashes on its own.
  - Tag Team: team costumes and stock steals follow the character select.
  - Training: D-pad shortcuts work while paused, frame advance works at every game speed, a KO respawns at 0%.
  - Home-Run Contest: camera speed, the record is saved only on a completed attempt, the Sandbag takes no input.
  - Items: Franklin Badge reflects with its sound and effect, Mr. Saturn no longer vanishes on landing, the Waddle
    Doo beam works, mushrooms picked one after the other, Cloaking Device opacity.
  - Fighters: Peach floats out of a double jump, Marina's air jumps get shorter, Kirby's copies of Dedede, Lanky and
    Crash, Wolf and Slippy reflect speed, Wario's Body Slam clang, Mad Piano's bonus books.
  - Character select: stock counter and Manual stock arrows in VS, token pick-up area, team costume kept with C.
  - Pause and results: extra pause controls on every pause type, Match Stats underlines and Highest Taken, Player Tag
    outline.
  - Announcer calls (Long KO, Clang, Close match) and the level 10 CPU in a few situations.
  - CPUs avoid the tornado on the Hyrule Castle variants and leave the dropping platforms of Big Boo's Haunt,
    Yoshi's Island II and Great Bay.
  - Stage select: the random stage does not repeat the last one, page switches play their sound.
  - 12-Character Battle character select: J/E flags, leaving with B keeps the battle.
  - Special Zoom also runs in 1P; a Game Over or pause exit no longer resets your Size setting.
- **Windows: a clear message when Windows blocks the ROM extractor.** When Smart App Control or another application
  control policy stops `torch.exe` on the first launch, the error now says so instead of only showing a code.
- **Packaging.** The Linux AppImage uses the Smash Remix icon instead of the vanilla one, the Windows zip no longer
  carries a loose vanilla icon file (the game keeps its icon), and the Switch zip only carries the fonts the game reads.

Nothing changed in the asset pipeline: on the Switch, keep the `BattleShip-Remix.o2r` and `.recipe` you already have.
Switch builds up to `v2.0.1-port.9` still download the bare `BattleShip-Remix.nro`, so that file stays in this release.

## Install

No game data is included: you need your own ROM.

- `baserom.remix.z64`: Smash Remix 2.0.1, made from your Super Smash Bros. NTSC-U v1.0 ROM with the
  [Smash Remix web patcher](https://smashremix.net/patcher/) (in the browser, nothing to install) or the Remix 2.0.1
  patch from the [Smash Remix releases](https://github.com/JSsixtyfour/smashremix/releases) (ROM SHA-1 `13716381ed5e29be2606e0f4724b18fd00789d04`; `.z64`, `.n64` or `.v64`).

**macOS, Linux, Windows:** run the app. On the first launch it asks for the ROM and extracts `BattleShip-Remix.o2r`
into its app data folder. The macOS app is not notarized: the first time, right-click it and choose Open (or run
`xattr -dr com.apple.quarantine /Applications/BattleShip-Remix.app`). On macOS and Linux later updates install from
inside the game; on Windows the game tells you about them and you extract the new zip into the game folder.

**Nintendo Switch:** the console does not read the ROM. Run a desktop build once to extract `BattleShip-Remix.o2r`
and `BattleShip-Remix.o2r.recipe`, unzip `BattleShip-Remix-switch.zip` to the root of the SD card, copy the two files
next to `BattleShip-Remix.nro` in `sdmc:/switch/BattleShip-Remix/`, and launch it in full-memory mode (hold R while
opening an installed game). Both files must come from a desktop build of the same release as the `.nro`: the game
refuses to start if the `.recipe` is missing or does not match, and the error screen shows both recipes. When a release
changes the asset pipeline, copy the newly extracted `.o2r` and `.recipe` again.

**Android (arm64):** install `BattleShip-Remix-android.apk` (allow installs from unknown sources), open it and pick the
Smash Remix 2.0.1 ROM; it extracts `BattleShip-Remix.o2r` on the phone. It installs next to the vanilla BattleShip app.
32-bit Android (armeabi-v7a) is not shipped yet.

**PlayStation 5 (homebrew):** needs a jailbroken PS5 with ShadowMountPlus. Unzip
`BattleShip-Remix-ps5-PPSA99264.zip` to `/data/homebrew/` (or the same folder on an external drive), so the game sits
in `PPSA99264/`, copy your ROM into that folder as `baserom.remix.z64` (by FTP), and open BattleShip Remix from the
Games row. The first launch extracts `BattleShip-Remix.o2r` into the same folder (a system notification tells when it
starts and ends); saves, settings and logs stay there too. L3+R3 opens the port menu (the Create button belongs to the
system).

## Files

| File | Platform |
|---|---|
| `BattleShip-Remix.dmg` | macOS (Apple Silicon) |
| `BattleShip-Remix-macos.zip` | macOS (Apple Silicon), the `.app` zipped; the in-game updater downloads it |
| `BattleShip-Remix-x86_64.AppImage` | Linux x86_64 |
| `BattleShip-Remix-windows-x64.zip` | Windows x64 |
| `BattleShip-Remix-switch.zip` | Nintendo Switch homebrew (`.nro`, `f3d.o2r`, `config.yml`, the fonts in `assets/custom/fonts/`) |
| `BattleShip-Remix.nro` | The Switch `.nro` alone, kept for builds up to `v2.0.1-port.9` (their updater downloads it); newer builds update from the Switch zip |
| `BattleShip-Remix-android.apk` | Android arm64-v8a, Android 7+ |
| `BattleShip-Remix-ps5-PPSA99264.zip` | PlayStation 5 homebrew title folder (`PPSA99264/`) |
| `SHA256SUMS.txt` | Checksums |

Report bugs on [this repository's issue tracker](https://github.com/lucasliet/BattleShip-remix/issues), not on
BattleShip, Smash Remix or Harbour Masters.
