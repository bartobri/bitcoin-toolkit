#!/usr/bin/env python3
"""Phase 6 btk config CLI tests."""

import json
import os
import pathlib
import stat
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
BTK = ROOT / "bin" / "btk"

CONFIG_HELP = """btk config — defaults for RPC

Usage:
  btk config set <key>=<value>
  btk config unset <key>
  btk config get <key>
  btk config dump

Keys: rpc.host, rpc.port, rpc.auth
File: ~/.btk/config.json (or --config / $BTK_CONFIG), mode 0600.
dump and get redact rpc.auth as ********.
"""


def run(args, input_bytes=None, home=None, env=None, timeout=5):
    if isinstance(input_bytes, str):
        input_bytes = input_bytes.encode()
    if env is None:
        env = os.environ.copy()
    else:
        env = dict(env)
    if home is not None:
        env["HOME"] = home
    return subprocess.run(
        [str(BTK), *args],
        input=input_bytes,
        capture_output=True,
        timeout=timeout,
        cwd=str(ROOT),
        env=env,
    )


def ndjson(stdout):
    return [json.loads(ln) for ln in stdout.decode().splitlines() if ln]


def mode_of(path):
    return stat.S_IMODE(path.stat().st_mode)


def main():
    r = run(["config", "--help"])
    assert r.returncode == 0, r.stderr.decode()
    assert r.stdout.decode() == CONFIG_HELP, repr(r.stdout.decode())

    with tempfile.TemporaryDirectory() as tmp:
        tmp = pathlib.Path(tmp)
        cfg = tmp / "nested" / "config.json"

        r = run(["--config", str(cfg), "config", "dump"])
        assert r.returncode == 0, r.stderr.decode()
        assert ndjson(r.stdout) == [{"type": "config"}]
        assert not cfg.exists()
        assert not cfg.parent.exists()

        r = run(["--config", str(cfg), "config", "get", "rpc.host"])
        assert r.returncode == 1
        assert "no such key" in r.stderr.decode()
        assert not cfg.exists()

        r = run(["--config", str(cfg), "config", "unset", "rpc.host"])
        assert r.returncode == 1
        assert "no such key" in r.stderr.decode()
        assert not cfg.exists()

        r = run(["--config", str(cfg), "config", "set", "rpc.host=127.0.0.1"])
        assert r.returncode == 0, r.stderr.decode()
        assert r.stdout == b""
        assert cfg.is_file()
        assert mode_of(cfg) == 0o600
        assert mode_of(cfg.parent) == 0o700
        on_disk = json.loads(cfg.read_text())
        assert on_disk == {"rpc": {"host": "127.0.0.1"}}

        r = run(["--config", str(cfg), "config", "set", "rpc.port=8332"])
        assert r.returncode == 0, r.stderr.decode()
        r = run(["--config", str(cfg), "config", "set", "rpc.auth=alice:s3cret"])
        assert r.returncode == 0, r.stderr.decode()
        on_disk = json.loads(cfg.read_text())
        assert on_disk["rpc"]["host"] == "127.0.0.1"
        assert on_disk["rpc"]["port"] == 8332
        assert on_disk["rpc"]["auth"] == "alice:s3cret"

        r = run(["--config", str(cfg), "config", "dump"])
        assert r.returncode == 0, r.stderr.decode()
        obj = ndjson(r.stdout)[0]
        assert obj == {
            "type": "config",
            "rpc.host": "127.0.0.1",
            "rpc.port": 8332,
            "rpc.auth": "********",
        }
        assert b"s3cret" not in r.stdout

        r = run(["--config", str(cfg), "config", "dump", "--out", "plain"])
        assert r.returncode == 0
        assert r.stdout.decode() == (
            "rpc.host=127.0.0.1\nrpc.port=8332\nrpc.auth=********\n"
        )
        assert "s3cret" not in r.stdout.decode()

        r = run(["--config", str(cfg), "config", "get", "rpc.host"])
        assert r.returncode == 0
        assert ndjson(r.stdout) == [{"type": "config", "rpc.host": "127.0.0.1"}]

        r = run(["--config", str(cfg), "config", "get", "rpc.port", "--out", "plain"])
        assert r.returncode == 0
        assert r.stdout.decode() == "8332\n"

        r = run(["--config", str(cfg), "config", "get", "rpc.auth", "--out", "plain"])
        assert r.returncode == 0
        assert r.stdout.decode() == "********\n"

        r = run(["--config", str(cfg), "config", "get", "rpc.auth"])
        assert r.returncode == 0
        assert ndjson(r.stdout) == [{"type": "config", "rpc.auth": "********"}]
        assert b"s3cret" not in r.stdout

        r = run(["--config", str(cfg), "config", "unset", "rpc.host"])
        assert r.returncode == 0
        assert r.stdout == b""
        r = run(["--config", str(cfg), "config", "get", "rpc.host"])
        assert r.returncode == 1
        assert "no such key" in r.stderr.decode()
        on_disk = json.loads(cfg.read_text())
        assert "host" not in on_disk["rpc"]
        assert on_disk["rpc"]["port"] == 8332

        r = run(["--config", str(cfg), "config", "unset", "rpc.port"])
        assert r.returncode == 0
        r = run(["--config", str(cfg), "config", "unset", "rpc.auth"])
        assert r.returncode == 0
        r = run(["--config", str(cfg), "config", "dump"])
        assert r.returncode == 0
        assert ndjson(r.stdout) == [{"type": "config"}]
        on_disk = json.loads(cfg.read_text())
        assert on_disk == {}

        r = run(["--config", str(cfg), "config", "set", "foo=bar"])
        assert r.returncode == 1
        assert "unknown config key 'foo'" in r.stderr.decode()

        r = run(["--config", str(cfg), "config", "get", "balance.path"])
        assert r.returncode == 1
        assert "unknown config key 'balance.path'" in r.stderr.decode()

        r = run(["--config", str(cfg), "config", "set", "rpc.port=0"])
        assert r.returncode == 1
        assert "invalid rpc.port" in r.stderr.decode()
        r = run(["--config", str(cfg), "config", "set", "rpc.port=65536"])
        assert r.returncode == 1
        assert "invalid rpc.port" in r.stderr.decode()
        r = run(["--config", str(cfg), "config", "set", "rpc.port=nope"])
        assert r.returncode == 1
        assert "invalid rpc.port" in r.stderr.decode()

        r = run(["--config", str(cfg), "config", "set", "rpc.host"])
        assert r.returncode == 1
        assert "expected key=value" in r.stderr.decode()

        r = run(["--config", str(cfg), "config"])
        assert r.returncode == 1
        assert "expected set, get, unset, or dump" in r.stderr.decode()

        r = run(["--config", str(cfg), "config", "list"])
        assert r.returncode == 1
        assert "unknown config verb 'list'" in r.stderr.decode()

        r = run(["--config", str(cfg), "config", "dump", "extra"])
        assert r.returncode == 1
        assert "unexpected argument" in r.stderr.decode()

        r = run(["--config", str(cfg), "config", "--stream", "dump"])
        assert r.returncode == 1
        assert "config does not stream" in r.stderr.decode()

        r = run(["--config", str(cfg), "config", "--count", "1", "dump"])
        assert r.returncode == 1
        assert "unknown option '--count'" in r.stderr.decode()

        r = run(["--config", str(cfg), "config", "--from", "address", "dump"])
        assert r.returncode == 1
        assert "unknown option '--from'" in r.stderr.decode()

        r = run(["--config", str(cfg), "config", "--show-secrets", "dump"])
        assert r.returncode == 1
        assert "unknown option '--show-secrets'" in r.stderr.decode()

        r = run(["--config", str(cfg), "config", "set", "rpc.auth=alice:x=y=z"])
        assert r.returncode == 0, r.stderr.decode()
        assert json.loads(cfg.read_text())["rpc"]["auth"] == "alice:x=y=z"

        # Preserve unknown on-disk keys.
        other = tmp / "preserve.json"
        other.write_text(
            json.dumps({"rpc": {"host": "10.0.0.1", "extra": True}, "other": 1})
        )
        r = run(["--config", str(other), "config", "set", "rpc.port=8332"])
        assert r.returncode == 0, r.stderr.decode()
        saved = json.loads(other.read_text())
        assert saved["rpc"]["host"] == "10.0.0.1"
        assert saved["rpc"]["port"] == 8332
        assert saved["rpc"]["extra"] is True
        assert saved["other"] == 1
        r = run(["--config", str(other), "config", "dump"])
        assert ndjson(r.stdout)[0] == {
            "type": "config",
            "rpc.host": "10.0.0.1",
            "rpc.port": 8332,
        }

        # Invalid file: no mkdir of a missing parent.
        bad_dir = tmp / "missing-parent"
        bad = bad_dir / "bad.json"
        # Write invalid JSON at a path whose parent exists; then point at a
        # missing nested path after a failed parse of a sibling.
        sibling = tmp / "bad.json"
        sibling.write_text("{")
        r = run(["--config", str(sibling), "config", "dump"])
        assert r.returncode == 1
        assert "invalid config file" in r.stderr.decode()
        r = run(["--config", str(bad), "config", "dump"])
        assert r.returncode == 0
        assert not bad_dir.exists()

        # Phases 1–4 must not open the config file.
        r = run(["--config", str(sibling), "privkey", "--new", "--out", "plain"])
        assert r.returncode == 0, r.stderr.decode()
        r = run(["--config", str(sibling), "node", "--help"])
        assert r.returncode == 0

        # Default path under HOME.
        home = tmp / "home"
        home.mkdir()
        r = run(["config", "set", "rpc.host=192.0.2.1"], home=str(home))
        assert r.returncode == 0, r.stderr.decode()
        default = home / ".btk" / "config.json"
        assert default.is_file()
        assert mode_of(default) == 0o600
        assert mode_of(default.parent) == 0o700
        assert json.loads(default.read_text())["rpc"]["host"] == "192.0.2.1"
        r = run(["config", "get", "rpc.host", "--out", "plain"], home=str(home))
        assert r.stdout.decode() == "192.0.2.1\n"

        # $BTK_CONFIG and --config precedence.
        env_path = tmp / "from-env.json"
        flag_path = tmp / "from-flag.json"
        env = os.environ.copy()
        env["BTK_CONFIG"] = str(env_path)
        r = run(["config", "set", "rpc.host=env-host"], env=env)
        assert r.returncode == 0, r.stderr.decode()
        assert json.loads(env_path.read_text())["rpc"]["host"] == "env-host"
        r = run(
            ["--config", str(flag_path), "config", "set", "rpc.host=flag-host"],
            env=env,
        )
        assert r.returncode == 0, r.stderr.decode()
        assert json.loads(flag_path.read_text())["rpc"]["host"] == "flag-host"
        assert json.loads(env_path.read_text())["rpc"]["host"] == "env-host"

        # HOME unset without --config / $BTK_CONFIG.
        no_home = os.environ.copy()
        no_home.pop("HOME", None)
        no_home.pop("BTK_CONFIG", None)
        r = run(["config", "dump"], env=no_home)
        assert r.returncode == 1
        assert "HOME is not set" in r.stderr.decode()

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except AssertionError as e:
        print("assertion failed:", e, file=sys.stderr)
        sys.exit(1)
