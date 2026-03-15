# wiRedPanda Element & Spacing Reference Guide

**For MCP Users** - Comprehensive reference of all elements, their dimensions, port positions, and proper spacing for IC generators.

**Location:** `test/integration/ic/generators/`
**Last Updated:** 2026-02-03

---

## Table of Contents

1. [Grid & Sizing System](#grid--sizing-system)
2. [Element Reference](#element-reference)
   - [Input Elements](#input-elements)
   - [Logic Gates](#logic-gates)
   - [Sequential Logic](#sequential-logic)
   - [Multiplexing Elements](#multiplexing-elements)
   - [Output Elements](#output-elements)
   - [Other Elements](#other-elements)
3. [Spacing Constants & Standards](#spacing-constants--standards)
4. [Practical Spacing Patterns](#practical-spacing-patterns)
5. [Implementation Templates](#implementation-templates)
6. [Port Positioning Reference](#port-positioning-reference)
7. [Port Configuration Reference](#port-configuration-reference)
8. [Quick Reference Lookup](#quick-reference-lookup)

---

## Grid & Sizing System

### Base Grid Metrics

From `App/Scene/Scene.h`:

| Property | Value | Purpose |
| --- | --- | --- |
| Grid Size | 16 pixels | Base grid unit for snapping |
| Half Step | 8 pixels | Port vertical spacing |
| Port Radius | 5 pixels | Visual size of connection point |
| Port Margin | 2 pixels | Text label spacing |

### Standard Element Dimensions

**Most elements:** 64×64 pixels (including all gates, inputs, memory elements, displays)

**Exceptions:**

- Node: 32×32 pixels (smaller junction point)
- Buzzer: 68.27×68.27 pixels (slightly larger)
- AudioBox: 68.27×68.27 pixels (slightly larger)
- IC (panda2 variant): 34.79×30.16 pixels (compact)

### Bounding Rectangle Calculation

```text
BoundingRect = Pixmap Rectangle ∪ Ports Bounding Rectangle
```

The visual bounding box of an element expands to include:

1. The base pixmap (64×64 in most cases)
2. All input/output ports and their labels

---

## Element Reference

### Input Elements

#### Clock

| Property | Value |
| --- | --- |
| **Size** | 64×64 pixels |
| **Ports** | 0 inputs, 1 output |
| **Port Position** | Output: (64, 32) |
| **Rotating** | Yes |
| **Bounding Box** | ~74×74 |

#### InputButton, InputSwitch

| Property | Value |
| --- | --- |
| **Size** | 64×64 pixels |
| **Ports** | 0 inputs, 1 output |
| **Port Position** | Output: (64, 32) |
| **Rotating** | Yes |
| **Bounding Box** | ~74×74 |

#### InputRotary

| Property | Value |
| --- | --- |
| **Size** | 64×64 pixels |
| **Ports** | 0 inputs, 8-16 outputs (variable) |
| **Port Positions** | Multiple outputs, distributed around element |
| **Rotating** | Yes |
| **Bounding Box** | Expands based on output count |

#### InputVcc (Power High) & InputGnd (Ground/Low)

| Property | Value |
| --- | --- |
| **Size** | 64×64 pixels |
| **Ports** | 0 inputs, 1 output |
| **Port Position** | Output: (64, 32) |
| **Rotating** | No (static) |
| **Bounding Box** | ~74×74 |

---

### Logic Gates

#### AND, OR, NAND, NOR, XOR, XNOR

| Property | Value |
| --- | --- |
| **Size** | 64×64 pixels |
| **Ports** | 2-8 inputs (configurable), 1 output |
| **Input Positions** | Left side: x=0, spaced 16 pixels apart |
| **Output Position** | Right side: (64, 32) centered |
| **Rotating** | Yes |
| **Bounding Box** | Depends on input count |

**Vertical Spacing Formula:**

- Center Y = 32 pixels (middle of 64×64)
- Spacing = 16 pixels per port (2 × step)
- Start Y = 32 - (numInputs × 8) + 8

#### NOT Gate

| Property | Value |
| --- | --- |
| **Size** | 64×64 pixels |
| **Ports** | 1 input, 1 output |
| **Input Position** | (0, 32) |
| **Output Position** | (64, 32) |
| **Rotating** | Yes |
| **Bounding Box** | ~74×74 |

---

### Sequential Logic

#### D Flip-Flop, D Latch, SR Flip-Flop, T Flip-Flop

| Property | Value |
| --- | --- |
| **Size** | 64×64 pixels |
| **Ports** | 2 inputs, 2 outputs (typical) |
| **Input Positions** | (0, 24) and (0, 40) |
| **Output Positions** | (64, 24) and (64, 40) |
| **Rotating** | Yes |
| **Bounding Box** | ~74×74 |

#### JK Flip-Flop

| Property | Value |
| --- | --- |
| **Size** | 64×64 pixels |
| **Ports** | 3 inputs (J, K, CLK), 2 outputs (Q, Q') |
| **Input Positions** | (0, 16), (0, 32), (0, 48) |
| **Output Positions** | (64, 24) and (64, 40) |
| **Rotating** | Yes |
| **Bounding Box** | ~74×74 |

---

### Multiplexing Elements

#### Multiplexer (Mux)

| Property | Value |
| --- | --- |
| **Size** | 64×64 pixels |
| **Ports** | 3-11 inputs total (data + select), 1 output |
| **Data Input Positions** | Left: x=16, spaced vertically |
| **Select Positions** | Bottom: y=56, spaced horizontally |
| **Output Position** | (64, 32) |
| **Bounding Box** | Expands based on data input count |

#### Demultiplexer (Demux)

| Property | Value |
| --- | --- |
| **Size** | 64×64 pixels |
| **Ports** | 2-4 inputs total (data + select), 2-8 outputs |
| **Data Input Position** | (16, 32) - single data line |
| **Select Positions** | Bottom: y=56, spaced horizontally |
| **Output Positions** | Right: x=64, spaced vertically |
| **Bounding Box** | Expands based on output count |

---

### Output Elements

#### LED

| Property | Value |
| --- | --- |
| **Size** | 64×64 pixels |
| **Ports** | 1-4 inputs (configurable), 0 outputs |
| **Input Positions** | Left side: x=0, spaced 16 pixels apart |
| **Rotating** | Yes |
| **Bounding Box** | ~74×74 (1 input) to ~74×88 (4 inputs) |

#### Buzzer, AudioBox

| Property | Value |
| --- | --- |
| **Size** | 68.27×68.27 pixels |
| **Ports** | 1-4 inputs, 0 outputs |
| **Input Positions** | Left side: x=0, spaced 16 pixels apart |
| **Rotating** | Yes |
| **Bounding Box** | ~78×78 to ~78×92 |

#### 7/14/16-Segment Display

| Property | Value |
| --- | --- |
| **Size** | 64×64 pixels (base) |
| **Ports** | 7/14/16 inputs (fixed), 0 outputs |
| **Input Positions** | 3-4 per side (left & right) |
| **Rotating** | Yes (limited) |
| **Bounding Box** | ~110×90 to ~110×130 |

---

### Other Elements

#### Node (Wire Junction)

| Property | Value |
| --- | --- |
| **Size** | 32×32 pixels (smallest) |
| **Ports** | 1 input, 1 output |
| **Input Position** | (0, 16) |
| **Output Position** | (32, 16) |
| **Rotating** | No |
| **Bounding Box** | ~42×42 |

#### Truth Table

| Property | Value |
| --- | --- |
| **Size** | 64×64 pixels (base, expands with ports) |
| **Ports** | 1-8 inputs, 1-16 outputs (variable) |
| **Input Positions** | Left side: x=0, spaced 16 pixels |
| **Output Positions** | Right side: x=64, spaced based on output count |
| **Rotating** | Yes |
| **Bounding Box** | Dynamic: min 64×64, expands with ports |

#### Text, Line

| Property | Value |
| --- | --- |
| **Size** | 64×64 pixels |
| **Ports** | 0 inputs, 0 outputs |
| **Rotating** | Yes |
| **Bounding Box** | Variable / ~74×74 |

---

## Spacing Constants & Standards

### 📏 Spacing Constants

All spacing constants are defined in `element_spacing.py`:

```python
from element_spacing import (
    HORIZONTAL_GATE_SPACING,      # 104px - between gates left/right
    VERTICAL_STAGE_SPACING,       # 104px - between logic stages top/bottom
    VERTICAL_INPUT_SPACING,       # 96px - between inputs in a column
    RECOMMENDED_SPACING,          # 104px - standard center-to-center
    calculate_grid_positions,     # Helper for 2D grids
)
```

### Key Constants Reference

| Constant | Value | Usage |
| --- | --- | --- |
| `MIN_SAFE_SPACING` | 96px | Minimum safe distance between element centers |
| `VERTICAL_INPUT_SPACING` | 96px | Between vertically stacked inputs |
| `HORIZONTAL_GATE_SPACING` | 104px | Between horizontally placed gates |
| `VERTICAL_STAGE_SPACING` | 104px | Between logic stages |
| `RECOMMENDED_SPACING` | 104px | Standard spacing (most common) |

### Why These Values?

- Element size: 64×64 pixels
- Port circle radius: 5 pixels on each side = 10 pixels total
- Safe margin: 22-48 pixels
- **Minimum safe center-to-center: 96 pixels**
- **Recommended spacing: 104 pixels** (64 + 40 margin)

### 🎯 The Golden Rule

**Never use hardcoded offsets. Always use spacing constants.**

```python
❌ WRONG:  "x": 50.0 + 60.0      # Too close!
✅ RIGHT:  "x": 50.0 + HORIZONTAL_GATE_SPACING
```

---

## Practical Spacing Patterns

### 📍 Quick Position Patterns

#### Pattern A: Horizontal Row (Inputs or Gates)

```python
x_pos = base_x + i * HORIZONTAL_GATE_SPACING
# Creates: 50, 154, 258, 362, ... (104px apart)
```

#### Pattern B: Vertical Column (Inputs or Gates)

```python
y_pos = base_y + i * VERTICAL_STAGE_SPACING
# Creates: 100, 204, 308, 412, ... (104px apart)
```

#### Pattern C: Multi-Stage Flow (Left→Right)

```python
input_x = 50.0
gate_x = input_x + HORIZONTAL_GATE_SPACING
output_x = gate_x + HORIZONTAL_GATE_SPACING
# Creates: 50 → 154 → 258 → ...
```

#### Pattern D: 2×2 Grid (4 elements)

```python
positions = calculate_grid_positions(
    num_elements=4, cols=2,
    start_x=300.0, start_y=100.0,
    h_spacing=HORIZONTAL_GATE_SPACING,
    v_spacing=VERTICAL_STAGE_SPACING
)
# Returns: [(300,100), (404,100), (300,204), (404,204)]
```

---

## Implementation Templates

### Template 1: Vertical Inputs

```python
from element_spacing import VERTICAL_STAGE_SPACING

for i in range(num_inputs):
    response = await mcp.send_command("create_element", {
        "type": "InputSwitch",
        "x": 50.0,
        "y": 100.0 + i * VERTICAL_STAGE_SPACING,
        "label": f"In{i}"
    })
```

### Template 2: Horizontal Gates

```python
from element_spacing import HORIZONTAL_GATE_SPACING

for i in range(num_gates):
    response = await mcp.send_command("create_element", {
        "type": "And",
        "x": 154.0 + i * HORIZONTAL_GATE_SPACING,
        "y": 100.0,
        "label": f"Gate{i}"
    })
```

### Template 3: 2×2 Grid

```python
from element_spacing import calculate_grid_positions

positions = calculate_grid_positions(
    num_elements=4, cols=2,
    start_x=300.0, start_y=100.0,
    h_spacing=HORIZONTAL_GATE_SPACING,
    v_spacing=VERTICAL_STAGE_SPACING
)

for i, (x, y) in enumerate(positions):
    response = await mcp.send_command("create_element", {
        "type": "And",
        "x": x,
        "y": y,
        "label": f"and[{i}]"
    })
```

---

## Port Positioning Reference

### Standard Port Calculation

All elements use this standard port positioning system:

```cpp
// Port radius for visual feedback
const int portRadius = 5;

// Standard element width
const int elementWidth = 64;

// For element with N inputs:
int centerY = 32;  // Middle of 64×64 pixmap
int spacing = 16;  // pixels between ports (2 × step of 8)
int startY = centerY - (numInputs * 8) + 8;

// Input port positions: (x=0, y varies)
for (int i = 0; i < numInputs; i++) {
    int portX = 0;
    int portY = startY + (i * spacing);
}

// Output port positions: (x=64, y varies)
for (int i = 0; i < numOutputs; i++) {
    int portX = elementWidth;
    int portY = startY + (i * spacing);
}
```

### Label Positioning

**Default:**

- Position: y = 64 (directly below element)
- Font: SansSerif, Bold
- Alignment: Centered horizontally on element
- Color: Adjusts to theme (dark/light)

---

## Port Configuration Reference

### Min/Max Input/Output Constraints

| Element | Min In | Max In | Min Out | Max Out |
| --- | --- | --- | --- | --- |
| And/Or/Nand/Nor/Xor/Xnor | 2 | 8 | 1 | 1 |
| Not | 1 | 1 | 1 | 1 |
| D Flip-Flop | 2 | 2 | 2 | 2 |
| D Latch | 2 | 2 | 2 | 2 |
| JK Flip-Flop | 3 | 3 | 2 | 2 |
| SR Flip-Flop | 2 | 2 | 2 | 2 |
| T Flip-Flop | 2 | 2 | 2 | 2 |
| Clock | 0 | 0 | 1 | 1 |
| InputButton/Switch/Vcc/Gnd | 0 | 0 | 1 | 1 |
| InputRotary | 0 | 0 | 8 | 16 |
| Led | 1 | 4 | 0 | 0 |
| Buzzer/AudioBox | 1 | 4 | 0 | 0 |
| Display7/14/16 | 7/14/16 | 7/14/16 | 0 | 0 |
| Node | 1 | 1 | 1 | 1 |
| Mux | 3 | 11 | 1 | 1 |
| Demux | 2 | 4 | 2 | 8 |
| TruthTable | 1 | 8 | 1 | 16 |
| Line/Text | 0 | 0 | 0 | 0 |

---

## Quick Reference Lookup

### By Size

**Smallest (32×32):** Node
**Standard (64×64):** Most logic gates, all inputs, flip-flops, latches, displays, Mux, Demux
**Larger (68.27×68.27):** Buzzer, AudioBox
**Compact (34.79×30.16):** IC (panda2)

### By Port Count

**0 inputs, 1 output:** Clock, InputButton, InputSwitch, InputVcc, InputGnd
**1 input, 1 output:** Not, Node
**2 inputs, 1 output:** AND/OR gates (minimum), Mux (minimum)
**Fixed Multi-Port:** D Flip-Flop (2→2), JK Flip-Flop (3→2)
**Input Only:** LED, Buzzer, AudioBox, Displays

---

## ⚠️ Important: Port Ordering & Test Updates

**When updating a generator with proper spacing, tests may need refactoring.**

The IC class sorts input/output ports by element position (Y coordinate first, then X). This means:

- Tests using hardcoded port indices **will break** if element positions change
- **Solution:** Use label-based connections instead of hardcoded indices

**Key Change Pattern:**

```cpp
// Before (breaks with position changes)
builder.connect(data[i], 0, mux, i);

// After (position-independent)
builder.connect(data[i], 0, mux, QString("data%1").arg(i));
```

---

## ✅ Implementation Checklist

Before submitting your IC generator:

- [ ] All hardcoded offsets replaced with constants
- [ ] Imports include `element_spacing` module
- [ ] No two elements closer than 96px center-to-center
- [ ] Visual layout makes logical sense
- [ ] Port labels don't overlap elements
- [ ] Connections don't cross unnecessarily
- [ ] Grid layouts use `calculate_grid_positions()`
- [ ] Position variables declared at method start
- [ ] Clear comments explaining layout strategy
- [ ] Python syntax valid and indentation correct

---

## 📖 Related Documentation

- `test/integration/ic/generators/element_spacing.py` - Spacing constants and helper functions
- `test/integration/ic/generators/run_all_generators.py` - Script to run all IC generators
- `element_spacing.py` - Comprehensive reference implementation

---

**Last Updated:** 2026-02-03
**Document Version:** 2.0 (Consolidated)
**Target Audience:** MCP Users, IC Generator Developers, Circuit Designers
**Status:** Complete Reference with Implementation Guides
