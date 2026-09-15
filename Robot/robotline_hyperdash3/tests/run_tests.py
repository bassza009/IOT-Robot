"""Test the runnable profile and the optional declared-rating lock."""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile


root = Path(__file__).resolve().parent.parent
compiler = shutil.which("g++") or shutil.which("clang++")
if not compiler:
    raise SystemExit("Install g++ or clang++ to run the host tests.")

tracking_scenarios = (
    "steering", "stop", "startup_stop", "loss_left", "wrap",
    "unknown_line", "lost_center", "turn_timeout", "black_strip",
    "ambiguous", "overrun", "serial_busy", "turn_recovery", "reverse_wrap",
    "reverse_flicker", "stop_error_brake", "stop_reverse", "stop_line_brake",
    "reverse_overrun", "wait_line_stop",
)

with tempfile.TemporaryDirectory(prefix="robotline-hyperdash3-") as folder:
    build = Path(folder)

    def check(sketch, name, scenarios):
        executable = str(build / name)
        subprocess.run([
            compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
            "-I", str(sketch / "tests/stubs"), str(sketch / "tests/sketch_tests.cpp"),
            "-o", executable,
        ], check=True)
        print(f"Configuration: {name}", flush=True)
        for scenario in scenarios:
            subprocess.run([executable, scenario], check=True)

    # First exercise the unmodified files users will download.
    check(root, "shipped_l298n_6800mv_override", ("run_profile",) + tracking_scenarios)
    for name, millivolts, milliamps, scenarios in (
        ("locked_l298n_6800mv", 6800, 2000, ("unsupported_power",)),
        ("voltage_too_high", 6800, 5000, ("unsupported_power",)),
        ("voltage_below_profile", 2000, 5000, ("unsupported_power",)),
        ("driver_dc_too_low", 3000, 2000, ("unsupported_power",)),
        ("declared_3000mv_5000ma", 3000, 5000, tracking_scenarios),
    ):
        # Only edit declared settings in temporary copies; keep the actual
        # controller identical. These fixtures do not simulate electrical loads.
        sketch = build / (name + "_source")
        shutil.copytree(root, sketch)
        settings = sketch / "Settings.h"
        source = settings.read_text()
        source, count = re.subn(
            r"(const bool ALLOW_UNVERIFIED_MOTOR_POWER = )(true|false);",
            r"\g<1>false;", source,
        )
        assert count == 1, "Missing or duplicate power override setting"
        for key, value in (("MOTOR_SUPPLY_MV", millivolts), ("DRIVER_DC_LIMIT_MA", milliamps)):
            source, count = re.subn(rf"(const int {key} = )\d+;", rf"\g<1>{value};", source)
            assert count == 1, f"Missing or duplicate setting: {key}"
        settings.write_text(source)
        check(sketch, name, scenarios)
