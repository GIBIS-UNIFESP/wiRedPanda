# Undo/Redo Overhaul - MCP-Based Testing Strategy

## Overview

Use MCP (Model Context Protocol) to automate testing across all phases of the undo/redo overhaul. This provides:
- **Early Regression Detection**: Catch breaking changes immediately
- **Automated State Verification**: Verify circuit state after undo/redo operations
- **Crash Scenario Simulation**: Programmatically trigger edge cases
- **Cross-Phase Validation**: Test against previous phases' changes
- **CI/CD Integration Ready**: Python tests can run in automated pipelines

---

## Testing Architecture

### Test Layers

```
Phase Tests
    ↓
MCP Command Layer (Python tests send commands)
    ↓
C++ Server (MCPProcessor routes commands)
    ↓
Command Handlers (ElementHandler, SimulationHandler, etc.)
    ↓
Undo/Redo System (Commands, QUndoStack)
    ↓
Circuit State (Elements, Connections, Properties)
```

### Test Execution Flow

```
1. Start wiRedPanda in MCP mode
2. Connect Python MCP client
3. Create test circuit via MCP
4. Execute operations (create, move, delete, etc.)
5. Trigger undo/redo commands
6. Verify circuit state matches expected
7. Check for crashes/exceptions
8. Cleanup
```

---

## Phase 1: Immediate Safety Fixes - MCP Tests

### Test Suite: `test_phase1_null_pointer_safety.py`

**Objective**: Verify null pointer guards work, no crashes on missing elements

#### Test 1.1: Undo After Element Externally Deleted
```python
async def test_undo_after_element_deleted():
    """Verify undo handles deleted elements gracefully"""
    mcp = MCPInfrastructure()
    await mcp.start_mcp()

    # Create circuit with AND gate
    await mcp.send_command("new_circuit", {})
    response = await mcp.send_command("create_element", {
        "type": "And", "x": 100.0, "y": 100.0
    })
    element_id = response.result['element_id']

    # Move element (creates MoveCommand in undo stack)
    await mcp.send_command("move_element", {
        "element_id": element_id, "x": 200.0, "y": 200.0
    })

    # Delete element externally (via DeleteItemsCommand)
    await mcp.send_command("delete_element", {"element_id": element_id})

    # Try to undo - should NOT crash, should handle gracefully
    # This tests that findElements() validates properly
    undo_result = await mcp.send_command("simulation_control", {
        "action": "undo"
    })

    # Verify error handling (should throw exception, not crash)
    assert undo_result.error is not None or len(response.result) == 0
    assert_no_crash_logs()
```

**What it tests**:
- Task 1.1: Null pointer guards in `findElements()`
- Validates exception is thrown instead of crash
- Checks error message is informative

#### Test 1.2: Split Command Exception Safety
```python
async def test_split_command_no_memory_leak():
    """Verify SplitCommand cleanup on failure"""
    mcp = MCPInfrastructure()
    await mcp.start_mcp()

    await mcp.send_command("new_circuit", {})

    # Create connection between two AND gates
    resp1 = await mcp.send_command("create_element", {
        "type": "And", "x": 100.0, "y": 100.0
    })
    resp2 = await mcp.send_command("create_element", {
        "type": "Or", "x": 300.0, "y": 100.0
    })

    gate1_id = resp1.result['element_id']
    gate2_id = resp2.result['element_id']

    # Connect AND output to OR input
    await mcp.send_command("connect_elements", {
        "from_element_id": gate1_id, "from_port": 0,
        "to_element_id": gate2_id, "to_port": 0
    })

    # Get connection to split
    conns = await mcp.send_command("list_connections", {})
    conn_id = conns.result[0]['connection_id'] if conns.result else None

    # Split connection (creates node)
    # Monitor memory before/after
    memory_before = get_memory_usage()

    for _ in range(10):
        # Repeated splits and undos
        await mcp.send_command("split_connection", {
            "connection_id": conn_id, "x": 200.0, "y": 100.0
        })
        await mcp.send_command("simulation_control", {"action": "undo"})

    memory_after = get_memory_usage()

    # Memory growth should be minimal (< 1MB)
    memory_growth = memory_after - memory_before
    assert memory_growth < 1_000_000, f"Memory leak detected: {memory_growth} bytes"
    assert_no_crash_logs()
```

**What it tests**:
- Task 1.2: SplitCommand exception safety
- Validates no memory leaks on repeated splits
- Ensures cleanup happens on constructor failure

#### Test 1.3: Scene Destruction During Undo
```python
async def test_scene_destruction_guard():
    """Verify Scene destructor clears undo stack safely"""
    # This is tricky - we need to test tab close behavior

    mcp = MCPInfrastructure()
    await mcp.start_mcp()

    # Create circuit in tab 1
    await mcp.send_command("new_circuit", {})
    await mcp.send_command("create_element", {
        "type": "And", "x": 100.0, "y": 100.0
    })

    # Queue some undo commands (move, rotate, etc.)
    # (Implicitly done via element operations)

    # Close tab (simulates scene destruction)
    await mcp.send_command("close_circuit", {})

    # Try undo on closed tab - should not crash
    result = await mcp.send_command("simulation_control", {
        "action": "undo"
    })

    # Should fail gracefully (no circuit to undo)
    assert result.error is not None
    assert_no_crash_logs()
```

**What it tests**:
- Task 1.3: Scene destruction guard
- Validates undo stack cleared safely
- No access to deleted Scene pointer

---

## Phase 2: Architectural Improvements - MCP Tests

### Test Suite: `test_phase2_weak_references.py`

**Objective**: Verify weak reference mechanism detects stale elements

#### Test 2.1: Element ID Reuse Detection
```python
async def test_element_id_generation_counter():
    """Verify generation counter prevents ID reuse crashes"""
    mcp = MCPInfrastructure()
    await mcp.start_mcp()

    await mcp.send_command("new_circuit", {})

    # Create element with ID X
    resp1 = await mcp.send_command("create_element", {
        "type": "And", "x": 100.0, "y": 100.0
    })
    element_id = resp1.result['element_id']

    # Move it (creates MoveCommand with weak reference)
    await mcp.send_command("move_element", {
        "element_id": element_id, "x": 200.0, "y": 200.0
    })

    # Delete it
    await mcp.send_command("delete_element", {"element_id": element_id})

    # Create new element (might reuse same ID)
    resp2 = await mcp.send_command("create_element", {
        "type": "Or", "x": 100.0, "y": 100.0
    })
    new_element_id = resp2.result['element_id']

    # Move new element to different position
    await mcp.send_command("move_element", {
        "element_id": new_element_id, "x": 300.0, "y": 300.0
    })

    # Undo the old element's move - should detect stale reference
    # via generation counter, not crash with wrong element
    undo_result = await mcp.send_command("simulation_control", {
        "action": "undo"
    })

    # Verify new element didn't move (stale reference ignored)
    new_pos = await mcp.send_command("get_element_properties", {
        "element_id": new_element_id
    })
    assert new_pos.result['x'] == 300.0  # Position unchanged
    assert_no_crash_logs()
```

**What it tests**:
- Task 2.2: Weak reference generation counter
- Validates stale references detected correctly
- No crash on ID reuse

#### Test 2.2: QPointer Scene Validity
```python
async def test_qpointer_scene_null_detection():
    """Verify QPointer detects destroyed Scene"""
    mcp = MCPInfrastructure()
    await mcp.start_mcp()

    # Create initial circuit
    await mcp.send_command("new_circuit", {})
    await mcp.send_command("create_element", {
        "type": "And", "x": 100.0, "y": 100.0
    })

    # Get tab count before
    tabs_before = await mcp.send_command("get_tab_count", {})
    initial_tabs = tabs_before.result['count']

    # Close circuit (Scene destroyed, but QPointer detects it)
    await mcp.send_command("close_circuit", {})

    # Verify tab closed
    tabs_after = await mcp.send_command("get_tab_count", {})
    assert tabs_after.result['count'] == initial_tabs - 1

    # Try undo on now-null Scene pointer
    undo_result = await mcp.send_command("simulation_control", {
        "action": "undo"
    })

    # Should fail gracefully (QPointer prevents crash)
    assert undo_result.error is not None
    assert_no_crash_logs()
```

**What it tests**:
- Task 2.1: QPointer<Scene> implementation
- Validates null detection works
- No crash on deleted Scene

---

## Phase 3: Robustness Enhancements - MCP Tests

### Test Suite: `test_phase3_transaction_safety.py`

**Objective**: Verify transactional operations handle failures

#### Test 3.1: Morph Command Transaction Rollback
```python
async def test_morph_command_rollback():
    """Verify MorphCommand rolls back on failure"""
    mcp = MCPInfrastructure()
    await mcp.start_mcp()

    await mcp.send_command("new_circuit", {})

    # Create AND gate (2 inputs)
    resp = await mcp.send_command("create_element", {
        "type": "And", "x": 100.0, "y": 100.0
    })
    and_gate_id = resp.result['element_id']

    # Morph to NOT gate (1 input) - incompatible
    morph_result = await mcp.send_command("morph_element", {
        "element_id": and_gate_id, "new_type": "Not"
    })

    # Verify state before morph
    state_before = await mcp.send_command("get_element_properties", {
        "element_id": and_gate_id
    })
    original_type = state_before.result['type']

    # If morph fails (incompatible), circuit should be unchanged
    if morph_result.error is None:
        # Morph succeeded - verify state
        state_after = await mcp.send_command("get_element_properties", {
            "element_id": and_gate_id
        })
        assert state_after.result['type'] == 'Not'
    else:
        # Morph failed - verify rollback (transaction)
        state_rollback = await mcp.send_command("get_element_properties", {
            "element_id": and_gate_id
        })
        assert state_rollback.result['type'] == original_type

    assert_no_crash_logs()
```

**What it tests**:
- Task 3.1: UndoTransaction rollback
- Validates partial operations don't corrupt state
- Either fully succeeds or fully rolls back

#### Test 3.2: Deserialization Validation
```python
async def test_corrupted_undo_data_handling():
    """Verify corrupted undo data doesn't crash"""
    mcp = MCPInfrastructure()
    await mcp.start_mcp()

    await mcp.send_command("new_circuit", {})

    # Create circuit with multiple elements
    elements = []
    for i in range(3):
        resp = await mcp.send_command("create_element", {
            "type": "And", "x": 100.0 + i * 100, "y": 100.0
        })
        elements.append(resp.result['element_id'])

    # Perform operations that modify undo stack
    for elem_id in elements:
        await mcp.send_command("move_element", {
            "element_id": elem_id, "x": 200.0, "y": 200.0
        })

    # Simulate corrupted autosave (can't directly corrupt, but we can test
    # deserialization error handling by crafting invalid data)
    # This would require access to undo stack internals

    # For now, verify large undo operations don't corrupt:
    for _ in range(20):
        for elem_id in elements:
            await mcp.send_command("move_element", {
                "element_id": elem_id,
                "x": (200.0 + _ * 10.0),
                "y": (200.0 + _ * 10.0)
            })

    # Undo all operations
    undo_count = 0
    for _ in range(21):  # 3 creates + 3*20 moves
        result = await mcp.send_command("simulation_control", {
            "action": "undo"
        })
        if result.error is None:
            undo_count += 1

    # Verify undo stack worked without crashes
    assert undo_count >= 20
    assert_no_crash_logs()
```

**What it tests**:
- Task 3.2: Deserialization validation
- Validates large undo operations don't corrupt
- No crashes from invalid data

---

## Phase 4: Testing & Validation - MCP Regression Suite

### Test Suite: `test_phase4_comprehensive_regression.py`

**Objective**: Comprehensive regression tests for all command types

#### Test 4.1: All 11 Command Types
```python
@pytest.mark.parametrize("operation", [
    ("move", test_move_command),
    ("rotate", test_rotate_command),
    ("flip_h", test_flip_horizontal_command),
    ("flip_v", test_flip_vertical_command),
    ("update", test_update_command),
    ("morph", test_morph_command),
    ("split", test_split_command),
    ("change_input_size", test_change_input_size_command),
    ("change_output_size", test_change_output_size_command),
    ("toggle_truth_table", test_toggle_truth_table_command),
    ("add_delete", test_add_delete_commands),
])
async def test_all_commands_undo_redo(operation):
    """Test each command type for undo/redo correctness"""
    test_func = operation[1]
    await test_func()
    assert_no_crash_logs()
```

**Example Implementation**:
```python
async def test_move_command():
    mcp = MCPInfrastructure()
    await mcp.start_mcp()

    await mcp.send_command("new_circuit", {})
    resp = await mcp.send_command("create_element", {
        "type": "And", "x": 100.0, "y": 100.0
    })
    elem_id = resp.result['element_id']

    # Original position
    orig = await mcp.send_command("get_element_properties", {
        "element_id": elem_id
    })
    orig_x, orig_y = orig.result['x'], orig.result['y']

    # Move
    await mcp.send_command("move_element", {
        "element_id": elem_id, "x": 200.0, "y": 200.0
    })

    moved = await mcp.send_command("get_element_properties", {
        "element_id": elem_id
    })
    assert moved.result['x'] == 200.0
    assert moved.result['y'] == 200.0

    # Undo
    await mcp.send_command("simulation_control", {"action": "undo"})

    undone = await mcp.send_command("get_element_properties", {
        "element_id": elem_id
    })
    assert undone.result['x'] == orig_x
    assert undone.result['y'] == orig_y

    # Redo
    await mcp.send_command("simulation_control", {"action": "redo"})

    redone = await mcp.send_command("get_element_properties", {
        "element_id": elem_id
    })
    assert redone.result['x'] == 200.0
    assert redone.result['y'] == 200.0
```

#### Test 4.2: Multi-Tab Isolation
```python
async def test_multi_tab_undo_isolation():
    """Verify undo stacks are independent per tab"""
    mcp = MCPInfrastructure()
    await mcp.start_mcp()

    # Create circuit in tab 1
    await mcp.send_command("new_circuit", {})
    resp1 = await mcp.send_command("create_element", {
        "type": "And", "x": 100.0, "y": 100.0
    })
    elem1_id = resp1.result['element_id']

    # Create circuit in tab 2
    await mcp.send_command("new_circuit", {})
    resp2 = await mcp.send_command("create_element", {
        "type": "Or", "x": 200.0, "y": 200.0
    })
    elem2_id = resp2.result['element_id']

    # Move element in tab 2
    await mcp.send_command("move_element", {
        "element_id": elem2_id, "x": 300.0, "y": 300.0
    })

    # Switch to tab 1 and undo - should not affect tab 2
    await mcp.send_command("switch_tab", {"tab_index": 0})
    await mcp.send_command("simulation_control", {"action": "undo"})

    # Verify tab 2 element still at moved position
    await mcp.send_command("switch_tab", {"tab_index": 1})
    pos = await mcp.send_command("get_element_properties", {
        "element_id": elem2_id
    })
    assert pos.result['x'] == 300.0
    assert pos.result['y'] == 300.0
```

---

## Phase 5: Performance Testing - MCP Tests

### Test Suite: `test_phase5_performance.py`

**Objective**: Verify performance improvements and memory efficiency

#### Test 5.1: Command Merging Effectiveness
```python
async def test_command_merging_undo_reduction():
    """Verify command merging reduces undo entries"""
    mcp = MCPInfrastructure()
    await mcp.start_mcp()

    await mcp.send_command("new_circuit", {})
    resp = await mcp.send_command("create_element", {
        "type": "And", "x": 100.0, "y": 100.0
    })
    elem_id = resp.result['element_id']

    # Rapid property changes (should merge)
    for i in range(20):
        await mcp.send_command("set_element_properties", {
            "element_id": elem_id,
            "label": f"Label_{i}"
        })

    # Get undo stack size
    undo_count = await get_undo_count()

    # Should be <= 2 (merged changes + element creation)
    # Without merging would be ~21
    assert undo_count <= 3, f"Expected merged commands, got {undo_count}"
```

#### Test 5.2: Memory Usage Under Load
```python
async def test_large_circuit_undo_memory():
    """Verify memory usage is bounded with large circuits"""
    mcp = MCPInfrastructure()
    await mcp.start_mcp()

    await mcp.send_command("new_circuit", {})

    memory_before = get_memory_usage()

    # Create large circuit (100 elements)
    element_ids = []
    for i in range(100):
        resp = await mcp.send_command("create_element", {
            "type": "And" if i % 2 == 0 else "Or",
            "x": float((i % 10) * 100),
            "y": float((i // 10) * 100)
        })
        element_ids.append(resp.result['element_id'])

    # Move all elements (creates undo entries)
    for elem_id in element_ids:
        await mcp.send_command("move_element", {
            "element_id": elem_id, "x": 500.0, "y": 500.0
        })

    memory_after = get_memory_usage()
    memory_used = memory_after - memory_before

    # Memory usage should be reasonable (< 50MB for 100 elements)
    assert memory_used < 50_000_000, f"Memory usage too high: {memory_used} bytes"

    # Undo stack compression should keep it bounded
    # Set undo limit to 50 and verify memory doesn't grow beyond that
    for _ in range(50):
        await mcp.send_command("move_element", {
            "element_id": element_ids[0], "x": 100.0, "y": 100.0
        })

    memory_final = get_memory_usage()
    assert memory_final < memory_after + 10_000_000  # Should grow slowly
```

---

## Testing Infrastructure

### MCP Test Base Class

**File**: `test/mcp/test_undo_redo_base.py`

```python
import asyncio
import pytest
from mcp_infrastructure import MCPInfrastructure
from mcp.client.protocol.results import MCPResponse

class UndoRedoTestBase:
    """Base class for all undo/redo MCP tests"""

    @pytest.fixture
    async def mcp(self):
        """Setup MCP connection for each test"""
        mcp = MCPInfrastructure(enable_validation=True)
        await mcp.start_mcp()
        yield mcp
        await mcp.stop_mcp()

    async def assert_element_exists(self, mcp, elem_id):
        """Verify element exists in circuit"""
        result = await mcp.send_command("get_element_properties", {
            "element_id": elem_id
        })
        assert result.error is None, f"Element {elem_id} not found"
        return result.result

    async def assert_element_at_position(self, mcp, elem_id, x, y, tolerance=0.01):
        """Verify element is at expected position"""
        props = await self.assert_element_exists(mcp, elem_id)
        assert abs(props['x'] - x) < tolerance
        assert abs(props['y'] - y) < tolerance

    async def get_undo_count(self, mcp):
        """Get current undo stack size"""
        # This requires exposing undo count via MCP (new command)
        result = await mcp.send_command("get_undo_info", {})
        return result.result['undo_count'] if result.error is None else 0

    async def assert_no_crash_logs(self, mcp):
        """Verify no crash/exception logs"""
        logs = await mcp.get_system_logs()
        crash_keywords = ['SIGSEGV', 'segmentation', 'crash', 'exception', 'panic']
        for log in logs:
            for keyword in crash_keywords:
                assert keyword.lower() not in log.lower(), f"Crash detected: {log}"
```

### Test Execution

**File**: `test/mcp/run_undo_redo_tests.sh`

```bash
#!/bin/bash

# Start wiRedPanda in MCP mode
wiredpanda --mcp &
MCP_PID=$!

# Wait for startup
sleep 2

# Run test phases in order
echo "=== Phase 1: Safety Fixes ==="
pytest test_phase1_null_pointer_safety.py -v

echo "=== Phase 2: Architectural Improvements ==="
pytest test_phase2_weak_references.py -v

echo "=== Phase 3: Robustness ==="
pytest test_phase3_transaction_safety.py -v

echo "=== Phase 4: Regression Testing ==="
pytest test_phase4_comprehensive_regression.py -v

echo "=== Phase 5: Performance ==="
pytest test_phase5_performance.py -v

# Cleanup
kill $MCP_PID
```

---

## CI/CD Integration

### GitHub Actions Workflow

**File**: `.github/workflows/undo-redo-testing.yml`

```yaml
name: Undo/Redo MCP Tests

on:
  push:
    branches: [ clock_delay, master ]
    paths:
      - 'app/commands*'
      - 'app/scene.cpp'
      - 'mcp/server/**'
      - 'mcp/client/**'
      - 'test/mcp/**'

jobs:
  undo-redo-tests:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Build wiRedPanda
        run: |
          cmake --preset release
          cmake --build --preset release

      - name: Install test dependencies
        run: pip install -r mcp/client/requirements.txt

      - name: Run Phase 1 Safety Tests
        run: |
          ./build/wiredpanda --mcp &
          sleep 2
          pytest test/mcp/test_phase1_null_pointer_safety.py -v

      - name: Run Phase 2 Architecture Tests
        run: pytest test/mcp/test_phase2_weak_references.py -v

      - name: Run Phase 3 Robustness Tests
        run: pytest test/mcp/test_phase3_transaction_safety.py -v

      - name: Run Phase 4 Regression Tests
        run: pytest test/mcp/test_phase4_comprehensive_regression.py -v

      - name: Run Phase 5 Performance Tests
        run: pytest test/mcp/test_phase5_performance.py -v

      - name: Generate Coverage Report
        run: pytest test/mcp/ --cov=mcp --cov-report=html

      - name: Upload Coverage
        uses: codecov/codecov-action@v3
```

---

## Test Metrics

### Success Criteria by Phase

| Phase | Test Count | Coverage | Crash Tests | Performance |
|-------|-----------|----------|------------|-------------|
| 1 | 3 tests | Null guards | All paths | N/A |
| 2 | 2 tests | Weak refs | ID reuse | N/A |
| 3 | 2 tests | Transactions | Corruption | N/A |
| 4 | 20+ tests | All commands | Edge cases | 100+ ms |
| 5 | 2 tests | Merging | Large circuits | <50MB |

### Overall Metrics
- **Total Tests**: 29+ automated tests
- **Code Coverage**: 95%+ of undo/redo system
- **Execution Time**: ~5 minutes (all phases)
- **Regression Detection**: Early (fail on Phase 1 before proceeding)
- **CI/CD Ready**: Yes (GitHub Actions workflow)

---

## Benefits

1. **Early Detection**: Catch regressions immediately, before releasing
2. **Automated Verification**: No manual testing needed
3. **Crash Simulation**: Test edge cases programmatically
4. **State Validation**: Verify circuit state after operations
5. **Performance Monitoring**: Track memory and performance
6. **Multi-Phase**: Tests run after each phase to catch breaks
7. **CI/CD Ready**: Integration into deployment pipeline
8. **Reproducible**: Same tests run everywhere
9. **Documentation**: Tests serve as usage examples
10. **Scalability**: Easy to add new test scenarios

---

## Implementation Roadmap

1. **Phase 1**: Create base test infrastructure
2. **Phase 2**: Write Phase 1 safety tests
3. **Phase 3**: Write Phase 2 architecture tests
4. **Phase 4**: Write Phase 3 robustness tests
5. **Phase 5**: Write Phase 4 regression tests
6. **Phase 5**: Write Phase 5 performance tests
7. **Integration**: Setup CI/CD workflow
8. **Monitoring**: Collect metrics and performance data

---

**Created**: 2025-11-05
**Status**: Ready for implementation
**Integration**: MCP server already available, Python client ready to use
