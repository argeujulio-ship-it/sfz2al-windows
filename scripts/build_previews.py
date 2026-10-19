"""Build local animated gameplay previews from the user's own ROM. Never upload them."""
# SPDX-License-Identifier: GPL-3.0-only
import argparse
import json
import pathlib
import shutil
import subprocess
import tempfile
from PIL import Image

def main():
    root = pathlib.Path(__file__).resolve().parents[1]
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--rom", required=True, type=pathlib.Path)
    ap.add_argument("--game", type=pathlib.Path, default=root / "dist/sfz2al-game.exe")
    ap.add_argument("--out", type=pathlib.Path, default=root / "dist/user/previews")
    ap.add_argument("--seed-eeprom", type=pathlib.Path)
    ap.add_argument("--movies", type=pathlib.Path, default=root / "profiles/previews")
    args = ap.parse_args()
    args.rom = args.rom.resolve(strict=True)
    args.game = args.game.resolve(strict=True)
    args.out.mkdir(parents=True, exist_ok=True)
    scratch = root / "work/preview-generation"
    scratch.mkdir(parents=True, exist_ok=True)
    seed = args.seed_eeprom
    if seed is None and (root / "dist/user/board.eeprom").is_file():
        seed = root / "dist/user/board.eeprom"
    reports = []
    for movie in sorted(args.movies.glob("*.txt")):
        # Keep generated frames only in the ignored workspace, never in the sources.
        folder = pathlib.Path(tempfile.mkdtemp(prefix=movie.stem + "-", dir=scratch))
        board = folder / "board.eeprom"
        if seed is not None:
            shutil.copyfile(seed, board)
        process = subprocess.run([
            str(args.game), "--rom", str(args.rom), "--cpu", "native", "--sound", "z80",
            "--headless", "--frames", "2760", "--movie", str(movie.resolve()),
            "--eeprom", str(board), "--shot-every", "2:" + str(folder / "frame_"),
            "--features", "-infinite_time,-refill_health",
        ], capture_output=True, timeout=180)
        (folder / "run.log").write_bytes(process.stdout + process.stderr)
        if process.returncode:
            raise RuntimeError(f"Game failed while rendering preview {movie.stem}")
        frames = []
        for tick in range(2402, 2761, 2):
            with Image.open(folder / f"frame_{tick:06d}.png") as source:
                # CPS-2 pixels are displayed in the original monitor's 4:3 shape.
                frames.append(source.convert("RGB").resize((320, 240), Image.Resampling.LANCZOS))
        target = args.out / (movie.stem + ".gif")
        temporary = target.with_suffix(".gif.tmp")
        frames[0].save(
            temporary, format="GIF", save_all=True, append_images=frames[1:],
            duration=[(30, 40, 30)[i % 3] for i in range(len(frames))],
            loop=0, disposal=2, optimize=False,
        )
        with Image.open(temporary) as verify:
            count = verify.n_frames
            duration = 0
            for index in range(count):
                verify.seek(index)
                duration += verify.info.get("duration", 0)
            if count < 2 or duration < 5000:
                raise RuntimeError(f"Invalid animated preview: {movie.stem}")
        temporary.replace(target)
        for frame in frames:
            frame.close()
        reports.append({"clip": target.name, "frames": count, "duration_ms": duration, "bytes": target.stat().st_size})
        print(json.dumps(reports[-1]), flush=True)
    if not reports:
        raise RuntimeError("No preview input movies found")
    (scratch / "validation.json").write_text(json.dumps(reports, indent=2), encoding="utf-8")

if __name__ == "__main__":
    main()
