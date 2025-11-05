# MCP Test Expansion Plan - Edge Cases & Stress Testing

## Executive Summary
Now that all 6 element manipulation commands are available via MCP, tests need comprehensive edge case coverage to ensure robustness. This plan identifies critical edge cases across 3 dimensions: command semantics, undo/redo behavior, and circuit state integrity.

---

## 1. ROTATE_ELEMENT Edge Cases

### Semantic Edge Cases
- **Angle wrapping**: 360° should equal 0°, 450° should equal 90°
- **No-op rotation**: Rotating 0° should not change element
- **Inverse rotation**: Rotate 90°, then -90° (if allowed)
- **Sequential rotations**: 90° + 90° + 90° + 90° = 360° (should be original)
- **Invalid angles**: Negative values, >360°, non-integers, non-numeric

### Undo/Redo Chains
- Rotate 5 times, undo all 5, redo all 5
- Rotate, move, rotate, undo back to original position
- Rotate, then modify other properties (flip, update)
- Undo chain of 20 rotations
- Rotate after element creation vs on existing element

### Element Type Variations
- Rotate logic elements (AND, OR, NOT)
- Rotate IC instances
- Rotate elements with connections
- Rotate output elements (LED, Display)

---

## 2. FLIP_ELEMENT Edge Cases

### Semantic Edge Cases
- **Double flip**: Flip axis 0 twice = original, flip axis 1 twice = original
- **Combined flips**: Flip axis 0, then axis 1 (should create mirror effect)
- **Flip order matters**: Flip(0) then Flip(1) vs Flip(1) then Flip(0)
- **Invalid axes**: -1, 2, 3, non-numeric, null
- **No-op flip**: Symmetrical elements flipped should be detectable

### Undo/Redo Chains
- Flip, unflip (via undo), reflip (via redo)
- Flip on both axes in sequence
- Flip rotated elements
- Mixed flip and rotate operations
- Deep undo stacks (20+ flips)

### Connection Sensitivity
- Flip element with input connections
- Flip element with output connections
- Flip element with both input/output connected
- Flip when connection is in progress (should fail)

---

## 3. UPDATE_ELEMENT Edge Cases

### Property Validation
- **Empty update**: No properties specified (should be no-op)
- **Single property**: Update only label, only color, only frequency
- **Multiple properties**: Update label + color + frequency simultaneously
- **Conflicting properties**: Frequency on non-clock element
- **Invalid values**: Empty string label, invalid color, negative frequency

### Property Interactions
- Update frequency on AND gate (should be ignored/error)
- Update color on all element types
- Update label with special characters/unicode
- Update label with very long strings (1000+ chars)
- Update rotation property via update vs rotate command (consistency)

### Undo/Redo Chains
- Update 5 properties, undo all
- Update property repeatedly with different values
- Update after create in same undo chain
- Undo update, redo update, update again

---

## 4. CHANGE_INPUT_SIZE Edge Cases

### Boundary Cases
- **Size to 0**: Should fail or minimum to 1
- **Size to 1**: Minimum valid size
- **Size to 64**: Maximum port count (16 in/out each)
- **Size unchanged**: Setting size to current size
- **Very large sizes**: 100, 1000 (should fail or limit)
- **Negative sizes**: -1, -5 (should fail)
- **Non-integer sizes**: 2.5, 3.14 (should fail)

### Port Connection Impact
- Change size with no connections
- Change size with some ports connected
- Change size REDUCING from 5 to 2 (ports 3,4,5 deleted - what about connections?)
- Change size on element with ALL ports connected
- Change size then disconnect, verify state

### Undo/Redo with Connections
- Create connections, change size (should fail if ports deleted)
- Change size, undo, verify connections restored
- Change size, redo, verify connections still intact or properly handled

---

## 5. CHANGE_OUTPUT_SIZE Edge Cases

### Same as Input Size
- All boundary cases apply
- Connection deletion/preservation on reduce

### Element-Specific Cases
- Demux element (multi-output): increase/decrease outputs
- AND gate with variable outputs (if supported)
- Elements with fixed output counts

---

## 6. TOGGLE_TRUTH_TABLE_OUTPUT Edge Cases

### Position Validation
- **Position 0**: First output
- **Position max-1**: Last output
- **Position max**: Out of bounds (should fail)
- **Position -1**: Negative (should fail)
- **Non-integer positions**: 2.5 (should fail)

### Toggle State Tracking
- Toggle position, verify state changed
- Toggle same position 2x (should be original)
- Toggle multiple positions in sequence
- Toggle, undo, toggle again (state should be consistent)
- Toggle after resize (if max position changes)

### Truth Table Size Variations
- Small truth table (1 output)
- Large truth table (16+ outputs)
- Toggle on element that isn't a truth table (should fail)

---

## 7. COMBINED OPERATION EDGE CASES

### Multi-Step Scenarios
1. Create element → Rotate → Flip → Update label → Toggle output (if TruthTable)
2. Create element → Change input size → Connect → Change input size again (reduce)
3. Create two elements → Rotate first → Flip second → Change sizes → Connect → Undo all
4. Morph element (type change) → Then rotate/flip/update (should work on new type)

### State Consistency
- After rotate+flip, connections should still be valid
- After size change, port indices should map correctly
- After update, properties should reflect changes
- Circuit state should remain valid after all operations

---

## 8. UNDO/REDO DEEP CHAINS

### Long Chains
- **Chain length 10**: 10 operations undo/redo all
- **Chain length 50**: 50 operations (stress test)
- **Chain length 100**: 100 operations (memory/performance)
- **Alternating types**: Rotate, flip, update, size... repeated

### Undo Stack State
- Get undo stack after each operation
- Verify count increments correctly
- Verify descriptions are accurate
- Verify undo/redo buttons would be enabled/disabled correctly

### Redo Stack Behavior
- Perform operation, redo should be empty
- Undo, redo should have 1 item
- Undo multiple, redo should have multiple items
- New operation clears redo stack

---

## 9. CIRCUIT STATE INTEGRITY

### Multi-Element Scenarios
- 10 elements with different operation sequences
- Connections between rotated/flipped elements
- Undo/redo preserves all connections
- Move + rotate element maintains connection validity

### Tab/Circuit Isolation
- Circuit A: rotate element
- Circuit B: flip element
- Switch back to A: rotation should persist
- Close A, reopen: state should be same
- Undo in A shouldn't affect B

### Corruption Resistance
- Undo after external element deletion
- Undo after circuit state corruption
- Invalid command in middle of chain
- Network disconnection during undo

---

## 10. STRESS TESTING

### Performance
- 100 sequential rotations
- 50 rapid flip operations (stress undo stack)
- 100 size changes on same element
- Circuit with 1000 elements, rotate 10 each

### Memory
- Deep undo stack (1000+ operations)
- Verify memory doesn't leak
- Verify undo stack has reasonable limits

### Edge Cases Under Load
- All operations on single element
- All operations on different elements
- Operations on connected elements
- Operations during simulation

---

## 11. ERROR HANDLING & RECOVERY

### Invalid Operations
- Rotate non-existent element → should error
- Flip with invalid axis → should error
- Change size on deleted element → should error
- Toggle position out of range → should error
- Update with invalid property → should error

### Partial Failures
- Operation fails, undo stack should be clean
- Operation fails, subsequent operations should work
- Error message should be clear and actionable

### State Recovery
- After invalid operation, state should be unchanged
- After error, undo/redo should still work
- Circuit should be usable after error

---

## 12. TEST IMPLEMENTATION PRIORITIES

### Tier 1 (Critical)
- Basic operation + undo/redo for each command
- Invalid input rejection
- State consistency after operations
- Connection preservation with size changes

### Tier 2 (Important)
- Combined operations (multi-step)
- Deep undo chains (10+ operations)
- Boundary conditions (0, max values)
- Multi-element scenarios

### Tier 3 (Nice to Have)
- Performance stress tests (100+ operations)
- Large circuit tests (1000+ elements)
- Timing/concurrency edge cases
- Memory profiling

---

## 13. SPECIFIC TEST EXAMPLES

### Example 1: Size Change with Connections
```
1. Create AND gate (2 inputs, 1 output)
2. Create OR gate (2 inputs, 1 output)
3. Connect AND output to OR input 0
4. Change AND input size from 2 to 3
   - Should succeed, new input 2 available
   - Existing connection should persist
5. Undo size change
   - Input 2 should disappear
   - Connection should still be valid
6. Redo size change
   - Input 2 should reappear
   - Connection should still be valid
```

### Example 2: Rotation + Flip Consistency
```
1. Create AND gate
2. Rotate 90°
3. Flip axis 0 (horizontal)
4. Rotate 180°
5. Flip axis 1 (vertical)
6. Undo 5 times
   - Should be original state
7. Verify element properties match state before any operations
```

### Example 3: Truth Table Toggle
```
1. Create TruthTable element
2. Get initial state (which outputs are enabled)
3. Toggle position 0
4. Verify position 0 changed, others unchanged
5. Toggle position 0 again
6. Verify back to original state
7. Undo toggle at position 0
8. Verify state matches step 3
9. Redo toggle at position 0
10. Verify state matches original
```

### Example 4: Deep Undo Chain
```
1. Create element
2. Repeat 50 times: Rotate(90°) → Flip(0) → Update(label)
3. Undo all 150 operations (50 * 3)
4. Verify element is in original state
5. Redo all 150 operations
6. Verify element matches state after step 2
7. Verify no memory leaks/crashes
```

---

## 14. REGRESSION TEST REQUIREMENTS

### Must Pass
- All existing Phase 1-4 tests continue passing
- No performance degradation
- No memory leaks
- All error cases handled gracefully

### Should Pass
- New edge case tests all pass
- 100% code coverage for new commands
- No race conditions in undo/redo

---

## Implementation Roadmap

1. **Phase 5.1**: Basic edge case tests (Tier 1)
   - Invalid input rejection
   - Boundary conditions
   - Basic undo/redo chains

2. **Phase 5.2**: Combined operation tests (Tier 2)
   - Multi-step scenarios
   - State consistency
   - Connection preservation

3. **Phase 5.3**: Stress & performance tests (Tier 3)
   - Deep undo chains (50+)
   - Large circuits
   - Memory profiling

4. **Phase 5.4**: Error recovery tests
   - Invalid states
   - Partial failures
   - Circuit recovery

---

## Success Criteria

- [ ] All Tier 1 tests pass (100% coverage of critical paths)
- [ ] All Tier 2 tests pass (multi-step scenarios work correctly)
- [ ] All Tier 3 tests pass (no performance degradation)
- [ ] Error handling tests pass (graceful failure)
- [ ] All existing tests still pass (no regressions)
- [ ] No memory leaks detected
- [ ] Test execution time < 60 seconds total
