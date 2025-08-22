# MCP wiRedPanda Example Usage

This document demonstrates how AI models can use the MCP server to control wiRedPanda.

## Basic Circuit Creation

### Example 1: Creating an AND Gate Circuit

```javascript
// Start a new circuit
await mcp.use_tool('new_circuit');

// Create two input switches
const switchA = await mcp.use_tool('create_element', {
  elementType: 'InputSwitch',
  x: 100,
  y: 100,
  label: 'Switch A'
});

const switchB = await mcp.use_tool('create_element', {
  elementType: 'InputSwitch',
  x: 100,
  y: 200,
  label: 'Switch B'
});

// Create an AND gate
const andGate = await mcp.use_tool('create_element', {
  elementType: 'And',
  x: 300,
  y: 150,
  label: 'AND1'
});

// Create an LED output
const led = await mcp.use_tool('create_element', {
  elementType: 'Led',
  x: 500,
  y: 150,
  label: 'Output'
});

// Connect the switches to the AND gate
await mcp.use_tool('connect_elements', {
  sourceId: switchA.element.id,
  sourcePort: 0,
  targetId: andGate.element.id,
  targetPort: 0
});

await mcp.use_tool('connect_elements', {
  sourceId: switchB.element.id,
  sourcePort: 0,
  targetId: andGate.element.id,
  targetPort: 1
});

// Connect AND gate to LED
await mcp.use_tool('connect_elements', {
  sourceId: andGate.element.id,
  sourcePort: 0,
  targetId: led.element.id,
  targetPort: 0
});

// Save the circuit
await mcp.use_tool('save_circuit', {
  filePath: 'and_gate_circuit.panda'
});
```

### Example 2: Creating a Half Adder

```javascript
// Create a half adder circuit
await mcp.use_tool('new_circuit');

// Inputs
const inputA = await mcp.use_tool('create_element', {
  elementType: 'InputButton',
  x: 50,
  y: 100,
  label: 'A'
});

const inputB = await mcp.use_tool('create_element', {
  elementType: 'InputButton',
  x: 50,
  y: 200,
  label: 'B'
});

// Logic gates
const xorGate = await mcp.use_tool('create_element', {
  elementType: 'Xor',
  x: 250,
  y: 100,
  label: 'Sum'
});

const andGate = await mcp.use_tool('create_element', {
  elementType: 'And',
  x: 250,
  y: 200,
  label: 'Carry'
});

// Outputs
const sumLed = await mcp.use_tool('create_element', {
  elementType: 'Led',
  x: 450,
  y: 100,
  label: 'Sum'
});

const carryLed = await mcp.use_tool('create_element', {
  elementType: 'Led',
  x: 450,
  y: 200,
  label: 'Carry'
});

// Wire up the XOR gate (Sum)
await mcp.use_tool('connect_elements', {
  sourceId: inputA.element.id,
  sourcePort: 0,
  targetId: xorGate.element.id,
  targetPort: 0
});

await mcp.use_tool('connect_elements', {
  sourceId: inputB.element.id,
  sourcePort: 0,
  targetId: xorGate.element.id,
  targetPort: 1
});

await mcp.use_tool('connect_elements', {
  sourceId: xorGate.element.id,
  sourcePort: 0,
  targetId: sumLed.element.id,
  targetPort: 0
});

// Wire up the AND gate (Carry)
await mcp.use_tool('connect_elements', {
  sourceId: inputA.element.id,
  sourcePort: 0,
  targetId: andGate.element.id,
  targetPort: 0
});

await mcp.use_tool('connect_elements', {
  sourceId: inputB.element.id,
  sourcePort: 0,
  targetId: andGate.element.id,
  targetPort: 1
});

await mcp.use_tool('connect_elements', {
  sourceId: andGate.element.id,
  sourcePort: 0,
  targetId: carryLed.element.id,
  targetPort: 0
});

// Save the circuit
await mcp.use_tool('save_circuit', {
  filePath: 'half_adder.panda'
});
```

## Simulation Control

### Running a Simulation

```javascript
// Load a circuit
await mcp.use_tool('load_circuit', {
  filePath: 'my_circuit.panda'
});

// Start the simulation
await mcp.use_tool('simulation_control', {
  action: 'start'
});

// Set input values
await mcp.use_tool('set_input_value', {
  elementId: 1,  // First input
  value: true    // HIGH
});

await mcp.use_tool('set_input_value', {
  elementId: 2,  // Second input
  value: false   // LOW
});

// Check output values
const output = await mcp.use_tool('get_output_value', {
  elementId: 10  // LED or output element
});
console.log('Output is:', output.value ? 'HIGH' : 'LOW');

// Step through simulation
await mcp.use_tool('simulation_control', {
  action: 'step'
});

// Stop simulation
await mcp.use_tool('simulation_control', {
  action: 'stop'
});
```

## Working with Flip-Flops

### Creating a D Flip-Flop Circuit

```javascript
await mcp.use_tool('new_circuit');

// Create clock input
const clock = await mcp.use_tool('create_element', {
  elementType: 'Clock',
  x: 50,
  y: 150
});

// Set clock frequency (e.g., 1 Hz)
await mcp.use_tool('set_element_property', {
  elementId: clock.element.id,
  property: 'frequency',
  value: 1
});

// Create data input
const dataInput = await mcp.use_tool('create_element', {
  elementType: 'InputSwitch',
  x: 50,
  y: 100,
  label: 'D'
});

// Create D flip-flop
const dFlipFlop = await mcp.use_tool('create_element', {
  elementType: 'DFlipFlop',
  x: 250,
  y: 125
});

// Create output LEDs
const qOutput = await mcp.use_tool('create_element', {
  elementType: 'Led',
  x: 450,
  y: 100,
  label: 'Q'
});

const qBarOutput = await mcp.use_tool('create_element', {
  elementType: 'Led',
  x: 450,
  y: 150,
  label: 'Q̄'
});

// Connect data input to D
await mcp.use_tool('connect_elements', {
  sourceId: dataInput.element.id,
  sourcePort: 0,
  targetId: dFlipFlop.element.id,
  targetPort: 0  // D input
});

// Connect clock
await mcp.use_tool('connect_elements', {
  sourceId: clock.element.id,
  sourcePort: 0,
  targetId: dFlipFlop.element.id,
  targetPort: 1  // Clock input
});

// Connect outputs
await mcp.use_tool('connect_elements', {
  sourceId: dFlipFlop.element.id,
  sourcePort: 0,  // Q output
  targetId: qOutput.element.id,
  targetPort: 0
});

await mcp.use_tool('connect_elements', {
  sourceId: dFlipFlop.element.id,
  sourcePort: 1,  // Q̄ output
  targetId: qBarOutput.element.id,
  targetPort: 0
});
```

## Circuit Analysis

### Listing and Analyzing Circuit Elements

```javascript
// Load a circuit
await mcp.use_tool('load_circuit', {
  filePath: 'complex_circuit.panda'
});

// List all elements
const elements = await mcp.use_tool('list_elements');
console.log('Circuit contains', elements.elements.length, 'elements');

// List all connections
const connections = await mcp.use_tool('list_connections');
console.log('Circuit has', connections.connections.length, 'connections');

// Get detailed info about a specific element
const elementInfo = await mcp.use_tool('get_element_info', {
  elementId: 5
});
console.log('Element 5 details:', elementInfo.element);

// Get simulation state
const simState = await mcp.use_tool('get_simulation_state');
console.log('Simulation running:', simState.state.running);
```

## Exporting Circuits

### Export as Image

```javascript
// Export circuit as PNG
await mcp.use_tool('export_image', {
  filePath: 'circuit_diagram.png',
  format: 'png'
});

// Export as SVG for scalable graphics
await mcp.use_tool('export_image', {
  filePath: 'circuit_diagram.svg',
  format: 'svg'
});

// Export as PDF for documentation
await mcp.use_tool('export_image', {
  filePath: 'circuit_diagram.pdf',
  format: 'pdf'
});
```

## Advanced Usage

### Creating a 4-bit Counter

```javascript
// This example shows how to create a more complex sequential circuit
await mcp.use_tool('new_circuit');

// Create clock source
const clock = await mcp.use_tool('create_element', {
  elementType: 'Clock',
  x: 50,
  y: 200
});

// Create 4 T flip-flops for 4-bit counter
const flipFlops = [];
for (let i = 0; i < 4; i++) {
  const ff = await mcp.use_tool('create_element', {
    elementType: 'TFlipFlop',
    x: 200 + (i * 150),
    y: 200,
    label: `T${i}`
  });
  flipFlops.push(ff);
}

// Create output display
const display = await mcp.use_tool('create_element', {
  elementType: 'Display7',
  x: 850,
  y: 200
});

// Connect clock to first flip-flop
await mcp.use_tool('connect_elements', {
  sourceId: clock.element.id,
  sourcePort: 0,
  targetId: flipFlops[0].element.id,
  targetPort: 1  // Clock input
});

// Chain flip-flops (ripple counter)
for (let i = 0; i < 3; i++) {
  await mcp.use_tool('connect_elements', {
    sourceId: flipFlops[i].element.id,
    sourcePort: 0,  // Q output
    targetId: flipFlops[i + 1].element.id,
    targetPort: 1   // Clock input of next FF
  });
}

// Connect outputs to display
for (let i = 0; i < 4; i++) {
  await mcp.use_tool('connect_elements', {
    sourceId: flipFlops[i].element.id,
    sourcePort: 0,  // Q output
    targetId: display.element.id,
    targetPort: i   // Display input pins
  });
}

// Save the counter circuit
await mcp.use_tool('save_circuit', {
  filePath: '4bit_counter.panda'
});
```

## Tips for AI Models

1. **Always start with `new_circuit` or `load_circuit`** before creating elements
2. **Element IDs are returned when creating elements** - store these for connections
3. **Port numbering starts at 0** for both inputs and outputs
4. **Save frequently** to avoid losing work
5. **Use labels** to make circuits more readable
6. **Check simulation state** before setting input values
7. **Export images** for documentation or sharing

## Error Handling

When using the MCP server, handle potential errors:

```javascript
try {
  const result = await mcp.use_tool('load_circuit', {
    filePath: 'nonexistent.panda'
  });
} catch (error) {
  if (error.message.includes('File not found')) {
    console.log('Circuit file does not exist');
  }
}
```

## Integration with AI Workflows

The MCP server enables AI models to:

1. **Generate circuits from descriptions**: Convert natural language to circuit designs
2. **Verify circuit behavior**: Test circuits against specifications
3. **Optimize designs**: Modify circuits for better performance
4. **Create educational examples**: Build circuits for teaching
5. **Debug issues**: Analyze and fix circuit problems
6. **Document circuits**: Generate reports with circuit diagrams
