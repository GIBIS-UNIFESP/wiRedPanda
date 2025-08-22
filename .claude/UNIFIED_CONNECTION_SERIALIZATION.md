# Unified Connection Serialization Proposal

## Current Serialization Architecture

The `Serialization::deserialize()` method has a switch statement with only two cases:

```cpp
switch (type) {
case GraphicElement::Type: {
    // Deserialize elements (AND, OR, LED, etc.)
    ElementType elmType; stream >> elmType;
    auto *elm = ElementFactory::buildElement(elmType);
    elm->load(stream, portMap, version);
    break;
}

case QNEConnection::Type: {
    // Deserialize physical wire connections
    auto *conn = new QNEConnection();
    conn->load(stream, portMap);
    itemList.append(conn);
    break;
}
}
```

**Current Issue**: Wireless connections are handled separately via `WirelessConnectionManager::serialize()`/`deserialize()`, creating an asymmetric architecture.

## Proposed Solution: Unified Connection Class

Replace `QNEConnection` with a unified `Connection` class that can represent both physical and wireless connections in the serialization stream.

### New Unified Architecture

```cpp
class Connection : public QGraphicsItem {
public:
    enum ConnectionType {
        Physical,   // Traditional wire connection
        Wireless    // Label-based wireless connection
    };

    // Physical connection constructor (replaces QNEConnection)
    Connection(QNEOutputPort* startPort, QNEInputPort* endPort);
    
    // Wireless connection constructor (new)
    Connection(const QString& wirelessLabel, const QSet<Node*>& nodes);
    
    // Unified serialization
    void save(QDataStream& stream) const;
    void load(QDataStream& stream, QMap<quint64, QNEPort*> portMap);
    
    // Getters
    ConnectionType connectionType() const { return m_type; }
    QString wirelessLabel() const { return m_wirelessLabel; }
    QNEOutputPort* startPort() const { return m_startPort; }
    QNEInputPort* endPort() const { return m_endPort; }
    QSet<Node*> wirelessNodes() const { return m_wirelessNodes; }

private:
    ConnectionType m_type;
    
    // Physical connection data
    QNEOutputPort* m_startPort = nullptr;
    QNEInputPort* m_endPort = nullptr;
    
    // Wireless connection data  
    QString m_wirelessLabel;
    QSet<Node*> m_wirelessNodes;
};
```

### Updated Serialization Switch

```cpp
switch (type) {
case GraphicElement::Type: {
    ElementType elmType; stream >> elmType;
    auto *elm = ElementFactory::buildElement(elmType);
    elm->load(stream, portMap, version);
    break;
}

case Connection::Type: {  // Unified connection type
    auto *conn = new Connection();
    conn->load(stream, portMap);
    itemList.append(conn);
    break;
}
}
```

### Unified Serialization Format

```cpp
void Connection::save(QDataStream& stream) const {
    stream << static_cast<int>(m_type);
    
    switch (m_type) {
    case Physical:
        // Serialize physical connection (existing QNEConnection format)
        stream << m_startPort->graphicElement()->id();
        stream << m_startPort->index();
        stream << m_endPort->graphicElement()->id(); 
        stream << m_endPort->index();
        break;
        
    case Wireless:
        // Serialize wireless connection
        stream << m_wirelessLabel;
        stream << static_cast<int>(m_wirelessNodes.size());
        for (auto* node : m_wirelessNodes) {
            stream << node->id();
        }
        break;
    }
}

void Connection::load(QDataStream& stream, QMap<quint64, QNEPort*> portMap) {
    int typeInt; stream >> typeInt;
    m_type = static_cast<ConnectionType>(typeInt);
    
    switch (m_type) {
    case Physical:
        // Load physical connection (existing QNEConnection logic)
        quint64 startId, endId;
        int startPortIndex, endPortIndex;
        stream >> startId >> startPortIndex >> endId >> endPortIndex;
        
        auto startKey = (startId << 16) | startPortIndex;
        auto endKey = (endId << 16) | endPortIndex;
        
        m_startPort = qobject_cast<QNEOutputPort*>(portMap[startKey]);
        m_endPort = qobject_cast<QNEInputPort*>(portMap[endKey]);
        break;
        
    case Wireless:
        // Load wireless connection
        int nodeCount;
        stream >> m_wirelessLabel >> nodeCount;
        
        for (int i = 0; i < nodeCount; ++i) {
            int nodeId; stream >> nodeId;
            if (auto* node = scene()->findNode(nodeId)) {
                m_wirelessNodes.insert(node);
                node->setLabel(m_wirelessLabel);  // Restore wireless label
            }
        }
        break;
    }
}
```

## Benefits of Unified Approach

### 1. **Architectural Symmetry**
- Both connection types serialize through the same code path
- Single `Connection::Type` case in deserialize switch
- No special handling for wireless connections in workspace loading

### 2. **Simplified File Format**
- All connections appear in the main item stream
- No separate wireless section in files
- Version compatibility easier to manage

### 3. **Code Consolidation**
```cpp
// BEFORE: Asymmetric serialization
Serialization::serialize(elements + physicalConnections);  // Main stream
wirelessManager->serialize(stream);                        // Separate stream

// AFTER: Unified serialization  
Serialization::serialize(elements + allConnections);       // Single stream
```

### 4. **Future Extensibility**
- Easy to add new connection types (bidirectional, tri-state, etc.)
- Connection metadata (signal strength, delay, etc.) unified
- Plugin architecture for custom connection types

### 5. **UI Consistency**
- Copy/paste operations work identically for both connection types
- Selection, deletion, and manipulation unified
- Undo/redo commands simplified

## Implementation Strategy

### Phase 1: Create Connection Class
```cpp
// Inherit from QNEConnection to preserve UI behavior for physical connections
class Connection : public QNEConnection {
    // Add wireless connection support while maintaining backward compatibility
};
```

### Phase 2: Update Serialization
```cpp
// Replace QNEConnection::Type with Connection::Type in serialization
case Connection::Type: {
    auto *conn = new Connection();
    conn->load(stream, portMap);
    // Connection automatically determines type from serialized data
}
```

### Phase 3: Migrate Wireless Manager
```cpp
// WirelessConnectionManager becomes a factory for wireless Connection objects
class WirelessConnectionManager {
    void createWirelessConnection(const QString& label, const QSet<Node*>& nodes) {
        auto* conn = new Connection(label, nodes);
        scene()->addItem(conn);  // Adds to main serialization stream
    }
};
```

### Phase 4: Remove Separate Wireless Serialization
```cpp
// Remove these methods:
// WirelessConnectionManager::serialize()
// WirelessConnectionManager::deserialize()
// Workspace::restoreWirelessConnections()
```

## File Format Evolution

### Current Format
```
HEADER
GraphicElement (AND gate)
GraphicElement (LED)  
QNEConnection (wire)
WIRELESS_SECTION {
  nodeId1: "label_a"
  nodeId2: "label_a"  
}
```

### Unified Format
```
HEADER
GraphicElement (AND gate)
GraphicElement (LED)
Connection (Physical: port1 -> port2)
Connection (Wireless: "label_a" [nodeId1, nodeId2])
```

## Backward Compatibility

### Reading Old Files
```cpp
void Connection::load(QDataStream& stream, QMap<quint64, QNEPort*> portMap) {
    if (version < QVersionNumber(4, 4, 0)) {
        // Legacy QNEConnection format
        loadAsPhysicalConnection(stream, portMap);
    } else {
        // New unified format
        loadUnifiedFormat(stream, portMap);
    }
}
```

### Writing New Files
- Always use unified format for new saves
- Wireless connections appear as `Connection` objects in main stream
- No more separate wireless serialization section

## Risk Analysis

### Low Risk
- **UI Impact**: Physical connections continue to render as visible wires
- **Behavior**: Signal propagation logic unchanged
- **Compatibility**: Old files still load correctly

### Medium Risk
- **Serialization Changes**: Requires careful version handling
- **Testing**: Need comprehensive migration testing

### Mitigation
- **Feature Flag**: Implement behind runtime flag initially
- **Dual Format**: Support both formats during transition
- **Comprehensive Testing**: Verify all existing files load correctly

## Expected Outcomes

### Immediate Benefits
1. **Single Source of Truth**: All connections in main serialization stream
2. **Code Reduction**: ~100 lines of wireless-specific serialization code eliminated
3. **Architectural Clarity**: Connection handling becomes uniform

### Long-term Benefits
1. **Extensibility**: New connection types trivial to add
2. **Maintainability**: Single connection serialization path to debug
3. **Feature Parity**: All connection operations work on both types

## Conclusion

Merging `QNEConnection` and wireless connections into a unified `Connection` class in serialization is architecturally sound and provides significant benefits:

- **Eliminates asymmetry** in the serialization switch statement
- **Simplifies file format** by putting all connections in main stream
- **Reduces code duplication** in serialization logic
- **Improves extensibility** for future connection types

The implementation can be done incrementally with minimal risk, and the resulting architecture is cleaner and more maintainable.