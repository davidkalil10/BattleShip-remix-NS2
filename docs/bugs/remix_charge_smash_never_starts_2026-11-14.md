# Remix: Charge Smashes did nothing, and the charged hit, the Size damage and the charge flash were wrong

Code: `decomp/src/ft/ftanimend.c` (`ftAnimEndSetWait`), `decomp/src/ft/ftmain.c` (`ftMainParseMotionEvent`, the
`MakeAttackColl` damage store), `decomp/src/ft/ftremix/ftremixchargesmash.c` (`ftRemixChargeSmashLogic`,
`ftRemixChargeSmashSetGfx`, `ftRemixChargeSmashBegin`).

## Symptom

Issue #8 (PS5, v2.0.1-port.12): Charge Smashes set to ON in the Remix gameplay settings "still does nothing". The
cause is game code shared by every platform. Reproduced on Linux with Fox holding A through a forward, up and down
smash: the attacks had their vanilla length.

An audit of the whole feature against `ChargeSmashAttacks.asm` and `Size.asm` then found three more differences, all
hidden by the first one.

## Causes and fixes

1. **The charge never started.** `smash_attack_main` replaces the `ftAnimEndCheckSetStatus` call of
   `ftAnimEndSetWait`. With the animation running it calls the vanilla check and, when the wait transition is not
   taken, falls from `_normal` into `_charging` and runs `smash_attack_logic`: the logic runs on every frame of the
   attack, which is how it reaches the charge start frame. The port only took the `_charging` branch with animation
   speed 0 and otherwise ran the logic once, at the end of `ftMainSetStatus`, where the frame is 1; every start frame
   in the tables is 2 or more. `ftAnimEndSetWait` now mirrors the asm.
2. **Damage was scaled on the wrong motion event.** `Size.asm` `fix_damage_` patches 0x800DF34C. In the vanilla ROM
   that is the damage store of the hitbox creation (`sll 0x13`, `srl 0x18`: the `MakeAttack1` damage field, right
   after the joint store); the `SetAttackCollDamage` case is at 0x800DF63C and has no patch. The port scaled
   `SetAttackCollDamage` and left the created hitbox alone, so a charged smash did vanilla damage, and a Giant or
   Tiny fighter (Size) did vanilla damage with almost every attack. The scaling moved to the `MakeAttackColl` store;
   `SetAttackCollDamage` is vanilla again.
3. **The yellow charge flash never played.** The asm tests and writes the fighter's colour script pointer (0xA28,
   `colanim.cs[0].p_script`) with the address of `GFXRoutine.CHARGE_SMASH`. The port wrote the routine id 0x78 into
   `colanim_id`, which starts no script and blocked lower-priority colour animations until the next status.
   `ftRemixChargeSmashSetGfx` now writes the script pointer of routine 0x78.
4. **Charge sound.** `FGM.play(0x528)` is the direct sound call; the port went through `syAudioPlayFGM`, which needs
   a free tracked slot. It now calls `func_800269C0_275C0` like the other Remix ports of `FGM.play`.

## Verification

Smash length, `SSB64_REMIX_TOGGLES="Charge Smashes=<v>" SSB64_VS_STAGE=16 SSB64_VS_P2_HUMAN=1
SSB64_REMIX_FIGHTER_LOG=1 vs.sh 1,8 <input: stick + A held 180 frames for a forward, an up and a down smash>`:

| Value | Forward smash (0xCC) | Up smash (0xCF) | Down smash (0xD0) |
|---|---|---|---|
| 0 OFF | 51 | not run | not run |
| 1 ON | 110 | 127 | 108 |
| 2 UNLIMITED | 231 (ends 51 frames after A is released) | 238 | 229 |

With OFF the fighter kept walking with the held stick, so only the forward smash was measured.

Damage of Mario's forward smash on Kirby (`[remix-fighter] hit`):

| Case | Damage |
|---|---|
| toggle off, or on with A tapped at once | 17 |
| toggle on, full charge | 24 (17 x 1.4) |
| Giant (`SSB64_REMIX_PANEL="Size.1=1"`) | 22 (17 x 1.3) |
| Tiny (`Size.1=2`) | 12 (17 x 0.7) |

Down smash on Kirby, normal / Giant / Tiny: Fox 14 / 18 / 10, Marina 16 / 21 / 11. A Giant forward smash passes over
Kirby, so the down smash was used there.

Other runs:

- Screenshots every 2 frames of a charge: the fighter pulses yellow, with the shake and the dust.
- Item swings with A held (item dropped on the fighter): Fan 29 to 88 frames and Star Rod 59 to 118 with the toggle
  on; Home-Run Bat 79 frames with and without it, as in the asm, where the Bat is left out.
- Beam Sword, taken from the Training pause menu (Item row) and picked up with A, swung at Kirby with A held: 25
  damage with the toggle off, 35 (25 x 1.4) with it on, and the fighter pulses yellow during the charge.
- A jab landing on a fighter held in an UNLIMITED charge: it goes to the damage status, no shake or colour is left,
  and its next smash has the normal length.
- Two four-CPU matches (level 9, values 1 and 2, 2400 frames): 13 smashes each, none stuck, no error.

## Not checked

Rollback: `sFTRemixChargeStates` is a file static outside `FTStruct`, as in the asm, and is not part of the state
hash. The port has no netplay.
