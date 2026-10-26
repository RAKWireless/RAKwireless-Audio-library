#!/usr/bin/env python3
"""
regen_data.py - regenerate the eSpeak-NG PROGMEM data headers.

Standalone replacement for setup.py's create_data(). It does NOT use the
obsolete 'command' module - only the standard library plus the `xxd` tool.

WHAT IT DOES
  Walks  espeak-ng-data/**          (the data files bundled in this library)
  Runs   xxd -i  on each file       (binary -> C array)
  Writes src/data/**/*.h            (PROGMEM const arrays)
  Writes src/espeak-ng-data.h       (master #include list)

This is the same transformation create_data() performs, so the generated
symbol names (e.g. espeak_ng_data_en_dict) are unchanged.

USAGE
  cd ~/Documents/Arduino/libraries/arduino-espeak-ng
  python3 regen_data.py

REQUIREMENTS
  python3 and xxd  (both standard on macOS)

NOTE
  xxd derives the C identifier from the path string it is given, so this
  script must be run from the library root and pass paths that begin with
  'espeak-ng-data/'. That is what produces names like 'espeak_ng_data_en_dict'
  which the library's source expects.
"""
import os
import subprocess
import sys

DATA_DIR = "espeak-ng-data"
SRC_DATA = "src/data"


def run_xxd(src_path, out_path):
    """xxd -i <src_path>  >  <out_path>"""
    try:
        with open(out_path, "w") as out:
            result = subprocess.run(
                ["xxd", "-i", src_path],
                stdout=out,
                stderr=subprocess.PIPE,
            )
    except FileNotFoundError:
        sys.exit("ERROR: 'xxd' not found. Install it (it ships with macOS / "
                 "vim) and try again.")
    if result.returncode != 0:
        sys.exit("ERROR: xxd failed on %s\n%s"
                 % (src_path, result.stderr.decode(errors="replace")))


def patch_header(path):
    """Make the array const + PROGMEM, matching create_data()."""
    with open(path, "r") as f:
        text = f.read()
    text = text.replace("unsigned char", "const unsigned char")
    text = text.replace("[]", "[] PROGMEM")
    with open(path, "w") as f:
        f.write(text)


def main():
    if not os.path.isdir(DATA_DIR):
        sys.exit("ERROR: no '%s/' directory here.\n"
                 "Run this from the arduino-espeak-ng library root:\n"
                 "  cd ~/Documents/Arduino/libraries/arduino-espeak-ng"
                 % DATA_DIR)

    includes = "#pragma once\n\n"
    count = 0

    for root, dirs, files in os.walk(DATA_DIR):
        dirs.sort()
        for name in sorted(files):
            if name.startswith("."):
                continue
            src_path = os.path.join(root, name)
            out_dir = root.replace(DATA_DIR, SRC_DATA, 1)
            out_path = os.path.join(out_dir, name + ".h")

            os.makedirs(out_dir, exist_ok=True)
            run_xxd(src_path, out_path)
            patch_header(out_path)

            # path inside src/espeak-ng-data.h is relative to the src/ folder
            includes += '#include "%s"\n' % out_path.replace("src/", "", 1)
            count += 1
            print("  " + out_path)

    with open("src/espeak-ng-data.h", "w") as f:
        f.write(includes)

    print("\nDone. Regenerated %d data header(s) + src/espeak-ng-data.h" % count)
    print("Now recompile your sketch in the Arduino IDE.")


if __name__ == "__main__":
    main()
