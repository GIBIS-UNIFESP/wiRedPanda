# Delete Element Command - Code Snippets

## Root Cause Function

**File**: app/commands.cpp (lines 33-46)

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

PROBLEM: Adds both endpoints of each connection without checking for duplicates.
Multi-input element gets added multiple times.

## Bug Manifestation

**File**: app/commands.cpp (lines 242-260)

for (auto *elm : findElements(otherIds)) {
    elm->load(stream, portMap, version);  
}

If otherIds has duplicates, same element loads multiple times:
- Stream position advances incorrectly
- portMap gets corrupted by duplicate loads
- Connections fail to deserialize

## Connection Deserialization

**File**: app/nodes/qneconnection.cpp (lines 195-198)

if (!portMap.isEmpty()) {
    if (!portMap.contains(ptr1) || !portMap.contains(ptr2)) {
        return;  // Connection lost here!
    }

When ports missing from corrupted portMap, connection fails to restore.

## Solution

Deduplicate otherIds:

void storeOtherIds(const QList<QGraphicsItem *> &connections, const QList<int> &ids, QList<int> &otherIds)
{
    QSet<int> added;
    for (auto *item : connections) {
        if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
            if (auto *port1 = conn->startPort(); port1 && port1->graphicElement() && !ids.contains(port1->graphicElement()->id())) {
                int id = port1->graphicElement()->id();
                if (!added.contains(id)) {
                    otherIds.append(id);
                    added.insert(id);
                }
            }

            if (auto *port2 = conn->endPort(); port2 && port2->graphicElement() && !ids.contains(port2->graphicElement()->id())) {
                int id = port2->graphicElement()->id();
                if (!added.contains(id)) {
                    otherIds.append(id);
                    added.insert(id);
                }
            }
        }
    }
}

This ensures each element is loaded only once, fixing stream position and portMap corruption.
