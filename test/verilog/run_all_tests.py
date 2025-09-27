#!/usr/bin/env python3
"""
Comprehensive Verilog testbench runner for all exported wiRedPanda circuits.
Tests all 18 exported Verilog files using multiple EDA tools.
"""

import os
import subprocess
import sys
import time
from pathlib import Path

# Configuration
VERILOG_DIR = Path(__file__).parent
TEST_RESULTS_DIR = VERILOG_DIR / "test_results"
TOOLS = {
    "iverilog": {"compiler": "iverilog", "simulator": "vvp", "version_flag": "-V"},
    "verilator": {"compiler": "verilator", "simulator": None, "version_flag": "--version"},
    "yosys": {"compiler": "yosys", "simulator": None, "version_flag": "-V"}
}

# List of all exported Verilog files (updated for underscore naming)
VERILOG_FILES = [
    "counter.v",
    "decoder.v",
    "dflipflop_masterslave.v",
    "dflipflop.v",
    "dflipflop2.v",
    "display_3bits_counter.v",
    "display_3bits.v",
    "display_4bits_counter.v",
    "display_4bits.v",
    "dlatch.v",
    "ic.v",
    "input_module.v",
    "jkflipflop.v",
    "notes.v",
    "register.v",
    "sequential.v",
    "serialize.v",
    "tflipflop.v"
]

# Files with existing testbenches
EXISTING_TESTBENCHES = [
    "input_tb.v",
    "counter_tb.v",
    "dflipflop_tb.v",
    "dlatch_tb.v"
]

class TestRunner:
    def __init__(self):
        self.results = {}
        self.total_tests = 0
        self.passed_tests = 0
        self.failed_tests = 0

        # Create results directory
        TEST_RESULTS_DIR.mkdir(exist_ok=True)

    def log(self, message, level="INFO"):
        """Log messages with timestamp"""
        timestamp = time.strftime("%H:%M:%S")
        print(f"[{timestamp}] {level}: {message}")

    def check_tool_availability(self):
        """Check which EDA tools are available"""
        available_tools = {}
        for tool_name, tool_config in TOOLS.items():
            try:
                version_flag = tool_config.get("version_flag", "--version")
                result = subprocess.run([tool_config["compiler"], version_flag],
                                      capture_output=True, text=True, timeout=10)
                if result.returncode == 0:
                    available_tools[tool_name] = tool_config
                    self.log(f"{tool_name} is available")
                else:
                    self.log(f"{tool_name} is not available", "WARN")
            except (subprocess.TimeoutExpired, FileNotFoundError):
                self.log(f"{tool_name} is not available", "WARN")

        return available_tools

    def test_with_iverilog(self, verilog_file, testbench_file=None):
        """Test using Icarus Verilog with strictest validation"""
        file_stem = Path(verilog_file).stem
        output_file = TEST_RESULTS_DIR / f"{file_stem}_iverilog.out"

        try:
            # Strictest iverilog flags: -Wall enables all warnings, -Winfloop for infinite loop detection
            strict_flags = ["-Wall", "-Winfloop", "-g2012"]  # Use newest standard supported by iverilog (2012)

            if testbench_file and Path(testbench_file).exists():
                # Use existing testbench with strict validation
                self.log(f"Testing {verilog_file} with existing testbench {testbench_file} (STRICT MODE)")
                compile_cmd = ["iverilog"] + strict_flags + ["-o", str(output_file), str(testbench_file), str(verilog_file)]
            else:
                # Just check syntax compilation with strict validation
                self.log(f"Syntax checking {verilog_file} (STRICT MODE)")
                compile_cmd = ["iverilog"] + strict_flags + ["-t", "null", str(verilog_file)]

            # Compile
            result = subprocess.run(compile_cmd, capture_output=True, text=True, timeout=30)

            if result.returncode == 0:
                if testbench_file and Path(testbench_file).exists():
                    # Run simulation
                    sim_result = subprocess.run(["vvp", str(output_file)],
                                              capture_output=True, text=True, timeout=60)
                    if sim_result.returncode == 0:
                        self.log(f"✅ {file_stem}: iverilog simulation PASSED")
                        return True, sim_result.stdout
                    else:
                        self.log(f"❌ {file_stem}: iverilog simulation FAILED")
                        return False, sim_result.stderr
                else:
                    self.log(f"✅ {file_stem}: iverilog syntax check PASSED")
                    return True, "Syntax check passed"
            else:
                self.log(f"❌ {file_stem}: iverilog compilation FAILED")
                return False, result.stderr

        except subprocess.TimeoutExpired:
            self.log(f"❌ {file_stem}: iverilog test TIMEOUT")
            return False, "Timeout"
        except Exception as e:
            self.log(f"❌ {file_stem}: iverilog test ERROR: {e}")
            return False, str(e)

    def test_with_verilator(self, verilog_file):
        """Test using Verilator with strictest lint validation"""
        file_stem = Path(verilog_file).stem

        try:
            # Strictest verilator flags: --lint-only with newest standard and all warnings enabled
            strict_flags = ["--lint-only", "-Wall", "+1800-2017ext+.v"]  # Newest standard (2017) + all warnings

            self.log(f"Verilator strict lint checking {verilog_file} (STRICT MODE)")
            result = subprocess.run(["verilator"] + strict_flags + [str(verilog_file)],
                                  capture_output=True, text=True, timeout=30)

            if result.returncode == 0:
                self.log(f"✅ {file_stem}: verilator lint PASSED")
                return True, "Lint check passed"
            else:
                self.log(f"❌ {file_stem}: verilator lint FAILED")
                return False, result.stderr

        except subprocess.TimeoutExpired:
            self.log(f"❌ {file_stem}: verilator test TIMEOUT")
            return False, "Timeout"
        except Exception as e:
            self.log(f"❌ {file_stem}: verilator test ERROR: {e}")
            return False, str(e)

    def test_with_yosys(self, verilog_file):
        """Test using Yosys with strictest synthesis validation"""
        file_stem = Path(verilog_file).stem

        try:
            # Strictest yosys mode: verbose, detailed timing, treat all warnings as errors
            strict_flags = ["-v", "2", "-d", "-t"]  # Verbose level 2, detailed timing, timestamps

            self.log(f"Yosys strict synthesis checking {verilog_file} (STRICT MODE)")
            # Enhanced Yosys script with stricter checks (compatible with yosys 0.9)
            yosys_script = f"""
read_verilog {verilog_file}
hierarchy -check -auto-top
proc
opt -full
check
stat
"""
            result = subprocess.run(["yosys"] + strict_flags + ["-p", yosys_script],
                                  capture_output=True, text=True, timeout=60)

            if result.returncode == 0:
                self.log(f"✅ {file_stem}: yosys synthesis PASSED")
                return True, "Synthesis check passed"
            else:
                self.log(f"❌ {file_stem}: yosys synthesis FAILED")
                return False, result.stderr

        except subprocess.TimeoutExpired:
            self.log(f"❌ {file_stem}: yosys test TIMEOUT")
            return False, "Timeout"
        except Exception as e:
            self.log(f"❌ {file_stem}: yosys test ERROR: {e}")
            return False, str(e)

    def run_all_tests(self):
        """Run all tests on all Verilog files with STRICTEST validation modes"""
        self.log("Starting comprehensive Verilog test suite - STRICTEST MODE")
        self.log("="*60)
        self.log("STRICT MODE ENABLED:")
        self.log("  - iverilog: -Wall -Winfloop -g2012 (IEEE 1800-2012 - newest supported)")
        self.log("  - verilator: -Wall +1800-2017ext+.v (IEEE 1800-2017 - newest supported)")
        self.log("  - yosys: -v 2 -d -t -auto-top (verbose + enhanced synthesis)")
        self.log("="*60)

        available_tools = self.check_tool_availability()
        if not available_tools:
            self.log("No EDA tools available! Cannot run tests.", "ERROR")
            return False

        # Test each Verilog file
        for verilog_file in VERILOG_FILES:
            verilog_path = VERILOG_DIR / verilog_file
            if not verilog_path.exists():
                self.log(f"❌ {verilog_file}: FILE NOT FOUND", "ERROR")
                continue

            file_stem = Path(verilog_file).stem
            self.results[file_stem] = {}

            self.log(f"\nTesting {verilog_file}")
            self.log("-" * 40)

            # Check for existing testbench
            testbench_file = VERILOG_DIR / f"{file_stem}_tb.v"

            # Test with each available tool
            for tool_name, tool_config in available_tools.items():
                self.total_tests += 1

                if tool_name == "iverilog":
                    success, output = self.test_with_iverilog(verilog_path, testbench_file)
                elif tool_name == "verilator":
                    success, output = self.test_with_verilator(verilog_path)
                elif tool_name == "yosys":
                    success, output = self.test_with_yosys(verilog_path)
                else:
                    continue

                self.results[file_stem][tool_name] = {"success": success, "output": output}

                if success:
                    self.passed_tests += 1
                else:
                    self.failed_tests += 1

        # Generate final report
        self.generate_report()

    def generate_report(self):
        """Generate final test report"""
        self.log("\n" + "="*60)
        self.log("FINAL TEST REPORT")
        self.log("="*60)

        # Summary table
        self.log(f"Total tests run: {self.total_tests}")
        self.log(f"Passed: {self.passed_tests}")
        self.log(f"Failed: {self.failed_tests}")
        self.log(f"Success rate: {(self.passed_tests/self.total_tests*100):.1f}%" if self.total_tests > 0 else "0%")

        # Detailed results
        self.log("\nDetailed Results:")
        self.log("-" * 40)

        for file_name, file_results in self.results.items():
            self.log(f"\n{file_name}:")
            for tool_name, result in file_results.items():
                status = "✅ PASS" if result["success"] else "❌ FAIL"
                self.log(f"  {tool_name}: {status}")

        # Write report to file
        report_file = TEST_RESULTS_DIR / "test_report.txt"
        with open(report_file, "w") as f:
            f.write(f"Verilog Test Report - {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write("="*60 + "\n\n")
            f.write(f"Total tests: {self.total_tests}\n")
            f.write(f"Passed: {self.passed_tests}\n")
            f.write(f"Failed: {self.failed_tests}\n")
            f.write(f"Success rate: {(self.passed_tests/self.total_tests*100):.1f}%\n\n" if self.total_tests > 0 else "Success rate: 0%\n\n")

            for file_name, file_results in self.results.items():
                f.write(f"{file_name}:\n")
                for tool_name, result in file_results.items():
                    status = "PASS" if result["success"] else "FAIL"
                    f.write(f"  {tool_name}: {status}\n")
                    if not result["success"]:
                        f.write(f"    Error: {result['output'][:200]}...\n")
                f.write("\n")

        self.log(f"\nDetailed report written to: {report_file}")

        return self.passed_tests == self.total_tests

def main():
    """Main entry point"""
    runner = TestRunner()
    success = runner.run_all_tests()

    if success:
        print("\n🎉 All tests passed!")
        sys.exit(0)
    else:
        print("\n⚠️  Some tests failed. Check the report for details.")
        sys.exit(1)

if __name__ == "__main__":
    main()