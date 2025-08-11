#!/usr/bin/env python3
"""
Breakthrough seeds targeting ElementFactory::buildElement and QNEConnection parsing.
Uses exact Qt type constants: GraphicElement::Type = 65539, QNEConnection::Type = 65538
"""
import struct
from pathlib import Path

# Qt type constants (QGraphicsItem::UserType = 65536)  
QNEPORT_TYPE = 65537      # UserType + 1
QNECONNECTION_TYPE = 65538 # UserType + 2  
GRAPHICELEMENT_TYPE = 65539 # UserType + 3

def create_panda_header():
    """Create valid WPCF header that passes readPandaHeader()"""
    data = bytearray()
    data.extend(b'WPCF')  # Magic header (MAGIC_HEADER_CIRCUIT)
    
    # Version (4.2.6) - readVersionSafely expects this format
    data.extend(struct.pack('<I', 3))  # segment count  
    data.extend(struct.pack('<I', 4))  # major
    data.extend(struct.pack('<I', 2))  # minor  
    data.extend(struct.pack('<I', 6))  # patch
    
    # Canvas rect - loadRect expects QRectF
    data.extend(struct.pack('<f', 0.0))    # x
    data.extend(struct.pack('<f', 0.0))    # y
    data.extend(struct.pack('<f', 1024.0)) # width  
    data.extend(struct.pack('<f', 768.0))  # height
    
    # Dolphin filename (empty) - loadDolphinFileName
    data.extend(struct.pack('<I', 0))  # empty string
    
    return data

def create_graphic_element_data(element_type=1):
    """Create GraphicElement data that triggers ElementFactory::buildElement"""
    data = bytearray()
    
    # *** CRITICAL: Qt type marker for GraphicElement ***
    data.extend(struct.pack('<I', GRAPHICELEMENT_TYPE))
    
    # ElementType enum (triggers ElementFactory::buildElement)
    data.extend(struct.pack('<I', element_type))
    
    # GraphicElement::load() expects these fields:
    # - ID (quint64)
    data.extend(struct.pack('<Q', 1))  # element ID
    
    # - Position (QPointF) 
    data.extend(struct.pack('<ff', 100.0, 200.0))  # x, y
    
    # - Rotation (float)
    data.extend(struct.pack('<f', 0.0))
    
    # - Input/Output sizes
    data.extend(struct.pack('<I', 2))  # input count
    data.extend(struct.pack('<I', 1))  # output count
    
    # - Label (QString)
    label = "ELEM1"
    data.extend(struct.pack('<I', len(label)))
    for char in label:
        data.extend(struct.pack('<H', ord(char)))  # UTF-16
        
    # - Element-specific properties (minimal)
    data.extend(struct.pack('<I', 0))  # no additional properties
    
    return data

def create_connection_data():
    """Create QNEConnection data that triggers connection parsing"""
    data = bytearray()
    
    # *** CRITICAL: Qt type marker for QNEConnection ***
    data.extend(struct.pack('<I', QNECONNECTION_TYPE))
    
    # QNEConnection::load() expects these fields:
    # - Source port ID (quint64)
    data.extend(struct.pack('<Q', 1))  # source port
    
    # - Target port ID (quint64)  
    data.extend(struct.pack('<Q', 2))  # target port
    
    # - Connection properties
    data.extend(struct.pack('<I', 1))  # signal value
    
    return data

def create_element_factory_breakthrough():
    """Create file that successfully reaches ElementFactory::buildElement"""
    data = create_panda_header()
    
    # Add single GraphicElement with valid type marker
    element_data = create_graphic_element_data(element_type=5)  # AND gate
    data.extend(element_data)
    
    return data

def create_connection_breakthrough():
    """Create file that reaches QNEConnection parsing"""  
    data = create_panda_header()
    
    # Add connection with valid type marker
    conn_data = create_connection_data()
    data.extend(conn_data)
    
    return data

def create_comprehensive_breakthrough():
    """Create file with both elements and connections"""
    data = create_panda_header()
    
    # Element 1: InputButton
    data.extend(create_graphic_element_data(element_type=1))
    
    # Element 2: AND gate  
    data.extend(create_graphic_element_data(element_type=5))
    
    # Element 3: LED output
    data.extend(create_graphic_element_data(element_type=3))
    
    # Connection 1: InputButton -> AND
    data.extend(create_connection_data())
    
    # Connection 2: AND -> LED
    data.extend(create_connection_data())
    
    return data

def create_all_element_types_breakthrough():
    """Create file with all known ElementType values"""
    data = create_panda_header()
    
    # All ElementType enum values from enums.h
    element_types = [
        1,   # InputButton
        2,   # InputSwitch  
        3,   # Led
        5,   # And
        6,   # Nand
        7,   # Nor
        8,   # Or
        9,   # Clock
        10,  # Not
        11,  # Xor
        12,  # InputVcc
        13,  # InputGnd
        14,  # Display7
        15,  # DLatch
        17,  # DFlipFlop
        18,  # JKFlipFlop
        21,  # InputRotary
        22,  # IC
        25,  # Demux
        26,  # Buzzer
        27,  # Display14
        28,  # Mux
        29,  # Line
        30,  # Node
        31,  # AudioBox
        33,  # Display16
    ]
    
    for elem_type in element_types:
        element_data = create_graphic_element_data(element_type=elem_type)
        data.extend(element_data)
    
    return data

def main():
    corpus_dir = Path("fuzz/corpus")
    
    breakthrough_files = [
        ("element_factory_breakthrough.panda", create_element_factory_breakthrough()),
        ("connection_breakthrough.panda", create_connection_breakthrough()), 
        ("comprehensive_breakthrough.panda", create_comprehensive_breakthrough()),
        ("all_elements_breakthrough.panda", create_all_element_types_breakthrough()),
    ]
    
    for filename, data in breakthrough_files:
        filepath = corpus_dir / filename
        with open(filepath, "wb") as f:
            f.write(data)
        print(f"✅ Generated {filename} ({len(data)} bytes)")
    
    print(f"\n🎯 Breakthrough Seeds Created:")
    print(f"  - GraphicElement::Type = {GRAPHICELEMENT_TYPE} (0x{GRAPHICELEMENT_TYPE:08x})")
    print(f"  - QNEConnection::Type = {QNECONNECTION_TYPE} (0x{QNECONNECTION_TYPE:08x})")
    print(f"  - Target: ElementFactory::buildElement() for all 26 element types")
    print(f"  - Target: QNEConnection::load() for connection parsing")

if __name__ == "__main__":
    main()