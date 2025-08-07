# Detailed Plan: Add Verilog HDL Export Feature

## Current Analysis

**Project Context**: wiRedPanda currently supports Arduino export (limited/disabled) but lacks HDL export capabilities. Adding Verilog export would enable integration with FPGA development workflows and professional digital design tools.

**Existing Infrastructure:**
- **CodeGenerator pattern**: Arduino export provides template (`app/arduino/codegenerator.cpp`)
- **Element system**: 33+ element types with standardized interfaces
- **Connection system**: QNEPort-based connectivity with signal flow tracking
- **Simulation engine**: Event-driven simulation with timing support

## Verilog HDL Fundamentals

### Core Verilog Concepts for Export

#### Module Structure
```verilog
module top_module (
    input wire clk,
    input wire reset,
    input wire [3:0] data_in,
    output reg [7:0] data_out
);
    // Implementation
endmodule
```

#### Signal Types
- **wire**: Combinational connections
- **reg**: Storage elements (flip-flops, latches)
- **logic**: SystemVerilog (optional enhancement)

#### Timing Models
- **Combinational**: `assign` statements
- **Sequential**: `always @(posedge clk)` blocks
- **Clock domains**: Multiple clock handling

## Element Mapping Strategy

### 1. Combinational Logic

| wiRedPanda Element | Verilog Implementation |
|-------------------|------------------------|
| **And** | `assign out = in1 & in2 & in3;` |
| **Or** | `assign out = in1 | in2 | in3;` |
| **Not** | `assign out = ~in;` |
| **Nand** | `assign out = ~(in1 & in2);` |
| **Nor** | `assign out = ~(in1 | in2);` |
| **Xor** | `assign out = in1 ^ in2;` |
| **Xnor** | `assign out = ~(in1 ^ in2);` |

### 2. Sequential Logic

| Element | Verilog Implementation |
|---------|------------------------|
| **DFlipFlop** | ```verilog<br>always @(posedge clk)<br>  if (~preset)<br>    q <= 1;<br>  else if (~clear)<br>    q <= 0;<br>  else<br>    q <= d;<br>assign qn = ~q;``` |
| **JKFlipFlop** | ```verilog<br>always @(posedge clk)<br>  case({j,k})<br>    2'b00: q <= q;<br>    2'b01: q <= 0;<br>    2'b10: q <= 1;<br>    2'b11: q <= ~q;<br>  endcase``` |
| **TFlipFlop** | ```verilog<br>always @(posedge clk)<br>  if (t)<br>    q <= ~q;``` |

### 3. Special Elements

| Element | Verilog Strategy |
|---------|------------------|
| **Clock** | Module input port `clk` |
| **InputVcc** | `assign signal = 1'b1;` |
| **InputGnd** | `assign signal = 1'b0;` |
| **Mux** | ```verilog<br>assign out = sel ? in1 : in0;``` |
| **Demux** | ```verilog<br>assign {out1, out0} = sel ? {1'b0, in} : {in, 1'b0};``` |
| **TruthTable** | Custom `always_comb` block |
| **Display7/14/16** | Comment blocks (non-synthesizable) |

### 4. IC Hierarchy

**Strategy**: Flatten ICs or generate hierarchical modules
- **Flattening**: Expand ICs into primitive gates (simpler)
- **Hierarchical**: Generate sub-modules (cleaner, more readable)

## Implementation Architecture

### Phase 1: Core Infrastructure

#### 1.1 VerilogGenerator Class
```cpp
// app/verilog/veriloggenerator.h
class VerilogGenerator {
public:
    VerilogGenerator(const QString &fileName, const QVector<GraphicElement*> &elements);
    void generate();
    
private:
    void generateHeader();
    void generateModuleDeclaration();
    void generatePortDeclarations();
    void generateWireDeclarations();
    void generateCombinationalLogic();
    void generateSequentialLogic();
    void generateModuleEnd();
    
    QFile m_file;
    QTextStream m_stream;
    QVector<GraphicElement*> m_elements;
    QHash<QNEPort*, QString> m_signalMap;
    QStringList m_clockSignals;
    int m_wireCounter = 0;
};
```

#### 1.2 Signal Management
```cpp
class SignalManager {
public:
    QString getSignalName(QNEPort* port);
    QString generateUniqueWire();
    void mapConnection(QNEConnection* conn);
    
private:
    QHash<QNEPort*, QString> m_portToSignal;
    QSet<QString> m_usedNames;
    int m_counter = 0;
};
```

#### 1.3 Element Visitors
```cpp
class VerilogElementVisitor {
public:
    virtual QString generateVerilog(GraphicElement* element) = 0;
};

class CombinationalVisitor : public VerilogElementVisitor {
    QString generateVerilog(GraphicElement* element) override;
};

class SequentialVisitor : public VerilogElementVisitor {
    QString generateVerilog(GraphicElement* element) override;
};
```

### Phase 2: Advanced Features

#### 2.1 Hierarchical IC Support
```cpp
class ICHierarchyManager {
public:
    void processIC(IC* ic, int depth = 0);
    QString generateSubModule(IC* ic);
    void flattenIC(IC* ic, QVector<GraphicElement*>& flatElements);
    
private:
    QMap<QString, IC*> m_subModules;
    bool m_useHierarchy = true;
};
```

#### 2.2 Clock Domain Analysis
```cpp
class ClockDomainAnalyzer {
public:
    QVector<ClockDomain> analyzeDomains(const QVector<GraphicElement*>& elements);
    void generateClockConstraints(QTextStream& stream);
    
private:
    struct ClockDomain {
        Clock* source;
        QVector<GraphicElement*> sequential_elements;
        float frequency;
    };
};
```

#### 2.3 Timing Constraints
```verilog
// Generated timing constraints (.sdc file)
create_clock -name clk0 -period 10.0 [get_ports clk0]
create_clock -name clk1 -period 5.0 [get_ports clk1]
set_false_path -from [get_clocks clk0] -to [get_clocks clk1]
```

### Phase 3: Multi-Target Support

#### 3.1 Target Platforms
```cpp
enum class VerilogTarget {
    Generic,          // IEEE 1364-2005 compatibility
    Xilinx,           // Vivado/ISE optimization
    Intel,            // Quartus optimization
    Simulation,       // Testbench generation
    SystemVerilog     // SV-2012 features
};

class TargetOptimizer {
public:
    void optimizeForTarget(VerilogTarget target);
    QString getTargetSpecificCode(ElementType type, VerilogTarget target);
};
```

#### 3.2 Synthesis Attributes
```verilog
// Xilinx attributes
(* KEEP = "TRUE" *) wire important_signal;
(* dont_touch = "true" *) reg debug_register;

// Intel attributes
(* preserve *) reg intel_debug;
(* max_fanout = 50 *) wire high_fanout_net;
```

## Testing Strategy

### Phase 4: Validation Framework

#### 4.1 Unit Tests
```cpp
// test/testverilogexport.cpp
class TestVerilogExport : public QObject {
    Q_OBJECT
    
private slots:
    void testSimpleGateGeneration();
    void testSequentialLogic();
    void testClockDomainHandling();
    void testICHierarchy();
    void testSignalNaming();
    void testSynthesisValidity();
};
```

#### 4.2 Integration Tests

**Simulation Validation:**
```bash
# Compile with open-source tools
iverilog -o testbench generated_circuit.v testbench.v
vvp testbench
gtkwave dump.vcd

# Commercial tools
vivado -mode batch -source verify_circuit.tcl
quartus_sh --flow compile generated_project
```

**Test Cases:**
- Simple combinational circuits
- Sequential circuits with single clock
- Multi-clock domain designs
- IC hierarchies (flattened vs. hierarchical)
- Large circuits (performance testing)

#### 4.3 Compatibility Testing

**Simulator Support:**
- ✅ **Icarus Verilog** (open-source)
- ✅ **Verilator** (high-performance)
- ✅ **ModelSim/QuestaSim** (commercial)
- ✅ **Vivado Simulator** (Xilinx)
- ✅ **Questa Intel Edition** (Intel)

**Synthesis Tools:**
- ✅ **Yosys** (open-source)
- ✅ **Vivado Synthesis** (Xilinx FPGAs)
- ✅ **Quartus Prime** (Intel FPGAs)
- ✅ **Design Compiler** (ASIC)

## Implementation Timeline

### Sprint 1 (Week 1-2): Foundation
- [ ] Create VerilogGenerator class
- [ ] Implement basic combinational logic export
- [ ] Signal name management system
- [ ] Simple module generation

### Sprint 2 (Week 3-4): Sequential Logic
- [ ] Clock detection and management
- [ ] Flip-flop and latch generation
- [ ] Reset/preset handling
- [ ] Basic timing constraints

### Sprint 3 (Week 5-6): IC Support
- [ ] IC flattening algorithm
- [ ] Hierarchical module generation
- [ ] Connection mapping across hierarchy
- [ ] Name collision resolution

### Sprint 4 (Week 7-8): Advanced Features
- [ ] Multi-clock domain analysis
- [ ] Truth table to case statement conversion
- [ ] Mux/demux optimization
- [ ] Target-specific optimizations

### Sprint 5 (Week 9-10): Testing & Polish
- [ ] Comprehensive test suite
- [ ] Simulator compatibility validation
- [ ] Performance optimization
- [ ] Documentation and examples

### Sprint 6 (Week 11-12): UI Integration
- [ ] Menu integration
- [ ] Export dialog with options
- [ ] Error handling and user feedback
- [ ] Example circuit library

## Export Options Dialog

### 4.1 User Interface
```cpp
class VerilogExportDialog : public QDialog {
    Q_OBJECT

public:
    struct ExportOptions {
        VerilogTarget target = VerilogTarget::Generic;
        bool useHierarchy = false;
        bool generateTestbench = false;
        bool addTimingConstraints = true;
        QString moduleName = "top_module";
        QString clockName = "clk";
        bool resetActiveHigh = false;
    };
    
    ExportOptions getOptions() const;
};
```

### 4.2 Export Dialog Features
- **Target Platform**: Generic/Xilinx/Intel/Simulation
- **Hierarchy**: Flatten ICs vs. Generate sub-modules
- **Testbench**: Auto-generate verification testbench
- **Constraints**: Generate .sdc timing constraints
- **Naming**: Module and signal naming preferences
- **Reset Logic**: Active high/low reset convention

## File Structure

### Generated Files
```
output_directory/
├── top_module.v           # Main Verilog module
├── sub_module1.v          # IC sub-modules (if hierarchical)
├── sub_module2.v
├── testbench.v            # Auto-generated testbench
├── timing_constraints.sdc # Clock constraints
└── synthesis_script.tcl   # Tool-specific scripts
```

### Example Output
```verilog
//================================================
// Generated by wiRedPanda Verilog Export
// Date: 2025-01-07 10:30:00
// Source: counter_circuit.panda
//================================================

module top_module (
    input wire clk,
    input wire reset,
    input wire enable,
    output wire [3:0] count,
    output wire overflow
);

// Wire declarations
wire d0_q, d1_q, d2_q, d3_q;
wire and1_out, xor1_out;

// Sequential logic - D Flip Flops
always @(posedge clk or posedge reset) begin
    if (reset) begin
        d0_q <= 1'b0;
        d1_q <= 1'b0;
        d2_q <= 1'b0;
        d3_q <= 1'b0;
    end else if (enable) begin
        d0_q <= ~d0_q;
        d1_q <= d0_q ? ~d1_q : d1_q;
        d2_q <= (&{d1_q, d0_q}) ? ~d2_q : d2_q;
        d3_q <= (&{d2_q, d1_q, d0_q}) ? ~d3_q : d3_q;
    end
end

// Output assignments
assign count = {d3_q, d2_q, d1_q, d0_q};
assign overflow = &count & enable;

endmodule
```

## Advanced Challenges & Solutions

### Challenge 1: Timing Closure
**Problem**: Generated Verilog may not meet timing requirements
**Solution**: 
- Generate conservative timing constraints
- Add pipeline registers for long combinational paths
- Use synthesis directives for critical nets

### Challenge 2: Resource Optimization  
**Problem**: Inefficient resource usage
**Solution**:
- Detect and merge equivalent logic
- Use target-specific primitives (LUTs, DSPs, BRAMs)
- Apply Boolean minimization

### Challenge 3: Verification Complexity
**Problem**: Large designs difficult to verify
**Solution**:
- Generate hierarchical testbenches
- Create assertion-based verification
- Support formal verification hooks

### Challenge 4: Multi-Clock Domains
**Problem**: Clock domain crossing issues
**Solution**:
- Detect CDC violations
- Insert synchronizer blocks
- Generate proper timing constraints

## Success Criteria

✅ **Functional**: All wiRedPanda elements export to valid Verilog  
✅ **Synthesizable**: Generated code passes synthesis tools  
✅ **Simulatable**: Compatible with major HDL simulators  
✅ **Hierarchical**: Support both flattened and modular output  
✅ **Timing-Aware**: Generate appropriate timing constraints  
✅ **Multi-Target**: Support FPGA and ASIC design flows  
✅ **Professional**: Industry-standard code quality and style  
✅ **Scalable**: Handle large, complex circuit designs efficiently

## Risk Mitigation

**Complexity Management:**
- Start with simple combinational circuits
- Incremental feature addition with thorough testing
- Modular architecture for easy debugging

**Tool Compatibility:**
- Target IEEE 1364-2005 standard for broad compatibility
- Test with multiple simulators and synthesis tools
- Provide target-specific optimizations as needed

**Performance Concerns:**
- Implement efficient algorithms for large circuit traversal
- Cache results where possible
- Provide progress feedback for long operations

This comprehensive plan establishes wiRedPanda as a complete digital design tool, bridging the gap between educational circuit simulation and professional FPGA development workflows.