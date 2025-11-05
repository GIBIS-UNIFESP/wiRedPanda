# Delete Element Undo/Redo Connection Loss Analysis

## Quick Summary

**Issue**: When deleting a multi-input element (like AND gate with 3 inputs) and undoing the deletion, one connection is lost.

**Root Cause**: The `storeOtherIds()` function adds the same element ID multiple times to the list, causing it to be loaded multiple times during undo. This corrupts the port mapping and stream position.

**Location**: `app/commands.cpp`, lines 33-46 in `storeOtherIds()` function.

## Delete Element Handler

**File**: `mcp/server/handlers/elementhandler.cpp` (lines 107-139)

The handler simply creates a `DeleteItemsCommand` and sends it to the scene:

```cpp
QJsonObject ElementHandler::handleDeleteElement(const QJsonObject &params, const QJsonValue &requestId)
{
    // ... validation ...
    
    auto *element = dynamic_cast<GraphicElement*>(item);
    Scene *scene = getCurrentScene();
    
    scene->receiveCommand(new DeleteItemsCommand({element}, scene));
    
    return createSuccessResponse(QJsonObject(), requestId);
}
```

## Root Cause: storeOtherIds Function

**File**: `app/commands.cpp` (lines 33-46)

```cpp
void storeOtherIds(const QList<QGraphicsItem *> &connections, const QList<int> &ids, QList<int> &otherIds)
{
    for (auto *item : connections) {
        if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
            if (auto *port1 = conn->startPort(); port1 && port1->graphicElement() && !ids.contains(port1->graphicElement()->id())) {
                otherIds.append(port1->graphicElement()->id());
            }

            if (auto *port2 = conn->endPort(); port2 && port2->graphicElement() && !ids.contains(port2->graphicElement()->id())) {
                otherIds.append(port2->graphicElement()->id());
            }
        }
    }
}
```

**Problem**: For a multi-input element, each connection adds the target element to otherIds.

Example: AND gate with 3 inputs
```
Input1 --[conn1]--> AND
Input2 --[conn2]--> AND  
Input3 --[conn3]--> AND

storeOtherIds() adds:
  conn1: Input1, AND
  conn2: Input2, AND
  conn3: Input3, AND

Result: otherIds = [Input1, AND, Input2, AND, Input3, AND]
                         ^^^ AND duplicated 3 times!
```

## The Bug in loadItems()

**File**: `app/commands.cpp` (lines 242-317)

When undo is called, loadItems() is invoked with the duplicated otherIds list:

```cpp
for (auto *elm : findElements(otherIds)) {
    elm->load(stream, portMap, version);  // Element loaded multiple times!
}
```

**What happens**:
1. Elements are saved once: [Input1, Input2, Input3, AND, Conn1, Conn2, Conn3]
2. Elements are loaded multiple times: [Input1, AND, Input2, AND, Input3, AND]
3. Stream position advances too far (6 element loads instead of 4)
4. portMap gets corrupted by duplicate loads
5. Connections fail to deserialize because their ports are missing from portMap

**Connection::load()** (app/nodes/qneconnection.cpp, line 196-197):
```cpp
if (!portMap.contains(ptr1) || !portMap.contains(ptr2)) {
    return;  // Connection silently fails to load
}
```

## Complete File Locations

| Component | File | Lines | Role |
|-----------|------|-------|------|
| MCP Handler | mcp/server/handlers/elementhandler.cpp | 107-139 | Entry point for delete_element |
| DeleteItemsCommand | app/commands.h | 110-127 | Command class that implements delete |
| **storeOtherIds** | **app/commands.cpp** | **33-46** | **ADDS DUPLICATES - ROOT CAUSE** |
| loadList() | app/commands.cpp | 48-96 | Calls storeOtherIds |
| saveItems() | app/commands.cpp | 214-227 | Serializes items |
| **loadItems()** | **app/commands.cpp** | **242-317** | **LOADS DUPLICATES - BUG MANIFESTS** |
| DeleteItems::undo() | app/commands.cpp | 390-413 | Calls loadItems |
| GraphicElement::save | app/graphicelement.cpp | 142-214 | Saves ports |
| GraphicElement::load | app/graphicelement.cpp | 250-362 | Loads ports and builds portMap |
| QNEConnection::save | app/nodes/qneconnection.cpp | 160-164 | Saves port pointers |
| QNEConnection::load | app/nodes/qneconnection.cpp | 166-234 | Uses portMap to restore |

## Why One Connection is Lost

The exact connection lost is typically the LAST one added (most recently created) because:

1. Earlier connections are in the stream before duplicate element loads
2. During duplicate loads, portMap gets overwritten with same values but stream advances past real data
3. Later connections reference port pointers that are no longer in the corrupted portMap
4. Their QNEConnection::load() returns early without setting ports
5. The connection object exists but is not properly initialized = lost connection

## The Fix

Deduplicate otherIds before using it in loadItems():

```cpp
void storeOtherIds(const QList<QGraphicsItem *> &connections, const QList<int> &ids, QList<int> &otherIds)
{
    QSet<int> added;  // Track which IDs we've already added
    for (auto *item : connections) {
        if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
            if (auto *port1 = conn->startPort(); port1 && port1->graphicElement() && !ids.contains(port1->graphicElement()->id())) {
                int id = port1->graphicElement()->id();
                if (!added.contains(id)) {  // Only add if not already added
                    otherIds.append(id);
                    added.insert(id);
                }
            }

            if (auto *port2 = conn->endPort(); port2 && port2->graphicElement() && !ids.contains(port2->graphicElement()->id())) {
                int id = port2->graphicElement()->id();
                if (!added.contains(id)) {  // Only add if not already added
                    otherIds.append(id);
                    added.insert(id);
                }
            }
        }
    }
}
```

This ensures each element is loaded exactly once during undo, fixing the stream position corruption and port mapping issues.
