#!/usr/bin/env python3
"""
Create targeted seeds for ElementFactory breakthrough.
Based on our debug output, we need to hit GraphicElement::Type (65539) in deserialize().
"""
import struct
from pathlib import Path

def create_elementfactory_breakthrough_seed():
    """Create seed that will trigger ElementFactory::buildElement()"""
    data = bytearray()
    
    # Strategy: Create WPCF header followed by GraphicElement data
    
    # WPCF header
    data.extend(b'WPCF')                    # Magic header (4 bytes)
    data.extend(struct.pack('>I', 1))       # 1 version segment (4 bytes)  
    data.extend(struct.pack('>I', 4))       # Version 4 (4 bytes)
    # Total: 12 bytes consumed by readPandaHeader()
    
    # Now add GraphicElement serialization data that deserialize() will read
    data.extend(struct.pack('>I', 65539))   # GraphicElement::Type (big-endian)
    data.extend(struct.pack('>I', 5))       # ElementType: AND gate
    data.extend(struct.pack('>Q', 12345))   # ID (8 bytes)
    data.extend(struct.pack('>f', 100.0))   # x position
    data.extend(struct.pack('>f', 200.0))   # y position
    data.extend(struct.pack('>f', 0.0))     # rotation
    data.extend(struct.pack('>I', 2))       # input count
    data.extend(struct.pack('>I', 1))       # output count
    data.extend(struct.pack('>I', 0))       # empty label (QString length = 0)
    
    return data

def create_qneconnection_breakthrough_seed():
    """Create seed that will trigger QNEConnection processing"""
    data = bytearray()
    
    # WPCF header
    data.extend(b'WPCF')                    # Magic header
    data.extend(struct.pack('>I', 1))       # 1 version segment
    data.extend(struct.pack('>I', 4))       # Version 4
    
    # QNEConnection data
    data.extend(struct.pack('>I', 65538))   # QNEConnection::Type (big-endian)
    data.extend(struct.pack('>Q', 54321))   # ID (8 bytes)
    
    return data

def create_mixed_breakthrough_seed():
    """Create seed with both GraphicElement and QNEConnection"""
    data = bytearray()
    
    # WPCF header
    data.extend(b'WPCF')                    # Magic header
    data.extend(struct.pack('>I', 2))       # 2 version segments
    data.extend(struct.pack('>I', 4))       # Version 4
    data.extend(struct.pack('>I', 2))       # Version 4.2
    
    # First: GraphicElement
    data.extend(struct.pack('>I', 65539))   # GraphicElement::Type
    data.extend(struct.pack('>I', 1))       # ElementType: INPUT_BUTTON
    data.extend(struct.pack('>Q', 1))       # ID
    data.extend(struct.pack('>f', 50.0))    # x
    data.extend(struct.pack('>f', 100.0))   # y
    data.extend(struct.pack('>f', 0.0))     # rotation
    data.extend(struct.pack('>I', 0))       # input count
    data.extend(struct.pack('>I', 1))       # output count
    data.extend(struct.pack('>I', 0))       # empty label
    
    # Second: QNEConnection
    data.extend(struct.pack('>I', 65538))   # QNEConnection::Type
    data.extend(struct.pack('>Q', 2))       # ID
    
    return data

def create_direct_elementfactory_seed():
    """Direct approach - just ElementFactory data without header (for fallback strategy)"""
    data = bytearray()
    
    # Direct GraphicElement data (no header)
    data.extend(struct.pack('>I', 65539))   # GraphicElement::Type
    data.extend(struct.pack('>I', 7))       # ElementType: OR gate
    data.extend(struct.pack('>Q', 999))     # ID
    data.extend(struct.pack('>f', 300.0))   # x
    data.extend(struct.pack('>f', 400.0))   # y
    data.extend(struct.pack('>f', 90.0))    # rotation (90 degrees)
    data.extend(struct.pack('>I', 2))       # input count
    data.extend(struct.pack('>I', 1))       # output count
    data.extend(struct.pack('>I', 0))       # empty label
    
    return data

def main():
    corpus_dir = Path("fuzz/corpus")
    corpus_dir.mkdir(exist_ok=True)
    
    # Generate breakthrough seeds
    seeds = [
        ("elementfactory_and_gate.panda", create_elementfactory_breakthrough_seed()),
        ("qneconnection_basic.panda", create_qneconnection_breakthrough_seed()),
        ("mixed_elements.panda", create_mixed_breakthrough_seed()),
        ("direct_or_gate.panda", create_direct_elementfactory_seed()),
    ]
    
    for filename, seed_data in seeds:
        filepath = corpus_dir / filename
        with open(filepath, "wb") as f:
            f.write(seed_data)
        
        # Verify the key integers
        first_int = struct.unpack('>I', seed_data[:4])[0]
        print(f"✅ {filename} ({len(seed_data)} bytes)")
        print(f"   First 4 bytes: {first_int} ({'WPCF' if first_int == 0x46434057 else 'GraphicElement::Type' if first_int == 65539 else 'Other'})")
        
        # Find GraphicElement::Type in the data
        for i in range(len(seed_data) - 3):
            test_int = struct.unpack('>I', seed_data[i:i+4])[0]
            if test_int == 65539:
                print(f"   GraphicElement::Type found at offset {i}")
                break
        
        print()
    
    print("🎯 ELEMENTFACTORY BREAKTHROUGH SEEDS READY!")
    print("   These seeds should trigger:")
    print("   1. readPandaHeader() success (WPCF magic)")
    print("   2. deserialize() finding GraphicElement::Type (65539)")
    print("   3. ElementFactory::buildElement() call")
    print("   4. Maximum coverage of serialization.cpp!")

if __name__ == "__main__":
    main()