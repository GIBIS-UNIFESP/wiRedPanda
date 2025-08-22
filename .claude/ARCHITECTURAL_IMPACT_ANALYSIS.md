# Architectural Impact Analysis: Unified Connection Class

## The Ripple Effect

Unifying connections at the serialization level would trigger a **cascade of changes** throughout the entire codebase. This is not just a serialization change - it's a fundamental architectural overhaul.

## Systems Requiring Major Changes

### 1. **Scene Management System**

#### Current State:
```cpp
// Physical connections are scene objects
class QNEConnection : public QGraphicsItem {
    void paint(QPainter *painter, ...);  // Visible wire rendering
    QRectF boundingRect() const;         // Hit testing for selection
};

// Wireless connections are NOT scene objects
class WirelessConnectionManager {
    QMap<QString, QSet<Node*>> m_labelGroups;  // Lightweight manager
};
```

#### Required Changes:
```cpp
class Connection : public QGraphicsItem {
    // Must handle BOTH visible and invisible rendering
    void paint(QPainter *painter, ...) {
        if (m_type == Physical) {
            drawWire();  // Current QNEConnection logic
        } else {
            // Draw nothing? Draw wireless indicator? 
        }
    }
    
    // Must handle BOTH port-level and node-level hit testing
    QRectF boundingRect() const {
        if (m_type == Physical) {
            return wireBounds();
        } else {
            return QRectF();  // Invisible wireless - how to select?
        }
    }
};
```

**Impact**: Every scene operation (selection, hit testing, rendering) needs dual logic.

### 2. **UI Operations System**

#### Current State:
```cpp
// Can select and delete physical wires directly
scene->selectedItems();  // Returns QNEConnection objects
delete connection;       // Removes wire from scene

// Cannot select wireless connections directly
// Must go through node properties or manager
```

#### Required Changes:
```cpp
// How do you SELECT an invisible wireless connection?
// How do you DELETE a wireless connection from UI?
// How do you show wireless connections in selection?

// Possible solutions:
class ConnectionInspector {
    void showWirelessConnections();  // New UI panel needed
    void selectWirelessGroup(QString label);  // New selection mode
};
```

**Impact**: Entire UI interaction model needs redesign for wireless connections.

### 3. **Command System (Undo/Redo)**

#### Current State:
```cpp
// Physical connection commands
class AddItemsCommand {
    void undo() { scene->removeItem(connection); }  // Simple object removal
    void redo() { scene->addItem(connection); }     // Simple object addition
};

// Wireless handled by manager
wirelessManager->setNodeLabel(node, "label");  // Implicit connection creation
```

#### Required Changes:
```cpp
class AddConnectionCommand {
    void undo() {
        if (m_type == Physical) {
            scene->removeItem(connection);     // Object removal
        } else {
            // Remove wireless label from ALL nodes in group?
            // What if other connections use same label?
            // Complex cascade removal logic needed
        }
    }
};
```

**Impact**: Undo/redo semantics become complex for wireless connections.

### 4. **Simulation System**

#### Current State:
```cpp
void Simulation::update() {
    // Separate update phases
    updatePhysicalConnections();  // Port-to-port signal flow
    updateWirelessConnections();  // Label-based signal flow
}
```

#### Required Changes:
```cpp
void Simulation::update() {
    for (auto* connection : allConnections) {
        connection->updateSignal();  // Unified interface needed
    }
}

class Connection {
    void updateSignal() {
        if (m_type == Physical) {
            updatePort(m_endPort);  // Single port update
        } else {
            // Update ALL nodes in wireless group
            for (auto* node : m_wirelessNodes) {
                updatePort(node->inputPort());
                updatePort(node->outputPort());
            }
        }
    }
};
```

**Impact**: Simulation loop becomes more complex, performance implications.

### 5. **Copy/Paste System**

#### Current State:
```cpp
// Physical connections copy as QNEConnection objects
QList<QGraphicsItem*> items = scene->selectedItems();
Serialization::serialize(items, stream);  // Includes connections

// Wireless connections don't participate in copy/paste
// Must manually recreate wireless labels
```

#### Required Changes:
```cpp
// Copying wireless connections is complex:
// - What if you copy some but not all nodes in a wireless group?
// - How do you handle label conflicts on paste?
// - Should wireless connections copy independently of nodes?

class ConnectionCopyHandler {
    void copyWirelessGroup(QString label) {
        // Need to decide: copy all nodes or just the connection?
        // What about label naming conflicts?
    }
};
```

**Impact**: Copy/paste semantics become ill-defined for wireless connections.

### 6. **Performance System**

#### Current State:
```cpp
// Physical: O(1) scene objects per connection
QNEConnection wire;  // Single object in scene

// Wireless: O(1) manager entries for unlimited nodes
manager.setLabel("bus", {node1, node2, node3, ...});  // Lightweight
```

#### Required Changes:
```cpp
// Unified: Every wireless group becomes scene object
Connection wirelessBus({node1, node2, node3, ...});  // Heavy scene object

// Scene operations become O(wireless_groups):
scene->items();           // Now includes wireless connection objects
scene->selectedItems();   // Must check wireless connections
scene->itemAt(point);     // Must test wireless connections
```

**Impact**: Performance degradation with many wireless connections.

### 7. **Port Connection System**

#### Current State:
```cpp
class QNEConnection {
    QNEOutputPort* m_startPort;  // Specific port connection
    QNEInputPort* m_endPort;     // Specific port connection
    
    // Clear semantics: startPort -> endPort
};

class WirelessConnectionManager {
    // Node-level connections via labels
    QMap<QString, QSet<Node*>> m_labelGroups;
};
```

#### Required Changes:
```cpp
class Connection {
    // Must handle BOTH port-level AND node-level semantics
    QNEOutputPort* m_startPort;     // For physical
    QNEInputPort* m_endPort;        // For physical
    QSet<Node*> m_wirelessNodes;    // For wireless
    
    // Which ports are actually connected?
    QList<QNEPort*> getConnectedPorts() {
        if (m_type == Physical) {
            return {m_startPort, m_endPort};
        } else {
            // Return ALL input/output ports of ALL wireless nodes?
            // Complex logic needed
        }
    }
};
```

**Impact**: Connection semantics become ambiguous and complex.

### 8. **Element Factory System**

#### Current State:
```cpp
// Elements and connections created separately
auto* element = ElementFactory::buildElement(type);
auto* connection = new QNEConnection();  // Simple constructor
```

#### Required Changes:
```cpp
// Need factory for unified connections
class ConnectionFactory {
    static Connection* createPhysical(QNEOutputPort* start, QNEInputPort* end);
    static Connection* createWireless(QString label, QSet<Node*> nodes);
    
    // How to handle conversion between types?
    static Connection* convertToWireless(QNEConnection* physical);
};
```

**Impact**: Connection creation logic becomes complex.

## Alternative Approaches (Lower Impact)

### Option 1: **Serialization-Only Unification**

#### Detailed Implementation:
```cpp
// Current serialization switch unchanged in structure
switch (type) {
case GraphicElement::Type: { /* existing logic */ }
case QNEConnection::Type: {  // Keep this case
    // But handle both physical and wireless
    int connectionSubtype; stream >> connectionSubtype;
    
    if (connectionSubtype == 0) {  // Physical
        auto* conn = new QNEConnection();
        conn->load(stream, portMap);
        itemList.append(conn);
    } else {  // Wireless
        QString label; QList<int> nodeIds;
        stream >> label >> nodeIds;
        
        // Store for later wireless restoration
        pendingWirelessConnections[label] = nodeIds;
    }
}
}

// After deserialization, restore wireless connections
for (auto& [label, nodeIds] : pendingWirelessConnections) {
    for (int nodeId : nodeIds) {
        if (auto* node = scene->findNode(nodeId)) {
            node->setLabel(label);
        }
    }
}
```

#### File Format Evolution:
```
// Current format
HEADER
GraphicElement (AND)
GraphicElement (LED)  
QNEConnection (wire)
WIRELESS_SECTION { nodeId1: "label", nodeId2: "label" }

// New unified format
HEADER
GraphicElement (AND)
GraphicElement (LED)
QNEConnection subtype=0 (physical wire data)
QNEConnection subtype=1 (wireless: label="bus" nodes=[1,2,3])
```

#### Pros:
- **Minimal Code Changes**: Only affects serialization methods
- **Unified File Format**: All connections in main stream
- **Backward Compatibility**: Easy to maintain with version checks
- **No UI Changes**: Physical wires and wireless manager unchanged
- **No Performance Impact**: Object models remain separate

#### Cons:
- **Conceptual Inconsistency**: `QNEConnection::Type` now includes wireless
- **Asymmetric Deserialization**: Physical creates objects, wireless sets labels
- **Limited Extensibility**: Still two separate code paths for operations
- **Debugging Complexity**: Connection type only known during serialization

#### Implementation Complexity: **Low**
- Modify `QNEConnection::save()/load()` methods
- Add subtype field to distinguish physical vs wireless
- Update version handling for backward compatibility

---

### Option 2: **Backend Unification Only** (SignalBus Architecture)

#### Detailed Implementation:
```cpp
// Abstract signal propagation
class SignalBus {
public:
    virtual ~SignalBus() = default;
    virtual Status calculateSignal() const = 0;
    virtual void propagateSignal() = 0;
    virtual QList<QNEPort*> getAffectedPorts() const = 0;
};

// Physical connection bus (wraps existing QNEConnection)
class PhysicalBus : public SignalBus {
private:
    QNEConnection* m_connection;
    
public:
    PhysicalBus(QNEConnection* conn) : m_connection(conn) {}
    
    Status calculateSignal() const override {
        auto* startPort = m_connection->startPort();
        return startPort->status();
    }
    
    void propagateSignal() override {
        auto* endPort = m_connection->endPort();
        auto signal = calculateSignal();
        endPort->setStatus(signal);
        
        // Update connected logic element
        if (auto* element = endPort->graphicElement()) {
            updatePort(endPort);
        }
    }
    
    QList<QNEPort*> getAffectedPorts() const override {
        return {m_connection->startPort(), m_connection->endPort()};
    }
};

// Wireless connection bus (wraps wireless group)
class WirelessBus : public SignalBus {
private:
    QString m_label;
    QSet<Node*> m_nodes;
    
public:
    WirelessBus(const QString& label, const QSet<Node*>& nodes) 
        : m_label(label), m_nodes(nodes) {}
    
    Status calculateSignal() const override {
        QSet<Status> uniqueSignals;
        
        for (auto* node : m_nodes) {
            if (node->logic() && node->logic()->isValid()) {
                Status signal = static_cast<Status>(node->logic()->outputValue(0));
                if (signal != Status::Invalid) {
                    uniqueSignals.insert(signal);
                }
            }
        }
        
        // Unified contention detection (same as physical)
        if (uniqueSignals.contains(Status::Active) && 
            uniqueSignals.contains(Status::Inactive)) {
            return Status::Invalid;  // Contention
        }
        
        if (uniqueSignals.contains(Status::Active)) return Status::Active;
        if (uniqueSignals.contains(Status::Inactive)) return Status::Inactive;
        return Status::Invalid;
    }
    
    void propagateSignal() override {
        Status groupSignal = calculateSignal();
        
        for (auto* node : m_nodes) {
            node->outputPort()->setStatus(groupSignal);
            
            if (node->logic() && groupSignal != Status::Invalid) {
                node->logic()->setOutputValue(0, static_cast<bool>(groupSignal));
            }
            
            // Update connected elements
            const auto connections = node->outputPort()->connections();
            for (auto* connection : connections) {
                if (auto* inputPort = connection->endPort()) {
                    updatePort(inputPort);
                }
            }
        }
    }
    
    QList<QNEPort*> getAffectedPorts() const override {
        QList<QNEPort*> ports;
        for (auto* node : m_nodes) {
            ports.append(node->inputPort());
            ports.append(node->outputPort());
        }
        return ports;
    }
};

// Unified bus manager
class BusManager {
private:
    std::vector<std::unique_ptr<SignalBus>> m_buses;
    
public:
    void addPhysicalConnection(QNEConnection* conn) {
        m_buses.push_back(std::make_unique<PhysicalBus>(conn));
    }
    
    void addWirelessGroup(const QString& label, const QSet<Node*>& nodes) {
        m_buses.push_back(std::make_unique<WirelessBus>(label, nodes));
    }
    
    void updateAllBuses() {
        for (auto& bus : m_buses) {
            bus->propagateSignal();
        }
    }
    
    // For debugging/analysis
    QList<SignalBus*> getBusesAffectingPort(QNEPort* port) const {
        QList<SignalBus*> result;
        for (auto& bus : m_buses) {
            if (bus->getAffectedPorts().contains(port)) {
                result.append(bus.get());
            }
        }
        return result;
    }
};

// Updated simulation loop
void Simulation::update() {
    // ... existing input/logic updates ...
    
    // Single unified connection update
    m_busManager->updateAllBuses();
    
    // ... existing output updates ...
}
```

#### Integration with Existing Systems:
```cpp
// Scene integration
void Scene::addConnection(QNEConnection* conn) {
    QGraphicsScene::addItem(conn);  // Add to scene as before
    m_busManager->addPhysicalConnection(conn);  // Add to bus system
}

void Scene::removeConnection(QNEConnection* conn) {
    m_busManager->removePhysicalConnection(conn);  // Remove from bus system
    QGraphicsScene::removeItem(conn);  // Remove from scene
}

// Wireless manager integration  
void WirelessConnectionManager::setNodeWirelessLabel(Node* node, const QString& label) {
    // ... existing label management ...
    
    // Update bus system
    if (!label.isEmpty()) {
        auto nodes = getWirelessGroup(label);
        m_scene->busManager()->updateWirelessGroup(label, nodes);
    }
}
```

#### Pros:
- **Guaranteed Behavioral Consistency**: Impossible for physical/wireless to diverge
- **Performance Improvement**: Single-pass signal propagation vs current dual-phase
- **Code Deduplication**: ~50 lines of duplicate contention detection eliminated
- **Clean Separation**: UI/scene logic separate from simulation logic
- **Extensibility**: New bus types (tri-state, bidirectional) easy to add
- **Debugging**: Can analyze signal flow uniformly across all connection types
- **No UI Changes**: Physical wires and wireless manager unchanged

#### Cons:
- **Increased Memory Usage**: Each connection gets wrapped in bus object
- **Abstraction Overhead**: Virtual function calls in simulation loop
- **Implementation Complexity**: Need to sync bus system with UI operations
- **Migration Risk**: All simulation logic needs careful testing

#### Implementation Complexity: **Medium**
- Create SignalBus hierarchy and manager
- Update simulation loop to use unified system
- Integrate with scene and wireless manager operations
- Comprehensive testing of signal propagation

---

### Option 3: **Adapter Pattern** (Unified Interfaces)

#### Detailed Implementation:
```cpp
// Base interface for all connections
class IConnection {
public:
    virtual ~IConnection() = default;
    
    // Core connection operations
    virtual void updateSignal() = 0;
    virtual Status calculateSignal() const = 0;
    virtual QList<QNEPort*> getConnectedPorts() const = 0;
    
    // Serialization
    virtual void save(QDataStream& stream) const = 0;
    virtual void load(QDataStream& stream, QMap<quint64, QNEPort*> portMap) = 0;
    
    // UI operations
    virtual bool isSelectable() const = 0;
    virtual QRectF boundingRect() const = 0;
    virtual void render(QPainter* painter, const QRectF& rect) = 0;
    
    // Type identification
    virtual QString typeName() const = 0;
    virtual bool isWireless() const = 0;
};

// Adapter for physical connections
class PhysicalConnectionAdapter : public IConnection {
private:
    QNEConnection* m_connection;
    
public:
    PhysicalConnectionAdapter(QNEConnection* conn) : m_connection(conn) {}
    
    void updateSignal() override {
        // Delegate to existing QNEConnection logic
        auto* startPort = m_connection->startPort();
        auto* endPort = m_connection->endPort();
        
        if (startPort && endPort) {
            endPort->setStatus(startPort->status());
            updatePort(endPort);
        }
    }
    
    Status calculateSignal() const override {
        auto* startPort = m_connection->startPort();
        return startPort ? startPort->status() : Status::Invalid;
    }
    
    QList<QNEPort*> getConnectedPorts() const override {
        return {m_connection->startPort(), m_connection->endPort()};
    }
    
    void save(QDataStream& stream) const override {
        stream << 0;  // Type marker: physical
        m_connection->save(stream);
    }
    
    void load(QDataStream& stream, QMap<quint64, QNEPort*> portMap) override {
        m_connection->load(stream, portMap);
    }
    
    bool isSelectable() const override { return true; }
    QRectF boundingRect() const override { return m_connection->boundingRect(); }
    void render(QPainter* painter, const QRectF& rect) override {
        m_connection->paint(painter, nullptr, nullptr);
    }
    
    QString typeName() const override { return "Physical"; }
    bool isWireless() const override { return false; }
    
    // Access to underlying object
    QNEConnection* underlyingConnection() const { return m_connection; }
};

// Adapter for wireless connections
class WirelessConnectionAdapter : public IConnection {
private:
    QString m_label;
    QSet<Node*> m_nodes;
    Scene* m_scene;
    
public:
    WirelessConnectionAdapter(const QString& label, const QSet<Node*>& nodes, Scene* scene)
        : m_label(label), m_nodes(nodes), m_scene(scene) {}
    
    void updateSignal() override {
        // Same contention detection logic as current implementation
        QSet<Status> uniqueSignals;
        
        for (auto* node : m_nodes) {
            if (node->logic() && node->logic()->isValid()) {
                Status signal = static_cast<Status>(node->logic()->outputValue(0));
                if (signal != Status::Invalid) {
                    uniqueSignals.insert(signal);
                }
            }
        }
        
        Status groupSignal = Status::Invalid;
        if (uniqueSignals.contains(Status::Active) && 
            uniqueSignals.contains(Status::Inactive)) {
            groupSignal = Status::Invalid;  // Contention
        } else if (uniqueSignals.contains(Status::Active)) {
            groupSignal = Status::Active;
        } else if (uniqueSignals.contains(Status::Inactive)) {
            groupSignal = Status::Inactive;
        }
        
        // Propagate to all nodes
        for (auto* node : m_nodes) {
            node->outputPort()->setStatus(groupSignal);
            if (node->logic() && groupSignal != Status::Invalid) {
                node->logic()->setOutputValue(0, static_cast<bool>(groupSignal));
            }
            
            // Update connected elements
            const auto connections = node->outputPort()->connections();
            for (auto* connection : connections) {
                if (auto* inputPort = connection->endPort()) {
                    updatePort(inputPort);
                }
            }
        }
    }
    
    Status calculateSignal() const override {
        // Same calculation as updateSignal() but without propagation
        QSet<Status> uniqueSignals;
        for (auto* node : m_nodes) {
            if (node->logic() && node->logic()->isValid()) {
                Status signal = static_cast<Status>(node->logic()->outputValue(0));
                if (signal != Status::Invalid) {
                    uniqueSignals.insert(signal);
                }
            }
        }
        
        if (uniqueSignals.contains(Status::Active) && 
            uniqueSignals.contains(Status::Inactive)) {
            return Status::Invalid;
        }
        if (uniqueSignals.contains(Status::Active)) return Status::Active;
        if (uniqueSignals.contains(Status::Inactive)) return Status::Inactive;
        return Status::Invalid;
    }
    
    QList<QNEPort*> getConnectedPorts() const override {
        QList<QNEPort*> ports;
        for (auto* node : m_nodes) {
            ports.append(node->inputPort());
            ports.append(node->outputPort());
        }
        return ports;
    }
    
    void save(QDataStream& stream) const override {
        stream << 1;  // Type marker: wireless
        stream << m_label;
        stream << static_cast<int>(m_nodes.size());
        for (auto* node : m_nodes) {
            stream << node->id();
        }
    }
    
    void load(QDataStream& stream, QMap<quint64, QNEPort*> portMap) override {
        int nodeCount; stream >> m_label >> nodeCount;
        
        for (int i = 0; i < nodeCount; ++i) {
            int nodeId; stream >> nodeId;
            if (auto* node = m_scene->findNode(nodeId)) {
                m_nodes.insert(node);
                node->setLabel(m_label);
            }
        }
    }
    
    bool isSelectable() const override { return false; }  // Or implement special selection
    QRectF boundingRect() const override { return QRectF(); }  // Invisible
    void render(QPainter* painter, const QRectF& rect) override { /* No rendering */ }
    
    QString typeName() const override { return "Wireless"; }
    bool isWireless() const override { return true; }
    
    // Access to wireless data
    QString label() const { return m_label; }
    QSet<Node*> nodes() const { return m_nodes; }
};

// Unified connection manager
class ConnectionManager {
private:
    QList<std::unique_ptr<IConnection>> m_connections;
    
public:
    void addPhysicalConnection(QNEConnection* conn) {
        m_connections.append(std::make_unique<PhysicalConnectionAdapter>(conn));
    }
    
    void addWirelessConnection(const QString& label, const QSet<Node*>& nodes, Scene* scene) {
        m_connections.append(std::make_unique<WirelessConnectionAdapter>(label, nodes, scene));
    }
    
    void updateAllConnections() {
        for (auto& conn : m_connections) {
            conn->updateSignal();
        }
    }
    
    void serializeConnections(QDataStream& stream) const {
        stream << static_cast<int>(m_connections.size());
        for (auto& conn : m_connections) {
            conn->save(stream);
        }
    }
    
    void deserializeConnections(QDataStream& stream, QMap<quint64, QNEPort*> portMap, Scene* scene) {
        int count; stream >> count;
        
        for (int i = 0; i < count; ++i) {
            int type; stream >> type;
            
            if (type == 0) {  // Physical
                auto* qneConn = new QNEConnection();
                auto adapter = std::make_unique<PhysicalConnectionAdapter>(qneConn);
                adapter->load(stream, portMap);
                scene->addItem(qneConn);
                m_connections.append(std::move(adapter));
            } else {  // Wireless
                auto adapter = std::make_unique<WirelessConnectionAdapter>("", QSet<Node*>(), scene);
                adapter->load(stream, portMap);
                m_connections.append(std::move(adapter));
            }
        }
    }
    
    // Query operations
    QList<IConnection*> getConnectionsOfType(bool wireless) const {
        QList<IConnection*> result;
        for (auto& conn : m_connections) {
            if (conn->isWireless() == wireless) {
                result.append(conn.get());
            }
        }
        return result;
    }
    
    QList<IConnection*> getConnectionsAffectingPort(QNEPort* port) const {
        QList<IConnection*> result;
        for (auto& conn : m_connections) {
            if (conn->getConnectedPorts().contains(port)) {
                result.append(conn.get());
            }
        }
        return result;
    }
};
```

#### Pros:
- **Clean Abstraction**: Uniform interface for all connection operations
- **Flexible Implementation**: Each connection type optimized independently
- **Unified Serialization**: All connections serialize through same interface
- **Extensibility**: New connection types just implement IConnection
- **Debugging**: Can query/analyze all connections uniformly
- **Migration Safety**: Wraps existing objects, minimal risk

#### Cons:
- **Indirection Overhead**: Extra layer between callers and implementation
- **Memory Usage**: Adapter objects for every connection
- **Interface Complexity**: Many methods needed for full abstraction
- **Async Issues**: Adapters must stay in sync with underlying objects
- **Limited Polymorphism**: Some operations still need type-specific handling

#### Implementation Complexity: **Medium-High**
- Design comprehensive IConnection interface
- Implement adapters for both connection types
- Update all connection consumers to use interface
- Handle lifecycle management of adapter objects

---

## Comprehensive Comparison

| Aspect | Serialization-Only | Backend Unification | Adapter Pattern |
|--------|-------------------|-------------------|-----------------|
| **Code Changes** | Minimal | Medium | High |
| **Performance** | No impact | Improved | Slight overhead |
| **Type Safety** | Weak | Strong | Strong |
| **Extensibility** | Limited | High | Very High |
| **Risk Level** | Very Low | Low-Medium | Medium |
| **Debugging** | Limited | Good | Excellent |
| **UI Impact** | None | None | None |
| **Behavioral Consistency** | Not guaranteed | Guaranteed | Guaranteed |

## Risk Assessment

### **High Risk** (Full Unification)
- **Scope**: Affects 8+ major systems
- **Complexity**: Dual semantics throughout codebase  
- **Performance**: Scene object overhead for wireless
- **UI**: Interaction model needs complete redesign
- **Testing**: All existing functionality at risk

### **Medium Risk** (Backend Only)
- **Scope**: Affects simulation + serialization
- **Complexity**: Moderate interface changes
- **Performance**: Potential improvements
- **UI**: Minimal changes required

### **Low Risk** (Serialization Only)
- **Scope**: Affects only file format
- **Complexity**: Version handling
- **Performance**: No impact
- **UI**: No changes required

## Recommendation

**Avoid full unification.** The architectural complexity and risk far outweigh the benefits. Instead:

1. **Keep separate object models** (QNEConnection vs WirelessConnectionManager)
2. **Unify simulation backend** (SignalBus abstraction from earlier proposal)
3. **Optionally unify serialization format** (but deserialize to appropriate types)

This achieves most benefits (behavioral consistency, code deduplication) without the massive architectural overhaul.