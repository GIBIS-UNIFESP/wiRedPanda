#!/usr/bin/env python3
"""
Create seeds with CORRECT byte order for QDataStream.
The issue was QDataStream reads as big-endian, not little-endian!
"""
import struct
from pathlib import Path

def create_big_endian_seeds():
    """Create seeds with big-endian byte order for QDataStream"""
    
    GRAPHICELEMENT_TYPE = 65539  # 0x00010003
    QNECONNECTION_TYPE = 65538   # 0x00010002
    
    seeds = []
    
    # Seed 1: Direct GraphicElement (big-endian)
    data = bytearray()
    data.extend(struct.pack('>I', GRAPHICELEMENT_TYPE))  # BIG-ENDIAN!
    data.extend(struct.pack('>I', 1))  # ElementType: InputButton
    
    # Add element data (big-endian)
    data.extend(struct.pack('>Q', 1))      # ID
    data.extend(struct.pack('>f', 100.0))  # x position
    data.extend(struct.pack('>f', 200.0))  # y position
    data.extend(struct.pack('>f', 0.0))    # rotation
    data.extend(struct.pack('>I', 2))      # inputs
    data.extend(struct.pack('>I', 1))      # outputs
    data.extend(struct.pack('>I', 0))      # empty label
    
    seeds.append(("be_direct_element.panda", data))
    
    # Seed 2: Direct QNEConnection (big-endian)
    data = bytearray()
    data.extend(struct.pack('>I', QNECONNECTION_TYPE))  # BIG-ENDIAN!
    data.extend(struct.pack('>Q', 1))  # start port ID
    data.extend(struct.pack('>Q', 2))  # end port ID
    
    seeds.append(("be_direct_connection.panda", data))
    
    # Seed 3: Both element and connection
    data = bytearray()
    
    # GraphicElement first
    data.extend(struct.pack('>I', GRAPHICELEMENT_TYPE))
    data.extend(struct.pack('>I', 5))   # AND gate
    data.extend(struct.pack('>Q', 1))   # ID
    data.extend(struct.pack('>f', 50.0)) # position
    data.extend(struct.pack('>f', 100.0))
    data.extend(struct.pack('>f', 0.0))  # rotation
    data.extend(struct.pack('>I', 2))    # inputs
    data.extend(struct.pack('>I', 1))    # outputs
    data.extend(struct.pack('>I', 0))    # empty label
    
    # Connection second  
    data.extend(struct.pack('>I', QNECONNECTION_TYPE))
    data.extend(struct.pack('>Q', 1))    # start port
    data.extend(struct.pack('>Q', 2))    # end port
    
    seeds.append(("be_element_and_connection.panda", data))
    
    # Seed 4: All element types (big-endian)
    data = bytearray()
    element_types = [1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 17, 18, 21, 22, 25, 26, 27, 28, 29, 30, 31, 33]
    
    for elem_type in element_types:
        data.extend(struct.pack('>I', GRAPHICELEMENT_TYPE))
        data.extend(struct.pack('>I', elem_type))
        data.extend(struct.pack('>Q', elem_type))       # unique ID
        data.extend(struct.pack('>f', float(elem_type * 50)))  # x position
        data.extend(struct.pack('>f', 100.0))           # y position
        data.extend(struct.pack('>f', 0.0))             # rotation
        data.extend(struct.pack('>I', 2))               # inputs
        data.extend(struct.pack('>I', 1))               # outputs
        data.extend(struct.pack('>I', 0))               # empty label
    
    seeds.append(("be_all_elements.panda", data))
    
    return seeds

def main():
    corpus_dir = Path("fuzz/corpus")
    
    seeds = create_big_endian_seeds()
    
    for filename, data in seeds:
        with open(corpus_dir / filename, "wb") as f:
            f.write(data)
        print(f"✅ Generated {filename} ({len(data)} bytes)")
        
        # Show first few bytes
        print(f"   First 8 bytes: {[hex(b) for b in data[:8]]}")
        
        # Verify the first integer when read as big-endian
        if len(data) >= 4:
            first_int = struct.unpack('>I', data[:4])[0]
            print(f"   First int (BE): {first_int} (expected: 65539 or 65538)")
            if first_int == 65539:
                print(f"   ✅ MATCH: GraphicElement::Type!")
            elif first_int == 65538:
                print(f"   ✅ MATCH: QNEConnection::Type!")
        print()
    
    print("🎯 BIG-ENDIAN BREAKTHROUGH SEEDS:")
    print("  - Fixed byte order issue: QDataStream reads big-endian")
    print("  - GraphicElement::Type = 65539 with correct byte order")
    print("  - QNEConnection::Type = 65538 with correct byte order")
    print("  - Should FINALLY trigger ElementFactory::buildElement()!")

if __name__ == "__main__":
    main()