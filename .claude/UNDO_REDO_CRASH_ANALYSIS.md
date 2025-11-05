# Undo/Redo System - Crash Analysis & Overhaul Plan

## Executive Summary

The wiRedPanda undo/redo system is built on Qt's QUndoStack framework with 11 command types implementing the Command Pattern. While architecturally sound, **critical memory safety issues exist that can cause crashes during undo/redo operations**. The primary vulnerability is **dangling pointer access** through the ElementFactory ID registry.

**Crash Severity: HIGH** - Multiple paths can lead to segmentation faults and use-after-free errors.

---

## Critical Crash Vulnerabilities

### 🔴 **CRITICAL #1: Dangling Pointer Access via ElementFactory Registry**

**Location**: `commands.cpp` - `findElements()`, `findItems()`, `findConn()`, `findElm()`
**Root Cause**: Commands store element IDs and retrieve objects through `ElementFactory::itemById()`, which returns raw pointers from a global registry. If an element is deleted outside the undo system, the registry entry is removed but undo commands still hold the ID.

**Crash Scenario**:
```cpp
// Step 1: User creates element (ID = 42)
scene->receiveCommand(new AddItemsCommand({element}, scene));

// Step 2: Element is deleted through non-undoable path (e.g., file close)
delete element;  // ItemWithId destructor calls ElementFactory::removeItem(this)

// Step 3: User tries to undo in a different context
undoStack->undo();
// AddItemsCommand::undo() calls findItems({42})
// ElementFactory::itemById(42) returns nullptr
// dynamic_cast<QGraphicsItem*>(nullptr) returns nullptr
// findItems() throws PANDACEPTION
// OR worse: stale pointer not yet removed, use-after-free crash
```

**Affected Commands**: ALL 11 commands rely on `findElements()`/`findItems()`

**Evidence**:
- `commands.cpp:104-110` - `findItems()` throws exception if count mismatch
- `commands.cpp:122-128` - `findElements()` throws exception if count mismatch
- `elementfactory.cpp:108-110` - `itemById()` returns `nullptr` if ID not found
- No null pointer checks before dynamic_cast usage

**Current Mitigation**: Exception throwing prevents nullptr dereference but crashes with PANDACEPTION instead.

---

### 🔴 **CRITICAL #2: Race Condition - Tab Close During Undo/Redo**

**Location**: Multi-tab architecture (`mainwindow.cpp`, `scene.cpp`)
**Root Cause**: Each tab has an independent `Scene` with its own `QUndoStack`. When a tab is closed, the `Scene` is destroyed, but if an undo/redo operation is in progress or queued, it will access deleted memory.

**Crash Scenario**:
```cpp
// Tab A has pending undo command
Tab A: scene->receiveCommand(new MoveCommand(...));

// User closes Tab A while QUndoStack has commands
mainwindow->closeTab(A);  // Scene destructor runs

// Scene destructor does NOT clear undo stack properly
// QUndoStack may still have commands with raw Scene* pointers

// OR: User presses Ctrl+Z right after closing
// Stale undo action triggers on deleted Scene
```

**Evidence**:
- `scene.h:130` - `QUndoStack m_undoStack` is a member, destroyed with Scene
- Each command stores raw `Scene *m_scene` pointer
- No disconnect of undo/redo actions when Scene is destroyed
- Commands access `m_scene` in `undo()`/`redo()` without validation

**Impact**: Segmentation fault when undo/redo runs on destroyed Scene object.

---

### 🔴 **CRITICAL #3: SplitCommand Constructor Memory Leak + Exception Safety**

**Location**: `commands.cpp:423-462` - `SplitCommand::SplitCommand()`
**Root Cause**: Constructor creates objects (`QNEConnection`, `Node`) before storing them, then throws exceptions if validation fails. Qt's QUndoCommand expects RAII-clean constructors.

**Problematic Code**:
```cpp
SplitCommand::SplitCommand(QNEConnection *conn, QPointF mousePos, Scene *scene, ...)
{
    auto *node = ElementFactory::buildElement(ElementType::Node);  // ❌ Allocated

    // ... later ...
    auto *startPort = conn->startPort();
    auto *endPort = conn->endPort();
    if (!startPort || !endPort) {
        throw PANDACEPTION("Invalid connection ports");  // ❌ LEAK: node not deleted
    }

    // ... later ...
    m_c2Id = (new QNEConnection())->id();  // ❌ Allocated and leaked if next line throws

    m_nodeId = node->id();  // ❌ If this throws somehow, conn2 leaked
}
```

**Crash Path**:
1. Constructor allocates `Node` element
2. Validation fails (null ports)
3. Exception thrown → command object not fully constructed
4. QUndoStack discards command → destructor NOT called
5. Memory leaks AND node registered in ElementFactory with no owner
6. Later undo/redo operations may access orphaned node → crash

**Evidence**:
- `commands.cpp:427` - `buildElement(Node)` allocates
- `commands.cpp:444` - Exception thrown if ports null
- `commands.cpp:457` - `new QNEConnection()` allocated
- No RAII wrappers or exception guards

---

### 🟠 **HIGH #4: MorphCommand Unsafe Element Replacement**

**Location**: `commands.cpp:580-641` - `MorphCommand::transferConnections()`
**Root Cause**: Command deletes old elements and creates new ones in-place, but if any step fails mid-way, the circuit is left in corrupted state.

**Crash Path**:
```cpp
void MorphCommand::transferConnections(QList<GraphicElement *> from, QList<GraphicElement *> to)
{
    for (int elm = 0; elm < from.size(); ++elm) {
        auto *oldElm = from.at(elm);
        auto *newElm = to.at(elm);

        // Transfer connections (may fail if ports incompatible)
        for (int port = 0; port < oldElm->inputSize(); ++port) {
            while (!oldElm->inputPort(port)->connections().isEmpty()) {
                auto *conn = oldElm->inputPort(port)->connections().constFirst();
                // ❌ What if newElm has fewer ports? Crash here:
                conn->setEndPort(newElm->inputPort(port));  // Out of bounds access
            }
        }

        const int oldId = oldElm->id();
        m_scene->removeItem(oldElm);
        delete oldElm;  // ❌ Old element deleted, can't recover if exception below

        ElementFactory::updateItemId(newElm, oldId);  // ❌ If this throws, old element gone
        m_scene->addItem(newElm);
    }
}
```

**Evidence**:
- `commands.cpp:617-631` - Port iteration assumes size compatibility
- `commands.cpp:634-635` - Delete happens before updateItemId safety check
- No transaction mechanism or rollback capability

---

### 🟠 **HIGH #5: Deserialization Vulnerabilities**

**Location**: `commands.cpp:172-202` - `loadItems()`
**Root Cause**: No validation of deserialized data. Corrupted undo state or malformed files can cause crashes.

**Crash Vectors**:
```cpp
const QList<QGraphicsItem *> loadItems(Scene *scene, QByteArray &itemData, ...)
{
    QDataStream stream(&itemData, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    // ❌ No validation: What if itemData is corrupted?

    for (auto *elm : findElements(otherIds)) {
        elm->load(stream, portMap, version);  // ❌ No error handling
    }

    const auto items = Serialization::deserialize(stream, portMap, version);
    // ❌ No validation: What if deserialize() returns invalid items?

    if (items.size() != ids.size()) {
        throw PANDACEPTION(...);  // ❌ Throws mid-undo, state corrupted
    }
}
```

**Attack Vectors**:
1. Corrupted autosave file loaded
2. Manual file edit with invalid data
3. Version mismatch causes misaligned reads
4. Stream read errors not checked

**Evidence**:
- `commands.cpp:178-179` - No try-catch around header read
- `commands.cpp:189` - `deserialize()` success assumed
- `commands.cpp:191-193` - Exception thrown if size mismatch (no rollback)

---

### 🟡 **MEDIUM #6: ChangeInputSizeCommand / ChangeOutputSizeCommand Connection Leaks**

**Location**: `commands.cpp:699-788` (InputSize), `790-880` (OutputSize)
**Root Cause**: When reducing port count, connections are deleted but stored in serialized data. If undo fails mid-way, connections are lost.

**Issue**:
```cpp
void ChangeInputSizeCommand::redo()
{
    for (auto *elm : m_elements) {
        for (int port = m_newInputSize; port < elm->inputSize(); ++port) {
            while (!elm->inputPort(port)->connections().isEmpty()) {
                auto *conn = elm->inputPort(port)->connections().constFirst();
                conn->save(stream);  // Serialize
                m_scene->removeItem(conn);  // Remove from scene
                // ❌ Connection still exists in memory but not in scene
                // If exception thrown here, conn is orphaned
                elm->inputPort(port)->disconnect(conn);
                outputPort->disconnect(conn);
                // ❌ Connection not deleted, just disconnected - memory leak?
            }
        }
    }
}
```

**Evidence**:
- `commands.cpp:742-747` - Connections removed but not deleted
- `commands.cpp:779-781` - `new QNEConnection()` in undo but no matching delete
- No clear ownership model for disconnected connections

---

## Memory Safety Architecture Issues

### Issue A: Raw Pointer Proliferation

**Problem**: All commands use raw pointers (`Scene *`, `GraphicElement *`, `QNEConnection *`) without ownership semantics.

**Evidence**:
- `commands.h:42` - `Scene *m_scene` (non-owning)
- `commands.cpp:263` - `const auto items = findItems(m_ids)` returns raw pointers
- No `std::unique_ptr`, `std::shared_ptr`, or `QPointer` usage

**Risk**: Dangling pointers if objects deleted externally.

---

### Issue B: Exception Safety Violations

**Problem**: Commands throw exceptions in constructors and redo/undo, violating Qt's QUndoCommand contract.

**Evidence**:
- `commands.cpp:110` - `throw PANDACEPTION` in helper function
- `commands.cpp:444` - `throw PANDACEPTION` in constructor
- `commands.cpp:484` - `throw PANDACEPTION` in redo()
- No RAII wrappers for cleanup

**Risk**: Inconsistent state, memory leaks, crashes.

---

### Issue C: No State Validation

**Problem**: Commands assume scene state matches expectations (elements exist, IDs valid, ports compatible).

**Evidence**:
- No pre-condition checks in redo/undo
- `findElements()` only checks count, not validity
- No rollback mechanism if operation fails mid-way

**Risk**: Cascade failures, data corruption.

---

## Ownership Model Analysis

### Current Model (Broken)

```
ElementFactory (Global Singleton)
└── QMap<int, ItemWithId*> m_map  [NON-OWNING REGISTRY]
     ├── GraphicElement* (owned by Scene via QGraphicsScene)
     └── QNEConnection* (owned by Scene via QGraphicsScene)

Scene (Per-Tab)
├── QGraphicsScene items [OWNS via Qt parent-child]
└── QUndoStack
     └── Commands [STORE IDs, RETRIEVE via ElementFactory]
          └── findElements(ids) -> RAW POINTERS from registry

ItemWithId::~ItemWithId()
└── ElementFactory::removeItem(this)  [REMOVES from registry]

Problem: If Scene deletes element, registry updates, but commands still have ID
         → findElements() returns nullptr
         → Crash or exception
```

### Qt Ownership Rules

1. **QGraphicsScene** owns all `QGraphicsItem*` added via `addItem()`
2. When `QGraphicsScene` is destroyed, all items are deleted
3. When `removeItem()` is called, item is NOT deleted (ownership transferred)
4. Commands call `deleteItems()` which does `removeItem()` + manual `delete`

**Conflict**: ElementFactory registry is updated in destructor, but Scene may delete items without going through commands.

---

## Test Coverage Gaps

**Current Tests**: `test/testcommands.cpp` has **1 test function**

```cpp
void TestCommands::testAddDeleteCommands()
{
    // Only tests: AddItemsCommand, DeleteItemsCommand
    // With 6 undo/redo cycles
    // With undo limit = 1
}
```

**Missing Coverage**:
- ❌ MoveCommand (0 tests)
- ❌ RotateCommand (0 tests)
- ❌ FlipCommand (0 tests)
- ❌ UpdateCommand (0 tests)
- ❌ MorphCommand (0 tests)
- ❌ SplitCommand (0 tests)
- ❌ ChangeInputSizeCommand (0 tests)
- ❌ ChangeOutputSizeCommand (0 tests)
- ❌ ToggleTruthTableOutputCommand (0 tests)

**Missing Scenarios**:
- Multi-tab undo/redo
- Tab close during undo
- Exception handling paths
- Deserialization errors
- Large undo history (memory leaks)
- Concurrent undo/redo (if multi-threaded in future)

---

## Overhaul Plan

### 🎯 **Phase 1: Immediate Safety Fixes (Critical)**

**Goal**: Prevent crashes without architectural changes.

#### 1.1 Add Null Pointer Guards

**File**: `commands.cpp`

```cpp
// Replace all instances of findElements/findItems usage with validation:

const QList<GraphicElement *> findElements(const QList<int> &ids)
{
    QList<GraphicElement *> items;
    items.reserve(ids.size());

    for (const int id : ids) {
        auto *item = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id));
        if (!item) {
            qCWarning(commands) << "Element with ID" << id << "not found in registry";
            throw PANDACEPTION_WITH_CONTEXT("commands", "Element ID %1 no longer exists. Cannot undo/redo.", id);
        }
        items.append(item);
    }

    return items;
}

// Add try-catch in all undo/redo methods:
void SomeCommand::redo()
{
    try {
        const auto elements = findElements(m_ids);
        // ... operation ...
    } catch (const std::exception &e) {
        qCCritical(commands) << "Redo failed:" << e.what();
        // Mark command as failed, prevent further operations
        throw;  // Re-throw for QUndoStack to handle
    }
}
```

**Impact**: Prevents nullptr dereference crashes, provides error logging.

---

#### 1.2 Fix SplitCommand Exception Safety

**File**: `commands.cpp:423-462`

```cpp
SplitCommand::SplitCommand(QNEConnection *conn, QPointF mousePos, Scene *scene, ...)
{
    // VALIDATE FIRST, ALLOCATE LAST
    auto *startPort = conn->startPort();
    auto *endPort = conn->endPort();
    if (!startPort || !endPort) {
        throw PANDACEPTION("Invalid connection ports in SplitCommand constructor");
    }

    auto *startElement = startPort->graphicElement();
    auto *endElement = endPort->graphicElement();
    if (!startElement || !endElement) {
        throw PANDACEPTION("Invalid graphic elements in SplitCommand constructor");
    }

    // Store existing IDs
    m_elm1Id = startElement->id();
    m_elm2Id = endElement->id();
    m_c1Id = conn->id();

    // Calculate position
    auto *node = ElementFactory::buildElement(ElementType::Node);
    m_nodePos = mousePos - node->pixmapCenter();
    // ... grid snap ...
    m_nodeAngle = static_cast<int>(360 - 90 * (std::round(conn->angle() / 90.0)));
    m_nodeId = node->id();

    // Allocate secondary connection LAST
    auto *conn2 = new QNEConnection();
    m_c2Id = conn2->id();

    // Clean up temporary objects created during construction
    delete node;   // Will be recreated in redo()
    delete conn2;  // Will be recreated in redo()

    setText(tr("Wire split"));
}
```

**Impact**: Prevents memory leaks on construction failure.

---

#### 1.3 Add Scene Destruction Guard

**File**: `scene.cpp`

```cpp
Scene::~Scene()
{
    // Clear undo stack BEFORE scene items are deleted
    m_undoStack.clear();  // This deletes all commands safely

    // Disconnect undo/redo actions to prevent dangling signals
    disconnect(&m_undoStack, nullptr, this, nullptr);
}
```

**File**: `commands.cpp` - Add Scene validity checks

```cpp
// In every command's undo/redo, add:
void SomeCommand::redo()
{
    if (!m_scene) {
        throw PANDACEPTION("Scene pointer is null, cannot execute command");
    }
    // ... rest of operation ...
}
```

**Impact**: Prevents access to destroyed Scene objects.

---

### 🎯 **Phase 2: Architectural Improvements (High Priority)**

**Goal**: Eliminate dangling pointer vulnerabilities through ownership redesign.

#### 2.1 Replace Raw Scene Pointers with QPointer

**File**: `commands.h`

```cpp
#include <QPointer>

class AddItemsCommand : public QUndoCommand
{
    // ...
private:
    QPointer<Scene> m_scene;  // ✅ Auto-nulls when Scene destroyed
};

// Repeat for all 11 command classes
```

**File**: `commands.cpp`

```cpp
void AddItemsCommand::redo()
{
    if (m_scene.isNull()) {
        qCWarning(commands) << "Scene was destroyed, cannot redo command";
        return;  // Silently fail instead of crash
    }
    // ... rest of operation ...
}
```

**Impact**: Eliminates scene-related use-after-free crashes.

---

#### 2.2 Implement Weak Element References

**Current Problem**: Commands store IDs, but IDs become invalid when elements deleted.

**Solution**: Add generation counter to ElementFactory registry.

**File**: `elementfactory.h`

```cpp
class ElementFactory
{
    struct ItemEntry {
        ItemWithId *item;
        int generation;  // Incremented on each reuse of ID
    };

    QMap<int, ItemEntry> m_map;

public:
    struct ItemHandle {
        int id;
        int generation;

        bool isValid() const {
            auto entry = ElementFactory::instance().m_map.value(id);
            return entry.item && entry.generation == generation;
        }
    };

    static ItemHandle getHandle(ItemWithId *item);
    static ItemWithId *resolveHandle(ItemHandle handle);  // Returns nullptr if stale
};
```

**File**: `commands.h`

```cpp
class AddItemsCommand : public QUndoCommand
{
private:
    QList<ElementFactory::ItemHandle> m_itemHandles;  // Instead of QList<int> m_ids
};
```

**Impact**: Detects stale references instead of using dangling pointers.

---

#### 2.3 Add Command State Validation

**File**: `commands.cpp`

```cpp
// Base validation function used by all commands
bool validateItemHandles(const QList<ElementFactory::ItemHandle> &handles)
{
    for (const auto &handle : handles) {
        if (!handle.isValid()) {
            qCWarning(commands) << "Item handle" << handle.id << "is no longer valid";
            return false;
        }
    }
    return true;
}

void SomeCommand::redo()
{
    if (!validateItemHandles(m_itemHandles)) {
        qCCritical(commands) << "Cannot redo: elements have been deleted";
        throw PANDACEPTION("Command state is invalid");
    }
    // ... operation ...
}
```

**Impact**: Early detection of invalid state, better error messages.

---

### 🎯 **Phase 3: Robustness Enhancements (Medium Priority)**

**Goal**: Handle edge cases and improve error recovery.

#### 3.1 Add Transactional Undo/Redo

**Concept**: Operations either fully succeed or fully rollback.

**File**: `commands.cpp`

```cpp
class UndoTransaction
{
public:
    UndoTransaction(Scene *scene) : m_scene(scene), m_blocker(scene->simulation()) {}

    ~UndoTransaction() {
        if (!m_committed) {
            rollback();
        }
    }

    void commit() { m_committed = true; }

private:
    void rollback() {
        // Restore scene to snapshot taken in constructor
        qCWarning(commands) << "Transaction rolled back due to error";
    }

    Scene *m_scene;
    SimulationBlocker m_blocker;
    bool m_committed = false;
    QByteArray m_snapshot;  // Scene state before operation
};

void SomeCommand::redo()
{
    UndoTransaction transaction(m_scene);

    // ... operation that might fail ...

    transaction.commit();  // Only if successful
}
```

**Impact**: Prevents partial operations leaving circuit corrupted.

---

#### 3.2 Add Deserialization Validation

**File**: `commands.cpp`

```cpp
const QList<QGraphicsItem *> loadItems(Scene *scene, QByteArray &itemData, ...)
{
    if (itemData.isEmpty()) {
        return {};
    }

    try {
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        QVersionNumber version = Serialization::readPandaHeader(stream);

        // ✅ Validate version compatibility
        if (version > Serialization::currentVersion()) {
            throw PANDACEPTION("Cannot load: file version too new");
        }

        QMap<quint64, QNEPort *> portMap;

        for (auto *elm : findElements(otherIds)) {
            if (stream.status() != QDataStream::Ok) {
                throw PANDACEPTION("Stream error while loading element");
            }
            elm->load(stream, portMap, version);
        }

        const auto items = Serialization::deserialize(stream, portMap, version);

        // ✅ Validate deserialized items
        if (items.isEmpty() && !ids.isEmpty()) {
            throw PANDACEPTION("Deserialization failed: no items loaded");
        }

        if (items.size() != ids.size()) {
            throw PANDACEPTION_WITH_CONTEXT("commands",
                "Item count mismatch. Expected %1, got %2.", ids.size(), items.size());
        }

        // ✅ Validate item types
        for (int i = 0; i < items.size(); ++i) {
            if (!dynamic_cast<ItemWithId *>(items.at(i))) {
                throw PANDACEPTION("Deserialized item is not ItemWithId");
            }
        }

        addItems(scene, items);
        return items;

    } catch (const std::exception &e) {
        qCCritical(commands) << "Deserialization failed:" << e.what();
        // Clean up any partially loaded items
        throw;
    }
}
```

**Impact**: Prevents crashes from corrupted data, better error reporting.

---

#### 3.3 Fix MorphCommand Safety

**File**: `commands.cpp:580-641`

```cpp
void MorphCommand::transferConnections(QList<GraphicElement *> from, QList<GraphicElement *> to)
{
    UndoTransaction transaction(m_scene);  // ✅ Add transaction

    for (int elm = 0; elm < from.size(); ++elm) {
        auto *oldElm = from.at(elm);
        auto *newElm = to.at(elm);

        // ✅ VALIDATE port compatibility BEFORE modifying
        newElm->setInputSize(oldElm->inputSize());
        newElm->setOutputSize(oldElm->outputSize());  // Ensure compatibility

        newElm->setPos(oldElm->pos());
        // ... copy properties ...

        // ✅ Transfer connections with validation
        for (int port = 0; port < oldElm->inputSize(); ++port) {
            if (port >= newElm->inputSize()) {
                qCWarning(commands) << "Port" << port << "out of range, skipping";
                continue;  // Skip incompatible ports
            }

            while (!oldElm->inputPort(port)->connections().isEmpty()) {
                auto *conn = oldElm->inputPort(port)->connections().constFirst();
                if (conn && conn->endPort() == oldElm->inputPort(port)) {
                    conn->setEndPort(newElm->inputPort(port));
                }
            }
        }

        // Similar for output ports...

        const int oldId = oldElm->id();
        m_scene->removeItem(oldElm);

        // ✅ Update ID BEFORE deleting (in case updateItemId throws)
        ElementFactory::updateItemId(newElm, oldId);

        delete oldElm;  // Now safe to delete

        m_scene->addItem(newElm);
        newElm->updatePortsProperties();
    }

    transaction.commit();  // ✅ Only commit if no exceptions
}
```

**Impact**: Prevents corruption on morph failures.

---

### 🎯 **Phase 4: Testing & Validation (High Priority)**

**Goal**: Comprehensive test coverage to catch regressions.

#### 4.1 Add Command-Specific Tests

**File**: `test/testcommands.cpp`

```cpp
void TestCommands::testMoveCommand()
{
    QList<QGraphicsItem *> items{new And(), new Or()};
    WorkSpace workspace;
    auto *scene = workspace.scene();

    scene->receiveCommand(new AddItemsCommand(items, scene));

    auto elements = scene->elements();
    QList<QPointF> oldPos{elements[0]->pos(), elements[1]->pos()};

    elements[0]->setPos(100, 100);
    elements[1]->setPos(200, 200);

    scene->receiveCommand(new MoveCommand(elements, oldPos, scene));

    // Test undo
    scene->undoStack()->undo();
    QCOMPARE(elements[0]->pos(), oldPos[0]);
    QCOMPARE(elements[1]->pos(), oldPos[1]);

    // Test redo
    scene->undoStack()->redo();
    QCOMPARE(elements[0]->pos(), QPointF(100, 100));
    QCOMPARE(elements[1]->pos(), QPointF(200, 200));
}

void TestCommands::testRotateCommand() { /* ... */ }
void TestCommands::testFlipCommand() { /* ... */ }
void TestCommands::testUpdateCommand() { /* ... */ }
void TestCommands::testMorphCommand() { /* ... */ }
void TestCommands::testSplitCommand() { /* ... */ }
void TestCommands::testChangeInputSizeCommand() { /* ... */ }
void TestCommands::testChangeOutputSizeCommand() { /* ... */ }
void TestCommands::testToggleTruthTableOutputCommand() { /* ... */ }
```

---

#### 4.2 Add Crash Scenario Tests

**File**: `test/testcommands_crash.cpp` (new)

```cpp
void TestCommandsCrash::testUndoAfterElementDeleted()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *element = new And();
    scene->receiveCommand(new AddItemsCommand({element}, scene));

    int elementId = element->id();

    // Simulate external deletion (e.g., scene cleared)
    scene->removeItem(element);
    delete element;

    // Verify element is gone from registry
    QVERIFY(!ElementFactory::contains(elementId));

    // Try to undo - should NOT crash, should throw exception
    try {
        scene->undoStack()->undo();
        QFAIL("Expected exception was not thrown");
    } catch (const std::exception &e) {
        QVERIFY(QString(e.what()).contains("Element"));  // Verify error message
    }
}

void TestCommandsCrash::testUndoAfterSceneDeleted()
{
    auto *workspace = new WorkSpace();
    auto *scene = workspace->scene();
    auto *undoStack = scene->undoStack();

    scene->receiveCommand(new AddItemsCommand({new And()}, scene));

    // Simulate tab close
    delete workspace;  // Scene is destroyed

    // Undo stack should be destroyed too, cannot test this directly
    // But commands should use QPointer to detect this
}

void TestCommandsCrash::testMorphWithIncompatiblePorts()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Create AND gate with 2 inputs
    auto *andGate = new And();
    scene->receiveCommand(new AddItemsCommand({andGate}, scene));

    // Try to morph to NOT gate (1 input) - should handle gracefully
    scene->receiveCommand(new MorphCommand({andGate}, ElementType::Not, scene));

    // Undo/redo should work without crash
    scene->undoStack()->undo();
    scene->undoStack()->redo();

    QVERIFY(scene->elements().size() == 1);
}

void TestCommandsCrash::testCorruptedUndoData()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    scene->receiveCommand(new AddItemsCommand({new And()}, scene));

    // Corrupt the serialized data in undo stack (simulate file corruption)
    // This requires access to command internals - may need friend class

    // Try to undo with corrupted data - should throw exception, not crash
    try {
        scene->undoStack()->undo();
    } catch (const std::exception &e) {
        // Expected
    }
}
```

---

#### 4.3 Add Multi-Tab Tests

**File**: `test/testcommands_multitab.cpp` (new)

```cpp
void TestCommandsMultiTab::testIndependentUndoStacks()
{
    WorkSpace workspace1;
    WorkSpace workspace2;

    auto *scene1 = workspace1.scene();
    auto *scene2 = workspace2.scene();

    // Add element to tab 1
    scene1->receiveCommand(new AddItemsCommand({new And()}, scene1));
    QCOMPARE(scene1->elements().size(), 1);
    QCOMPARE(scene2->elements().size(), 0);

    // Add element to tab 2
    scene2->receiveCommand(new AddItemsCommand({new Or()}, scene2));
    QCOMPARE(scene1->elements().size(), 1);
    QCOMPARE(scene2->elements().size(), 1);

    // Undo in tab 1 - should not affect tab 2
    scene1->undoStack()->undo();
    QCOMPARE(scene1->elements().size(), 0);
    QCOMPARE(scene2->elements().size(), 1);

    // Undo in tab 2
    scene2->undoStack()->undo();
    QCOMPARE(scene1->elements().size(), 0);
    QCOMPARE(scene2->elements().size(), 0);
}

void TestCommandsMultiTab::testTabCloseWithPendingUndo()
{
    auto *workspace1 = new WorkSpace();
    auto *scene1 = workspace1->scene();

    scene1->receiveCommand(new AddItemsCommand({new And()}, scene1));

    // Close tab 1 (delete workspace)
    delete workspace1;  // Scene and undo stack destroyed

    // No crash should occur
    QVERIFY(true);  // If we get here, test passed
}
```

---

### 🎯 **Phase 5: Long-Term Optimizations (Low Priority)**

**Goal**: Performance and memory improvements.

#### 5.1 Implement Command Merging

**Problem**: Rapid property changes (e.g., dragging slider) create many undo entries.

**Solution**: Merge consecutive similar commands.

**File**: `commands.h`

```cpp
class UpdateCommand : public QUndoCommand
{
public:
    // Override QUndoCommand::mergeWith()
    bool mergeWith(const QUndoCommand *other) override;
    int id() const override { return 1; }  // Unique ID for UpdateCommand type
};
```

**File**: `commands.cpp`

```cpp
bool UpdateCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id()) return false;

    const auto *otherUpdate = static_cast<const UpdateCommand *>(other);

    // Only merge if same elements
    if (m_ids != otherUpdate->m_ids) return false;

    // Keep our m_oldData, take their m_newData
    m_newData = otherUpdate->m_newData;

    return true;  // Commands merged
}
```

**Impact**: Reduces undo stack size, improves performance.

---

#### 5.2 Implement Undo Stack Compression

**Problem**: Large circuits with many operations consume excessive memory.

**Solution**: Compress old undo data.

**File**: `scene.cpp`

```cpp
Scene::Scene(QObject *parent)
{
    // ...
    m_undoStack.setUndoLimit(100);  // Limit stack depth

    // Connect to compress old commands
    connect(&m_undoStack, &QUndoStack::indexChanged, this, [this]() {
        if (m_undoStack.count() > 50) {
            compressOldUndoData();
        }
    });
}

void Scene::compressOldUndoData()
{
    // Serialize old commands to compressed format
    // Replace with lightweight "compressed" versions
}
```

**Impact**: Reduces memory usage for long editing sessions.

---

#### 5.3 Add Delta Encoding for UpdateCommand

**Problem**: UpdateCommand stores full element state before/after, wasteful for single property changes.

**Solution**: Store only changed properties.

**File**: `commands.cpp`

```cpp
class UpdateCommand : public QUndoCommand
{
private:
    struct PropertyDelta {
        int elementId;
        QString propertyName;
        QVariant oldValue;
        QVariant newValue;
    };

    QList<PropertyDelta> m_deltas;  // Instead of full QByteArray
};

void UpdateCommand::redo()
{
    for (const auto &delta : m_deltas) {
        auto *elm = findElm(delta.elementId);
        elm->setProperty(delta.propertyName.toLatin1(), delta.newValue);
    }
}
```

**Impact**: Reduces memory usage for property updates.

---

## Implementation Priority

### ⚠️ **Critical (Fix Immediately)**
1. ✅ Add null pointer guards in `findElements()`/`findItems()` (Phase 1.1)
2. ✅ Fix SplitCommand exception safety (Phase 1.2)
3. ✅ Add Scene destruction guard (Phase 1.3)

**Time Estimate**: 2-4 hours
**Risk**: High - Current code has crash vulnerabilities

---

### 🔴 **High (Fix Within 1 Week)**
1. ✅ Replace raw Scene pointers with QPointer (Phase 2.1)
2. ✅ Implement weak element references (Phase 2.2)
3. ✅ Add command state validation (Phase 2.3)
4. ✅ Write comprehensive tests (Phase 4.1, 4.2, 4.3)

**Time Estimate**: 1-2 weeks
**Risk**: Medium - Architectural improvements, need thorough testing

---

### 🟡 **Medium (Fix Within 1 Month)**
1. ✅ Add transactional undo/redo (Phase 3.1)
2. ✅ Add deserialization validation (Phase 3.2)
3. ✅ Fix MorphCommand safety (Phase 3.3)

**Time Estimate**: 1 week
**Risk**: Low - Robustness improvements, not critical

---

### 🟢 **Low (Future Enhancement)**
1. ✅ Implement command merging (Phase 5.1)
2. ✅ Implement undo stack compression (Phase 5.2)
3. ✅ Add delta encoding (Phase 5.3)

**Time Estimate**: 2-3 weeks
**Risk**: None - Performance optimizations

---

## Success Metrics

### Before Overhaul
- ❌ Crash on undo after tab close
- ❌ Crash on undo after external element deletion
- ❌ Memory leaks in SplitCommand
- ❌ Corrupted state after MorphCommand failure
- ❌ No test coverage for 9/11 commands
- ❌ No validation of deserialized data

### After Overhaul
- ✅ No crashes on undo/redo in any scenario
- ✅ Graceful error handling with user-friendly messages
- ✅ 100% test coverage for all 11 command types
- ✅ Validated deserialization with corruption detection
- ✅ Transactional operations prevent partial state
- ✅ Memory leaks eliminated
- ✅ Performance improvements from command merging

---

## Additional Notes

### Code Locations Reference
- **Commands**: `app/commands.h`, `app/commands.cpp` (~1,155 lines)
- **Scene**: `app/scene.h`, `app/scene.cpp`
- **ElementFactory**: `app/elementfactory.h`, `app/elementfactory.cpp`
- **ItemWithId**: `app/itemwithid.h`, `app/itemwithid.cpp`
- **Tests**: `test/testcommands.cpp` (currently 66 lines)

### Related Issues
- **flipHorizontally() bug**: `scene.cpp:930` has inverted condition (unrelated to undo/redo)
- **Sentry integration**: Check for WIREDPANDA-* crash reports related to undo/redo

### Documentation
- Qt QUndoCommand: https://doc.qt.io/qt-5/qundocommand.html
- Qt QUndoStack: https://doc.qt.io/qt-5/qundostack.html
- Command Pattern: https://refactoring.guru/design-patterns/command

---

**Document Created**: 2025-11-05
**Last Updated**: 2025-11-05
**Status**: Initial Analysis Complete, Ready for Implementation
