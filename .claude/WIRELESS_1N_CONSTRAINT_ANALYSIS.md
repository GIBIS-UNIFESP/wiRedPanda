# Wireless 1-N Constraint Analysis

## Current Problem: N-N Wireless Model

### Current Behavior
- Any node with label "A" can both send and receive signals
- All nodes with same label are in a bus where any can drive the signal
- Leads to contention when multiple nodes try to drive different values
- Required complex contention detection logic

### Issues with N-N Model
1. **Unrealistic**: Real wireless is typically 1-to-many (broadcast)
2. **Confusing UX**: User doesn't know which node is "driving" the signal
3. **Contention complexity**: Need special logic to handle conflicting drivers
4. **Unpredictable behavior**: Signal value depends on which nodes happen to be active

## Proposed Solution: 1-N Wireless Model

### New Constraint
- **Each wireless label can have only ONE source node**
- **Each wireless label can have MANY sink nodes**
- **Source node transmits signal to all sinks**

### Benefits
1. **Realistic model**: Matches real wireless/radio broadcast behavior
2. **Eliminates contention**: Only one driver per label by design
3. **Predictable behavior**: Signal always comes from the designated source
4. **Simpler logic**: No need for contention detection
5. **Clear semantics**: User knows exactly which node drives each signal

## Implementation Strategy

### Automatic Source/Sink Determination

```cpp
class Node {
    bool isWirelessSource() const {
        // Node can be source if it has input connection AND wireless label
        // (receives signal physically, transmits wirelessly)
        return hasInputConnection() && hasWirelessLabel();
    }
    
    bool isWirelessSink() const {
        // Node is sink if it has wireless label but NO input connection
        // (only receives wireless signals)
        return !hasInputConnection() && hasWirelessLabel();
    }
};
```

### WirelessConnectionManager Changes

```cpp
class WirelessConnectionManager {
private:
    QMap<QString, Node*> m_sourcesMap;      // label -> source node
    QMap<QString, QSet<Node*>> m_sinksMap;  // label -> sink nodes
    
public:
    bool setNodeWirelessLabel(Node* node, const QString& label) {
        if (node->hasInputConnection()) {
            // Node wants to be source
            if (m_sourcesMap.contains(label) && m_sourcesMap[label] != node) {
                // Another node is already source for this label
                return false; // Reject - only one source allowed
            }
            m_sourcesMap[label] = node;
        } else {
            // Node is sink
            m_sinksMap[label].insert(node);
        }
        return true;
    }
    
    Node* getWirelessSource(const QString& label) const {
        return m_sourcesMap.value(label, nullptr);
    }
    
    QSet<Node*> getWirelessSinks(const QString& label) const {
        return m_sinksMap.value(label, {});
    }
};
```

### Simplified Simulation Logic

```cpp
void Simulation::updateWirelessConnections() {
    const QStringList activeLabels = m_scene->wirelessManager()->getActiveLabels();
    
    for (const QString& label : activeLabels) {
        Node* source = m_scene->wirelessManager()->getWirelessSource(label);
        QSet<Node*> sinks = m_scene->wirelessManager()->getWirelessSinks(label);
        
        if (!source || sinks.isEmpty()) {
            continue; // No complete connection
        }
        
        // Get signal from source
        Status sourceSignal = Status::Invalid;
        if (source->logic() && source->logic()->isValid()) {
            sourceSignal = static_cast<Status>(source->logic()->outputValue(0));
        }
        
        // Broadcast to all sinks
        for (Node* sink : sinks) {
            sink->outputPort()->setStatus(sourceSignal);
            if (sink->logic() && sourceSignal != Status::Invalid) {
                sink->logic()->setOutputValue(0, static_cast<bool>(sourceSignal));
            }
            
            // Update connected elements
            const auto connections = sink->outputPort()->connections();
            for (auto* connection : connections) {
                if (auto* inputPort = connection->endPort()) {
                    updatePort(inputPort);
                }
            }
        }
    }
}
```

## UI/UX Implications

### Validation Feedback
```cpp
void ElementEditor::updateWirelessLabel() {
    QString newLabel = m_ui->lineEditLabel->text().trimmed();
    
    if (!newLabel.isEmpty()) {
        if (!m_scene->wirelessManager()->setNodeWirelessLabel(m_element, newLabel)) {
            // Show error: another node is already source for this label
            QMessageBox::warning(this, tr("Wireless Conflict"), 
                tr("Another node is already transmitting on label '%1'.\n"
                   "Each wireless label can have only one source.").arg(newLabel));
            return;
        }
    }
}
```

### Visual Indicators
- **Source nodes**: Different visual style (e.g., antenna icon, transmit symbol)
- **Sink nodes**: Different visual style (e.g., receive symbol)
- **Wireless groups**: Show source → sinks relationship in UI

## Migration Strategy

### Phase 1: Update Data Model
- Modify WirelessConnectionManager to track sources vs sinks
- Update Node class with source/sink detection
- Add validation for single source constraint

### Phase 2: Update Simulation Logic
- Replace contention detection with simple source→sinks broadcast
- Remove complex signal resolution logic
- Simplify wireless update code

### Phase 3: Update UI/UX
- Add validation feedback for multiple sources
- Visual distinction between source and sink nodes
- Update wireless connection displays

### Phase 4: Update Tests
- Test single source constraint enforcement
- Test source→sinks signal propagation
- Test UI validation behavior

## Example Scenarios

### Valid Configuration
```
VCC → Node(label="CLK") → [wireless] → Node(label="CLK") → LED1
                                   → Node(label="CLK") → LED2
                                   → Node(label="CLK") → LED3
```
- One source (VCC→Node), three sinks
- All LEDs receive the CLK signal

### Invalid Configuration (Rejected)
```
VCC1 → Node(label="CLK") → [wireless] → Node(label="CLK") → LED1
VCC2 → Node(label="CLK") → [wireless] → (CONFLICT!)
```
- Two sources for same label - second one rejected

### Mixed Labels (Valid)
```
VCC1 → Node(label="CLK") → [wireless] → Node(label="CLK") → Counter
VCC2 → Node(label="RST") → [wireless] → Node(label="RST") → Counter
```
- Different labels can have different sources
- One source per label constraint satisfied

## Benefits Summary

1. **Eliminates Contention**: No more conflicting drivers
2. **Clearer Semantics**: Always know which node drives which signal
3. **Realistic Model**: Matches real wireless/broadcast behavior
4. **Simpler Code**: No complex contention detection needed
5. **Better UX**: Clear error messages for conflicts
6. **Predictable Behavior**: Signal source is always explicit

## Breaking Changes

- Existing circuits with multiple sources per label will need updates
- UI will show validation errors for conflicting configurations
- Simulation behavior changes for multi-source scenarios

This constraint transforms wireless connections from a confusing bus model to a clear broadcast model that users can easily understand and predict.