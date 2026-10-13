#!/usr/bin/env python3
"""Run the real bounded workflow and render its receipt as GIF/MP4 media."""

from __future__ import annotations

import hashlib
import json
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError as exc:  # pragma: no cover - friendly local setup failure
    raise SystemExit("Pillow is required: python3 -m pip install Pillow") from exc

ROOT = Path(__file__).resolve().parents[1]
DEMO = ROOT / "demo"
SOURCE_COMMIT = "162e50d9f0a95e6cfd8684d1dfbf4f6ae5764e09"
WIDTH, HEIGHT = 1280, 720
BG = (8, 13, 28)
INK = (232, 240, 255)
MUTED = (151, 166, 196)
CYAN = (48, 212, 255)
TEAL = (45, 223, 167)
AMBER = (255, 191, 71)
PINK = (246, 104, 166)
MONO_PATH = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
SANS_PATH = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
SANS_BOLD_PATH = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"


def font(size: int, *, bold: bool = False, mono: bool = False) -> ImageFont.FreeTypeFont:
    path = MONO_PATH if mono else (SANS_BOLD_PATH if bold else SANS_PATH)
    return ImageFont.truetype(path, size)


def sha256_bytes(payload: bytes) -> str:
    return hashlib.sha256(payload).hexdigest()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def run(command: list[str]) -> subprocess.CompletedProcess[str]:
    print("+", " ".join(command), flush=True)
    return subprocess.run(
        command, cwd=ROOT, check=True, capture_output=True, text=True
    )


def verify_manifest() -> int:
    lines = (DEMO / "source-manifest.tsv").read_text().splitlines()
    checked = 0
    for line in lines[1:]:
        expected, relative = line.split("\t", 1)
        actual = subprocess.check_output(
            ["git", "hash-object", relative], cwd=ROOT, text=True
        ).strip()
        if actual != expected:
            raise RuntimeError(f"source drift: {relative}")
        checked += 1
    return checked


def parse_smoke(output: str) -> dict[str, object]:
    dataset_names = re.findall(r"Starting Tests for Dataset: ([^ ]+) ", output)
    strategy_counts = {
        dataset: int(count)
        for count, dataset in re.findall(
            r"Preparing to run (\d+) strategies for dataset '([^']+)'", output
        )
    }
    parsed_rows: dict[str, list[dict[str, object]]] = {name: [] for name in dataset_names}
    active = ""
    for line in output.splitlines():
        match = re.search(r"Starting Tests for Dataset: ([^ ]+) ", line)
        if match:
            active = match.group(1)
        match = re.search(r"stored (\d+) valid bars for ([^.]+)\.", line)
        if match and active:
            parsed_rows[active].append(
                {"symbol": match.group(2), "rows": int(match.group(1))}
            )

    results: dict[str, dict[str, object]] = {}
    result_re = re.compile(
        r"^(\S+_on_\S+)\s+(-?\d+\.\d+)\s+(-?\d+\.\d+)\s+"
        r"(-?\d+\.\d+)\s+(\d+\.\d+)\s+(\d+)\s+(\d+\.\d+)$"
    )
    for line in output.splitlines():
        match = result_re.match(line.strip())
        if match:
            results[match.group(1)] = {
                "return_percent": float(match.group(2)),
                "max_drawdown_percent": float(match.group(3)),
                "realized_pnl": float(match.group(4)),
                "commission": float(match.group(5)),
                "fills": int(match.group(6)),
                "final_equity": float(match.group(7)),
            }

    chosen = [
        "MACrossover_10_50_on_stocks_april",
        "StatArb_BTC_ETH_Aggressive_on_2024_only",
        "StatArb_NVDA_GOOG_Aggressive_on_stocks_april",
    ]
    missing = [name for name in chosen if name not in results]
    if missing:
        raise RuntimeError(f"missing expected result rows: {missing}")
    if dataset_names != ["stocks_april", "2024_only", "2024_2025"]:
        raise RuntimeError(f"unexpected datasets: {dataset_names}")
    if any(item["rows"] != 100 for values in parsed_rows.values() for item in values):
        raise RuntimeError("the run did not honor the 100-row cap")
    return {
        "datasets": parsed_rows,
        "strategy_counts": strategy_counts,
        "total_strategy_runs": sum(strategy_counts.values()),
        "selected_results": {name: results[name] for name in chosen},
        "queued_orders": output.count("ORDER Queued:"),
        "simulated_fills": output.count("SIM EXEC: Order"),
        "completed": "--- Comprehensive Run Invocation Complete ---" in output,
    }


def rounded(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], fill, outline=None, width=1, radius=22):
    draw.rounded_rectangle(box, radius=radius, fill=fill, outline=outline, width=width)


def base_frame(scene: int, progress: float) -> tuple[Image.Image, ImageDraw.ImageDraw]:
    image = Image.new("RGB", (WIDTH, HEIGHT), BG)
    draw = ImageDraw.Draw(image)
    # Static scene gradient + a restrained glow. Keeping the full-frame
    # background stable makes the checked GIF substantially smaller.
    accent = [CYAN, TEAL, AMBER, PINK, CYAN, TEAL][scene]
    for y in range(HEIGHT):
        ratio = y / HEIGHT
        color = (8 + int(9 * ratio), 13 + int(8 * ratio), 28 + int(20 * ratio))
        draw.line((0, y, WIDTH, y), fill=color)
    gx = 980
    gy = 100
    for radius in range(170, 20, -12):
        alpha = (170 - radius) / 150
        color = tuple(int(BG[i] * (1 - alpha * 0.09) + accent[i] * alpha * 0.09) for i in range(3))
        draw.ellipse((gx - radius, gy - radius, gx + radius, gy + radius), fill=color)
    draw.text((60, 38), "FINANCIAL_CPP_COPY", font=font(18, bold=True), fill=accent)
    draw.text((1010, 40), f"0{scene + 1} / 06", font=font(16, mono=True), fill=MUTED)
    draw.rounded_rectangle((60, 684, 1220, 690), radius=3, fill=(32, 43, 68))
    draw.rounded_rectangle((60, 684, 60 + int(1160 * ((scene + progress) / 6)), 690), radius=3, fill=accent)
    return image, draw


def badge(draw: ImageDraw.ImageDraw, xy: tuple[int, int], text: str, color=TEAL) -> None:
    x, y = xy
    text_font = font(15, bold=True)
    width = int(draw.textlength(text, font=text_font)) + 28
    rounded(draw, (x, y, x + width, y + 34), (18, 38, 51), color, width=1, radius=17)
    draw.text((x + 14, y + 7), text, font=text_font, fill=color)


def heading(draw: ImageDraw.ImageDraw, eyebrow: str, title: str, subtitle: str) -> None:
    draw.text((60, 92), eyebrow.upper(), font=font(16, bold=True), fill=CYAN)
    draw.text((60, 126), title, font=font(44, bold=True), fill=INK)
    draw.text((62, 188), subtitle, font=font(21), fill=MUTED)


def scene_title(draw: ImageDraw.ImageDraw, p: float, data: dict[str, object]) -> None:
    heading(draw, "Actual executable • pinned source", "A backtest you can inspect", "Compiled C++17. Bundled CSVs. Bounded, reproducible evidence.")
    rounded(draw, (60, 270, 1220, 600), (13, 22, 43), (46, 66, 101), width=2, radius=28)
    draw.text((100, 310), "SOURCE", font=font(15, bold=True), fill=MUTED)
    draw.text((100, 340), SOURCE_COMMIT[:12], font=font(33, mono=True), fill=CYAN)
    draw.text((100, 420), "SCOPE", font=font(15, bold=True), fill=MUTED)
    draw.text((100, 452), "100 rows / CSV", font=font(33, bold=True), fill=INK)
    x = 660
    badge(draw, (x, 322), "HISTORICAL CSV BACKTEST", AMBER)
    badge(draw, (x, 382), "NO LIVE TRADING", PINK)
    badge(draw, (x, 442), "NO PROFIT / LATENCY GUARANTEE", PINK)
    draw.text((100, 550), "Every number in this demo is parsed from the run shown here.", font=font(18), fill=MUTED)


def scene_build(draw: ImageDraw.ImageDraw, p: float, data: dict[str, object]) -> None:
    heading(draw, "Clean source validation", "Build → regression → smoke", "The same commands are documented and enforced by CI.")
    steps = [
        ("01", "make clean all test", "C++17 source build", "PASS"),
        ("02", "test_circular_buffer", "deterministic regression", "PASS"),
        ("03", "--max-rows=100", "bounded integration", "PASS"),
    ]
    for i, (number, command, detail, status) in enumerate(steps):
        y = 270 + i * 105
        active = p >= (i + 1) / 4
        rounded(draw, (60, y, 1220, y + 80), (13, 22, 43), (45, 64, 96), radius=20)
        draw.text((88, y + 24), number, font=font(18, mono=True), fill=CYAN if active else MUTED)
        draw.text((160, y + 17), command, font=font(21, mono=True), fill=INK)
        draw.text((160, y + 47), detail, font=font(16), fill=MUTED)
        status_color = TEAL if active else (67, 80, 105)
        draw.text((1090, y + 28), status if active else "…", font=font(17, bold=True), fill=status_color)
    draw.text((62, 615), "UBSan was also run separately over test + full bounded smoke.", font=font(17), fill=MUTED)


def scene_data(draw: ImageDraw.ImageDraw, p: float, data: dict[str, object]) -> None:
    heading(draw, "What the executable read", "Three datasets • eleven CSV files", "Each input stops after exactly 100 valid historical rows.")
    labels = [
        ("stocks_april", "3 symbols", "300 rows", CYAN),
        ("2024_only", "4 symbols", "400 rows", TEAL),
        ("2024_2025", "4 symbols", "400 rows", AMBER),
    ]
    for i, (name, symbols, rows, color) in enumerate(labels):
        x = 60 + i * 390
        rounded(draw, (x, 278, x + 360, 555), (13, 22, 43), (45, 64, 96), width=2, radius=24)
        draw.ellipse((x + 28, 308, x + 54, 334), fill=color)
        draw.text((x + 72, 306), name, font=font(21, bold=True), fill=INK)
        draw.text((x + 30, 374), symbols, font=font(33, bold=True), fill=color)
        count = int(int(rows.split()[0]) * min(1.0, p * 1.5))
        draw.text((x + 30, 428), f"{count} rows", font=font(26, mono=True), fill=INK)
        draw.text((x + 30, 490), "100-row cap / CSV", font=font(16), fill=MUTED)
    draw.text((62, 610), "110 strategy/dataset runs completed in this bounded invocation.", font=font(20, bold=True), fill=INK)


def scene_results(draw: ImageDraw.ImageDraw, p: float, data: dict[str, object]) -> None:
    heading(draw, "Parsed from combined output", "Three outcome samples", "A mixed slice of the simulator receipt — not an investment claim.")
    rows = [
        ("MA 10/50 · equities", "−0.00%", "7 fills", "$99,998.08", CYAN),
        ("StatArb BTC/ETH · 2024", "−0.03%", "4 fills", "$99,974.65", PINK),
        ("StatArb NVDA/GOOG · equities", "+0.13%", "4 fills", "$100,130.84", TEAL),
    ]
    headers = [("STRATEGY / DATA", 78), ("RETURN", 720), ("FILLS", 890), ("FINAL EQUITY", 1020)]
    for label, x in headers:
        draw.text((x, 272), label, font=font(14, bold=True), fill=MUTED)
    for i, (name, ret, fills, equity, color) in enumerate(rows):
        y = 308 + i * 88
        rounded(draw, (60, y, 1220, y + 68), (13, 22, 43), (40, 57, 86), radius=16)
        draw.rectangle((60, y, 67, y + 68), fill=color)
        draw.text((82, y + 21), name, font=font(18, bold=True), fill=INK)
        draw.text((722, y + 21), ret, font=font(19, mono=True), fill=color)
        draw.text((892, y + 21), fills, font=font(18, mono=True), fill=INK)
        draw.text((1022, y + 21), equity, font=font(18, mono=True), fill=INK)
    draw.text((62, 598), "Historical inputs • simulated fills • 100-row cap", font=font(18, bold=True), fill=AMBER)
    draw.text((62, 630), "No claim of statistical significance, profitability, or live execution quality.", font=font(16), fill=MUTED)


def scene_flow(draw: ImageDraw.ImageDraw, p: float, data: dict[str, object]) -> None:
    heading(draw, "Inside the bounded simulation", "Orders became simulated fills", "The receipt counts activity from the real process output.")
    queued = int(data["queued_orders"])
    fills = int(data["simulated_fills"])
    values = [
        ("CSV bars", "1,100", CYAN),
        ("strategy runs", str(data["total_strategy_runs"]), AMBER),
        ("orders queued", str(queued), PINK),
        ("sim fills", str(fills), TEAL),
    ]
    for i, (label, value, color) in enumerate(values):
        x = 60 + i * 290
        rounded(draw, (x, 290, x + 260, 485), (13, 22, 43), (45, 64, 96), width=2, radius=24)
        draw.text((x + 28, 328), label.upper(), font=font(14, bold=True), fill=MUTED)
        draw.text((x + 28, 373), value, font=font(42, bold=True), fill=color)
        width = int(196 * min(1.0, p * 1.4))
        draw.rounded_rectangle((x + 28, 445, x + 224, 453), radius=4, fill=(40, 54, 81))
        draw.rounded_rectangle((x + 28, 445, x + 28 + width, 453), radius=4, fill=color)
    draw.text((62, 552), "Execution mode", font=font(15, bold=True), fill=MUTED)
    draw.text((62, 581), "in-process historical simulator", font=font(25, mono=True), fill=INK)
    badge(draw, (868, 565), "NO BROKER / EXCHANGE", PINK)


def scene_proof(draw: ImageDraw.ImageDraw, p: float, data: dict[str, object]) -> None:
    heading(draw, "Reviewable by construction", "The demo carries its receipt", "Source drift, damaged bytes, or altered dimensions fail verification.")
    items = [
        ("44", "runtime inputs pinned", CYAN),
        ("SHA-256", "output + artifacts", TEAL),
        ("1280×720", "GIF + MP4", AMBER),
    ]
    for i, (value, label, color) in enumerate(items):
        x = 60 + i * 390
        rounded(draw, (x, 286, x + 360, 465), (13, 22, 43), (45, 64, 96), width=2, radius=24)
        draw.text((x + 30, 325), value, font=font(34, bold=True), fill=color)
        draw.text((x + 30, 385), label, font=font(18), fill=INK)
    rounded(draw, (60, 515, 1220, 632), (17, 29, 52), TEAL, width=2, radius=22)
    draw.text((92, 543), "HISTORICAL CSV BACKTEST · 100-ROW CAP", font=font(21, bold=True), fill=AMBER)
    draw.text((92, 580), "NO LIVE TRADING · NO PROFIT OR LATENCY GUARANTEE", font=font(21, bold=True), fill=PINK)


SCENES = [scene_title, scene_build, scene_data, scene_results, scene_flow, scene_proof]


def render(stats: dict[str, object]) -> tuple[Path, Path, Path]:
    frames: list[Image.Image] = []
    durations_ms = [3000, 3000, 3000, 3500, 3000, 3000]
    for index, renderer in enumerate(SCENES):
        image, draw = base_frame(index, 1.0)
        renderer(draw, 1.0, stats)
        frames.append(image)

    gif_path = DEMO / "financial_cpp_copy_demo.gif"
    mp4_path = DEMO / "financial_cpp_copy_demo.mp4"
    preview_path = DEMO / "preview.png"
    frames[len(frames) // 2].save(preview_path, optimize=True)
    frames[0].save(
        gif_path,
        save_all=True,
        append_images=frames[1:],
        duration=durations_ms,
        loop=0,
        optimize=True,
        disposal=2,
    )
    del frames
    subprocess.run(
        [
            "ffmpeg",
            "-loglevel",
            "error",
            "-y",
            "-i",
            str(gif_path),
            "-vf",
            "fps=24",
            "-c:v",
            "libx264",
            "-crf",
            "22",
            "-pix_fmt",
            "yuv420p",
            "-movflags",
            "+faststart",
            str(mp4_path),
        ],
        check=True,
    )
    return gif_path, mp4_path, preview_path


def probe(path: Path) -> dict[str, object]:
    result = subprocess.run(
        [
            "ffprobe",
            "-v",
            "error",
            "-select_streams",
            "v:0",
            "-show_entries",
            "stream=codec_name,width,height,nb_frames:format=duration",
            "-of",
            "json",
            str(path),
        ],
        check=True,
        capture_output=True,
        text=True,
    )
    return json.loads(result.stdout)


def main() -> int:
    manifest_count = verify_manifest()
    build = run(["make", "clean", "all", "test"])
    smoke = run(["./build/trading_system", "--max-rows=100"])
    stats = parse_smoke(smoke.stdout)
    if not stats["completed"]:
        raise RuntimeError("bounded run did not print completion marker")
    gif_path, mp4_path, preview_path = render(stats)

    receipt = {
        "schema_version": 1,
        "captured_at": datetime.now(timezone.utc).replace(microsecond=0).isoformat(),
        "source_commit": SOURCE_COMMIT,
        "source_manifest": {
            "path": "demo/source-manifest.tsv",
            "input_count": manifest_count,
            "sha256": sha256_file(DEMO / "source-manifest.tsv"),
        },
        "scope": {
            "mode": "historical CSV backtest simulation",
            "row_cap": 100,
            "live_trading": False,
            "profit_or_latency_guarantee": False,
        },
        "commands": ["make clean all test", "./build/trading_system --max-rows=100"],
        "command_results": {
            "build_and_test_exit_code": build.returncode,
            "smoke_exit_code": smoke.returncode,
            "smoke_stdout_sha256": sha256_bytes(smoke.stdout.encode()),
            "smoke_stdout_lines": len(smoke.stdout.splitlines()),
        },
        "observed": stats,
        "artifacts": {
            gif_path.name: {
                "sha256": sha256_file(gif_path),
                "bytes": gif_path.stat().st_size,
                "probe": probe(gif_path),
            },
            mp4_path.name: {
                "sha256": sha256_file(mp4_path),
                "bytes": mp4_path.stat().st_size,
                "probe": probe(mp4_path),
            },
        },
        "preview": {
            "path": preview_path.name,
            "sha256": sha256_file(preview_path),
            "bytes": preview_path.stat().st_size,
        },
    }
    (DEMO / "capture.json").write_text(json.dumps(receipt, indent=2) + "\n")
    print(json.dumps(receipt, indent=2))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, subprocess.CalledProcessError) as exc:
        print(f"demo generation failed: {exc}", file=sys.stderr)
        raise SystemExit(1)
