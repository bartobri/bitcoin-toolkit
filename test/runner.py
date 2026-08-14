#!/usr/bin/env python3
"""Offline CLI test runner. Discovers test/cli/test_*.py."""

import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
BTK = ROOT / "bin" / "btk"


def run(args, input_bytes=None, timeout=5):
    return subprocess.run(
        [str(BTK), *args],
        input=input_bytes,
        capture_output=True,
        timeout=timeout,
        cwd=str(ROOT),
    )


def main():
    cli = ROOT / "test" / "cli"
    if not cli.is_dir():
        print("no test/cli directory")
        return 0
    mods = sorted(cli.glob("test_*.py"))
    if not mods:
        print("no CLI tests")
        return 0
    env = dict(**{**__import__("os").environ, "PYTHONPATH": str(ROOT / "test")})
    failed = 0
    for mod in mods:
        print(f"  {mod.name} ...", end=" ", flush=True)
        r = subprocess.run([sys.executable, str(mod)], cwd=str(ROOT), env=env)
        if r.returncode != 0:
            print("FAIL")
            failed += 1
        else:
            print("ok")
    if failed:
        print(f"{failed} CLI module(s) failed")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
