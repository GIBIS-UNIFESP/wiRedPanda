#!/usr/bin/env python3
"""
Advanced seed generation targeting specific uncovered functions:
- ElementFactory::buildElement for each element type
- Connection parsing (loadConnections)  
- Complex element structures with properties
"""
import struct
import random
from pathlib import Path

def create_complete_header():
    """Create proper WPCF header"""
    data = bytearray()
    data.extend(b'WPCF')  # Magic header
    
    # Version (4.2.6) - 3 segments
    data.extend(struct.pack('<I', 3))  # segment count
    data.extend(struct.pack('<I', 4))  # major
    data.extend(struct.pack('<I', 2))  # minor
    data.extend(struct.pack('<I', 6))  # patch
    
    # Canvas rect
    data.extend(struct.pack('<I', 0))    # x
    data.extend(struct.pack('<I', 0))    # y  
    data.extend(struct.pack('<I', 1024)) # width
    data.extend(struct.pack('<I', 768))  # height
    
    # Dolphin filename (empty)
    data.extend(struct.pack('<I', 0))
    
    return data

def create_element_with_properties(element_type, element_id, x=100, y=100):
    """Create a complete element with all properties that trigger deeper parsing"""
    data = bytearray()
    
    # Element type (triggers ElementFactory::buildElement)
    data.extend(struct.pack('<I', element_type))
    
    # Element ID
    data.extend(struct.pack('<I', element_id))
    
    # Position (QPointF) - triggers position parsing
    data.extend(struct.pack('<f', float(x)))  # x as float
    data.extend(struct.pack('<f', float(y)))  # y as float
    
    # Rotation angle
    data.extend(struct.pack('<f', 0.0))
    
    # Input size (variable inputs trigger different code paths)
    if element_type in [5, 6, 7, 8, 10, 11]:  # Logic gates that can have variable inputs
        data.extend(struct.pack('<I', random.randint(2, 8)))  # 2-8 inputs
    else:
        data.extend(struct.pack('<I', 2))  # default 2 inputs
    
    # Output size
    data.extend(struct.pack('<I', 1))  # most elements have 1 output
    
    # Label (QString) - triggers string parsing paths
    labels = ["", "TEST", "GATE1", "INPUT_A", "OUTPUT_1", "CLOCK_100Hz"]
    label = labels[element_id % len(labels)]
    if label:
        data.extend(struct.pack('<I', len(label)))
        for char in label:
            data.extend(struct.pack('<H', ord(char)))  # UTF-16 encoding
    else:
        data.extend(struct.pack('<I', 0))  # empty label
    
    # Element-specific properties
    if element_type == 9:  # Clock
        # Frequency property
        data.extend(struct.pack('<f', 1000.0))  # 1kHz
    elif element_type in [1, 2]:  # Input elements  
        # On/off state
        data.extend(struct.pack('<B', 1 if element_id % 2 else 0))
    elif element_type == 21:  # InputRotary
        # Value property
        data.extend(struct.pack('<I', element_id % 16))
    elif element_type in [14, 27, 33]:  # Display elements
        # Color property
        data.extend(struct.pack('<I', element_id % 8))
        
    return data

def create_connection_data(from_elem_id, from_port, to_elem_id, to_port):
    """Create connection data to trigger QNEConnection parsing"""
    data = bytearray()
    
    # Connection type marker (this might trigger different parsing paths)
    data.extend(struct.pack('<I', 0x434F4E4E))  # "CONN" marker
    
    # Source element ID
    data.extend(struct.pack('<I', from_elem_id))
    
    # Source port index  
    data.extend(struct.pack('<I', from_port))
    
    # Target element ID
    data.extend(struct.pack('<I', to_elem_id))
    
    # Target port index
    data.extend(struct.pack('<I', to_port))
    
    # Connection properties
    data.extend(struct.pack('<I', 1))  # signal value
    data.extend(struct.pack('<f', 1.0))  # strength/weight
    
    return data

def create_comprehensive_circuit():
    """Create a complex circuit with many element types and connections"""
    data = create_complete_header()
    
    # All known element types from enums.h  
    element_types = [
        (1, "InputButton"), (2, "InputSwitch"), (3, "Led"), (5, "And"), 
        (6, "Nand"), (7, "Nor"), (8, "Or"), (9, "Clock"), (10, "Not"),
        (11, "Xor"), (12, "InputVcc"), (13, "InputGnd"), (14, "Display7"),
        (15, "DLatch"), (17, "DFlipFlop"), (18, "JKFlipFlop"), 
        (21, "InputRotary"), (22, "IC"), (25, "Demux"), (26, "Buzzer"),
        (27, "Display14"), (28, "Mux"), (29, "Line"), (30, "Node"),
        (31, "AudioBox"), (33, "Display16")
    ]
    
    # Elements list
    data.extend(struct.pack('<I', len(element_types)))  # element count
    
    for i, (elem_type, name) in enumerate(element_types):
        element_data = create_element_with_properties(elem_type, i, x=50 + i*40, y=100 + (i%4)*60)
        data.extend(element_data)
        
    # Connections (create a chain connecting many elements)  
    connections = []
    for i in range(min(10, len(element_types)-1)):  # Connect first 10 elements in chain
        connections.append((i, 0, i+1, 0))  # output 0 of element i to input 0 of element i+1
    
    # Add some complex connections (multiple inputs)
    connections.extend([
        (0, 0, 4, 0),   # InputButton to And gate input 0
        (1, 0, 4, 1),   # InputSwitch to And gate input 1  
        (4, 0, 2, 0),   # And gate output to Led
        (8, 0, 14, 0),  # Clock to DLatch
    ])
    
    data.extend(struct.pack('<I', len(connections)))  # connection count
    
    for from_elem, from_port, to_elem, to_port in connections:
        if from_elem < len(element_types) and to_elem < len(element_types):
            conn_data = create_connection_data(from_elem, from_port, to_elem, to_port)
            data.extend(conn_data)
    
    return data

def create_edge_case_seeds():
    """Create edge cases that might trigger error handling and rare code paths"""
    seeds = []
    
    # Seed 1: Maximum element count
    data = create_complete_header()
    data.extend(struct.pack('<I', 255))  # Max elements
    for i in range(255):
        # Cycle through element types
        elem_type = [1, 2, 3, 5, 9, 12, 13][i % 7]
        element_data = create_element_with_properties(elem_type, i)
        data.extend(element_data)
    data.extend(struct.pack('<I', 0))  # No connections  
    seeds.append(("max_elements.panda", data))
    
    # Seed 2: Deeply nested/complex elements
    data = create_complete_header()
    data.extend(struct.pack('<I', 1))  # 1 element
    # IC element (complex type)
    ic_data = create_element_with_properties(22, 1)  # IC type
    # Add IC-specific data
    ic_data.extend(struct.pack('<I', 16))  # 16 inputs
    ic_data.extend(struct.pack('<I', 8))   # 8 outputs
    ic_data.extend(struct.pack('<I', 100)) # Internal element count
    data.extend(ic_data)
    data.extend(struct.pack('<I', 0))  # No connections
    seeds.append(("complex_ic.panda", data))
    
    # Seed 3: All connection types
    data = create_complete_header()
    data.extend(struct.pack('<I', 4))  # 4 elements
    for i in range(4):
        elem_data = create_element_with_properties([5, 8, 10, 3][i], i)  # And, Or, Not, Led
        data.extend(elem_data)
    
    # Many connections (stress test connection parsing)
    data.extend(struct.pack('<I', 20))  # 20 connections
    for i in range(20):
        from_elem = i % 4
        to_elem = (i + 1) % 4
        conn_data = create_connection_data(from_elem, 0, to_elem, i % 2)
        data.extend(conn_data)
    seeds.append(("stress_connections.panda", data))
    
    # Seed 4: Invalid but parseable element types
    data = create_complete_header()
    data.extend(struct.pack('<I', 3))  # 3 elements
    for elem_type in [999, 0, 65535]:  # Invalid element types
        elem_data = bytearray()
        elem_data.extend(struct.pack('<I', elem_type))
        elem_data.extend(struct.pack('<I', 1))  # ID
        elem_data.extend(struct.pack('<ff', 100.0, 100.0))  # position
        elem_data.extend(struct.pack('<f', 0.0))  # rotation
        elem_data.extend(struct.pack('<II', 2, 1))  # inputs, outputs
        elem_data.extend(struct.pack('<I', 0))  # empty label
        data.extend(elem_data)
    data.extend(struct.pack('<I', 0))  # No connections
    seeds.append(("invalid_types.panda", data))
    
    return seeds

def main():
    corpus_dir = Path("fuzz/corpus")
    
    # Generate comprehensive circuit
    comprehensive_data = create_comprehensive_circuit()
    with open(corpus_dir / "comprehensive_circuit.panda", "wb") as f:
        f.write(comprehensive_data)
    print(f"✅ Generated comprehensive_circuit.panda ({len(comprehensive_data)} bytes)")
    
    # Generate edge case seeds
    edge_cases = create_edge_case_seeds()
    for filename, data in edge_cases:
        with open(corpus_dir / filename, "wb") as f:
            f.write(data)
        print(f"✅ Generated {filename} ({len(data)} bytes)")
    
    print(f"\n📊 Advanced seeds created: {len(edge_cases) + 1}")
    print("🎯 Targets:")
    print("  - ElementFactory::buildElement for all 26 element types") 
    print("  - Connection parsing (QNEConnection creation)")
    print("  - Element property parsing (labels, frequencies, colors)")
    print("  - Error handling paths (invalid types, malformed data)")
    print("  - Memory stress (255 elements, 20 connections)")

if __name__ == "__main__":
    main()