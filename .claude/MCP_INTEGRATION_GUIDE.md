# wiRedPanda MCP - Integration & Examples Guide

## File Locations Summary

### Server Implementation (C++)
- mcp/server/core/mcpprocessor.h/cpp - Central command processor
- mcp/server/core/mcpvalidator.h/cpp - JSON Schema validation
- mcp/server/handlers/elementhandler.h/cpp - Element operations
- mcp/server/handlers/connectionhandler.h/cpp - Connection operations
- mcp/server/handlers/filehandler.h/cpp - File operations
- mcp/server/handlers/simulationhandler.h/cpp - Simulation operations
- mcp/schema-mcp.json - JSON Schema definition

### Client Implementation (Python)
- mcp/client/mcp_infrastructure.py - Process management
- mcp/client/mcp_runner.py - Test orchestration
- mcp/client/mcp_models.py - Pydantic response models
- mcp/client/protocol/commands.py - Command classes
- mcp/client/domain/enums.py - Element types
- mcp/client/examples/simple_circuit.py - Working examples
- mcp/client/tests/ - Comprehensive test suite

### Integration Points
- app/main.cpp - MCP mode initialization
- CMakeLists.txt - Build configuration

## Launching MCP Server

Command line options:
  --mcp              Run in MCP headless mode
  --mcp-gui          Show GUI window

Examples:
  ./wiredpanda --mcp
  ./wiredpanda --mcp --mcp-gui
  ./wiredpanda --mcp /path/to/circuit.panda

## Python Client Usage Pattern

1. Import and initialize:
   from mcp_infrastructure import MCPInfrastructure
   mcp = MCPInfrastructure(enable_validation=True)

2. Start server:
   await mcp.start_mcp()

3. Send commands:
   response = await mcp.send_command("create_element", {
       "type": "And", "x": 200.0, "y": 100.0, "label": "Gate1"
   })

4. Check response:
   if response.success:
       element_id = response.result["element_id"]

5. Clean up:
   await mcp.stop_mcp()

## 32 Supported Element Types

Logic Gates (7):
  And, Or, Not, Nor, Nand, Xor, Xnor

Input Elements (7):
  InputButton, InputSwitch, InputRotary, InputVcc, InputGnd, Clock, TruthTable

Output Elements (6):
  Led, Buzzer, AudioBox, Display7, Display14, Display16

Memory Elements (6):
  DFlipFlop, JKFlipFlop, TFlipFlop, SRFlipFlop, DLatch, SRLatch

Multiplexer Elements (2):
  Mux, Demux

Structural Elements (4):
  Node, Line, Text, IC

## Port System

Each element has input and output ports numbered 0-63.

AND gate:
  Input ports: 0 (A), 1 (B)
  Output ports: 0 (Q)

D Flip-Flop:
  Input ports: 0 (D), 1 (Clock), 2 (Reset)
  Output ports: 0 (Q), 1 (NOT Q)

Port connections: source_output -> target_input

## Validation Layers

1. JSON-RPC 2.0 Format (version, method, params, id)
2. JSON Schema Draft 7 (structure validation)
3. Pydantic Models (Python runtime validation)
4. beartype Decorators (function type checking)
5. Handler-Level (business logic validation)

## Performance

Response time: <10ms average
Throughput: 100+ commands/second
Validation overhead: <1ms
Memory: <50MB additional
Scalability: 1000+ elements

## Error Codes

-32700: Parse error
-32601: Method not found
-32602: Invalid params
-32603: Internal error
