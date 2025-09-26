#!/usr/bin/env python3

import subprocess
import os

def test_verilog_files():
    # Change to test/verilog directory
    os.chdir('test/verilog')

    # Get list of all .v files
    verilog_files = []
    for f in os.listdir('.'):
        if f.endswith('.v') and not f.endswith('-FIXED.v'):
            verilog_files.append(f)

    verilog_files.sort()
    results = {}

    print('Testing', len(verilog_files), 'Verilog files against EDA tools...')
    print('='*80)

    for filename in verilog_files:
        if not os.path.exists(filename):
            continue

        file_size = os.path.getsize(filename)
        results[filename] = {
            'size': file_size,
            'icarus': {'issues': 0, 'status': 'OK'},
            'verilator': {'issues': 0, 'status': 'OK'},
            'yosys': {'issues': 0, 'status': 'OK'}
        }

        print(f'Testing {filename} ({file_size} bytes)...')

        # Test with Icarus Verilog
        try:
            result = subprocess.run(['iverilog', '-Wall', filename],
                                  capture_output=True, text=True, timeout=10)
            if result.returncode != 0:
                results[filename]['icarus']['status'] = 'FAIL'
                stderr_lines = result.stderr.split('\n')
                error_count = len([l for l in stderr_lines if 'error' in l.lower() or 'syntax error' in l.lower()])
                warning_count = len([l for l in stderr_lines if 'warning' in l.lower()])
                results[filename]['icarus']['issues'] = error_count + warning_count
            else:
                results[filename]['icarus']['status'] = 'PASS'
        except Exception as e:
            results[filename]['icarus']['status'] = 'ERR'
            results[filename]['icarus']['issues'] = -1

        # Test with Verilator
        try:
            result = subprocess.run(['verilator', '--lint-only', filename],
                                  capture_output=True, text=True, timeout=10)
            stderr_lines = result.stderr.split('\n')
            error_count = len([l for l in stderr_lines if '%Error' in l])
            warning_count = len([l for l in stderr_lines if '%Warning' in l])
            results[filename]['verilator']['issues'] = error_count + warning_count
            if error_count > 0:
                results[filename]['verilator']['status'] = 'FAIL'
            elif warning_count > 0:
                results[filename]['verilator']['status'] = 'WARN'
            else:
                results[filename]['verilator']['status'] = 'PASS'
        except Exception as e:
            results[filename]['verilator']['status'] = 'ERR'
            results[filename]['verilator']['issues'] = -1

        # Test with Yosys
        try:
            yosys_script = f'read_verilog {filename}; synth'
            result = subprocess.run(['yosys', '-p', yosys_script],
                                  capture_output=True, text=True, timeout=15)
            yosys_output = result.stdout + result.stderr
            output_lines = yosys_output.split('\n')
            error_count = len([l for l in output_lines if 'Error:' in l])
            warning_count = len([l for l in output_lines if 'Warning:' in l])
            results[filename]['yosys']['issues'] = error_count + warning_count
            if error_count > 0:
                results[filename]['yosys']['status'] = 'FAIL'
            elif warning_count > 0:
                results[filename]['yosys']['status'] = 'WARN'
            else:
                results[filename]['yosys']['status'] = 'PASS'
        except Exception as e:
            results[filename]['yosys']['status'] = 'ERR'
            results[filename]['yosys']['issues'] = -1

    # Print results table
    print()
    print('='*80)
    print('COMPREHENSIVE EDA TOOL ISSUE ANALYSIS')
    print('='*80)
    print()
    print('| File                     | Size  | Icarus      | Verilator   | Yosys       |')
    print('|--------------------------|-------|-------------|-------------|-------------|')

    for filename in sorted(results.keys()):
        r = results[filename]
        size_kb = f'{r["size"]/1024:.1f}K' if r['size'] > 1024 else f'{r["size"]}B'

        icarus_cell = f'{r["icarus"]["status"]:>4}({r["icarus"]["issues"]:>2})'
        verilator_cell = f'{r["verilator"]["status"]:>4}({r["verilator"]["issues"]:>2})'
        yosys_cell = f'{r["yosys"]["status"]:>4}({r["yosys"]["issues"]:>2})'

        print(f'| {filename:<24} | {size_kb:>5} | {icarus_cell:<11} | {verilator_cell:<11} | {yosys_cell:<11} |')

    print()
    print('Legend: PASS=No issues, WARN=Warnings only, FAIL=Errors, ERR=Tool error')

    # Summary statistics
    total_files = len(results)
    icarus_pass = len([r for r in results.values() if r['icarus']['status'] == 'PASS'])
    verilator_pass = len([r for r in results.values() if r['verilator']['status'] == 'PASS'])
    yosys_pass = len([r for r in results.values() if r['yosys']['status'] == 'PASS'])

    print()
    print('SUMMARY:')
    print(f'  Total files tested: {total_files}')
    print(f'  Icarus Verilog:  {icarus_pass}/{total_files} PASS ({icarus_pass/total_files*100:.1f}%)')
    print(f'  Verilator:       {verilator_pass}/{total_files} PASS ({verilator_pass/total_files*100:.1f}%)')
    print(f'  Yosys:           {yosys_pass}/{total_files} PASS ({yosys_pass/total_files*100:.1f}%)')

    return results

if __name__ == '__main__':
    results = test_verilog_files()