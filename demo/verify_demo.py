#!/usr/bin/env python3
"""Verify the checked-in demo's source pinning, checksums, and media shape."""

from __future__ import annotations

import hashlib
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEMO = ROOT / "demo"


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def verify_manifest() -> int:
    lines = (DEMO / "source-manifest.tsv").read_text().splitlines()
    if not lines or lines[0] != "git_blob_sha\tpath":
        raise RuntimeError("invalid source manifest header")
    checked = 0
    for line in lines[1:]:
        expected, relative = line.split("\t", 1)
        path = ROOT / relative
        if not path.is_file():
            raise RuntimeError(f"missing manifest input: {relative}")
        actual = subprocess.check_output(
            ["git", "hash-object", str(path)], cwd=ROOT, text=True
        ).strip()
        if actual != expected:
            raise RuntimeError(
                f"source drift: {relative}: expected {expected}, got {actual}"
            )
        checked += 1
    return checked


def probe(path: Path) -> dict[str, object]:
    completed = subprocess.run(
        [
            "ffprobe",
            "-v",
            "error",
            "-select_streams",
            "v:0",
            "-show_entries",
            "stream=width,height,codec_name:format=duration",
            "-of",
            "json",
            str(path),
        ],
        check=True,
        capture_output=True,
        text=True,
    )
    payload = json.loads(completed.stdout)
    stream = payload["streams"][0]
    duration = float(payload["format"]["duration"])
    return {
        "codec": stream["codec_name"],
        "width": int(stream["width"]),
        "height": int(stream["height"]),
        "duration_seconds": duration,
    }


def main() -> int:
    checked = verify_manifest()
    receipt = json.loads((DEMO / "capture.json").read_text())
    if receipt["scope"]["row_cap"] != 100:
        raise RuntimeError("capture receipt row cap is not 100")
    if receipt["scope"]["mode"] != "historical CSV backtest simulation":
        raise RuntimeError("capture scope label changed")

    for name, evidence in receipt["artifacts"].items():
        path = DEMO / name
        if sha256(path) != evidence["sha256"]:
            raise RuntimeError(f"checksum mismatch: {name}")
        actual = probe(path)
        if actual["width"] != 1280 or actual["height"] != 720:
            raise RuntimeError(f"unexpected dimensions for {name}: {actual}")
        if not 17.5 <= actual["duration_seconds"] <= 19.5:
            raise RuntimeError(f"unexpected duration for {name}: {actual}")

    preview = DEMO / "preview.png"
    if sha256(preview) != receipt["preview"]["sha256"]:
        raise RuntimeError("preview checksum mismatch")

    print(
        f"verified {checked} source inputs, {len(receipt['artifacts'])} media files, "
        "and preview receipt"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (KeyError, OSError, RuntimeError, subprocess.CalledProcessError) as exc:
        print(f"demo verification failed: {exc}", file=sys.stderr)
        raise SystemExit(1)
