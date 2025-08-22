# MCP wiRedPanda Examples

This directory contains practical examples demonstrating how to use the MCP wiRedPanda server for digital circuit design and simulation.

## Available Examples

### 1. Simple AND Gate Circuit (`simple_and_gate.py`)

**What it demonstrates:**
- Creating basic circuit elements (inputs, logic gates, outputs)
- Connecting elements together
- Testing circuit behavior with simulation
- Saving circuits and exporting diagrams

**Run the example:**
```bash
cd examples
python3 simple_and_gate.py
```

**Expected output:**
- Creates a 2-input AND gate circuit
- Tests all 4 input combinations
- Verifies correct truth table behavior
- Saves circuit file and PNG diagram

### 2. Advanced Examples (Coming Soon)

- **D Flip-Flop Circuit**: Sequential logic with clock and data inputs
- **4-Bit Counter**: Multi-stage sequential circuit
- **7-Segment Display**: Decoder and display driver
- **Combinational Logic**: Complex logic functions
- **Memory Circuits**: Latches and registers

## Using Examples as Templates

Each example follows a standard pattern:

1. **Setup**: Start application and create new circuit
2. **Elements**: Create inputs, logic gates, and outputs
3. **Connections**: Wire elements together
4. **Testing**: Simulate and verify behavior
5. **Export**: Save circuit and generate documentation
6. **Cleanup**: Stop application

## Example Structure

```python
# Import required modules
import subprocess
import json

# Helper function for MCP calls
def call_mcp_tool(tool_name, args=None):
    # Implementation...

# Main example logic
def main():
    # 1. Start application
    # 2. Create circuit elements
    # 3. Connect elements
    # 4. Test circuit
    # 5. Save and export
    # 6. Cleanup

if __name__ == "__main__":
    main()
```

## Running Examples

### Prerequisites

1. **MCP Server Built**: Ensure the TypeScript server is compiled
   ```bash
   cd /workspace/mcp-wiredpanda
   npm run build
   ```

2. **Python Bridge**: Bridge script must be executable
   ```bash
   chmod +x src/wiredpanda_bridge.py
   ```

3. **wiRedPanda Built**: Main application should be available
   ```bash
   # Check if executable exists
   ls -la ../build/wiredpanda
   ```

### Execution

```bash
# Navigate to examples directory
cd /workspace/mcp-wiredpanda/examples

# Run a specific example
python3 simple_and_gate.py

# With debug output
DEBUG=1 python3 simple_and_gate.py
```

## Example Output Files

Running examples creates various output files:

- **`.panda` files**: Circuit definitions (wiRedPanda format)
- **`.png` files**: Circuit diagrams (PNG images)
- **`.svg` files**: Vector circuit diagrams (SVG format)
- **`.pdf` files**: Printable circuit documentation

## Customizing Examples

### Modifying Circuit Elements

```python
# Change element types
element = call_mcp_tool("create_element", {
    "element_type": "Nand",  # Try: Or, Nor, Xor, Xnor, Not
    "x": 150,
    "y": 125,
    "label": "MyGate"
})

# Different input types
input_element = call_mcp_tool("create_element", {
    "element_type": "InputSwitch",  # Try: InputVcc, InputGnd, Clock
    "x": 50,
    "y": 100
})

# Various output types
output_element = call_mcp_tool("create_element", {
    "element_type": "Display7",  # Try: Buzzer, AudioBox
    "x": 250,
    "y": 125
})
```

### Adding More Inputs

```python
# Create multiple inputs
inputs = []
for i in range(4):  # 4-input gate
    input_elem = call_mcp_tool("create_element", {
        "element_type": "InputButton",
        "x": 50,
        "y": 100 + (i * 25),
        "label": f"Input{i+1}"
    })
    inputs.append(input_elem['element'])

# Connect to multi-input gate
for i, input_elem in enumerate(inputs):
    call_mcp_tool("connect_elements", {
        "source_id": input_elem['id'],
        "source_port": 0,
        "target_id": gate['element']['id'],
        "target_port": i
    })
```

### Sequential Circuits

```python
# Create clock signal
clock = call_mcp_tool("create_element", {
    "element_type": "Clock",
    "x": 50,
    "y": 50,
    "label": "Clock"
})

# Set clock frequency
call_mcp_tool("set_element_property", {
    "element_id": clock['element']['id'],
    "property": "frequency", 
    "value": 2  # 2 Hz
})

# Create flip-flop
flipflop = call_mcp_tool("create_element", {
    "element_type": "DFlipFlop",
    "x": 150,
    "y": 100,
    "label": "FF1"
})

# Connect clock to flip-flop
call_mcp_tool("connect_elements", {
    "source_id": clock['element']['id'],
    "source_port": 0,
    "target_id": flipflop['element']['id'],
    "target_port": 1  # Clock input port
})
```

## Testing and Validation

### Automated Testing

```python
def test_truth_table(gate_id, input_ids, output_id, expected_table):
    """Test a logic gate against its expected truth table"""
    print(f"Testing gate {gate_id}...")
    
    for inputs, expected_output in expected_table:
        # Set all inputs
        for i, input_value in enumerate(inputs):
            call_mcp_tool("set_input_value", {
                "element_id": input_ids[i],
                "value": input_value
            })
        
        # Read output
        result = call_mcp_tool("get_output_value", {
            "element_id": output_id
        })
        
        actual_output = result['value']
        status = "PASS" if actual_output == expected_output else "FAIL"
        
        print(f"  Inputs: {inputs} → Output: {actual_output} ({status})")

# Usage
and_truth_table = [
    ([False, False], False),
    ([False, True], False), 
    ([True, False], False),
    ([True, True], True)
]

test_truth_table(and_gate_id, [input1_id, input2_id], output_id, and_truth_table)
```

### Performance Measurement

```python
import time

def measure_simulation_performance():
    """Measure simulation performance"""
    start_time = time.time()
    
    # Start simulation
    call_mcp_tool("simulation_control", {"action": "start"})
    
    # Run multiple simulation steps
    for i in range(100):
        call_mcp_tool("simulation_control", {"action": "step"})
    
    end_time = time.time()
    
    print(f"100 simulation steps took {end_time - start_time:.3f} seconds")
    
    # Stop simulation
    call_mcp_tool("simulation_control", {"action": "stop"})
```

## Troubleshooting Examples

### Common Issues

1. **Bridge script not found**
   ```bash
   # Ensure you're in the examples directory
   pwd  # Should be /workspace/mcp-wiredpanda/examples
   ls -la ../src/wiredpanda_bridge.py
   ```

2. **Permission denied**
   ```bash
   chmod +x ../src/wiredpanda_bridge.py
   ```

3. **Application not starting**
   ```bash
   # Check if wiRedPanda is built
   ls -la ../../build/wiredpanda
   
   # Test bridge directly
   python3 ../src/wiredpanda_bridge.py "start" "{}"
   ```

### Debug Mode

Enable debug output:

```python
import os
os.environ['DEBUG'] = '1'

# Add debug prints to your example
def call_mcp_tool(tool_name, args=None):
    print(f"DEBUG: Calling {tool_name} with {args}")
    # ... existing implementation
    print(f"DEBUG: Result: {result}")
    return result
```

## Contributing Examples

To add new examples:

1. **Follow naming convention**: `descriptive_name.py`
2. **Include header comment**: Explain what the example demonstrates
3. **Add error handling**: Use try/catch for robustness
4. **Document parameters**: Comment complex configurations
5. **Test thoroughly**: Verify example works in clean environment
6. **Update this README**: Add your example to the list

### Example Template

```python
#!/usr/bin/env python3
"""
Example: [Brief description]

This example demonstrates how to:
1. [Key feature 1]
2. [Key feature 2]
3. [Key feature 3]
"""

import subprocess
import json

def call_mcp_tool(tool_name, args=None):
    """Helper function to call MCP tools via the bridge script"""
    # Standard implementation...

def main():
    """Main example logic"""
    print("🔧 [Example Name]")
    print("=" * 40)
    
    try:
        # Your example implementation here
        pass
        
    except Exception as e:
        print(f"Error: {e}")
        return 1
    
    print("\n🎉 Example completed successfully!")
    return 0

if __name__ == "__main__":
    exit(main())
```

Happy circuit designing! 🔌⚡