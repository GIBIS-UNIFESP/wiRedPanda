# 🔧 **PHASE 4: Circuit Builder DSL**

**Duration**: 3-4 days | **Risk**: Medium | **Impact**: Very High

### **4.1 Fluent Circuit Builder Core** (Day 8-9)

**Objective**: Create a Domain-Specific Language for circuit construction that reduces verbose boilerplate from 50+ lines to 5-10 lines while improving readability and maintainability.

**Tasks**:
```python
# Create: framework/circuit_builder.py
from typing import Dict, List, Optional, Callable, Any, Union
from dataclasses import dataclass, field
from .domain_types import ElementID, Position, Connection, Port, CircuitLayout, CircuitResult
from .base_validator import CPUValidationFramework
import time

@dataclass
class AutoLayoutManager:
    """Manages automatic positioning of circuit elements."""
    
    def __init__(self, grid_config: GridConfig):
        self.grid = grid_config
        self.input_positions: List[Position] = []
        self.gate_positions: List[Position] = []
        self.output_positions: List[Position] = []
        self.current_gate_row = 0
        self.current_gate_col = 2  # Start gates at column 2
        
    def get_input_position(self, index: int) -> Position:
        """Calculate input position with collision avoidance."""
        pos = self.grid.get_position(0, index)
        self.input_positions.append(pos)
        return pos
        
    def get_next_gate_position(self, gate_type: str) -> Position:
        """Smart gate positioning based on circuit flow."""
        pos = self.grid.get_position(self.current_gate_col, self.current_gate_row)
        
        # Update position for next gate
        self.current_gate_row += 1
        if self.current_gate_row > 3:  # Max 4 gates per column
            self.current_gate_row = 0
            self.current_gate_col += 1
            
        self.gate_positions.append(pos)
        return pos
        
    def get_output_position(self, index: int) -> Position:
        """Calculate output position."""
        # Place outputs to the right of all gates
        output_col = max(self.current_gate_col + 1, 4)  # Minimum column 4
        pos = self.grid.get_position(output_col, index)
        self.output_positions.append(pos)
        return pos
        
    def optimize_layout(self) -> None:
        """Post-build layout optimization for better visual appeal."""
        # Future: Implement layout optimization algorithms
        pass

@dataclass
class GateSpec:
    """Specification for creating a gate with automatic connections."""
    gate_type: str
    name: str
    inputs: Optional[List[str]] = None
    auto_connect: bool = True

class CircuitBuilder:
    """Fluent interface for building circuits declaratively."""
    
    def __init__(self, validator: CPUValidationFramework):
        self.validator = validator
        self.elements: Dict[str, ElementID] = {}
        self.connections: List[Connection] = []
        self.input_names: List[str] = []
        self.output_names: List[str] = []
        self.layout_manager = AutoLayoutManager(
            GridConfig(
                start_x=validator.GRID_START_X,
                start_y=validator.GRID_START_Y,
                horizontal_spacing=validator.MIN_HORIZONTAL_SPACING,
                vertical_spacing=validator.MIN_VERTICAL_SPACING
            )
        )
        
    def add_inputs(self, *names: str) -> 'CircuitBuilder':
        """Add input elements with automatic positioning."""
        for i, name in enumerate(names):
            pos = self.layout_manager.get_input_position(i)
            element_id_value = self.validator.create_element("InputButton", pos.x, pos.y, name)
            if element_id_value is None:
                raise ValueError(f"Failed to create input element '{name}'")
            
            element_id = ElementID.create(element_id_value, "InputButton", name, pos)
            self.elements[name] = element_id
            self.input_names.append(name)
            
        return self
        
    def add_gate(self, gate_type: str, name: str, inputs: Optional[List[str]] = None) -> 'CircuitBuilder':
        """Add logic gate with automatic positioning and optional auto-connection."""
        pos = self.layout_manager.get_next_gate_position(gate_type)
        element_id_value = self.validator.create_element(gate_type, pos.x, pos.y, name)
        if element_id_value is None:
            raise ValueError(f"Failed to create gate '{name}' of type '{gate_type}'")
        
        element_id = ElementID.create(element_id_value, gate_type, name, pos)
        self.elements[name] = element_id
        
        # Auto-connect if inputs specified
        if inputs:
            self._auto_connect_inputs(name, inputs)
            
        return self
        
    def add_gates(self, gates: List[Union[GateSpec, Tuple[str, str, List[str]]]]) -> 'CircuitBuilder':
        """Add multiple gates at once for more compact circuit definitions."""
        for gate in gates:
            if isinstance(gate, tuple):
                gate_type, name, inputs = gate
                self.add_gate(gate_type, name, inputs)
            else:
                self.add_gate(gate.gate_type, gate.name, gate.inputs)
        return self
        
    def connect(self, source: str, target: str, source_port: int = 0, target_port: int = 0) -> 'CircuitBuilder':
        """Connect two elements by name."""
        if source not in self.elements:
            raise ValueError(f"Source element '{source}' not found")
        if target not in self.elements:
            raise ValueError(f"Target element '{target}' not found")
            
        source_element = self.elements[source]
        target_element = self.elements[target]
        
        connection = Connection(
            source=Port(source_element, source_port),
            target=Port(target_element, target_port)
        )
        
        # Validate and create physical connection
        connection.validate()
        success = self.validator.connect_elements(
            source_element.value, source_port,
            target_element.value, target_port
        )
        
        if not success:
            raise ValueError(f"Failed to connect {source} to {target}")
            
        self.connections.append(connection)
        return self
        
    def connect_chain(self, *element_names: str) -> 'CircuitBuilder':
        """Connect elements in a chain: A -> B -> C -> D."""
        for i in range(len(element_names) - 1):
            self.connect(element_names[i], element_names[i + 1])
        return self
        
    def connect_fan_out(self, source: str, *targets: str) -> 'CircuitBuilder':
        """Connect one source to multiple targets."""
        for target in targets:
            self.connect(source, target)
        return self
        
    def connect_fan_in(self, *sources: str, target: str) -> 'CircuitBuilder':
        """Connect multiple sources to one target (for multi-input gates)."""
        for i, source in enumerate(sources):
            self.connect(source, target, target_port=i)
        return self
        
    def add_outputs(self, *names: str) -> 'CircuitBuilder':
        """Add output LEDs with automatic positioning."""
        for i, name in enumerate(names):
            pos = self.layout_manager.get_output_position(i)
            element_id_value = self.validator.create_element("Led", pos.x, pos.y, name)
            if element_id_value is None:
                raise ValueError(f"Failed to create output element '{name}'")
            
            element_id = ElementID.create(element_id_value, "Led", name, pos)
            self.elements[name] = element_id
            self.output_names.append(name)
            
        return self
        
    def with_layout_optimization(self) -> 'CircuitBuilder':
        """Enable layout optimization (applied during build)."""
        # Future: Enable various layout optimization algorithms
        return self
        
    def build(self) -> CircuitResult:
        """Finalize circuit and return result."""
        # Optimize layout if requested
        self.layout_manager.optimize_layout()
        
        # Create circuit layout
        layout = CircuitLayout(
            elements=self.elements,
            connections=self.connections,
            input_elements=self.input_names,
            output_elements=self.output_names
        )
        
        # Collect input/output IDs
        input_ids = [self.elements[name] for name in self.input_names]
        output_ids = [self.elements[name] for name in self.output_names]
        
        return CircuitResult(
            layout=layout,
            input_ids=input_ids,
            output_ids=output_ids,
            creation_time=time.time(),
            element_count=len(self.elements),
            connection_count=len(self.connections)
        )
        
    def _auto_connect_inputs(self, gate_name: str, input_names: List[str]) -> None:
        """Automatically connect specified inputs to gate ports."""
        for i, input_name in enumerate(input_names):
            if input_name in self.elements:
                self.connect(input_name, gate_name, target_port=i)
            else:
                raise ValueError(f"Input '{input_name}' not found for auto-connection to '{gate_name}'")
                
    def _get_input_names(self) -> List[str]:
        """Get list of input element names."""
        return self.input_names
        
    def _get_output_names(self) -> List[str]:
        """Get list of output element names."""
        return self.output_names

# Convenience functions for common circuit patterns
def create_simple_gate_circuit(validator: CPUValidationFramework, 
                              gate_type: str, 
                              inputs: List[str],
                              output_name: str = "OUT") -> CircuitResult:
    """Create a simple gate circuit with specified inputs."""
    return (CircuitBuilder(validator)
            .add_inputs(*inputs)
            .add_gate(gate_type, "GATE", inputs)
            .add_outputs(output_name)
            .connect("GATE", output_name)
            .build())

def create_compound_circuit(validator: CPUValidationFramework,
                          inputs: List[str],
                          gates: List[Tuple[str, str, List[str]]],
                          outputs: List[str],
                          final_connections: List[Tuple[str, str]]) -> CircuitResult:
    """Create a compound circuit with multiple gates and connections."""
    builder = (CircuitBuilder(validator)
              .add_inputs(*inputs)
              .add_gates(gates)
              .add_outputs(*outputs))
    
    # Add final connections
    for source, target in final_connections:
        builder.connect(source, target)
    
    return builder.build()
```

### **4.2 Advanced DSL Features** (Day 9)

**Objective**: Add advanced DSL features for complex circuits and reusable patterns.

**Tasks**:
```python
# Extend: framework/circuit_builder.py

class CircuitTemplate:
    """Template for creating reusable circuit patterns."""
    
    def __init__(self, name: str, 
                 input_spec: List[str], 
                 output_spec: List[str],
                 builder_func: Callable[['CircuitBuilder'], 'CircuitBuilder']):
        self.name = name
        self.input_spec = input_spec
        self.output_spec = output_spec
        self.builder_func = builder_func
    
    def create_circuit(self, validator: CPUValidationFramework, 
                      input_names: Optional[List[str]] = None,
                      output_names: Optional[List[str]] = None) -> CircuitResult:
        """Create circuit from template."""
        actual_inputs = input_names or self.input_spec
        actual_outputs = output_names or self.output_spec
        
        if len(actual_inputs) != len(self.input_spec):
            raise ValueError(f"Expected {len(self.input_spec)} inputs, got {len(actual_inputs)}")
        if len(actual_outputs) != len(self.output_spec):
            raise ValueError(f"Expected {len(self.output_spec)} outputs, got {len(actual_outputs)}")
        
        builder = CircuitBuilder(validator).add_inputs(*actual_inputs)
        builder = self.builder_func(builder)
        return builder.add_outputs(*actual_outputs).build()

# Pre-defined circuit templates
class StandardCircuits:
    """Library of standard circuit templates."""
    
    @staticmethod
    def half_adder() -> CircuitTemplate:
        """Half adder template: Sum = A XOR B, Carry = A AND B."""
        def build_half_adder(builder: CircuitBuilder) -> CircuitBuilder:
            return (builder
                    .add_gate("Xor", "XOR1", ["A", "B"])
                    .add_gate("And", "AND1", ["A", "B"])
                    .add_outputs("Sum", "Carry")
                    .connect("XOR1", "Sum")
                    .connect("AND1", "Carry"))
        
        return CircuitTemplate("half_adder", ["A", "B"], ["Sum", "Carry"], build_half_adder)
    
    @staticmethod  
    def full_adder() -> CircuitTemplate:
        """Full adder template: Sum = A XOR B XOR Cin, Carry = (A AND B) OR (Cin AND (A XOR B))."""
        def build_full_adder(builder: CircuitBuilder) -> CircuitBuilder:
            return (builder
                    .add_gates([
                        ("Xor", "XOR1", ["A", "B"]),
                        ("Xor", "XOR2", ["XOR1", "Cin"]),
                        ("And", "AND1", ["A", "B"]),
                        ("And", "AND2", ["XOR1", "Cin"]),
                        ("Or", "OR1", ["AND1", "AND2"])
                    ])
                    .add_outputs("Sum", "Carry")
                    .connect("XOR2", "Sum")
                    .connect("OR1", "Carry"))
        
        return CircuitTemplate("full_adder", ["A", "B", "Cin"], ["Sum", "Carry"], build_full_adder)
    
    @staticmethod
    def multiplexer_2to1() -> CircuitTemplate:
        """2-to-1 multiplexer template."""
        def build_mux2to1(builder: CircuitBuilder) -> CircuitBuilder:
            return (builder
                    .add_gate("Not", "NOT_S", ["S"])
                    .add_gates([
                        ("And", "AND0", ["D0", "NOT_S"]),
                        ("And", "AND1", ["D1", "S"]),
                        ("Or", "OR1", ["AND0", "AND1"])
                    ])
                    .add_outputs("OUT")
                    .connect("OR1", "OUT"))
        
        return CircuitTemplate("mux2to1", ["D0", "D1", "S"], ["OUT"], build_mux2to1)
    
    @staticmethod
    def decoder_2to4() -> CircuitTemplate:
        """2-to-4 decoder with enable."""
        def build_decoder(builder: CircuitBuilder) -> CircuitBuilder:
            return (builder
                    .add_gates([
                        ("Not", "NOT_A", ["A"]),
                        ("Not", "NOT_B", ["B"]),
                        ("And", "AND0", ["NOT_A", "NOT_B", "EN"]),
                        ("And", "AND1", ["A", "NOT_B", "EN"]),
                        ("And", "AND2", ["NOT_A", "B", "EN"]),
                        ("And", "AND3", ["A", "B", "EN"])
                    ])
                    .add_outputs("OUT0", "OUT1", "OUT2", "OUT3")
                    .connect_fan_out("AND0", "OUT0")
                    .connect_fan_out("AND1", "OUT1")
                    .connect_fan_out("AND2", "OUT2")
                    .connect_fan_out("AND3", "OUT3"))
        
        return CircuitTemplate("decoder_2to4", ["A", "B", "EN"], ["OUT0", "OUT1", "OUT2", "OUT3"], build_decoder)

# Extended CircuitBuilder with template support
class CircuitBuilder:
    # ... existing methods ...
    
    def use_template(self, template: CircuitTemplate, 
                    input_mapping: Optional[Dict[str, str]] = None,
                    output_mapping: Optional[Dict[str, str]] = None) -> 'CircuitBuilder':
        """Incorporate a circuit template into the current builder."""
        # This is a complex feature for future implementation
        # Would need to merge template circuits into current circuit
        raise NotImplementedError("Template incorporation not yet implemented")
    
    def create_bus(self, name_prefix: str, width: int, element_type: str = "InputButton") -> 'CircuitBuilder':
        """Create a bus of elements (e.g., 4-bit input bus)."""
        names = [f"{name_prefix}[{i}]" for i in range(width)]
        
        if element_type == "InputButton":
            return self.add_inputs(*names)
        elif element_type == "Led":
            return self.add_outputs(*names)
        else:
            # Create gates in a bus configuration
            for i, name in enumerate(names):
                pos = self.layout_manager.get_next_gate_position(element_type)
                element_id_value = self.validator.create_element(element_type, pos.x, pos.y, name)
                if element_id_value is None:
                    raise ValueError(f"Failed to create bus element '{name}'")
                element_id = ElementID.create(element_id_value, element_type, name, pos)
                self.elements[name] = element_id
            
        return self
    
    def connect_bus_to_bus(self, source_prefix: str, target_prefix: str, width: int) -> 'CircuitBuilder':
        """Connect two buses bit by bit."""
        for i in range(width):
            source_name = f"{source_prefix}[{i}]"
            target_name = f"{target_prefix}[{i}]"
            self.connect(source_name, target_name)
        return self
    
    def add_intermediate_node(self, name: str) -> 'CircuitBuilder':
        """Add an intermediate connection node (useful for complex routing)."""
        # Use a buffer (two NOT gates) as an intermediate node
        pos1 = self.layout_manager.get_next_gate_position("Not")
        pos2 = self.layout_manager.get_next_gate_position("Not")
        
        not1_id = self.validator.create_element("Not", pos1.x, pos1.y, f"{name}_NOT1")
        not2_id = self.validator.create_element("Not", pos2.x, pos2.y, f"{name}_NOT2")
        
        if not1_id is None or not2_id is None:
            raise ValueError(f"Failed to create intermediate node '{name}'")
        
        # Connect the two NOTs to form a buffer
        self.validator.connect_elements(not1_id, 0, not2_id, 0)
        
        # Store the output of the buffer as the named node
        element_id = ElementID.create(not2_id, "Not", name, pos2)
        self.elements[name] = element_id
        
        return self
```

### **4.3 DSL Integration & Migration** (Day 10-11)

**Objective**: Systematically migrate existing verbose circuit creation methods to use the new DSL.

**Before & After Examples**:
```python
# BEFORE: 50+ lines (from existing code)
def _create_full_adder_circuit(self) -> Optional[Tuple[InputIDs, OutputIDs]]:
    """Create full adder circuit."""
    # Create inputs with proper grid positioning
    a_x, a_y = self._get_input_position(0)
    a_id = self.create_element("InputButton", a_x, a_y, "A")
    b_x, b_y = self._get_input_position(1)
    b_id = self.create_element("InputButton", b_x, b_y, "B")
    cin_x, cin_y = self._get_input_position(2)
    cin_id = self.create_element("InputButton", cin_x, cin_y, "Cin")

    if not all([a_id, b_id, cin_id]):
        return None

    # Create XOR gates for sum calculation with proper positioning
    xor1_x, xor1_y = self._get_grid_position(1, 0)
    xor1_id = self.create_element("Xor", xor1_x, xor1_y, "XOR1")  # A XOR B
    xor2_x, xor2_y = self._get_grid_position(2, 0)
    xor2_id = self.create_element("Xor", xor2_x, xor2_y, "XOR2")  # (A XOR B) XOR Cin

    # Create AND gates for carry calculation with proper positioning
    and1_x, and1_y = self._get_grid_position(1, 1)
    and1_id = self.create_element("And", and1_x, and1_y, "AND1")  # A AND B
    and2_x, and2_y = self._get_grid_position(2, 1)
    and2_id = self.create_element(
        "And", and2_x, and2_y, "AND2"
    )  # Cin AND (A XOR B)
    or_x, or_y = self._get_grid_position(3, 1)
    or_id = self.create_element("Or", or_x, or_y, "OR")  # Final carry

    gates = [xor1_id, xor2_id, and1_id, and2_id, or_id]
    if not all(gates):
        return None

    # Type assertions after validation
    assert all(gate_id is not None for gate_id in gates)
    assert all(x is not None for x in [a_id, b_id, cin_id])

    # Connect sum logic
    sum_connections = [
        (cast(int, a_id), 0, cast(int, xor1_id), 0),
        (cast(int, b_id), 0, cast(int, xor1_id), 1),
        (cast(int, xor1_id), 0, cast(int, xor2_id), 0),
        (cast(int, cin_id), 0, cast(int, xor2_id), 1),
    ]

    for source_id, source_port, target_id, target_port in sum_connections:
        if not self.connect_elements(source_id, source_port, target_id, target_port):
            return None

    # Connect carry logic
    carry_connections = [
        (cast(int, a_id), 0, cast(int, and1_id), 0),
        (cast(int, b_id), 0, cast(int, and1_id), 1),
        (cast(int, cin_id), 0, cast(int, and2_id), 0),
        (cast(int, xor1_id), 0, cast(int, and2_id), 1),
        (cast(int, and1_id), 0, cast(int, or_id), 0),
        (cast(int, and2_id), 0, cast(int, or_id), 1),
    ]

    for source_id, source_port, target_id, target_port in carry_connections:
        if not self.connect_elements(source_id, source_port, target_id, target_port):
            return None

    # Create outputs with proper grid positioning
    sum_out_x, sum_out_y = self._get_grid_position(3, 0)
    sum_out_id = self.create_element("Led", sum_out_x, sum_out_y, "SUM")
    carry_out_x, carry_out_y = self._get_grid_position(4, 1)
    carry_out_id = self.create_element("Led", carry_out_x, carry_out_y, "CARRY")

    if not all([sum_out_id, carry_out_id]):
        return None

    # Type assertions after validation
    assert all(x is not None for x in [sum_out_id, carry_out_id])

    # Connect outputs
    if not self.connect_elements(cast(int, xor2_id), 0, cast(int, sum_out_id), 0):
        return None
    if not self.connect_elements(cast(int, or_id), 0, cast(int, carry_out_id), 0):
        return None

    return [cast(int, a_id), cast(int, b_id), cast(int, cin_id)], [
        cast(int, sum_out_id),
        cast(int, carry_out_id),
    ]

# AFTER: 8 lines using DSL
def _create_full_adder_circuit(self) -> CircuitResult:
    """Create full adder circuit using DSL."""
    return (CircuitBuilder(self)
            .add_inputs("A", "B", "Cin")
            .add_gates([
                ("Xor", "XOR1", ["A", "B"]),
                ("Xor", "XOR2", ["XOR1", "Cin"]),
                ("And", "AND1", ["A", "B"]),
                ("And", "AND2", ["XOR1", "Cin"]),
                ("Or", "OR1", ["AND1", "AND2"])
            ])
            .add_outputs("Sum", "Carry")
            .connect("XOR2", "Sum")
            .connect("OR1", "Carry")
            .build())

# EVEN SHORTER: Using templates
def _create_full_adder_circuit(self) -> CircuitResult:
    """Create full adder circuit using template."""
    return StandardCircuits.full_adder().create_circuit(self)
```

**Migration Plan**:
1. **Phase 4a (Day 10)**: Migrate simple circuits (AND, OR, XOR gates)
2. **Phase 4b (Day 11)**: Migrate complex circuits (full adder, multiplexer, decoder)

**Implementation Tasks**:
```python
# Update: validators/combinational.py
class AdvancedCombinationalValidator(CPUValidationFramework):
    # ... existing methods ...
    
    def _create_3_input_and_circuit(self) -> CircuitResult:
        """Create 3-input AND gate circuit - MIGRATED TO DSL."""
        return create_simple_gate_circuit(self, "And", ["A", "B", "C"])
    
    def _create_4_input_or_circuit(self) -> CircuitResult:
        """Create 4-input OR gate circuit - MIGRATED TO DSL."""
        # For >2 inputs, need cascade approach
        return (CircuitBuilder(self)
                .add_inputs("A", "B", "C", "D")
                .add_gate("Or", "OR1", ["A", "B"])
                .add_gate("Or", "OR2", ["C", "D"])  
                .add_gate("Or", "OR_FINAL", ["OR1", "OR2"])
                .add_outputs("OUT")
                .connect("OR_FINAL", "OUT")
                .build())
    
    def _create_aoi22_circuit(self) -> CircuitResult:
        """Create AOI22 circuit: F = NOT((A AND B) OR (C AND D)) - MIGRATED TO DSL."""
        return (CircuitBuilder(self)
                .add_inputs("A", "B", "C", "D")
                .add_gates([
                    ("And", "AND1", ["A", "B"]),
                    ("And", "AND2", ["C", "D"]),
                    ("Or", "OR1", ["AND1", "AND2"]),
                    ("Not", "NOT1", ["OR1"])
                ])
                .add_outputs("OUT")
                .connect("NOT1", "OUT")
                .build())
    
    def _create_decoder_2_to_4_circuit(self) -> CircuitResult:
        """Create 2-to-4 decoder - MIGRATED TO DSL."""
        return StandardCircuits.decoder_2to4().create_circuit(
            self, ["A", "B", "EN"], ["OUT0", "OUT1", "OUT2", "OUT3"])
    
    def _create_mux_4_to_1_circuit(self) -> CircuitResult:
        """Create 4-to-1 multiplexer - MIGRATED TO DSL."""
        return (CircuitBuilder(self)
                .add_inputs("D0", "D1", "D2", "D3", "S0", "S1")
                .add_gates([
                    ("Not", "NOT_S0", ["S0"]),
                    ("Not", "NOT_S1", ["S1"]),
                    # Select decoding
                    ("And", "SEL0", ["NOT_S1", "NOT_S0"]),  # S1=0, S0=0
                    ("And", "SEL1", ["NOT_S1", "S0"]),     # S1=0, S0=1
                    ("And", "SEL2", ["S1", "NOT_S0"]),     # S1=1, S0=0
                    ("And", "SEL3", ["S1", "S0"]),        # S1=1, S0=1
                    # Data gating
                    ("And", "GATE0", ["D0", "SEL0"]),
                    ("And", "GATE1", ["D1", "SEL1"]),
                    ("And", "GATE2", ["D2", "SEL2"]),
                    ("And", "GATE3", ["D3", "SEL3"]),
                    # Final OR
                    ("Or", "OR1", ["GATE0", "GATE1"]),
                    ("Or", "OR2", ["GATE2", "GATE3"]),
                    ("Or", "OR_FINAL", ["OR1", "OR2"])
                ])
                .add_outputs("OUT")
                .connect("OR_FINAL", "OUT")
                .build())

# Update: validators/arithmetic.py  
class ArithmeticBlocksValidator(CPUValidationFramework):
    # ... existing methods ...
    
    def _create_half_adder_circuit(self) -> CircuitResult:
        """Create half adder circuit - MIGRATED TO DSL."""
        return StandardCircuits.half_adder().create_circuit(self)
    
    def _create_full_adder_circuit(self) -> CircuitResult:
        """Create full adder circuit - MIGRATED TO DSL."""  
        return StandardCircuits.full_adder().create_circuit(self)
    
    def _create_4_bit_adder_circuit(self) -> CircuitResult:
        """Create 4-bit ripple carry adder - MIGRATED TO DSL."""
        return (CircuitBuilder(self)
                .create_bus("A", 4, "InputButton")    # A[3:0]
                .create_bus("B", 4, "InputButton")    # B[3:0]
                .add_inputs("Cin")
                # Create 4 full adder stages
                .add_gates([
                    # Stage 0
                    ("Xor", "XOR1_0", ["A[0]", "B[0]"]),
                    ("Xor", "XOR2_0", ["XOR1_0", "Cin"]),
                    ("And", "AND1_0", ["A[0]", "B[0]"]),
                    ("And", "AND2_0", ["XOR1_0", "Cin"]),
                    ("Or", "OR_0", ["AND1_0", "AND2_0"]),
                    # Stage 1  
                    ("Xor", "XOR1_1", ["A[1]", "B[1]"]),
                    ("Xor", "XOR2_1", ["XOR1_1", "OR_0"]),
                    ("And", "AND1_1", ["A[1]", "B[1]"]),
                    ("And", "AND2_1", ["XOR1_1", "OR_0"]),
                    ("Or", "OR_1", ["AND1_1", "AND2_1"]),
                    # Continue for stages 2 and 3...
                ])
                .create_bus("Sum", 4, "Led")         # Sum[3:0]
                .add_outputs("Cout")
                .connect("XOR2_0", "Sum[0]")
                .connect("XOR2_1", "Sum[1]")  
                # Continue connections...
                .build())
```

**Testing Migration**:
```bash
# Test individual migrated circuits
python3 -c "
from validators.combinational import AdvancedCombinationalValidator
validator = AdvancedCombinationalValidator()
validator.create_new_circuit()
circuit = validator._create_full_adder_circuit()
print(f'Created circuit with {circuit.element_count} elements, {circuit.connection_count} connections')
validator.save_circuit('test_dsl_full_adder.panda')
"

# Run full validation to ensure no regressions
python3 validators/combinational.py   # Must still show 4/4 tests passed
python3 validators/arithmetic.py     # Must still show 4/4 tests passed
```

**Success Metrics**:
- Circuit creation code reduced by 80%+ (50+ lines → 5-10 lines)
- Improved readability and maintainability
- Zero functional regressions
- All tests continue to pass with identical results