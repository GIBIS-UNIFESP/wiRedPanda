#!/usr/bin/env python3
"""
Generate Tinkercad circuit setup files for wiRedPanda Arduino exports.
Creates JSON component lists and markdown setup guides.
"""

import os
import re
import json
from pathlib import Path

def parse_arduino_file(file_path):
    """Parse an Arduino .ino file to extract pin assignments and components."""
    with open(file_path, 'r') as f:
        content = f.read()

    # Extract inputs
    inputs = []
    input_matches = re.finditer(r'const int (\w+) = (A?\d+);.*// Input', content)
    for match in input_matches:
        inputs.append({
            'name': match.group(1),
            'pin': match.group(2),
            'type': 'INPUT'
        })

    # Extract outputs
    outputs = []
    output_matches = re.finditer(r'const int (\w+) = (A?\d+);', content)
    for match in output_matches:
        name = match.group(1)
        pin = match.group(2)

        # Determine component type from name
        if any(word in name.lower() for word in ['led', 'light']):
            comp_type = 'LED'
        elif any(word in name.lower() for word in ['buzzer', 'speaker']):
            comp_type = 'BUZZER'
        elif any(word in name.lower() for word in ['servo', 'motor']):
            comp_type = 'SERVO'
        elif any(word in name.lower() for word in ['display', 'segment']):
            comp_type = 'DISPLAY'
        else:
            comp_type = 'OUTPUT'

        outputs.append({
            'name': name,
            'pin': pin,
            'type': comp_type
        })

    # Look for specific input types in setup or comments
    input_types = []
    for line in content.split('\n'):
        if 'pinMode(' in line and 'INPUT' in line:
            pin_match = re.search(r'pinMode\((\w+), INPUT\)', line)
            if pin_match:
                pin_name = pin_match.group(1)
                if any(word in pin_name.lower() for word in ['button', 'switch']):
                    if 'button' in pin_name.lower():
                        comp_type = 'PUSHBUTTON'
                    else:
                        comp_type = 'SWITCH'
                else:
                    comp_type = 'INPUT'

                input_types.append({
                    'name': pin_name,
                    'type': comp_type
                })

    # Extract clock frequencies for timing info
    clocks = []
    clock_matches = re.finditer(r'aux_clock_\d+_interval = (\d+(?:\.\d+)?);', content)
    for match in clock_matches:
        interval = float(match.group(1))
        frequency = 1000.0 / interval if interval > 0 else 0
        clocks.append({
            'interval_ms': interval,
            'frequency_hz': frequency
        })

    return {
        'inputs': input_types,
        'outputs': outputs,
        'clocks': clocks
    }

def generate_tinkercad_json(circuit_data, filename):
    """Generate a JSON file with component specifications for Tinkercad."""
    components = []

    # Add Arduino board
    components.append({
        'type': 'Arduino Uno R3',
        'id': 'arduino1',
        'position': {'x': 0, 'y': 0}
    })

    # Add breadboard
    components.append({
        'type': 'Breadboard',
        'id': 'breadboard1',
        'position': {'x': 200, 'y': 0}
    })

    # Add input components
    y_offset = 100
    for i, inp in enumerate(circuit_data['inputs']):
        if inp['type'] == 'PUSHBUTTON':
            comp_type = 'Pushbutton'
        elif inp['type'] == 'SWITCH':
            comp_type = 'Slide Switch'
        else:
            comp_type = 'Generic Input'

        components.append({
            'type': comp_type,
            'id': f"input_{i}",
            'name': inp['name'],
            'pin': inp.get('pin', f'A{i}'),
            'position': {'x': 400, 'y': y_offset + i * 50}
        })

    # Add output components
    for i, out in enumerate(circuit_data['outputs']):
        if out['type'] == 'LED':
            comp_type = 'LED'
            resistor_needed = True
        elif out['type'] == 'BUZZER':
            comp_type = 'Piezo Buzzer'
            resistor_needed = False
        elif out['type'] == 'SERVO':
            comp_type = 'Servo'
            resistor_needed = False
        elif out['type'] == 'DISPLAY':
            comp_type = '7-Segment Display'
            resistor_needed = True
        else:
            comp_type = 'Generic Output'
            resistor_needed = False

        components.append({
            'type': comp_type,
            'id': f"output_{i}",
            'name': out['name'],
            'pin': out['pin'],
            'position': {'x': 600, 'y': y_offset + i * 50},
            'resistor_needed': resistor_needed
        })

        # Add resistor if needed
        if resistor_needed:
            components.append({
                'type': 'Resistor',
                'id': f"resistor_{i}",
                'value': '220 Ω',
                'position': {'x': 550, 'y': y_offset + i * 50}
            })

    return {
        'filename': filename,
        'components': components,
        'clock_info': circuit_data['clocks'],
        'total_pins_used': len(circuit_data['inputs']) + len(circuit_data['outputs'])
    }

def generate_setup_guide(tinkercad_data, filename):
    """Generate a markdown setup guide for Tinkercad."""
    base_name = Path(filename).stem

    guide = f"""# Tinkercad Setup Guide: {base_name}

## Overview
This guide helps you recreate the circuit for `{filename}` in Tinkercad.

## Required Components
"""

    # Count components by type
    component_counts = {}
    for comp in tinkercad_data['components']:
        comp_type = comp['type']
        component_counts[comp_type] = component_counts.get(comp_type, 0) + 1

    for comp_type, count in component_counts.items():
        guide += f"- {count}x {comp_type}\n"

    guide += f"\n## Pin Usage\nTotal pins used: {tinkercad_data['total_pins_used']}/20 available\n\n"

    # Input connections
    inputs = [c for c in tinkercad_data['components'] if 'input_' in c.get('id', '')]
    if inputs:
        guide += "### Input Connections\n"
        for inp in inputs:
            guide += f"- **{inp['name']}** ({inp['type']}) → Arduino pin {inp['pin']}\n"
        guide += "\n"

    # Output connections
    outputs = [c for c in tinkercad_data['components'] if 'output_' in c.get('id', '')]
    if outputs:
        guide += "### Output Connections\n"
        for out in outputs:
            resistor_note = " (with 220Ω resistor)" if out.get('resistor_needed') else ""
            guide += f"- **{out['name']}** ({out['type']}) → Arduino pin {out['pin']}{resistor_note}\n"
        guide += "\n"

    # Clock information
    if tinkercad_data['clock_info']:
        guide += "### Timing Information\n"
        for i, clock in enumerate(tinkercad_data['clock_info']):
            guide += f"- Clock {i+1}: {clock['interval_ms']}ms interval ({clock['frequency_hz']:.2f} Hz)\n"
        guide += "\n"

    # Setup steps
    guide += """## Setup Steps in Tinkercad

1. **Add Arduino Uno R3**
   - Drag Arduino Uno R3 to the workspace

2. **Add Breadboard**
   - Place a breadboard next to the Arduino

3. **Add Input Components**
"""

    for inp in inputs:
        guide += f"   - Add {inp['type']} for {inp['name']}\n"
        guide += f"   - Connect to Arduino pin {inp['pin']}\n"
        guide += f"   - Connect other terminal to GND (for buttons/switches)\n\n"

    guide += "4. **Add Output Components**\n"
    for out in outputs:
        guide += f"   - Add {out['type']} for {out['name']}\n"
        if out.get('resistor_needed'):
            guide += f"   - Add 220Ω resistor in series\n"
        guide += f"   - Connect to Arduino pin {out['pin']}\n"
        if out['type'] == 'LED':
            guide += f"   - Connect cathode to GND\n"
        guide += "\n"

    guide += """5. **Upload Code**
   - Copy the contents of the .ino file
   - Paste into Tinkercad's code editor
   - Click "Start Simulation"

## Notes
- Ensure all connections are secure
- Check pin assignments match the code
- Use appropriate resistor values for LEDs
- Test inputs work correctly before running simulation
"""

    return guide

def main():
    """Process all .ino files and generate setup files."""
    examples_dir = Path('examples')
    output_dir = Path('.claude/tinkercad_setups')
    output_dir.mkdir(parents=True, exist_ok=True)

    # Summary for all circuits
    all_circuits = []

    for ino_file in examples_dir.glob('*.ino'):
        print(f"Processing {ino_file.name}...")

        try:
            # Parse the Arduino file
            circuit_data = parse_arduino_file(ino_file)

            # Generate Tinkercad JSON
            tinkercad_data = generate_tinkercad_json(circuit_data, ino_file.name)

            # Save JSON file
            json_file = output_dir / f"{ino_file.stem}_tinkercad.json"
            with open(json_file, 'w') as f:
                json.dump(tinkercad_data, f, indent=2)

            # Generate setup guide
            setup_guide = generate_setup_guide(tinkercad_data, ino_file.name)

            # Save markdown guide
            md_file = output_dir / f"{ino_file.stem}_setup.md"
            with open(md_file, 'w') as f:
                f.write(setup_guide)

            # Add to summary
            all_circuits.append({
                'name': ino_file.stem,
                'pins_used': tinkercad_data['total_pins_used'],
                'components': len(tinkercad_data['components']),
                'inputs': len(circuit_data['inputs']),
                'outputs': len(circuit_data['outputs'])
            })

            print(f"  ✓ Created {json_file.name} and {md_file.name}")

        except Exception as e:
            print(f"  ✗ Error processing {ino_file.name}: {e}")

    # Generate master summary
    summary = "# Tinkercad Setup Files Summary\n\n"
    summary += "| Circuit | Pins Used | Components | Inputs | Outputs | Setup Guide | JSON File |\n"
    summary += "|---------|-----------|------------|--------|---------|-------------|----------|\n"

    for circuit in all_circuits:
        name = circuit['name']
        summary += f"| {name} | {circuit['pins_used']}/20 | {circuit['components']} | {circuit['inputs']} | {circuit['outputs']} | [{name}_setup.md]({name}_setup.md) | [{name}_tinkercad.json]({name}_tinkercad.json) |\n"

    summary_file = output_dir / "README.md"
    with open(summary_file, 'w') as f:
        f.write(summary)

    print(f"\n✓ Generated setup files for {len(all_circuits)} circuits in {output_dir}")
    print(f"✓ Master summary: {summary_file}")

if __name__ == "__main__":
    main()