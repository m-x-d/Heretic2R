#!/usr/bin/env python3
#
# gen-case-symlinks.py -- recreate case-correct header symlinks for the Linux build.
#
# Heretic2R was developed on Windows (case-insensitive paths), so a number of
# #include directives spell header names with different casing than the file on
# disk (e.g. #include "q_shared.h" vs the actual q_Shared.h). Linux filesystems
# are case-sensitive, so those includes fail. Rather than edit hundreds of source
# files, this script creates a symlink next to each real header for every distinct
# include spelling that differs only by case. A symlink beside the real file
# resolves both "Subdir/foo.h" (from a base include dir) and "foo.h" (from a sibling).
#
# These symlinks are intentionally NOT committed (git symlinks misbehave on the
# project's primary Windows checkouts); run this once after a fresh clone:
#
#     python3 src/linux/gen-case-symlinks.py
#
# Also creates the one misspelled directory symlink (PlaguesSithra -> PlagueSsithra).

import os, re, sys

SRC = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
SRC = os.path.normpath(SRC)

# lowercased basename -> (actual_basename, dir)
actual = {}
for dp, _, fs in os.walk(SRC):
    if os.sep + ".git" in dp:
        continue
    for f in fs:
        if f.endswith((".h", ".c", ".cpp")):
            actual.setdefault(f.lower(), []).append((f, dp))

inc_re = re.compile(r'#\s*include\s+["<]([^">]+)[">]')
wanted = set()
for dp, _, fs in os.walk(SRC):
    if os.sep + ".git" in dp:
        continue
    for f in fs:
        if not f.endswith((".h", ".c", ".cpp")):
            continue
        try:
            for line in open(os.path.join(dp, f), encoding="utf-8", errors="ignore"):
                m = inc_re.search(line)
                if not m:
                    continue
                base = os.path.basename(m.group(1))
                lb = base.lower()
                if lb in actual and base != actual[lb][0][0]:
                    wanted.add(base)
        except OSError:
            pass

created = 0
for base in sorted(wanted):
    real_base, real_dir = actual[base.lower()][0]
    link = os.path.join(real_dir, base)
    if os.path.lexists(link):
        continue
    os.symlink(real_base, link)
    created += 1
    print(f"  {os.path.relpath(link, SRC)} -> {real_base}")

# Misspelled directory: includes reference Monsters/PlaguesSithra/... (real: PlagueSsithra).
mons = os.path.join(SRC, "game", "src", "Monsters")
real_dir = os.path.join(mons, "PlagueSsithra")
link_dir = os.path.join(mons, "PlaguesSithra")
if os.path.isdir(real_dir) and not os.path.lexists(link_dir):
    os.symlink("PlagueSsithra", link_dir)
    created += 1
    print(f"  Monsters/PlaguesSithra -> PlagueSsithra")

print(f"Created {created} case-correct symlink(s).")
