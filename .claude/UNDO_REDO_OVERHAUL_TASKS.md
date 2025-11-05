# Undo/Redo System Overhaul - Task Tracking

## Overview
Comprehensive overhaul of the undo/redo system to fix 6 critical/high vulnerabilities and improve robustness.

**Total Phases**: 5 | **Estimated Time**: 4-6 weeks | **Risk Level**: Medium (architectural changes)

---

## Phase 1: Immediate Safety Fixes (CRITICAL)
**Priority**: 🔴 URGENT | **Time**: 2-4 hours | **Status**: Not Started

### Task 1.1: Add Null Pointer Guards in findElements/findItems
- [ ] Update `findElements()` in `commands.cpp:116-132` to validate elements exist
- [ ] Update `findItems()` in `commands.cpp:98-114` to validate items exist
- [ ] Add try-catch wrappers in all command undo/redo methods
- [ ] Add qCWarning logging for missing elements
- **Files**: `app/commands.cpp`
- **Lines Changed**: ~30
- **Risk**: Low - adds validation, doesn't change logic

### Task 1.2: Fix SplitCommand Exception Safety
- [ ] Reorder SplitCommand constructor to validate before allocating
- [ ] Store existing IDs first, allocate temporary objects last
- [ ] Delete temporary Node and QNEConnection objects after storing IDs
- [ ] Add exception guards around allocation
- **Files**: `app/commands.cpp:423-462`
- **Lines Changed**: ~15
- **Risk**: Low - fixes memory leak

### Task 1.3: Add Scene Destruction Guard
- [ ] Add `m_undoStack.clear()` in Scene destructor
- [ ] Disconnect undo/redo signals from Scene
- [ ] Add null pointer checks for `m_scene` in all command redo/undo methods
- **Files**: `app/scene.cpp`, `app/commands.cpp`
- **Lines Changed**: ~10
- **Risk**: Low - prevents accessing deleted Scene

---

## Phase 2: Architectural Improvements (HIGH)
**Priority**: 🟠 HIGH | **Time**: 1-2 weeks | **Status**: Not Started

### Task 2.1: Replace Raw Scene Pointers with QPointer
- [ ] Add `#include <QPointer>` to `commands.h`
- [ ] Change all command classes: `Scene *m_scene` → `QPointer<Scene> m_scene`
- [ ] Update all 11 command classes (AddItems, DeleteItems, Move, Rotate, Flip, Update, Split, Morph, ChangeInputSize, ChangeOutputSize, ToggleTruthTable)
- [ ] Add null checks: `if (m_scene.isNull()) return;` in redo/undo
- **Files**: `app/commands.h`, `app/commands.cpp`
- **Lines Changed**: ~50
- **Risk**: Medium - architectural change, needs testing

### Task 2.2: Implement Weak Element References with Generation Counters
- [ ] Add `struct ItemEntry` to ElementFactory with item pointer + generation counter
- [ ] Create `ItemHandle` struct with id + generation
- [ ] Implement `ElementFactory::getHandle(ItemWithId*)` function
- [ ] Implement `ElementFactory::resolveHandle(ItemHandle)` function (returns nullptr if stale)
- [ ] Update command classes to store ItemHandle instead of bare IDs
- [ ] Add helper function `validateItemHandles()` for batch validation
- **Files**: `app/elementfactory.h`, `app/elementfactory.cpp`, `app/commands.h`, `app/commands.cpp`
- **Lines Changed**: ~80
- **Risk**: High - core architectural change, requires extensive testing

### Task 2.3: Add Command State Validation
- [ ] Implement `validateItemHandles()` helper in `commands.cpp`
- [ ] Add validation at start of every redo/undo method
- [ ] Log warnings for invalid handles with element IDs
- [ ] Throw PANDACEPTION with user-friendly message for stale references
- **Files**: `app/commands.cpp`
- **Lines Changed**: ~40
- **Risk**: Medium - adds guards, no logic changes

---

## Phase 3: Robustness Enhancements (MEDIUM)
**Priority**: 🟡 MEDIUM | **Time**: 1 week | **Status**: Not Started

### Task 3.1: Add Transactional Undo/Redo
- [ ] Create `UndoTransaction` RAII class in `commands.h`
- [ ] Implement rollback mechanism (snapshot-based or action tracking)
- [ ] Wrap all redo operations in UndoTransaction
- [ ] Test rollback on exceptions
- **Files**: `app/commands.h`, `app/commands.cpp`
- **Lines Changed**: ~60
- **Risk**: Medium - new abstraction, needs thorough testing

### Task 3.2: Add Deserialization Validation
- [ ] Add version compatibility checks in `loadItems()`
- [ ] Validate stream status after reads
- [ ] Validate deserialized items are non-null and correct type
- [ ] Add try-catch around deserialization paths
- [ ] Log detailed error messages for corruption detection
- **Files**: `app/commands.cpp:172-202`
- **Lines Changed**: ~30
- **Risk**: Low - adds validation only

### Task 3.3: Fix MorphCommand Safety
- [ ] Add UndoTransaction guard to `transferConnections()`
- [ ] Validate port compatibility before transferring
- [ ] Update ID before deleting old element (not after)
- [ ] Add try-catch for connection transfer failures
- [ ] Skip incompatible ports instead of crashing
- **Files**: `app/commands.cpp:580-641`
- **Lines Changed**: ~25
- **Risk**: Medium - modifies complex logic, needs testing

---

## Phase 4: Testing & Validation (HIGH)
**Priority**: 🟠 HIGH | **Time**: 1 week | **Status**: Not Started

### Task 4.1: Write Tests for All 11 Command Types
- [ ] Test MoveCommand (undo/redo)
- [ ] Test RotateCommand (undo/redo)
- [ ] Test FlipCommand (undo/redo, both axes)
- [ ] Test UpdateCommand (undo/redo)
- [ ] Test MorphCommand (undo/redo, type conversion)
- [ ] Test SplitCommand (undo/redo, connection creation)
- [ ] Test ChangeInputSizeCommand (undo/redo, connection removal)
- [ ] Test ChangeOutputSizeCommand (undo/redo, connection removal)
- [ ] Test ToggleTruthTableOutputCommand (undo/redo)
- **Files**: `test/testcommands.cpp`
- **Lines Added**: ~300
- **Risk**: Low - tests only, no changes to main code

### Task 4.2: Add Crash Scenario Tests
- [ ] Test undo after element deleted externally
- [ ] Test undo after scene destroyed
- [ ] Test morph with incompatible ports
- [ ] Test corrupted undo data deserialization
- [ ] Test exception recovery (partial state)
- **Files**: `test/testcommands_crash.cpp` (new)
- **Lines Added**: ~200
- **Risk**: Medium - edge cases, may need fixes in Phase 1-3

### Task 4.3: Add Multi-Tab Tests
- [ ] Test independent undo stacks per tab
- [ ] Test tab close with pending undo
- [ ] Test undo after switching tabs
- [ ] Test undo state isolation between tabs
- **Files**: `test/testcommands_multitab.cpp` (new)
- **Lines Added**: ~150
- **Risk**: Medium - integration testing

### Task 4.4: Run Full Test Suite
- [ ] Run `ctest --preset release` and fix any failures
- [ ] Run with AddressSanitizer for memory leak detection
- [ ] Run with ThreadSanitizer for race conditions
- [ ] Verify no new crashes in integration tests
- **Risk**: High - may discover additional issues

---

## Phase 5: Long-Term Optimizations (LOW)
**Priority**: 🟢 LOW | **Time**: 2-3 weeks | **Status**: Not Started

### Task 5.1: Implement Command Merging
- [ ] Add `id()` and `mergeWith()` to UpdateCommand
- [ ] Merge consecutive property changes on same elements
- [ ] Test merge functionality with rapid changes
- **Files**: `app/commands.h`, `app/commands.cpp`
- **Lines Changed**: ~30
- **Risk**: Low - performance optimization, optional

### Task 5.2: Implement Undo Stack Compression
- [ ] Set undo limit to 100 in Scene constructor
- [ ] Implement compression trigger on indexChanged signal
- [ ] Serialize old commands to compressed format
- [ ] Test compression with large undo histories
- **Files**: `app/scene.cpp`, `app/commands.cpp`
- **Lines Changed**: ~50
- **Risk**: Low - optimization only

### Task 5.3: Add Delta Encoding for UpdateCommand
- [ ] Create PropertyDelta structure (elementId, property, oldValue, newValue)
- [ ] Replace full QByteArray serialization with delta list
- [ ] Update redo/undo to apply deltas instead of full state
- [ ] Test with multi-property updates
- **Files**: `app/commands.h`, `app/commands.cpp`
- **Lines Changed**: ~60
- **Risk**: Medium - changes serialization, needs testing

---

## Summary by Phase

| Phase | Tasks | Priority | Time | Risk | Status |
|-------|-------|----------|------|------|--------|
| 1 | 3 | 🔴 CRITICAL | 2-4h | Low | Not Started |
| 2 | 3 | 🟠 HIGH | 1-2w | Medium | Not Started |
| 3 | 3 | 🟡 MEDIUM | 1w | Low-Medium | Not Started |
| 4 | 4 | 🟠 HIGH | 1w | Low-Medium | Not Started |
| 5 | 3 | 🟢 LOW | 2-3w | Low | Not Started |
| **TOTAL** | **16** | **Mixed** | **4-6w** | **Medium** | **Not Started** |

---

## Dependencies & Critical Path

```
Phase 1 (Safety Fixes)
    ↓
Phase 2 (Architecture) — Must complete before Phase 4
    ↓
Phase 3 (Robustness)
    ↓
Phase 4 (Testing) — Validates all previous phases
    ↓
Phase 5 (Optimizations) — Optional, after Phase 4 passes
```

**Critical Path**: Phase 1 → Phase 2 → Phase 4 (Minimum viable overhaul)

---

## Success Criteria

### Phase 1 Complete
- [ ] No crashes on null pointer dereference
- [ ] SplitCommand has no memory leaks
- [ ] Scene can be destroyed safely during undo/redo

### Phase 2 Complete
- [ ] No dangling pointer crashes
- [ ] Scene destruction detected automatically
- [ ] Stale element references detected before use

### Phase 3 Complete
- [ ] Partial operations don't corrupt state
- [ ] Corrupted undo data handled gracefully
- [ ] MorphCommand failures don't leave circuit broken

### Phase 4 Complete
- [ ] All 11 command types have full test coverage
- [ ] All crash scenarios tested and pass
- [ ] Multi-tab undo/redo works correctly

### Phase 5 Complete
- [ ] Command merging reduces undo entries 50%+
- [ ] Memory usage with large histories reduced 30%+
- [ ] Undo/redo performance improved

---

## Reference Documents

- **Detailed Analysis**: `.claude/UNDO_REDO_CRASH_ANALYSIS.md`
- **Code Locations**: See "Code Locations Reference" section in analysis
- **Related Issues**: flipHorizontally bug (FIXED in commit 5de42cd5)

---

**Created**: 2025-11-05
**Last Updated**: 2025-11-05
**Version**: 1.0
