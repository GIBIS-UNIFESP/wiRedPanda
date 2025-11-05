# Phase 7: Proposed Deep Edge Case Testing

**Purpose**: Build on Phase 6 (8 passing tests) with systematic testing of edge cases identified in ultrathink analysis
**Scope**: Focus on connections, deletions, and morphs with complex scenarios
**Target**: 40+ new test cases across 4 tiers
**Status**: Ready for implementation

---

## Overview

Phase 6 validated the undo/redo system with 8 basic connected element scenarios (100% passing). Phase 7 would expand to test the edge cases identified in the comprehensive ultrathink analysis, targeting potential vulnerabilities in:

1. **Connection Management**: Orphaning, port indexing, topology preservation
2. **Deletion Cascades**: Fan-out/fan-in deletion, complex topologies
3. **Morph State Transfer**: Port remapping, connection preservation
4. **Interaction Patterns**: Complex sequences of operations

---

## Phase 7.1: Connection Edge Cases (Tier 1 - Critical)

### Test 7.1.1: Dangling Connection - Source Deletion
```python
def test_dangling_connection_source_delete():
    """
    Test Case 1.1.1: Delete source element, verify connection orphaned connection removed

    Sequence:
      A (id:1) → B (id:2)
      Delete A
      Verify connection is removed
      Verify B still exists
      Undo deletion
      Verify connection restored

    Expected: 0 connections after A deleted, 1 connection after undo
    """
```

### Test 7.1.2: Dangling Connection - Sink Deletion
```python
def test_dangling_connection_sink_delete():
    """
    Test Case 1.1.2: Delete sink element, verify source disconnected

    Sequence:
      A → B
      Delete B
      Verify A still exists
      Verify A.out(0) status is "unconnected"
      Verify connection removed
      Undo B deletion
      Verify connection restored

    Expected: A disconnected, then A-B connected again after undo
    """
```

### Test 7.1.3: Self-Loop Connection (Circular)
```python
def test_self_loop_connection():
    """
    Test Case 1.1.3: Element connected to itself

    Sequence:
      A at (100, 100)
      Connect A.out(0) → A.in(0)  [Self-loop]
      Simulate and verify stable state
      Delete A
      Verify connection removed with element
      Undo deletion
      Verify self-loop restored

    Expected: Self-loop handled without deadlock/race condition
    """
```

### Test 7.1.4: Port Bound Checking
```python
def test_port_index_out_of_bounds():
    """
    Test Case 1.2.1: Morph changes port count, old connections invalid

    Sequence:
      AND (2-in, 1-out) with A → in(0), B → in(1), out(0) → C
      Morph AND to NOT (1-in, 1-out)
      Verify in(1) connection removed
      Verify out(0) → C preserved
      Simulate: NOT gate works correctly
      Undo morph
      Verify all original connections restored (A→in(0), B→in(1), AND→C)

    Expected: Automatic connection cleanup when ports removed
    """
```

### Test 7.1.5: Multiple Drivers (Conflict Detection)
```python
def test_multiple_drivers_same_port():
    """
    Test Case 1.2.4: Two sources connect to same sink port

    Sequence:
      A.out(0) → B.in(0)
      Try C.out(0) → B.in(0)  [Same port!]
      Verify behavior: either reject or replace
      If replaced: verify old connection removed
      If rejected: verify A still connects to B
      Simulation should have clear single-driver semantics

    Expected: Consistent conflict handling (either replace or reject)
    """
```

---

## Phase 7.2: Deletion Edge Cases (Tier 1 - Critical)

### Test 7.2.1: Delete with Complex Fan-Out
```python
def test_delete_source_fan_out():
    """
    Test Case 2.2.1: Delete element with multiple outputs

    Sequence:
      A with 4 outputs connected to B, C, D, E
      Delete A
      Verify all 4 connections removed
      Verify B, C, D, E still exist but disconnected
      Verify list_connections empty
      Undo deletion
      Verify A restored with all 4 connections in correct configuration

    Expected: All connections deleted, all properly restored on undo
    """
```

### Test 7.2.2: Delete with Complex Fan-In
```python
def test_delete_sink_fan_in():
    """
    Test Case 2.2.2: Delete element with multiple inputs

    Sequence:
      A, B, C all connected to Z (fan-in)
      Delete Z
      Verify all 3 connections removed
      Verify A, B, C still exist but unconnected
      Undo deletion
      Verify Z restored with all 3 inputs correctly mapped

    Expected: Port-specific connections correctly restored
    """
```

### Test 7.2.3: Delete in Feedback Loop
```python
def test_delete_in_feedback_loop():
    """
    Test Case 2.2.3: Delete element in feedback loop (e.g., SR Latch)

    Sequence:
      NAND1 → NAND2.in (feedback)
      NAND2 → NAND1.in (feedback back)
      Delete NAND1
      Verify NAND2 still exists
      Verify both directions of feedback removed
      Simulate: NAND2 now unconnected
      Undo deletion
      Verify feedback loop completely restored and functional

    Expected: Feedback loop correctly broken and restored
    """
```

### Test 7.2.4: Delete in Deep Undo Stack
```python
def test_delete_with_deep_undo_stack():
    """
    Test Case 2.3.3: Delete after 100+ operations

    Sequence:
      Create 50 elements
      Perform 100+ operations (rotates, flips, morphs, connections)
      Delete element from middle
      Verify deletion succeeds
      Undo ALL 100+ operations
      Verify deleted element restored in correct order
      Verify all state matches pre-deletion

    Expected: Deep undo stack doesn't corrupt deletion/restoration
    """
```

### Test 7.2.5: Delete-Undo-Delete-Undo Pattern
```python
def test_delete_undo_delete_pattern():
    """
    Test Case 2.3.1: Delete same element twice with undo between

    Sequence:
      A → B
      Delete A (undo stack: [delete_A])
      Undo deletion (A restored)
      Delete A again (undo stack: [delete_A, delete_A])
      Verify second delete identical to first
      Undo both deletes
      Verify A restored identically

    Expected: Multiple delete/undo cycles work consistently
    """
```

---

## Phase 7.3: Morph Edge Cases (Tier 1 - Critical)

### Test 7.3.1: Morph with Connection Remapping
```python
def test_morph_with_port_remapping():
    """
    Test Case 3.1.1: Morph with complex port remapping

    Sequence:
      4-input AND with A→in(0), B→in(1), C→in(2), D→in(3), out→Z
      Morph to 2-input OR
      Verify C, D connections removed (ports don't exist)
      Verify A→in(0), B→in(1) preserved
      Verify out→Z preserved
      Simulate with new gate type
      Undo morph
      Verify 4-input AND restored with all 5 connections

    Expected: Smart connection management during morph
    """
```

### Test 7.3.2: Morph with No Port Compatibility
```python
def test_morph_incompatible_ports():
    """
    Test Case 3.1.2: Morph to completely different port layout

    Sequence:
      2-input AND with A→in(0), B→in(1)
      Morph to 4-to-1 Multiplexer (4 inputs, 2 select, 1 output)
      Verify connection handling: which connections preserved?
      If A→in(0) preserved: verify it makes semantic sense
      If all removed: verify clean slate for new connections
      Simulate with new topology
      Undo morph
      Verify AND restored with original connections

    Expected: Clear documented behavior for incompatible morphs
    """
```

### Test 7.3.3: Combinational to Sequential Morph
```python
def test_morph_combinational_to_sequential():
    """
    Test Case 3.3.1: Morph AND gate to D Flip-Flop

    Sequence:
      AND gate with data/clock inputs and output
      Morph to D Flip-Flop
      Verify clock input now has semantic meaning
      Verify data captured on clock edge (not immediately)
      Simulate: output only updates on edge
      Verify behavior different from AND gate
      Undo morph
      Verify AND operates combinationally again (immediate)

    Expected: Simulation mode switches correctly with morph
    """
```

### Test 7.3.4: Sequential Element State During Morph
```python
def test_morph_sequential_state_preservation():
    """
    Test Case 3.3.2: Morph flip-flop with stored value

    Sequence:
      D Flip-Flop with Q=1 (stored value)
      Simulate several cycles to stabilize state
      Morph D Flip-Flop to SR Latch
      Verify what happens to stored value:
        Option A: Value preserved (Q=1 in SR latch)
        Option B: State lost (SR latch in unknown state)
      Simulate new element
      Undo morph
      Verify D Flip-Flop restored with Q=1

    Expected: Behavior documented and consistent
    """
```

### Test 7.3.5: Connection Survival Through Multiple Morphs
```python
def test_connection_through_multiple_morphs():
    """
    Test Case 4.3.1: A → B → C chain, morph B multiple times

    Sequence:
      B = AND gate initially
      A → AND → C (chain)
      Morph AND to OR (same ports)
      Verify A → OR → C preserved
      Morph OR to NAND (same ports)
      Verify A → NAND → C preserved
      Morph NAND to NOR (same ports)
      Verify A → NOR → C preserved
      Simulate: signal flows through all versions
      Undo 3 times
      Verify AND restored with connections

    Expected: Connections stable through multiple same-port morphs
    """
```

---

## Phase 7.4: Interaction & Stress Edge Cases (Tier 2)

### Test 7.4.1: Delete After Morph Sequence
```python
def test_delete_after_morph():
    """
    Test Case 4.1.1: Delete element that was morphed

    Sequence:
      AND → B → C
      Morph AND to OR
      Delete OR (formerly AND)
      Verify current morphed state (OR) is deleted
      Verify B disconnected
      Undo deletion
      Verify OR restored with connections (not AND)

    Expected: Delete works on morphed state
    """
```

### Test 7.4.2: Morph After Delete/Undo
```python
def test_morph_after_delete_undo():
    """
    Test Case 4.2.1: Create-Delete-Undo-Morph sequence

    Sequence:
      AND → B → C
      Delete B (breaks chain)
      Undo deletion (B restored, chain intact)
      Morph AND to OR
      Verify connections preserved (OR → B → C)
      Undo morph
      Verify AND restored
      Undo undo-delete (back to broken state)

    Expected: Complex operation sequences handled correctly
    """
```

### Test 7.4.3: Large Fan-Out Stress Test
```python
def test_large_fan_out_100_connections():
    """
    Test Case 5.2.1: Maximum fan-out scenario

    Sequence:
      Create source element A
      Create 100 sink elements (B1-B100)
      Connect A to each (100 connections)
      Verify all 100 connections exist
      Simulate: signal propagates to all 100
      Delete A
      Verify all 100 connections removed
      Undo deletion
      Verify all 100 connections restored exactly

    Expected: System handles large fan-out without issues
    """
```

### Test 7.4.4: Deep Undo Stack Stress
```python
def test_deep_undo_stack_100_operations():
    """
    Test Case 5.2.2: Deep undo stack with mixed operations

    Sequence:
      Create 10 elements
      Perform 100 mixed operations:
        - 40 connection operations
        - 30 morph operations
        - 20 move operations
        - 10 delete operations
      Undo all 100 operations
      Verify system returns to initial state
      Redo all 100 operations
      Verify final state matches original

    Expected: No memory leaks, state consistent, acceptable performance
    """
```

### Test 7.4.5: Alternating Morphs and Connections
```python
def test_alternating_morphs_and_connections():
    """
    Test Case 5.1.1: Complex interleaved operations

    Sequence:
      Morph A
      Connect A → B
      Morph B
      Connect B → C
      Morph C
      Delete A
      Undo delete
      Morph D
      Connect D → E
      Delete E
      Undo delete
      Redo delete
      Verify all operations tracked correctly

    Expected: No undo/redo confusion
    """
```

---

## Phase 7.5: Corner Cases & Validation (Tier 3)

### Test 7.5.1: Circular Feedback Loop
```python
def test_circular_feedback_loop_a_b_c_a():
    """
    Test Case 1.1.4: Circular connection A → B → C → A

    Sequence:
      Create elements A, B, C
      Connect A → B → C → A (circular!)
      Simulate: verify no deadlock
      Delete A (breaks circle)
      Verify B and C disconnected from each other
      Undo deletion
      Verify circle restored

    Expected: Circular loops handled without deadlock
    """
```

### Test 7.5.2: Orphaned Port Reference Handling
```python
def test_orphaned_port_reference():
    """
    Test Case 1.2.3: Stale port reference after deletion

    Sequence:
      Get reference to B.in(0) while connected
      Delete B
      Try to access port reference
      System should handle gracefully (no crash)
      Delete source of B
      Try to query connection state
      All should be safe and well-defined

    Expected: Graceful error handling for orphaned references
    """
```

### Test 7.5.3: Maximum Port Count Morph
```python
def test_morph_maximum_port_count():
    """
    Test Case 3.1.3: Morph to element with maximum ports

    Sequence:
      Simple 2-input gate
      Morph to 32-input OR gate (if supported)
      Verify port count changed to 32
      Verify only 2 original connections preserved
      Connect to remaining 30 ports
      Simulate: 32-input OR works correctly
      Undo connection additions
      Verify back to 2 connections

    Expected: System handles large port counts
    """
```

### Test 7.5.4: Rapid Create-Delete-Create
```python
def test_rapid_create_delete_create():
    """
    Test Case 5.1.1: Rapid element lifecycle

    Sequence:
      Create A at (100, 100)
      Delete A
      Create new element at same position
      Verify new element gets different ID
      Undo creation
      Undo deletion
      Verify A restored with original ID

    Expected: Element IDs unique, lifecycle tracked correctly
    """
```

### Test 7.5.5: Mixed Directed and Undirected Semantics
```python
def test_mixed_connection_semantics():
    """
    Test Case 1.1.4: Connections with different types

    Sequence:
      A → B (directional signal)
      B ↔ C (bidirectional? if supported)
      Delete A (verify directional A→B removed)
      Verify C still connected to B
      Delete B (verify both connections removed)
      Undo deletions
      Verify semantics preserved

    Expected: Connection type semantics preserved
    """
```

---

## Implementation Timeline

### Week 1: Tier 1 - Critical (Tests 7.1.1 - 7.3.5)
- 15 critical edge case tests
- Focus on connection management and morph/delete interactions
- Expected: Some tests may fail, exposing bugs
- Estimated time: 4-6 hours

### Week 2: Tier 2 - Important (Tests 7.4.1 - 7.4.5)
- 5 interaction and stress tests
- After Tier 1 bugs fixed
- Expected: Should all pass (validates fixes)
- Estimated time: 2-3 hours

### Week 3: Tier 3 - Corner Cases (Tests 7.5.1 - 7.5.5)
- 5 corner case and validation tests
- After Tier 2 confirms robustness
- Expected: All pass (validates complete robustness)
- Estimated time: 2-3 hours

### Total: 22 new tests across 3 tiers

---

## Expected Outcomes

### Optimistic Scenario (All Tests Pass)
- System validates as extremely robust
- Confidence in production readiness significantly increases
- Document as "production-grade quality"

### Realistic Scenario (Some Tests Fail)
- Identify specific bugs in edge cases
- Systematic debugging and fixes
- Each fix validated by re-running tests
- Phase 7 completion with 100% pass rate

### What We'll Learn
1. Actual behavior of complex connection topologies
2. Robustness of undo/redo with deep stacks
3. Performance characteristics under stress
4. Any remaining corner cases or bugs
5. Confidence metrics for production deployment

---

## Conclusion

Phase 7 would provide comprehensive validation of the edge cases identified in the ultrathink analysis. Starting with 22 critical tests, we can systematically validate connection management, deletion cascades, morph operations, and complex interaction patterns.

Based on Phase 6 results (8/8 passing), Phase 7 should reveal any remaining edge case vulnerabilities and provide final confidence in production readiness.

