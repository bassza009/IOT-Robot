"""Run the real Arduino sketch with simulated pins, sensors, time and Serial."""
from pathlib import Path
import shutil
import subprocess
import tempfile


root = Path(__file__).resolve().parent
compiler = shutil.which("g++") or shutil.which("clang++")
if not compiler:
    raise SystemExit("Install g++ or clang++ to run the host tests.")

with tempfile.TemporaryDirectory(prefix="robotline-simple-") as folder:
    executable = str(Path(folder) / "sketch_tests")
    subprocess.run([
        compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
        "-I", str(root / "stubs"), str(root / "sketch_tests.cpp"),
        "-o", executable,
    ], check=True)
    for scenario in (
        "steering", "stop", "startup_stop", "loss_left", "wrap",
        "unknown_line", "lost_center", "turn_timeout", "black_strip",
        "ambiguous", "overrun", "serial_busy",
        "turn_recovery", "reverse_wrap", "reverse_flicker",
        "stop_error_brake", "stop_reverse", "stop_line_brake", "reverse_overrun",
        "wait_line_stop",
    ):
        subprocess.run([executable, scenario], check=True)
