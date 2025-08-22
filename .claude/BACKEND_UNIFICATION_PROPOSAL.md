# Backend Unification Proposal: Unified Signal Bus Architecture

## Problem Statement

Currently, physical and wireless connections use completely separate backends:

### Physical Connections
- Direct `QNEConnection` objects between ports
- Contention detection via `QNEInputPort::isValid()` checking `m_connections.size() == 1`
- Signal propagation through direct port connections

### Wireless Connections  
- Label-based grouping via `WirelessConnectionManager`
- Contention detection via `QSet<Status>` analysis in simulation
- Signal propagation through `updateWirelessConnections()`

**Result**: Identical logic implemented twice, risk of behavioral divergence.

## Proposed Solution: Bus-Centric Architecture

### Core Concept
Both connection types are **signal buses** with identical signal propagation and contention detection requirements:

- **Physical Connection** = 2-node bus (output port → input port)
- **Wireless Connection** = N-node bus (all nodes with same label)

### Unified Bus Implementation

```cpp
class SignalBus {
public:
    void addSource(QNEOutputPort* source) { m_sources.insert(source); }
    void addSink(QNEInputPort* sink) { m_sinks.insert(sink); }
    
    Status calculateBusSignal() const {
        QSet<Status> uniqueSignals;
        
        // Collect all valid source signals
        for (auto* source : m_sources) {
            if (source->isValid() && source->status() != Status::Invalid) {
                uniqueSignals.insert(source->status());
            }
        }
        
        // Unified contention detection
        if (uniqueSignals.contains(Status::Active) && 
            uniqueSignals.contains(Status::Inactive)) {
            return Status::Invalid; // Contention detected
        }
        
        // Return consensus signal
        if (uniqueSignals.contains(Status::Active)) return Status::Active;
        if (uniqueSignals.contains(Status::Inactive)) return Status::Inactive;
        return Status::Invalid;
    }
    
    void propagateSignal() {
        Status busSignal = calculateBusSignal();
        for (auto* sink : m_sinks) {
            sink->setStatus(busSignal);
            // Update connected logic elements
            if (auto* element = sink->graphicElement()) {
                if (auto* logic = element->logic()) {
                    updatePort(sink); // Existing simulation logic
                }
            }
        }
    }

private:
    QSet<QNEOutputPort*> m_sources;
    QSet<QNEInputPort*> m_sinks;
};

class UnifiedConnectionManager {
public:
    // Physical connections create 2-node buses
    void addPhysicalConnection(QNEOutputPort* from, QNEInputPort* to) {
        auto bus = std::make_unique<SignalBus>();
        bus->addSource(from);
        bus->addSink(to);
        m_buses.push_back(std::move(bus));
    }
    
    // Wireless connections create N-node buses
    void addWirelessGroup(const QString& label, const QSet<Node*>& nodes) {
        auto bus = std::make_unique<SignalBus>();
        for (auto* node : nodes) {
            bus->addSource(node->outputPort());
            bus->addSink(node->inputPort());
        }
        m_buses.push_back(std::move(bus));
    }
    
    // Unified signal propagation
    void updateAllConnections() {
        for (auto& bus : m_buses) {
            bus->propagateSignal();
        }
    }

private:
    std::vector<std::unique_ptr<SignalBus>> m_buses;
};
```

## Implementation Strategy

### Phase 1: Extract Common Logic
- Create `SignalBus` class with unified contention detection
- Maintain existing interfaces for backward compatibility

### Phase 2: Migrate Physical Connections
- Replace direct port connections with bus system
- Preserve `QNEConnection` objects for UI/serialization
- Update `QNEInputPort::isValid()` to delegate to bus system

### Phase 3: Migrate Wireless Connections
- Replace `updateWirelessConnections()` with bus system
- Preserve `WirelessConnectionManager` for label management
- Remove duplicate contention detection code

### Phase 4: Consolidate Simulation
- Single `updateAllConnections()` call in simulation loop
- Remove separate physical/wireless update phases

## Benefits

### 1. **Guaranteed Consistency**
- Impossible for physical and wireless to have different behaviors
- Single source of truth for signal propagation logic

### 2. **Code Deduplication**
- Eliminates ~50 lines of duplicate contention detection logic
- Single implementation to maintain and debug

### 3. **Performance Improvement**
- More efficient signal propagation (single pass vs separate phases)
- Better cache locality with unified data structure

### 4. **Extensibility**
- New connection types automatically inherit correct behavior
- Features like Unknown/HighZ states work universally
- Easy to add bidirectional buses, tri-state logic, etc.

### 5. **Maintainability**
- Bugs fixed once affect both connection types
- Simpler simulation loop logic
- Clearer separation of concerns

## Preserved Characteristics

### UI/Visual Representation
- Physical connections keep visible wires (`QNEConnection` objects)
- Wireless connections remain invisible (label-based)
- No UI changes required

### Serialization
- Physical connections serialize as connection objects
- Wireless connections serialize as node labels
- File format compatibility maintained

### Performance
- Expected improvement due to unified processing
- Single signal propagation pass vs current dual-phase approach

## Risk Mitigation

### Backward Compatibility
- Implement behind feature flag initially
- Comprehensive regression testing
- Preserve existing APIs during transition

### Incremental Migration
- Phase-by-phase implementation
- Each phase maintains full functionality
- Easy rollback if issues discovered

### Testing Strategy
- All existing tests must pass
- Additional tests for bus edge cases
- Performance benchmarking

## Expected Outcomes

1. **Immediate**: Elimination of behavioral divergence risk
2. **Short-term**: Reduced maintenance burden
3. **Long-term**: Foundation for advanced connection features

## Implementation Effort

- **High impact, medium complexity**
- Estimated 2-3 weeks for full implementation
- Most complex part: ensuring zero behavioral changes
- Most beneficial part: future feature development becomes much easier

## Conclusion

Merging the backends through a unified bus architecture is not only feasible but represents a significant architectural improvement. The abstraction naturally captures both connection types while eliminating code duplication and guaranteeing behavioral consistency.

This change would transform ad-hoc signal propagation into a clean, extensible system that provides a foundation for future enhancements.