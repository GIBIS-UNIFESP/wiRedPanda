#!/usr/bin/env python3
# Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
# SPDX-License-Identifier: GPL-3.0-or-later

"""
Run all IC generator scripts in dependency order (topologically sorted).

This script executes all IC generator scripts in an order that respects
inter-generator dependencies, ensuring that each generator's required
components are available before it runs.

Usage:
    python3 run_all_generators.py [--verbose]

Level Structure:
- Level 1: Storage & Latching (4 generators)
- Level 2: Simple Arithmetic & Logic (8 generators)
- Level 3: Enhanced Logic & 1-bit Systems (4 generators)
- Level 4: 4-bit Arithmetic & Systems (12 generators)
- Level 5: 4-bit Control & Advanced (9 generators)
- Level 6: 8-bit Arithmetic & Storage (9 generators)
- Level 7: CPU Building Blocks (7 generators)
- Level 8: Pipeline Stages (4 generators)
- Level 9: Complete CPUs (3 generators)
"""

import asyncio
import collections
import os
import re
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Set, Tuple

def discover_generators(generators_dir: Path) -> List[str]:
    """Dynamically discover all generator scripts matching create_level*.py pattern"""
    pattern = "create_level*.py"
    generators = sorted([f.name for f in generators_dir.glob(pattern)])
    return generators

def extract_component_names(script_path: Path) -> List[str]:
    """Extract all output component names from a generator script.

    Reads the actual output_file variable from the script. For nbit generators
    that produce multiple outputs via f-strings (e.g. level2_mux_{size}to1 with
    sizes [2, 4, 8]), expands the template into all concrete output names.

    Returns a list of component names (without .panda extension).
    """
    try:
        content = script_path.read_text()

        # Look for literal string: output_file = ... / "component.panda"
        match = re.search(
            r'output_file\s*=\s*(?:str\()?\s*[^"]*?[/\\]\s*["\']([^"\']+)\.panda["\']',
            content
        )
        if match:
            return [match.group(1)]

        # Look for simple string without .panda extension
        match = re.search(
            r'output_file\s*=\s*str\([^)]*[/\\]\s*["\']([^"\']+)["\']', content
        )
        if match:
            return [match.group(1)]

        # Fallback to script name
        match = re.match(r'create_(.+?)\.py$', script_path.name)
        return [match.group(1)] if match else []
    except OSError as e:
        print(f"⚠️  Could not read {script_path.name}: {e}")
        match = re.match(r'create_(.+?)\.py$', script_path.name)
        return [match.group(1)] if match else []


def extract_dependencies(script_path: Path) -> Set[str]:
    """Extract IC component dependencies from a generator script.

    Looks for patterns like 'level6_stack_pointer_8bit' in the script,
    excluding the script's own output components and comments.
    """
    dependencies = set()
    output_components = set(extract_component_names(script_path))

    try:
        content = script_path.read_text()
        # Remove comments and docstrings to avoid false matches
        lines = content.split('\n')
        code_lines = []
        in_multiline_string = False
        for line in lines:
            # Skip pure comment lines
            stripped = line.lstrip()
            if stripped.startswith('#'):
                continue
            # Toggle multiline string state (basic detection)
            if '"""' in line or "'''" in line:
                in_multiline_string = not in_multiline_string
                continue
            if not in_multiline_string:
                code_lines.append(line)

        code_content = '\n'.join(code_lines)
        # Find all level[0-9]_component patterns, excluding variable assignments (levelN_name = ...)
        # Use negative lookbehind to exclude patterns preceded by assignment
        matches = re.findall(r'(?<![a-z_=\s])level\d+_[a-z_0-9]+(?!\s*=)(?!\w)', code_content)
        for match in matches:
            # Skip the script's own output components
            if match not in output_components:
                dependencies.add(match)
    except OSError as e:
        print(f"⚠️  Could not read {script_path.name}: {e}")

    return dependencies


def build_dependency_graph(generators_dir: Path, generator_scripts: List[str]) -> Tuple[Dict[str, Set[str]], Dict[str, str]]:
    """Build a dependency graph of generators.

    Returns:
    - Dict mapping generator script name to set of components it depends on
    - Dict mapping component name to generator script that produces it
    """
    dep_graph = {}
    component_to_gen = {}

    for script in generator_scripts:
        script_path = generators_dir / script
        deps = extract_dependencies(script_path)
        dep_graph[script] = deps

        # Track which generator produces which component(s)
        for component in extract_component_names(script_path):
            component_to_gen[component] = script

    return dep_graph, component_to_gen

def topological_sort(generators: List[str], dependency_graph: Dict[str, Set[str]], component_to_gen: Dict[str, str]) -> List[str]:
    """Topologically sort generators based on dependencies.

    Uses Kahn's algorithm to produce a valid execution order.
    Returns generators in dependency order (dependencies first).

    Args:
        generators: List of generator script names
        dependency_graph: Maps generator -> components it depends on
        component_to_gen: Maps component name -> generator that produces it

    Raises:
        RuntimeError: If a dependency cycle is detected or missing dependencies are found
    """
    # Build generator dependency graph (script to script)
    gen_graph: Dict[str, Set[str]] = {}
    in_degree: Dict[str, int] = {}

    for gen in generators:
        gen_graph[gen] = set()
        in_degree[gen] = 0

    # Build edges: if generator A depends on component from B, add edge B -> A
    # Track edges as a set to avoid inflating in_degree when a generator
    # depends on multiple outputs from the same producer (e.g. register_file
    # depends on decoder_2to4, decoder_3to8, decoder_4to16 all from decoder_nbit)
    seen_edges: Set[Tuple[str, str]] = set()
    missing_deps: Dict[str, Set[str]] = {}

    for gen in generators:
        for component_dep in dependency_graph[gen]:
            if component_dep in component_to_gen:
                producer = component_to_gen[component_dep]
                if producer != gen and producer in gen_graph:
                    edge = (producer, gen)
                    if edge not in seen_edges:
                        seen_edges.add(edge)
                        gen_graph[producer].add(gen)
                        in_degree[gen] += 1
            else:
                # Track missing dependencies for reporting
                if gen not in missing_deps:
                    missing_deps[gen] = set()
                missing_deps[gen].add(component_dep)

    # Report missing dependencies
    if missing_deps:
        print("⚠️  Missing dependencies detected:")
        for gen in sorted(missing_deps.keys()):
            missing_list = ', '.join(sorted(missing_deps[gen]))
            print(f"   • {gen} depends on: {missing_list}")
        raise RuntimeError(f"Cannot proceed: {len(missing_deps)} generator(s) have unmet dependencies")

    # Kahn's algorithm with deque for O(n) queue operations instead of O(n²)
    queue = collections.deque([gen for gen in generators if in_degree[gen] == 0])
    sorted_order = []

    while queue:
        current = queue.popleft()
        sorted_order.append(current)

        for neighbor in gen_graph[current]:
            in_degree[neighbor] -= 1
            if in_degree[neighbor] == 0:
                queue.append(neighbor)

    # Check for cycles
    if len(sorted_order) != len(generators):
        cyclic = sorted(set(generators) - set(sorted_order))
        error_msg = f"Dependency cycle detected among {len(cyclic)} generator(s): {', '.join(cyclic)}"
        print(f"❌ {error_msg}")
        raise RuntimeError(error_msg)

    return sorted_order


class GeneratorRunner:
    """Runs all IC generators and collects results"""

    def __init__(self, verbose: bool = False, timeout: int = 120):
        self.verbose = verbose
        self.timeout = timeout
        self.generators_dir = Path(__file__).parent
        self.results: Dict[str, Tuple[bool, str]] = {}
        self.start_time = None
        self.end_time = None
        self.dependency_graph: Dict[str, Set[str]] = {}
        self.component_to_gen: Dict[str, str] = {}

    async def run_generator(self, script_name: str) -> Tuple[bool, str]:
        """Run a single generator script"""
        script_path = self.generators_dir / script_name

        if not script_path.exists():
            return False, f"Script not found: {script_path}"

        try:
            # Build subprocess environment with MCP/Client on PYTHONPATH so
            # generator scripts can import mcp_infrastructure without sys.path hacks.
            mcp_client = str(self.generators_dir.parent.parent.parent.parent / "MCP" / "Client")
            env = os.environ.copy()
            env["PYTHONPATH"] = os.pathsep.join(
                filter(None, [mcp_client, env.get("PYTHONPATH", "")])
            )

            # Run the generator script
            result = await asyncio.to_thread(
                subprocess.run,
                ["python3", str(script_path)],
                cwd=str(self.generators_dir),
                capture_output=True,
                text=True,
                timeout=self.timeout,
                env=env,
            )

            if result.returncode == 0:
                return True, "Success"
            else:
                # Extract error message - take last 3 lines for context
                error_lines = result.stderr.strip().split('\n')
                error_msg = ' | '.join(error_lines[-3:]) if error_lines else "Unknown error"
                return False, f"Exit code {result.returncode}: {error_msg}"

        except subprocess.TimeoutExpired:
            return False, f"Timeout ({self.timeout}s)"
        except Exception as e:
            return False, str(e)

    async def run_all(self, generators: List[str]) -> Dict[str, Tuple[bool, str]]:
        """Run all generators in dependency order"""
        self.start_time = datetime.now()

        # Build dependency graph and sort generators
        self.dependency_graph, self.component_to_gen = build_dependency_graph(self.generators_dir, generators)
        sorted_generators = topological_sort(generators, self.dependency_graph, self.component_to_gen)

        print(f"🚀 Running {len(generators)} IC generators...")
        print(f"   Start time: {self.start_time.strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"   Execution order: Dependency-based (topologically sorted)")
        print()

        # Run generators sequentially in dependency order
        for i, script in enumerate(sorted_generators, 1):
            status_str = f"[{i:2d}/{len(generators)}]"
            print(f"{status_str} Running {script}...", end=" ", flush=True)

            success, message = await self.run_generator(script)
            self.results[script] = (success, message)

            if success:
                print("✅")
            else:
                print(f"❌ {message}")

        self.end_time = datetime.now()
        return self.results

    def print_summary(self):
        """Print summary of results"""
        print("\n" + "="*80)
        print("RESULTS SUMMARY")
        print("="*80)

        success_count = sum(1 for success, _ in self.results.values() if success)
        failure_count = len(self.results) - success_count
        total = len(self.results)

        # Summary statistics
        print(f"\n📊 Statistics:")
        print(f"   Total Generators:  {total}")
        print(f"   ✅ Successful:     {success_count}")
        print(f"   ❌ Failed:         {failure_count}")
        print(f"   Success Rate:      {success_count/total*100:.1f}%")

        if self.start_time and self.end_time:
            duration = (self.end_time - self.start_time).total_seconds()
            print(f"   Duration:          {duration:.1f}s")
            if total > 0:
                print(f"   Avg per Generator: {duration/total:.1f}s")

        # Failed generators
        if failure_count > 0:
            print(f"\n❌ Failed Generators ({failure_count}):")
            for script, (success, message) in sorted(self.results.items()):
                if not success:
                    print(f"   • {script}")
                    print(f"     → {message}")

        # Successful generators
        if success_count > 0 and self.verbose:
            print(f"\n✅ Successful Generators ({success_count}):")
            for script, (success, message) in sorted(self.results.items()):
                if success:
                    print(f"   • {script}")

        print("\n" + "="*80)

        # Exit code
        if failure_count == 0:
            print("✨ All generators completed successfully!")
            return 0
        else:
            print(f"⚠️  {failure_count} generator(s) failed. See details above.")
            return 1

    async def verify_output_files(self) -> None:
        """Verify that output .panda files were created and are valid"""
        print(f"\n🔍 Verifying output IC component files...")

        components_dir = self.generators_dir.parent / "Components"
        panda_files = list(components_dir.glob("*.panda")) if components_dir.exists() else []

        print(f"   Found {len(panda_files)} .panda files in {components_dir.name}/")

        invalid = []
        for panda_file in sorted(panda_files):
            size = panda_file.stat().st_size
            if size == 0:
                invalid.append((panda_file.name, "empty file"))
                continue
            with panda_file.open("rb") as f:
                magic = f.read(4)
            if magic != b"WPCF":
                invalid.append((panda_file.name, f"invalid header: {magic!r}"))

        if invalid:
            print(f"   ⚠️  {len(invalid)} invalid file(s):")
            for name, reason in invalid:
                print(f"     • {name}: {reason}")
        else:
            print(f"   ✅ All {len(panda_files)} files are valid")

        # List some files
        if panda_files:
            print(f"   Sample files:")
            for panda_file in sorted(panda_files)[:5]:
                size_kb = panda_file.stat().st_size / 1024
                print(f"     • {panda_file.name} ({size_kb:.1f} KB)")
            if len(panda_files) > 5:
                print(f"     ... and {len(panda_files) - 5} more")


async def main():
    """Main entry point"""
    import argparse

    parser = argparse.ArgumentParser(description="Run all IC generator scripts")
    parser.add_argument("--verbose", "-v", action="store_true", help="Verbose output")
    parser.add_argument("--timeout", "-t", type=int, default=120, help="Timeout in seconds per generator (default: 120)")
    args = parser.parse_args()

    runner = GeneratorRunner(verbose=args.verbose, timeout=args.timeout)

    try:
        # Discover generators dynamically
        generators = discover_generators(runner.generators_dir)
        if not generators:
            print(f"❌ No generator scripts found matching 'create_level*.py' pattern in {runner.generators_dir}")
            return 1

        # Run all generators
        await runner.run_all(generators)

        # Verify output files
        await runner.verify_output_files()

        # Print summary and return exit code
        return runner.print_summary()

    except KeyboardInterrupt:
        print("\n⏹️  Interrupted by user")
        return 1
    except RuntimeError as e:
        print(f"\n❌ Error: {e}")
        return 1
    except Exception as e:
        print(f"\n❌ Unexpected error: {e}")
        return 1


if __name__ == "__main__":
    exit_code = asyncio.run(main())
    sys.exit(exit_code)
