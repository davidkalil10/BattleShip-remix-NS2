#!/usr/bin/env python3
"""Unit tests for tools/verify_switch2_package.py."""

from __future__ import annotations

import struct
import tempfile
import unittest
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import verify_switch2_package as vsp  # noqa: E402


def _make_nro(magic: bytes = b"OUN0", extra_payload: bytes = b"") -> bytes:
    buf = bytearray(0x100)
    buf[0x10:0x14] = magic
    buf.extend(extra_payload)
    struct.pack_into("<I", buf, 0x18, len(buf))
    buf.extend(b"ASET" + b"\x00" * 32)
    return bytes(buf)


class TestVerifySwitch2Package(unittest.TestCase):
    def test_valid_oun0_nro_passes(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            nro = Path(tmp) / "BattleShip-Remix.nro"
            nro.write_bytes(_make_nro(b"OUN0"))
            self.assertEqual(vsp.verify_nro(nro), [])

    def test_switch1_nro0_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            nro = Path(tmp) / "BattleShip-Remix.nro"
            nro.write_bytes(_make_nro(b"NRO0"))
            errs = vsp.verify_nro(nro)
            self.assertTrue(any("NRO0" in e for e in errs), errs)

    def test_forbidden_clkrst_symbol_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            nro = Path(tmp) / "BattleShip-Remix.nro"
            nro.write_bytes(_make_nro(b"OUN0", b"clkrstOpenSession"))
            errs = vsp.verify_nro(nro)
            self.assertTrue(any("clkrstOpenSession" in e for e in errs), errs)

    def test_linker_map_validation(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            mp = Path(tmp) / "BattleShip-Remix-switch2.map"
            mp.write_text(
                "/opt/devkitpro/libounce/lib/libounce.a(applet.o)\n"
                "/opt/devkitpro/portlibs/switch2/lib/libEGL.a(egl.o)\n",
                encoding="utf-8",
            )
            self.assertEqual(vsp.verify_map(mp), [])

            mp_bad = Path(tmp) / "bad.map"
            mp_bad.write_text(
                "/opt/devkitpro/libnx/lib/libnx.a(applet.o)\n"
                "/opt/devkitpro/portlibs/switch/lib/libEGL.a(egl.o)\n",
                encoding="utf-8",
            )
            errs = vsp.verify_map(mp_bad)
            self.assertGreaterEqual(len(errs), 2)


if __name__ == "__main__":
    unittest.main()
