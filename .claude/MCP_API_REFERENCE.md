# wiRedPanda MCP - Complete API Reference

## Commands by Category

### FILE OPERATIONS (6 commands)

#### new_circuit
Create empty circuit in new tab.
Request: {"jsonrpc": "2.0", "method": "new_circuit", "params": {}, "id": 1}
Response: {"jsonrpc": "2.0", "result": {}, "id": 1}

#### load_circuit
Load .panda circuit file.
Request: {"method": "load_circuit", "params": {"filename": "/path/to/circuit.panda"}, "id": 2}

#### save_circuit
Save current circuit to file.
Request: {"method": "save_circuit", "params": {"filename": "/path/to/save.panda"}, "id": 3}

#### close_circuit
Close current circuit.

#### get_tab_count
Get number of open circuit tabs.

#### export_image
Export circuit as image file.
Formats: png, svg, pdf

### ELEMENT OPERATIONS (7 commands)

#### create_element
Create circuit element at position.
Params: type (required), x, y, label (optional)
Returns: element_id

#### delete_element
Delete element by ID.

#### list_elements
List all elements in circuit.

#### move_element
Reposition element.

#### set_element_properties
Modify element properties (label, etc).

#### set_input_value
Set input element state (boolean).

#### get_output_value
Read output element state.

### CONNECTION OPERATIONS (3 commands)

#### connect_elements
Connect output port to input port.
Params: source_id, source_port, target_id, target_port

#### disconnect_elements
Remove connection between elements.

#### list_connections
Get all connections in circuit.

### SIMULATION OPERATIONS (9 commands)

#### simulation_control
Control simulation.
Actions: start, stop, restart, update

#### undo
Execute undo operation on current circuit.
Returns: success status, can_undo, can_redo, undo_text, redo_text

#### redo
Execute redo operation on current circuit.
Returns: success status, can_undo, can_redo, undo_text, redo_text

#### get_undo_stack
Get undo/redo stack information.
Returns: can_undo, can_redo, undo_text, redo_text, undo_limit, undo_count, undo_index

#### create_waveform
Initialize waveform recording.

#### export_waveform
Export recorded waveform data.

#### create_ic
Create IC from current circuit.

#### instantiate_ic
Place IC instance in circuit.

#### list_ics
List all available ICs.

### SERVER INFO (1 command)

#### get_server_info
Get server version and capabilities.

## Element Types (32 total)

Logic Gates (7): And, Or, Not, Nor, Nand, Xor, Xnor
Inputs (7): InputButton, InputSwitch, InputRotary, InputVcc, InputGnd, Clock, TruthTable
Outputs (6): Led, Buzzer, AudioBox, Display7, Display14, Display16
Memory (6): DFlipFlop, JKFlipFlop, TFlipFlop, SRFlipFlop, DLatch, SRLatch
Mux (2): Mux, Demux
Structural (4): Node, Line, Text, IC

## Response Format

Success: {"jsonrpc": "2.0", "result": {...}, "id": 1}
Error: {"jsonrpc": "2.0", "error": {"code": -32603, "message": "..."}, "id": 1}

## Error Codes

-32700: Parse error (invalid JSON)
-32600: Invalid Request
-32601: Method not found
-32602: Invalid params
-32603: Internal error
