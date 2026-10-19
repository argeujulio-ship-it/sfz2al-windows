"""Audit the exact Git index before any upload. No game data is required."""
# SPDX-License-Identifier: GPL-3.0-only
import pathlib
import re
import subprocess

root = pathlib.Path(__file__).resolve().parents[1]
tracked = subprocess.check_output(["git", "-C", str(root), "ls-files", "-z"]).decode().split("\0")
bad = []
for name in filter(None, tracked):
    p = root / name
    if name.startswith(("work/", "build/", "dist/", ".git/")):
        bad.append((name, "private build folder"))
    if p.suffix.lower() in {".zip", ".7z", ".rar", ".key", ".bin", ".exe", ".dll", ".obj", ".pdb", ".eeprom", ".state", ".wav", ".mp4", ".gif"}:
        bad.append((name, "binary or game data"))
    if re.search(r"/(?:native|z80)_\d+\.cpp$", name):
        bad.append((name, "translated program"))
    data = subprocess.check_output(["git", "-C", str(root), "show", f":{name}"])
    if p.suffix.lower() == ".ico":
        if name != "src/windows/launcher.ico" or data[:4] != b"\x00\x00\x01\x00":
            bad.append((name, "unapproved icon"))
        continue
    if p.suffix.lower() == ".png":
        if name not in {"docs/previews/tema_fliperama.png", "docs/previews/tema_tv_tubo.png", "docs/previews/tema_gabinete_classico.png"}:
            bad.append((name, "unapproved image"))
        continue
    text = data.decode("utf-8-sig")
    patterns = [
        r"[A-Za-z]:[/\\]Users[/\\][^/\\\s]+",
        r"(?:ghp_|github_pat_|gho_)[A-Za-z0-9_]{15,}",
        r"-----BEGIN (?:RSA |OPENSSH )?PRIVATE KEY-----",
    ]
    if name != "scripts/audit_publication.py" and any(re.search(pattern, text) for pattern in patterns):
        bad.append((name, "personal path or credential pattern"))
    if name.endswith(("entries-main.txt", "entries-z80.txt")):
        for line in text.splitlines():
            line = line.split("#", 1)[0].strip()
            if line and not re.fullmatch(r"[0-9a-fA-F]{1,8}", line):
                bad.append((name, "unexpected entry metadata"))
if bad:
    for name, reason in bad:
        print(f"FAIL {name}: {reason}")
    raise SystemExit(1)
print(f"PASS: {len([p for p in tracked if p])} staged source/metadata/preview files; no game payload or credential pattern.")
