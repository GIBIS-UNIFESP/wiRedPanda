# Node Duplication Wireless Bug Analysis

## Bug Description
When a user duplicates (copy/paste) a node with a wireless label, the duplicated node retains the label but doesn't receive signals from other nodes with the same label.

## Root Cause Analysis

### Expected Behavior
1. User creates: PushButton → Node A → LED (works)
2. User duplicates Node A + LED
3. Duplicated Node A should have label "A" and receive signal from PushButton
4. Both LEDs should light up when PushButton is pressed

### Actual Behavior
1. Duplicated Node A has label "A" but doesn't receive signals
2. Only the original LED lights up when PushButton is pressed

### Technical Root Cause

The issue is in the **wireless connection restoration process**:

#### File Loading (Works Correctly)
```cpp
// workspace.cpp:203-204
const auto items = Serialization::deserialize(stream, {}, version);
for (auto *item : items) {
    m_scene.addItem(item);  // Items added to scene
}
// Restore wireless connections now that all nodes are added to the scene
restoreWirelessConnections();  // ✅ This is called
```

#### Copy/Paste (Bug Location)
```cpp
// scene.cpp:paste()
const auto itemList = Serialization::deserialize(stream, {}, version);
receiveCommand(new AddItemsCommand(itemList, this));  // Items added to scene
// ❌ NO call to restore wireless connections!
```

### Why Wireless Restoration is Needed

During deserialization, nodes load their wireless labels but **cannot register with WirelessConnectionManager**:

```cpp
// node.cpp:load() - During deserialization
if (map.contains("wirelessLabel")) {
    m_wirelessLabel = map.value("wirelessLabel").toString();
    // Note: Wireless connection will be restored later when node is added to scene
    // The scene() is null during deserialization, so we defer restoration
}
```

The `restoreWirelessConnections()` method fixes this by iterating through all nodes in the scene and registering those with wireless labels:

```cpp
void WorkSpace::restoreWirelessConnections() {
    for (auto *item : items) {
        if (auto *node = qobject_cast<Node *>(element)) {
            const QString wirelessLabel = node->getWirelessLabel();
            if (!wirelessLabel.isEmpty()) {
                m_scene.wirelessManager()->setNodeWirelessLabel(node, wirelessLabel);
                // ✅ This registers the node with the wireless manager
            }
        }
    }
}
```

## The Fix

Add wireless connection restoration to the paste operation.

### Option 1: Direct Fix in Scene::paste()
```cpp
void Scene::paste(QDataStream &stream, QVersionNumber version) {
    clearSelection();
    QPointF center; stream >> center;

    const auto itemList = Serialization::deserialize(stream, {}, version);
    const QPointF offset = m_mousePos - center - QPointF(32.0, 32.0);

    receiveCommand(new AddItemsCommand(itemList, this));

    for (auto *item : itemList) {
        if (item->type() == GraphicElement::Type) {
            item->setPos((item->pos() + offset));
        }
    }
    
    // ✅ FIX: Restore wireless connections for pasted items
    restoreWirelessConnectionsForItems(itemList);
    
    resizeScene();
}

private:
void Scene::restoreWirelessConnectionsForItems(const QList<QGraphicsItem *> &items) {
    if (!wirelessManager()) {
        return;
    }
    
    for (auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            if (auto *element = qgraphicsitem_cast<GraphicElement *>(item)) {
                if (element->elementType() == ElementType::Node) {
                    if (auto *node = qobject_cast<Node *>(element)) {
                        const QString wirelessLabel = node->getWirelessLabel();
                        if (!wirelessLabel.isEmpty()) {
                            wirelessManager()->setNodeWirelessLabel(node, wirelessLabel);
                        }
                    }
                }
            }
        }
    }
}
```

### Option 2: Fix in AddItemsCommand
```cpp
void AddItemsCommand::redo() {
    for (auto *item : m_items) {
        if (item->scene() != m_scene) {
            m_scene->addItem(item);
        }
        // ... existing logic ...
    }
    
    // ✅ FIX: Restore wireless connections for added items
    restoreWirelessConnectionsForItems(m_items);
}
```

### Option 3: Automatic Registration in Node::setLabel()
The most robust fix would be to ensure nodes automatically register when their scene becomes available:

```cpp
// Override QGraphicsItem::itemChange() in Node class
QVariant Node::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemSceneHasChanged && scene()) {
        // Node was just added to a scene, register wireless label if present
        if (!m_wirelessLabel.isEmpty()) {
            if (auto *s = qobject_cast<Scene*>(scene())) {
                if (s->wirelessManager()) {
                    s->wirelessManager()->setNodeWirelessLabel(this, m_wirelessLabel);
                }
            }
        }
    }
    return GraphicElement::itemChange(change, value);
}
```

## Reproduction Test Case

```cpp
void TestWireless::testNodeDuplicationBug() {
    auto *scene = new Scene();
    auto *simulation = scene->simulation();

    // Create original circuit: PushButton → Node A → LED
    auto *pushButton = qgraphicsitem_cast<InputButton*>(ElementFactory::buildElement(ElementType::InputButton));
    auto *nodeA1 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *led1 = qgraphicsitem_cast<Led*>(ElementFactory::buildElement(ElementType::Led));
    
    scene->addItem(pushButton);
    scene->addItem(nodeA1);
    scene->addItem(led1);
    
    nodeA1->setLabel("A");
    
    // Connect: PushButton → Node A → LED
    auto *conn1 = new QNEConnection();
    auto *conn2 = new QNEConnection();
    scene->addItem(conn1);
    scene->addItem(conn2);
    
    conn1->setStartPort(pushButton->outputPort());
    conn1->setEndPort(nodeA1->inputPort());
    conn2->setStartPort(nodeA1->outputPort());
    conn2->setEndPort(led1->inputPort());
    
    // Simulate copy/paste of Node A + LED
    QList<QGraphicsItem*> itemsToCopy = {nodeA1, led1};
    
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    writeStream << QPointF(0, 0);  // center
    scene->copy(itemsToCopy, writeStream);
    
    // Paste (this should trigger the bug)
    QDataStream readStream(&data, QIODevice::ReadOnly);
    auto version = Serialization::readPandaHeader(readStream);
    scene->paste(readStream, version);
    
    // Test: Press button and verify both LEDs light up
    pushButton->setOn(true);
    simulation->start();
    
    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }
    
    // Find the duplicated LED (should be the last one added)
    auto allLeds = scene->findItems<Led>();
    QCOMPARE(allLeds.size(), 2);
    
    auto *led2 = allLeds.last();
    
    // ❌ BUG: This assertion would fail before the fix
    // ✅ FIXED: This assertion should pass after the fix
    QCOMPARE(led1->inputPort()->status(), Status::Active);  // Original LED works
    QCOMPARE(led2->inputPort()->status(), Status::Active);  // Duplicated LED should also work
    
    simulation->stop();
    delete scene;
}
```

## Impact Assessment

- **Severity**: Medium-High (affects basic copy/paste functionality)
- **Scope**: All wireless node duplication operations
- **User Experience**: Confusing behavior - duplicated circuits don't work as expected
- **Workaround**: User must manually re-enter wireless labels on duplicated nodes

## Recommended Fix

**Option 3** (automatic registration in `itemChange()`) is the most robust because:
1. Fixes the issue at the source (when nodes are added to scenes)
2. Works for all operations (paste, undo/redo, drag/drop, etc.)
3. No need to remember to call restoration methods
4. Minimal code changes

The fix ensures that whenever a node with a wireless label is added to a scene, it automatically registers with the WirelessConnectionManager.