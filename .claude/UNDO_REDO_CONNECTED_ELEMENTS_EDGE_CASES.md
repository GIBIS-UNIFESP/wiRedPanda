# Undo/Redo System Edge Cases: Connected Elements Focus

## Executive Summary

The undo/redo system must be thoroughly tested with **connected elements** because connections create fundamentally different code paths and state management requirements. This document identifies critical edge cases that stress the undo/redo system's ability to preserve connection integrity across various operations.

**Critical Insight**: Operations on connected elements exercise different code paths than operations on isolated elements:
- Deletion with connections → must handle connection removal
- Morphing with connections → must validate port compatibility
- Moving with connections → must update connection geometry
- Rotating/flipping with connections → must handle port orientation changes
- Size changes with connections → must manage port index mapping

---

## 1. CONNECTION PRESERVATION EDGE CASES

### 1.1 Delete Element with Connections (Undo Restoration)

**Scenario**: Element in middle of chain has critical behavior
```
A (output) → B (input/output) → C (input)
```

**Test Cases**:
1. Delete B - what happens to A→B and B→C connections?
   - Undo delete B: Do A→B and B→C reconnect automatically?
   - Are connection IDs preserved or regenerated?
   - Is connection geometry rebuilt correctly?

2. Delete B, then redo: Connection state must match pre-delete
   - Are old connection objects reused or new ones created?
   - Port references must point to correct locations
   - Connection list must be consistent

3. Delete chain element, modify A or C, undo deletion
   - State must be consistent: A, B, C positions and B's connections
   - No orphaned connections

**Why It Matters**: Different code paths for:
- `DeleteCommand::undo()` - must recreate connections
- Connection rebuilding logic - may reuse or recreate
- Port reference validation

---

### 1.2 Move Element with Connections (Geometry Update)

**Scenario**: Connected element moves, connections must stretch
```
A at (100, 100) → B at (200, 100)
Move B to (300, 200)
Expected: Connection geometry updates, B's position changes
```

**Test Cases**:
1. Move connected element: does connection geometry update?
   - Is connection still valid after undo?
   - Is connection still valid after redo?

2. Move source element of connection:
   - Output port position changes
   - Connection visual might change
   - Undo: connection returns to original geometry

3. Move target element of connection:
   - Input port position changes
   - Undo: positions and connections both restored

4. Move multiple connected elements sequentially:
   - Undo all moves: topology and geometry match original
   - Redo all moves: geometry exactly reproduces

**Why It Matters**: MoveCommand may or may not update connection geometry in undo/redo
- Connection geometry cache vs recalculation
- Performance: do connections rebuild visually?
- Correctness: is geometry actually consistent?

---

### 1.3 Rotate Element with Connections (Port Orientation)

**Scenario**: Port positions/orientations change with rotation
```
AND gate at (100, 100) with inputs on left
Rotate 90°: inputs now on bottom
Connection to input may become visually invalid
```

**Test Cases**:
1. Rotate element with input connections:
   - Port positions change (0°, 90°, 180°, 270°)
   - Input port might move to different edge
   - Undo rotation: port returns to original position
   - Is connection still pointing to same port?

2. Rotate element with output connections:
   - Similar to above but for outputs
   - Multiple output connections to different targets

3. Rotate element 360° in undo/redo chain:
   - Should return to exact original state
   - All connections should be identical
   - Port orientation must be exactly same

4. Rotate, move, rotate sequence with connections:
   - Complex state: position + rotation + connection geometry
   - Undo all: must restore exact state

**Why It Matters**: RotateCommand with connections may:
- Update connection geometry (or not)
- Change port facing directions
- Affect visual connection routing
- Different code path vs non-connected rotation

---

### 1.4 Flip Element with Connections (Port Mirroring)

**Scenario**: Flip changes port positions across axis
```
AND gate: inputs [0, 1] on left
Flip horizontal: inputs [0, 1] might be on right or rearranged
Connections must track correct port indices
```

**Test Cases**:
1. Flip horizontal with input connections:
   - Port positions mirror across Y axis
   - Undo flip: ports return to original positions
   - Connections must point to same port indices

2. Flip vertical with output connections:
   - Port positions mirror across X axis
   - Multiple connections to different outputs

3. Flip both axes (H then V):
   - Ports may rotate/rearrange
   - Undo second flip: return to H-only state
   - Undo first flip: return to original

4. Double flip restoration with connections:
   - Flip axis 0, flip axis 0: should be original
   - Connections and geometry must be identical
   - No state corruption from double flip

**Why It Matters**: FlipCommand with connections may:
- Remap port indices
- Update connection point locations
- Change visual connection routing
- Different code path vs flip without connections

---

### 1.5 Morph Element with Connections (Port Compatibility)

**Scenario**: Element type change affects port count/layout
```
AND gate (2 inputs, 1 output) with connections on both
Morph to NOT gate (1 input, 1 output)
What happens to the extra connection to input[1]?
```

**Test Cases**:
1. Morph to fewer inputs (AND→NOT):
   - AND has 2 inputs, both connected
   - Morph to NOT (1 input)
   - What happens to connection on input[1]? (no longer exists)
   - Undo morph: connections should be restored

2. Morph to more inputs (NOT→AND):
   - NOT has 1 input
   - Morph to AND (2 inputs)
   - New input[1] is unconnected
   - Undo morph: return to 1 input

3. Morph with output connections:
   - Element has multiple outputs connected to different targets
   - Morph to element with fewer outputs
   - Undo: all connections restored

4. Morph in chain scenario:
   - Element in middle of chain: A→B→C
   - Morph B to incompatible type
   - Does A→B connection break or adapt?
   - Undo: B reverts with all connections

5. Morph sequence with undo:
   - Morph A→B→A: should return exactly
   - Connections must be preserved and restored

**Why It Matters**: MorphCommand with connections is complex:
- Port compatibility validation
- Connection breaking/preservation logic
- Different code paths for compatible vs incompatible morphs
- Error handling if morph would break connections

---

## 2. CONNECTION STATE CORRUPTION EDGE CASES

### 2.1 Orphaned Connections (Dangling References)

**Scenario**: Connection exists but references are invalid
```
Connection: A.output[0] → B.input[0]
Delete A: what happens to connection?
- Connection still in circuit? (orphaned)
- B still references it? (dangling reference)
- Undo delete A: is connection valid again?
```

**Test Cases**:
1. Delete source element:
   - Connection orphaned: target still expects connection
   - Undo: connection valid again
   - Redo: connection orphaned again
   - System should handle gracefully (no crashes)

2. Delete target element:
   - Connection orphaned: source no longer outputs anywhere
   - Undo: connection valid again
   - System remains stable

3. Delete both endpoints separately:
   - Create A→B connection
   - Delete A, undo, delete B, undo
   - Must be able to restore exact state

4. Create→Delete→Undo→Redo complex chains:
   - Create A and B, connect
   - Delete A, delete B
   - Undo delete B, undo delete A
   - Redo delete A, redo delete B
   - Final state must be consistent

**Why It Matters**:
- Deletion is one of the most complex operations
- Connection cleanup must work correctly
- Undo must properly restore connection objects
- Different code paths for deletion with/without connections

---

### 2.2 Port Index Invalidation (After Size Change)

**Scenario**: Changing port count affects connection indices
```
AND gate has inputs [0, 1], output [0]
Connection A→AND.input[1]
Change AND input size to 1: input[1] no longer exists!
Undo: input[1] should reappear, connection valid
```

**Test Cases**:
1. Reduce input size with connection to removed port:
   - AND inputs [0,1], connection exists to input[1]
   - Change size to 1 (only input[0] exists)
   - Connection to input[1] is now invalid
   - Undo: input[1] restored, connection valid
   - Redo: input[1] removed, connection invalid again

2. Increase input size with existing connections:
   - AND inputs [0,1], both connected
   - Increase to 3 inputs
   - New input[2] is unconnected
   - Undo: return to 2 inputs, both still connected
   - Redo: 3 inputs again, new one unconnected

3. Complex sequence with multiple size changes:
   - 2 inputs → 3 inputs → 1 input → 4 inputs → back to 2
   - Undo entire chain: exact state restored
   - Connections remain valid throughout

4. Output size changes with connections:
   - Similar to input: output[0] connected to targets
   - Reduce output size: some targets lose input
   - Undo: outputs restored

**Why It Matters**:
- ChangeInputSizeCommand with connections is complex
- Port mapping/remapping during size changes
- Connection validation after size change
- Undo must precisely restore port count

---

### 2.3 Connection Between Deleted and Recreated Elements

**Scenario**: Undo/redo cycle with connections
```
Create A (output), Create B (input), Create connection A→B
Undo create connection: connection gone
Undo create B: B gone
Undo create A: A gone
Redo create A: A back, but no connection
Redo create B: B back
Redo connection: A→B recreated
```

**Test Cases**:
1. Create A, create B, connect A→B:
   - 3 undo steps to undo all
   - 3 redo steps to redo all
   - Final state matches pre-undo

2. Delete A, undo create A, connection should reappear:
   - Delete command removes A (and its connections)
   - Undo recreates A
   - Are connections automatically restored?
   - Or must connection redo be separate?

3. Create A and B connected, delete A, delete B, undo both:
   - Both elements should be recreated
   - Connection between them should be valid
   - Port references should be correct

**Why It Matters**:
- Element creation/deletion and connections are tightly coupled
- Undo/redo order matters
- Connection dependencies on element existence
- State consistency across multi-operation undo chains

---

## 3. COMPLEX UNDO/REDO SCENARIOS WITH CONNECTIONS

### 3.1 Linear Connection Chain (A→B→C→D)

**Scenario**: Test undo/redo with series of connected elements
```
Create A, Create B, Create C, Create D
Connect A→B, Connect B→C, Connect C→D
(4 creates + 3 connections = 7 operations)
Undo all 7: everything gone
Redo all 7: everything recreated
```

**Test Cases**:
1. Create chain step by step:
   - Each create and each connection adds to undo stack
   - Undo partial chain: must be consistent
   - Some connections gone while elements exist

2. Undo in middle of operations:
   - Create A, create B, connect A→B
   - Create C, undo create C
   - Connection A→B still valid
   - Redo create C: C appears again

3. Modify middle element:
   - Chain: A→B→C→D
   - Rotate B: affects connection geometry
   - Undo rotate: B back to original position
   - A→B and B→C connections still valid

4. Delete middle element:
   - Chain: A→B→C→D
   - Delete B: A→B and B→C both disappear
   - Undo: B back with both connections
   - Redo: B deleted again

**Why It Matters**:
- Multi-element undo chains are complex
- Connections between operations must be tracked
- State consistency across many undo steps
- Performance with deep undo stacks

---

### 3.2 Branching Connection Network

**Scenario**: One element connected to multiple targets
```
AND gate outputs to [OR1, OR2, NOT]
Delete AND: all 3 connections gone
Undo: AND back, all 3 connections restored
```

**Test Cases**:
1. One source, multiple targets:
   - A outputs to [B, C, D, E]
   - Delete A: 4 connections gone
   - Undo: A back with all 4 connections valid

2. Multiple sources, one target:
   - [A, B, C, D] all input to E
   - Delete E: 4 connections gone
   - Undo: E back, all connections valid

3. Modify element with multiple connections:
   - AND outputs to [OR1, OR2, OR3]
   - Morph AND to NOT (1 output)
   - What happens to the 3 connections?
   - Undo: AND with 3 outputs restored

4. Complex branching network:
   - 5 source elements, 5 target elements, 15 connections
   - Delete random element: some connections gone
   - Undo: exact state restored

**Why It Matters**:
- Real circuits have complex topologies
- Multiple connections per element must be tracked
- Undo must handle many affected connections
- State consistency with high connection density

---

### 3.3 Alternating Undo/Redo Pattern

**Scenario**: User makes decisions about operations
```
Operation 1: Create A
Operation 2: Create B
Operation 3: Connect A→B
Undo (back to step 2)
Redo (forward to step 3)
Operation 4: Create C
Undo (back to step 3)
Operation 4b: Connect A→C (different path)
```

**Test Cases**:
1. Undo one operation, redo it, create new operation:
   - Redo stack should be cleared
   - New operation added to undo stack
   - Previous redo path is lost

2. Complex alternation with connections:
   - Create A, B, connect A→B
   - Undo connection: A and B still exist
   - Undo B: A still exists
   - Redo B: B back, but no connection (connection undo came after)
   - Redo connection: A→B restored

3. Multiple undo/redo cycles:
   - Perform 10 operations with connections
   - Undo 5, redo 3, undo 2, redo 4, undo 1
   - Final undo stack state must be consistent
   - Next operation must work correctly

**Why It Matters**:
- Real user workflows with undo/redo
- Redo stack management is critical
- Connection state must be tracked through complex patterns
- No corruption even with unusual undo/redo sequences

---

## 4. STRESS TESTING: LARGE NETWORKS WITH CONNECTIONS

### 4.1 Large Connected Circuit (50+ Elements, 100+ Connections)

**Test Cases**:
1. Create 50 elements in grid:
   ```
   5×10 grid of AND/OR gates
   ```

2. Connect in pattern:
   ```
   Row 0 elements → Row 1 elements (50 connections)
   Row 1 elements → Row 2 elements (50 connections)
   ...
   Total: ~100+ connections across 5 rows
   ```

3. Undo entire circuit creation:
   - 50 element creates + 100 connections = 150 operations
   - Undo all 150: circuit completely gone
   - Redo all 150: exact circuit recreated

4. Partial undo in large circuit:
   - Undo 50 operations: 50 elements + 50 connections, part of circuit visible
   - Undo 100: only 25 elements + 25 connections
   - Undo 150: completely empty
   - Redo through: circuit reconstructs correctly

5. Modify in large circuit:
   - Large circuit with 100+ connections
   - Rotate one element in middle: geometry updates
   - Move element: connections stretch
   - Delete element: cascading connections removed
   - Undo: complex state restored

**Why It Matters**:
- Performance with many connections in undo stack
- Memory efficiency with deep undo stacks
- Correctness at scale
- No performance degradation with large circuits

---

### 4.2 Deep Undo Stack (100+ Operations with Connections)

**Scenario**: User creates complex circuit then explores undo

**Test Cases**:
1. 100 operations including many connections:
   - 50 creates, 50 connections
   - Undo all: takes reasonable time
   - Redo all: recreates exactly

2. Undo/redo performance:
   - 100+ operations should undo/redo in reasonable time
   - No exponential slowdown
   - Memory usage should be reasonable

3. Partial undo in deep chain:
   - Undo to step 50: circuit in intermediate state
   - Undo to step 25: more intermediate state
   - Forward to step 75: different intermediate state
   - All states must be consistent and correct

**Why It Matters**:
- User experience: undo should be responsive
- Memory: undo stack shouldn't explode
- Correctness: large undo stacks are harder to test

---

## 5. ERROR HANDLING & EDGE CASES

### 5.1 Invalid Connection During Undo/Redo

**Scenario**: Connection becomes invalid during undo/redo

**Test Cases**:
1. Undo creates element with dangling connection:
   - A→B connection exists
   - Undo B creation: B gone, connection orphaned
   - System should handle gracefully (no crash)
   - Redo B: connection valid again

2. Morph creates incompatible ports:
   - AND (2 inputs) connected to OR
   - Morph AND to NOT (1 input)
   - Connection to input[1] becomes invalid
   - Undo: should gracefully handle or restore

3. Size change creates port index mismatch:
   - Reduce input count below connection index
   - Undo: port recreated, connection valid again
   - Redo: port removed, connection invalid again

**Why It Matters**:
- System robustness: should never crash
- Error handling is critical
- User needs to understand what happened

---

### 5.2 Connection Validation Across Undo/Redo

**Scenario**: Can the system verify connection consistency?

**Test Cases**:
1. Before/after comparison:
   - After every undo/redo, verify all connections are valid
   - Connection source element and port exist
   - Connection target element and port exist
   - Port indices are within bounds

2. Connection topology verification:
   - All connections in element's input list are actually connected
   - All connections in element's output list are actually connected
   - No orphaned connections

3. Port reference validation:
   - Each connection points to correct ports
   - No stale port references from before operation

**Why It Matters**:
- Corruption detection
- Test reliability: can verify state is correct
- Debugging: can identify when state diverges

---

## 6. TEST IMPLEMENTATION STRATEGY

### Priority 1: Critical Connection Edge Cases
1. Delete element with connections → undo restoration
2. Move/rotate/flip element with connections
3. Morph element with connection compatibility
4. Large circuit with 100+ connections

### Priority 2: Complex Undo/Redo Patterns
1. Linear connection chain (A→B→C→D)
2. Branching networks
3. Alternating undo/redo with connections
4. Deep undo stacks (50+ operations)

### Priority 3: Stress & Edge Cases
1. Large circuits (50+ elements, 100+ connections)
2. Deep undo stacks (100+ operations)
3. Error handling and corruption detection
4. Performance under load with connections

---

## 7. TEST SCENARIOS - DETAILED EXAMPLES

### Example 1: Delete Element in Chain with Full Undo/Redo

```
SETUP:
1. Create AND gate A at (100, 100)
2. Create OR gate B at (200, 100)
3. Create NOT gate C at (300, 100)
4. Connect A→B (A.output[0] → B.input[0])
5. Connect B→C (B.output[0] → C.input[0])

State: A-[out]→ B-[in/out]→ C-[in]

TEST SEQUENCE:
1. Delete B
   Expected: A→B and B→C connections both removed
   Expected: B no longer in circuit
   Expected: A and C still exist

2. Undo delete B
   Expected: B recreated at (200, 100)
   Expected: A→B connection recreated
   Expected: B→C connection recreated
   Expected: Exact state matches pre-delete

3. Redo delete B
   Expected: B removed again
   Expected: Both connections removed
   Expected: Exact state matches step 1

4. Undo delete B (back to normal)
5. Move B to (250, 150)
   Expected: Connections update geometry
   Expected: A→B and B→C still valid

6. Undo move
   Expected: B back to (200, 100)
   Expected: Connections back to original geometry

7. Undo delete B command entirely
   Expected: Back to state before delete
   Expected: All connections valid
```

**Why This Matters**:
- Tests most complex operation (deletion with connections)
- Tests undo/redo of deletion
- Tests that subsequent moves work with restored connections
- Tests geometry update on move
- Tests undo of move

---

### Example 2: Morph with Connection Compatibility

```
SETUP:
1. Create AND gate (2 inputs, 1 output) at (100, 100)
2. Create OR gate at (200, 100)
3. Create NOT gate at (300, 100)
4. Connect OR→AND.input[0]
5. Connect OR→AND.input[1]
6. Connect AND→NOT

State: OR-[out]→ AND-[in0] AND-[in1] (2 connections)
       AND-[out]→ NOT-[in]

TEST SEQUENCE:
1. Morph AND to NOT (1 input, 1 output)
   Expected: OR→AND.input[0] remains
   Expected: OR→AND.input[1] is removed (no longer exists)
   Expected: AND→NOT connection remains
   Question: Does system error? Delete extra connection? Keep it orphaned?

2. Undo morph
   Expected: AND back to 2 inputs
   Expected: OR→AND.input[0] restored
   Expected: OR→AND.input[1] restored
   Expected: AND→NOT still valid
   Expected: Exact state matches pre-morph

3. Redo morph
   Expected: Same as step 1

4. Undo morph again
5. Morph AND to OR (2 inputs, 2 outputs instead of 1)
   Expected: Existing input connections valid
   Expected: New output[1] available but unconnected
   Expected: Original output[0]→NOT still valid

6. Redo back to step 1
   Expected: Should match step 1 state exactly
```

**Why This Matters**:
- Tests complex morph operation with multiple connections
- Tests connection breaking/preservation logic
- Tests port compatibility validation
- Tests redo of morph (different target type)
- Tests state consistency after morph

---

### Example 3: Large Network Undo/Redo

```
SETUP:
Create 3×3 grid of AND gates:
A00-[out]→ A01
A00-[out]→ A10
A01-[out]→ A11
A10-[out]→ A11
...
Total: 9 elements, ~12 connections

TEST SEQUENCE:
1. Create all 9 elements (9 create operations)
2. Create all ~12 connections (12 connect operations)
   Total: 21 operations in undo stack

3. Undo all 21 operations
   Expected: Circuit completely gone
   Expected: Takes reasonable time (<1s)

4. Redo all 21 operations
   Expected: Circuit recreated exactly
   Expected: All connections valid
   Expected: All positions correct

5. Undo 10 operations (partially reconstructed)
   Expected: ~5 elements, ~5 connections visible
   Expected: Partial circuit is consistent
   Expected: No orphaned connections

6. Redo 5 operations
   Expected: More of circuit appears

7. Undo 15 operations total
   Expected: Only 6 operations remain
   Expected: Only 3 elements + 3 connections

8. Check undo stack integrity
   Expected: get_undo_stack shows correct number
   Expected: get_undo_stack shows correct operations
```

**Why This Matters**:
- Tests performance with large undo stacks
- Tests correctness with partial undo
- Tests state consistency at different depths
- Tests undo/redo stack management
- Tests real-world circuit size

---

## 8. SUCCESS CRITERIA

### Tier 1 (Critical - Must Pass)
- [ ] Delete element with connections → undo restores exact state
- [ ] Move element with connections → geometry updates, undo restores
- [ ] Rotate element with connections → port positions change, undo restores
- [ ] Flip element with connections → port mirroring, undo restores
- [ ] Morph element with connections → undo restores original
- [ ] No orphaned connections after any undo/redo

### Tier 2 (Important - Should Pass)
- [ ] Linear chain (A→B→C→D) undo/redo works perfectly
- [ ] Branching network undo/redo works perfectly
- [ ] Alternating undo/redo patterns work correctly
- [ ] Deep undo chains (50+ operations) work correctly
- [ ] Connection topology verified after every operation

### Tier 3 (Nice to Have - Stress)
- [ ] Large circuit (50+ elements, 100+ connections) handles well
- [ ] Very deep undo stacks (100+ operations) responsive
- [ ] Performance consistent under high connection load
- [ ] Memory doesn't leak or explode

---

## 9. IMPLEMENTATION ROADMAP

### Phase A: Single Element Connected Operations
- test_delete_element_with_connections_undo
- test_move_element_with_connections_undo
- test_rotate_element_with_connections_undo
- test_flip_element_with_connections_undo
- test_morph_element_with_connections_undo

### Phase B: Complex Undo/Redo Patterns
- test_linear_chain_undo_redo
- test_branching_network_undo_redo
- test_alternating_undo_redo_pattern
- test_deep_undo_stack_with_connections

### Phase C: Stress & Validation
- test_large_circuit_100_elements_50_connections
- test_deep_undo_stack_100_operations
- test_connection_validation_after_operations
- test_performance_with_many_connections

---

## Conclusion

The undo/redo system with connected elements is significantly more complex than undo/redo of isolated elements. The critical tests focus on:

1. **Deletion with connections** - most complex operation
2. **Transformations on connected elements** - geometry consistency
3. **Port changes with connections** - index mapping, compatibility
4. **Complex undo/redo patterns** - user workflows
5. **Large circuits** - real-world scale and stress

Each test must verify not just that operations work, but that **undo/redo correctly preserves and restores the exact state**, including:
- Element positions and properties
- Connection topology (which elements connect to which)
- Connection geometry (if tracked)
- Port indices and references
- No orphaned or dangling references

This comprehensive testing will ensure the undo/redo system is reliable and robust for all real-world usage patterns.
