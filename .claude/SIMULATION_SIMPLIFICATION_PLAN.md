# Simulation Architecture Simplification Plan

## Current Architecture Problems

### The Double Representation Problem

**Visual Elements (GraphicElement hierarchy):**
- `And`, `Or`, `Nand`, `Not`, etc. (`app/element/`)
- UI representation with Qt graphics
- Handle user interaction, rendering, properties
- Contain minimal logic - mostly just graphical state

**Virtual Elements (LogicElement hierarchy):**
- `LogicAnd`, `LogicOr`, `LogicNand`, `LogicNot`, etc. (`app/logicelement/`)
- Pure simulation logic with no graphics
- Connected in separate graph structure
- Actual computation happens here

**Complex Mapping Layer:**
- `ElementMapping` class creates virtual elements from visual ones
- `ElementFactory::buildLogicElement()` - massive switch statement
- Each visual element gets a corresponding virtual element
- Connections mapped separately between the two graphs

### IC Complexity Nightmare

**Current IC Processing:**
1. **IC contains visual elements** (`m_icElements`)
2. **IC.generateMap()** creates `ElementMapping` for internal elements
3. **Returns virtual logic elements** from internal mapping
4. **ElementMapping.generateMap()** recursively processes ICs
5. **Special case handling** in `applyConnection()` for IC ports
6. **Double lookup system**: `ic->inputLogic()` and `ic->outputLogic()`

**Problems:**
- **Recursive complexity**: ICs containing ICs create nested mappings
- **Memory overhead**: Every visual element has a corresponding virtual element
- **Debugging hell**: Hard to trace simulation through two separate graphs
- **Code duplication**: Logic duplicated between visual/virtual hierarchies
- **Performance impact**: Double traversal and mapping overhead

### Current Simulation Flow

```cpp
// Convoluted current flow:
ElementMapping mapping(visual_elements);     // Create virtual graph
mapping.generateMap();                       // Visual -> Virtual conversion
mapping.connectElements();                   // Rebuild connections in virtual graph
mapping.sort();                             // Sort by priority

// Simulation loop:
for (auto &logic : mapping.logicElms()) {   // Traverse VIRTUAL elements
    logic->updateLogic();                   // Update virtual state
}

// Update visual representation:
for (auto *connection : connections) {      // Update UI from virtual state
    updatePort(connection->startPort());    // Virtual -> Visual sync
}
```

## Proposed Simplified Architecture

### Core Principle: Direct Simulation on Visual Elements

**Eliminate virtual elements entirely** - make `GraphicElement` classes simulation-capable.

### Phase 1: Merge Visual and Virtual

#### 1.1 Add Simulation Methods to GraphicElement

```cpp
// app/graphicelement.h
class GraphicElement : public QGraphicsObject {
public:
    // Existing graphical methods...
    
    // NEW: Direct simulation methods
    virtual void updateLogic();
    virtual bool computeOutput(int outputIndex = 0);
    virtual void setInputValue(int inputIndex, bool value);
    virtual bool getInputValue(int inputIndex) const;
    virtual bool getOutputValue(int outputIndex = 0) const;
    
    // Connection state
    void connectInput(int inputIndex, GraphicElement* source, int sourceOutput);
    QVector<bool> getInputValues() const;
    
private:
    QVector<bool> m_inputValues;
    QVector<bool> m_outputValues;
    QVector<InputConnection> m_inputConnections;
};

struct InputConnection {
    GraphicElement* sourceElement = nullptr;
    int sourceOutputIndex = 0;
    bool isValid = false;
};
```

#### 1.2 Implement Logic in Visual Elements

```cpp
// app/element/and.h
class And : public GraphicElement {
public:
    void updateLogic() override {
        // Get input values directly from connected elements
        updateInputValues();
        
        // AND gate logic
        bool result = true;
        for (bool input : m_inputValues) {
            result &= input;
        }
        
        setOutputValue(result);
    }
};

// app/element/or.h  
class Or : public GraphicElement {
public:
    void updateLogic() override {
        updateInputValues();
        
        bool result = false;
        for (bool input : m_inputValues) {
            result |= input;
        }
        
        setOutputValue(result);
    }
};
```

#### 1.3 Simplify Sequential Elements

```cpp
// app/element/dflipflop.h
class DFlipFlop : public GraphicElement {
public:
    void updateLogic() override {
        updateInputValues();
        
        bool d = getInputValue(0);      // Data
        bool clk = getInputValue(1);    // Clock
        bool preset = getInputValue(2); // Preset
        bool clear = getInputValue(3);  // Clear
        
        // D Flip-flop logic with edge detection
        if (!preset) {
            m_q = true;
        } else if (!clear) {
            m_q = false;
        } else if (clk && !m_lastClock) { // Rising edge
            m_q = d;
        }
        
        m_lastClock = clk;
        setOutputValue(0, m_q);      // Q output
        setOutputValue(1, !m_q);     // Q̄ output
    }
    
private:
    bool m_q = false;
    bool m_lastClock = false;
};
```

### Phase 2: Simplify IC Architecture

#### 2.1 Direct IC Simulation

```cpp
// app/ic.h - SIMPLIFIED
class IC : public GraphicElement {
public:
    void updateLogic() override {
        // Update IC input nodes from external connections
        updateInputNodes();
        
        // Simulate internal circuit directly
        simulateInternalCircuit();
        
        // Propagate outputs to external connections  
        updateOutputNodes();
    }
    
private:
    void simulateInternalCircuit() {
        // Direct traversal of m_icElements (no virtual mapping!)
        auto sortedElements = topologicalSort(m_icElements);
        
        for (auto* element : sortedElements) {
            element->updateLogic();  // Direct simulation!
        }
    }
    
    void updateInputNodes() {
        for (int i = 0; i < m_icInputs.size(); ++i) {
            bool externalValue = getInputValue(i);
            m_icInputs[i]->graphicElement()->setOutputValue(externalValue);
        }
    }
    
    void updateOutputNodes() {
        for (int i = 0; i < m_icOutputs.size(); ++i) {
            bool internalValue = m_icOutputs[i]->graphicElement()->getInputValue(0);
            setOutputValue(i, internalValue);
        }
    }
    
    // REMOVE: generateMap(), inputLogic(), outputLogic()
    // REMOVE: ElementMapping* mapping
};
```

#### 2.2 Eliminate ElementMapping Class

**Delete entire class** - no longer needed!

```cpp
// DELETE: app/elementmapping.h
// DELETE: app/elementmapping.cpp
// DELETE: All LogicElement classes in app/logicelement/
```

### Phase 3: Simplified Simulation Engine

#### 3.1 Direct Simulation Loop

```cpp
// app/simulation.cpp - VASTLY SIMPLIFIED
class Simulation : public QObject {
public:
    void update() override {
        if (!initialize()) return;
        
        // Update clocks (timing elements)
        updateClocks();
        
        // Update inputs (user-controlled elements)
        updateInputs();
        
        // Direct simulation on visual elements!
        for (auto* element : m_sortedElements) {
            element->updateLogic();  // Single traversal!
        }
        
        // UI updates happen automatically via Qt signals
        updateVisualPorts();
    }
    
private:
    bool initialize() {
        if (m_initialized) return true;
        
        // Get all elements from scene
        m_elements = m_scene->elements();
        
        // Build connection graph directly
        buildConnectionGraph();
        
        // Topological sort for simulation order
        m_sortedElements = topologicalSort(m_elements);
        
        m_initialized = true;
        return true;
    }
    
    void buildConnectionGraph() {
        for (auto* element : m_elements) {
            for (int i = 0; i < element->inputSize(); ++i) {
                auto* inputPort = element->inputPort(i);
                auto connections = inputPort->connections();
                
                if (connections.size() == 1) {
                    auto* outputPort = connections[0]->startPort();
                    auto* sourceElement = outputPort->graphicElement();
                    
                    element->connectInput(i, sourceElement, outputPort->index());
                }
            }
        }
    }
    
    // REMOVE: ElementMapping* m_elmMapping
    // REMOVE: Complex IC handling code
    QVector<GraphicElement*> m_elements;
    QVector<GraphicElement*> m_sortedElements;
};
```

### Phase 4: Connection System Improvements

#### 4.1 Direct Port Communication

```cpp
// app/nodes/qneport.h - Enhanced
class QNEPort : public QGraphicsObject {
public:
    // Existing Qt graphics methods...
    
    // NEW: Direct value propagation
    void updateValue() {
        if (m_portType == PortType::Output) {
            // Get value directly from parent element
            bool value = graphicElement()->getOutputValue(index());
            setStatus(value ? Status::Active : Status::Inactive);
            
            // Propagate to connected ports immediately
            for (auto* connection : m_connections) {
                if (auto* targetPort = connection->endPort()) {
                    targetPort->receiveValue(value);
                }
            }
        }
    }
    
    void receiveValue(bool value) {
        if (m_portType == PortType::Input) {
            graphicElement()->setInputValue(index(), value);
            setStatus(value ? Status::Active : Status::Inactive);
        }
    }
};
```

#### 4.2 Eliminate ElementMapping Complexity

**Before (Complex):**
```cpp
ElementMapping mapping(elements);           // Create virtual graph
mapping.generateMap();                     // Visual->Virtual conversion
mapping.connectElements();                 // Rebuild connections  
mapping.sort();                           // Priority sorting

for (auto &logic : mapping.logicElms()) { // Traverse virtual elements
    logic->updateLogic();
}
```

**After (Simple):**
```cpp
auto sortedElements = topologicalSort(elements); // Direct sorting

for (auto* element : sortedElements) {           // Traverse visual elements
    element->updateLogic();                      // Direct simulation
}
```

## Migration Strategy

### Phase 1: Parallel Implementation (2-3 weeks)

1. **Add simulation methods to GraphicElement base class**
2. **Implement updateLogic() for simple gates (And, Or, Not)**
3. **Create dual-mode simulation** (old + new running in parallel)
4. **Validate identical results** between old and new systems

### Phase 2: Sequential Elements (2-3 weeks)

1. **Implement flip-flops and latches with direct simulation**
2. **Add edge detection and state management**
3. **Handle timing-critical elements (clocks)**
4. **Validate sequential circuit behavior**

### Phase 3: IC Simplification (3-4 weeks)

1. **Rewrite IC simulation without ElementMapping**
2. **Direct internal element traversal**
3. **Eliminate virtual logic element generation**
4. **Test nested IC hierarchies**

### Phase 4: Cleanup and Optimization (2-3 weeks)

1. **Remove all LogicElement classes**
2. **Delete ElementMapping infrastructure**
3. **Simplify Simulation class**
4. **Performance optimization and testing**

### Phase 5: Advanced Features (2-3 weeks)

1. **Enhanced debugging capabilities**
2. **Better error handling and validation**
3. **Performance monitoring and optimization**
4. **Documentation and examples**

## Expected Benefits

### 🎯 Simplicity
- **Single element hierarchy** instead of dual visual/virtual
- **Direct simulation** on visual elements
- **Eliminate mapping complexity** entirely
- **Easier debugging** - single code path

### ⚡ Performance  
- **Reduce memory usage** by 40-50% (no virtual elements)
- **Faster simulation** - eliminate mapping overhead
- **Better cache locality** - single traversal
- **Simpler IC handling** - no recursive mappings

### 🔧 Maintainability
- **Half the code** - eliminate entire LogicElement hierarchy
- **Single source of truth** for element behavior
- **Easier to add new elements** - implement once, not twice
- **Clearer simulation flow** - direct element traversal

### 🐛 Debugging
- **Unified debugging** - trace through visual elements directly
- **Simplified IC debugging** - no virtual element indirection
- **Better error messages** - direct element references
- **Visual debugging tools** - step through actual UI elements

## Risk Mitigation

### Compatibility Concerns
- **Parallel implementation** ensures behavioral compatibility
- **Extensive testing** with existing circuit files
- **Gradual migration** minimizes risk of regression
- **Rollback capability** if issues discovered

### Performance Validation
- **Benchmark existing circuits** before and after
- **Memory usage monitoring** throughout migration
- **Simulation accuracy verification** with test suite
- **Large circuit performance testing**

### IC Complexity
- **Start with simple ICs** - validate basic functionality
- **Incremental complexity** - nested ICs last
- **Extensive IC test suite** - cover edge cases
- **Backward compatibility** with existing IC files

## Success Metrics

✅ **Code Reduction**: 50%+ reduction in simulation-related code  
✅ **Performance**: 20%+ improvement in simulation speed  
✅ **Memory**: 40%+ reduction in simulation memory usage  
✅ **Maintainability**: Single codebase for visual and simulation logic  
✅ **Compatibility**: 100% backward compatibility with existing files  
✅ **Debugging**: Unified debugging experience through single element hierarchy

## Files to be Modified/Deleted

### Major Changes
- `app/graphicelement.h/.cpp` - Add simulation methods
- `app/simulation.h/.cpp` - Simplify simulation loop
- `app/ic.h/.cpp` - Direct simulation, remove mapping
- All `app/element/*.h/.cpp` - Add updateLogic() implementations

### Files to Delete
- `app/elementmapping.h/.cpp` - Entire mapping system
- `app/logicelement.h/.cpp` - Base virtual element class
- All `app/logicelement/*.h/.cpp` - All virtual element implementations

### Modified Infrastructure
- `app/elementfactory.h/.cpp` - Remove buildLogicElement()
- `app/nodes/qneport.h/.cpp` - Direct value propagation
- Test files - Update for new architecture

This plan transforms wiRedPanda's simulation from a complex dual-representation system into a clean, direct simulation architecture that's easier to understand, maintain, and debug while providing better performance.