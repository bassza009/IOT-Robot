"""Run the real Arduino sketch with simulated pins, sensors, time and Serial."""
from pathlib import Path
import shutil
import subprocess
import tempfile


root = Path(__file__).resolve().parent
compiler = shutil.which("g++") or shutil.which("clang++")
if not compiler:
    raise SystemExit("Install g++ or clang++ to run the host tests.")

with tempfile.TemporaryDirectory(prefix="robotline-hyper-fast-") as folder:
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
    fast_executable = str(Path(folder) / "fast_scan_tests")
    subprocess.run([
        compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
        "-I", str(root / "stubs"), str(root / "fast_scan_tests.cpp"),
        "-o", fast_executable,
    ], check=True)
    for scenario in (
        "fast_reaction", "scan_work", "stop_during_scan", "timing_report",
        "micro_wrap", "adc_overrun",
    ):
        subprocess.run([fast_executable, scenario], check=True)
    turn_executable = str(Path(folder) / "turn_brake_tests")
    subprocess.run([
        compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
        "-I", str(root / "stubs"), str(root / "turn_brake_tests.cpp"),
        "-o", turn_executable,
    ], check=True)
    for scenario in (
        "corner_brake", "turn_reversal_brake", "turn_brake_latest_side",
        "turn_brake_center", "search_brake", "lost_during_turn_brake",
        "stop_during_turn_brake", "turn_brake_wrap",
    ):
        subprocess.run([turn_executable, scenario], check=True)
    straight_executable = str(Path(folder) / "straight_speed_tests")
    subprocess.run([
        compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
        "-I", str(root / "stubs"), str(root / "straight_speed_tests.cpp"),
        "-o", straight_executable,
    ], check=True)
    for scenario in (
        "straight_profile", "straight_wrap", "straight_missed_boundary",
        "straight_curve_reset", "straight_corner_reset", "straight_cross_reset",
        "straight_recovery_reset", "straight_pattern_change", "straight_loss_reset",
        "straight_serial_busy", "straight_stop_up", "straight_stop_hold",
        "straight_stop_down",
    ):
        subprocess.run([straight_executable, scenario], check=True)
    # Exercise alternate Settings.h values without changing the user's sketch.
    variants = Path(folder) / "variants"
    (variants / "tests").mkdir(parents=True)
    sketch = "robotline_tamiya_hyper_fast.ino"
    shutil.copy2(root.parent / sketch, variants / sketch)
    shutil.copy2(root / "scan_mode_tests.cpp", variants / "tests")
    settings = (root.parent / "Settings.h").read_text()
    for scenario, reads, interval in (
        ("settling", 2, 0), ("median", 4, 0), ("timed", 1, 2),
    ):
        (variants / "Settings.h").write_text(
            settings.replace("SENSOR_READS = 1;", f"SENSOR_READS = {reads};")
                    .replace("SAMPLE_MS = 0;", f"SAMPLE_MS = {interval};")
        )
        executable = str(variants / "scan_mode_tests")
        subprocess.run([
            compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
            "-I", str(root / "stubs"), str(variants / "tests/scan_mode_tests.cpp"),
            "-o", executable,
        ], check=True)
        subprocess.run([executable, scenario], check=True)
    shutil.copy2(root / "turn_brake_tests.cpp", variants / "tests")
    (variants / "Settings.h").write_text(
        settings.replace("TURN_BRAKE_MS = 40;", "TURN_BRAKE_MS = 0;")
    )
    subprocess.run([
        compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
        "-I", str(root / "stubs"), str(variants / "tests/turn_brake_tests.cpp"),
        "-o", turn_executable,
    ], check=True)
    subprocess.run([turn_executable, "turn_brake_disabled"], check=True)
