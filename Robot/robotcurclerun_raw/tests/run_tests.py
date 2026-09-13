"""Compile/run the production controller and sketch against simulated hardware."""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", help="Path to g++, clang++, or zig executable")
    args = parser.parse_args()
    compiler = args.compiler
    if not compiler:
        compiler = next((shutil.which(name) for name in ("g++", "clang++", "zig")
                         if shutil.which(name)), None)
    if not compiler:
        cached = (Path(tempfile.gettempdir()) / "robot-line-review-tools" /
                  "zig-windows-x86_64-0.13.0" / "zig.exe")
        if cached.is_file():
            compiler = str(cached)
    if not compiler:
        parser.error("No native C++ compiler found. Pass --compiler <path-to-compiler>.")
    repo = Path(__file__).resolve().parent.parent
    build = Path(tempfile.gettempdir()) / "robot-line-follow-tests"
    build.mkdir(exist_ok=True)
    command = [compiler]
    if Path(compiler).stem.lower() == "zig":
        command.append("c++")
    flags = ["-std=c++11", "-Wall", "-Wextra", "-Werror", "-fno-exceptions", "-fno-rtti"]
    for name in ("controller_tests", "sketch_tests"):
        exe = build / (name + (".exe" if sys.platform == "win32" else ""))
        subprocess.run(command + flags + ["-I", str(repo / "tests/stubs"),
                       str(repo / "tests" / (name + ".cpp")), "-o", str(exe)], check=True)
        subprocess.run([str(exe)], check=True)


if __name__ == "__main__":
    main()
