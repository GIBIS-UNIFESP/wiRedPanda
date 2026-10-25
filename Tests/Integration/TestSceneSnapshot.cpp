// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestSceneSnapshot.h"

#include <algorithm>
#include <memory>
#include <tuple>
#include <utility>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QGraphicsSceneDragDropEvent>
#include <QHash>
#include <QIODevice>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMimeData>
#include <QPainterPath>
#include <QSet>
#include <QTest>
#include <QUndoStack>

#include "App/Core/MimeTypes.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementLabel.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/MainWindow.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "MCP/Server/Handlers/ElementHandler.h"
#include "MCP/Server/Handlers/ICHandler.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

namespace {

/// A directory of circuits; its goldens are in the subdirectory of sceneGoldensDir() named after it.
struct CircuitGroup {
    QString name;
    QString dir;
};

QList<CircuitGroup> circuitGroups()
{
    const QString testsDir(QUOTE(CURRENTDIR));
    return {
        {"Examples", TestUtils::examplesDir()},
        {"ICComponents", TestUtils::cpuComponentsDir()},
        {"Fixtures", testsDir + "/Fixtures/"},
        {"Perf", testsDir + "/Fixtures/Perf/"},
    };
}

QString sceneGoldensDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/SceneSnapshot/";
}

/// A port: its direction, index and name, its position in its element (pos()) and in the scene, its default and
/// current status, whether it is required, and how many wires it holds.
QString portLine(const Port *port)
{
    return QString("  P %1 idx=%2 name=%3 pos=%4 scene=%5 default=%6 required=%7 status=%8 wires=%9")
        .arg(QString(port->isInput() ? "in" : "out"), QString::number(port->index()),
             TestUtils::snapshotQuoted(port->name()), TestUtils::snapshotPoint(port->pos()),
             TestUtils::snapshotPoint(port->scenePos()), TestUtils::snapshotStatus(port->defaultValue()),
             QString::number(port->isRequired()), TestUtils::snapshotStatus(port->status()),
             QString::number(port->connections().size()));
}

/// One end of a wire: its element's id and its port's index among the element's ports, or -1 for a missing end.
std::pair<int, int> wireEnd(const Port *port)
{
    if (!port || !port->graphicElement()) {
        return {-1, -1};
    }
    return {port->graphicElement()->id(), port->globalIndex()};
}

/// The elements by id, each with its ports; then the wires, found through the ports and sorted by their ends; then
/// the counts.
QString dumpScene(const Scene *scene)
{
    const auto elements = TestUtils::elementsById(scene);

    QStringList lines;
    QSet<const Connection *> connections;
    qsizetype portCount = 0;
    for (const auto *element : elements) {
        lines << QString("E id=%1 type=%2 pos=%3 rot=%4 flipX=%5 flipY=%6 label=%7 in=%8 out=%9 pix=%10")
                     .arg(QString::number(element->id()), TestUtils::snapshotType(element->elementType()),
                          TestUtils::snapshotPoint(element->pos()), TestUtils::snapshotNumber(element->rotation()),
                          QString::number(element->isFlippedX()), QString::number(element->isFlippedY()),
                          TestUtils::snapshotQuoted(element->label()), QString::number(element->inputSize()),
                          QString::number(element->outputSize()), SnapshotAccess::pixmapIdentity(element));

        const auto ports = element->allPorts();
        portCount += ports.size();
        for (const auto *port : ports) {
            lines << portLine(port);
            for (const auto *connection : port->connections()) {
                connections.insert(connection);
            }
        }
    }

    QList<std::tuple<int, int, int, int, QString>> wires;
    wires.reserve(connections.size());
    for (const auto *connection : std::as_const(connections)) {
        const auto [startElement, startPort] = wireEnd(connection->startPort());
        const auto [endElement, endPort] = wireEnd(connection->endPort());
        wires.emplaceBack(startElement, startPort, endElement, endPort, TestUtils::snapshotStatus(connection->status()));
    }
    std::sort(wires.begin(), wires.end());
    for (const auto &[startElement, startPort, endElement, endPort, status] : std::as_const(wires)) {
        lines << QString("W %1:%2 -> %3:%4 status=%5")
                     .arg(QString::number(startElement), QString::number(startPort), QString::number(endElement),
                          QString::number(endPort), status);
    }

    lines << QString("COUNTS elements=%1 ports=%2 wires=%3")
                 .arg(QString::number(elements.size()), QString::number(portCount), QString::number(wires.size()));
    return lines.join('\n') + '\n';
}

/// The wires sorted by their ends, each with its ends' positions and every element of its path, in scene coordinates.
QString dumpWireGeometry(const Scene *scene)
{
    QSet<const Connection *> connections;
    for (const auto *element : TestUtils::elementsById(scene)) {
        for (const auto *port : element->allPorts()) {
            for (const auto *connection : port->connections()) {
                connections.insert(connection);
            }
        }
    }

    QList<std::tuple<int, int, int, int, const Connection *>> wires;
    wires.reserve(connections.size());
    for (const auto *connection : std::as_const(connections)) {
        const auto [startElement, startPort] = wireEnd(connection->startPort());
        const auto [endElement, endPort] = wireEnd(connection->endPort());
        wires.emplaceBack(startElement, startPort, endElement, endPort, connection);
    }
    std::sort(wires.begin(), wires.end(), [](const auto &a, const auto &b) {
        return std::tie(std::get<0>(a), std::get<1>(a), std::get<2>(a), std::get<3>(a))
             < std::tie(std::get<0>(b), std::get<1>(b), std::get<2>(b), std::get<3>(b));
    });

    QStringList lines;
    for (const auto &[startElement, startPort, endElement, endPort, connection] : std::as_const(wires)) {
        lines << QString("W %1:%2 -> %3:%4 start=%5 end=%6")
                     .arg(QString::number(startElement), QString::number(startPort), QString::number(endElement),
                          QString::number(endPort), TestUtils::snapshotPoint(connection->startPort()->scenePos()),
                          TestUtils::snapshotPoint(connection->endPort()->scenePos()));
        const QPainterPath path = connection->path();
        for (int i = 0; i < path.elementCount(); ++i) {
            const QPainterPath::Element pathElement = path.elementAt(i);
            lines << QString("  %1 %2").arg(QString::number(static_cast<int>(pathElement.type)),
                                             TestUtils::snapshotPoint(QPointF(pathElement.x, pathElement.y)));
        }
    }
    return lines.join('\n') + '\n';
}

/// The edited element's place, orientation and port counts, then each of its ports, so an edit that moves a port or
/// changes the count shows even where no wire moves with it.
QString dumpEditedElement(const GraphicElement *element)
{
    QStringList lines;
    lines << QString("E id=%1 type=%2 pos=%3 rot=%4 flipX=%5 flipY=%6 in=%7 out=%8")
                 .arg(QString::number(element->id()), TestUtils::snapshotType(element->elementType()),
                      TestUtils::snapshotPoint(element->pos()), TestUtils::snapshotNumber(element->rotation()),
                      QString::number(element->isFlippedX()), QString::number(element->isFlippedY()),
                      QString::number(element->inputSize()), QString::number(element->outputSize()));
    for (const auto *port : element->allPorts()) {
        lines << portLine(port);
    }
    return lines.join('\n') + '\n';
}

bool hasWire(const GraphicElement *element)
{
    const auto ports = element->allPorts();
    return std::any_of(ports.cbegin(), ports.cend(), [](const Port *port) { return !port->connections().isEmpty(); });
}

/// The lowest-id element with a wire attached that \a accepts, or null.
template <typename Predicate>
GraphicElement *firstWired(const Scene *scene, Predicate accepts)
{
    for (auto *element : TestUtils::elementsById(scene)) {
        if (hasWire(element) && accepts(element)) {
            return element;
        }
    }
    return nullptr;
}

/// The element an edit acts on. "move", "rotate", "flip" and "portcount" take the lowest-id wired element, in every
/// fixture an InputSwitch: its graphic does not rotate (its port is re-routed instead) and it has no inputs to add, so
/// "portcount" pins that the change does nothing to it. "resize" takes the lowest-id wired element whose inputs can
/// grow, and "rotate-body" and "flip-body" the lowest-id wired element whose graphic rotates.
GraphicElement *editTarget(const Scene *scene, const QString &edit)
{
    if (edit == "resize") {
        return firstWired(scene, [](const GraphicElement *element) {
            return element->maxInputSize() > element->inputSize();
        });
    }
    if (edit.endsWith("-body")) {
        return firstWired(scene, [](const GraphicElement *element) { return element->rotatesGraphic(); });
    }
    return firstWired(scene, [](const GraphicElement *) { return true; });
}

/// Four elements of different types and widths, none on the grid: placed before they join the scene, which snaps every
/// later position to the half grid.
void snapPathsSetup(Scene *scene)
{
    const QList<std::pair<ElementType, QPointF>> placements{
        {ElementType::InputSwitch, QPointF(3, 5)},
        {ElementType::And, QPointF(150, 37)},
        {ElementType::Led, QPointF(300, 91)},
        {ElementType::Or, QPointF(45, 210)},
    };
    for (const auto &[type, position] : placements) {
        auto *element = ElementFactory::buildElement(type);
        element->setPos(position);
        scene->addItem(element);
    }
}

/// The elements snapPathsSetup() places.
constexpr int kSnapPathsElements = 4;

/// Every element's id, type, position, scene bounding rectangle, rotation and flips, in id order: a paste numbers its
/// copies in the selection's id order.
QString snapPathsDump(const Scene *scene)
{
    QStringList lines;
    for (const auto *element : TestUtils::elementsById(scene)) {
        const QRectF rect = element->sceneBoundingRect();
        lines << QString("E%1 %2 pos=%3 rect=%4 %5 rot=%6 flipX=%7 flipY=%8")
                     .arg(QString::number(element->id()), TestUtils::snapshotType(element->elementType()),
                          TestUtils::snapshotPoint(element->pos()), TestUtils::snapshotPoint(rect.topLeft()),
                          TestUtils::snapshotPoint({rect.width(), rect.height()}),
                          TestUtils::snapshotNumber(element->rotation()), QString::number(element->isFlippedX()),
                          QString::number(element->isFlippedY()));
    }
    return lines.join('\n') + '\n';
}

/// Drops a new And from the palette at \a scenePos, carrying what the palette's entry puts in its drag, as the events a
/// drop sends to the scene: QDrag::exec() cannot be driven from a test.
void dropFromPalette(Scene *scene, const QPointF &scenePos)
{
    const ElementLabel entry(ElementFactory::pixmap(ElementType::And), ElementType::And, {});
    const std::unique_ptr<QMimeData> mimeData(entry.mimeData());
    for (const QEvent::Type type : {QEvent::GraphicsSceneDragEnter, QEvent::GraphicsSceneDragMove, QEvent::GraphicsSceneDrop}) {
        QGraphicsSceneDragDropEvent event(type);
        event.setMimeData(mimeData.get());
        event.setScenePos(scenePos);
        event.setProposedAction(Qt::CopyAction);
        event.setPossibleActions(Qt::CopyAction);
        QCoreApplication::sendEvent(scene, &event);
    }
}

/// Drops a copy of the selection where a Ctrl-drag that started at \a from ends at \a to, carrying what
/// ClipboardManager::cloneDrag() puts in its drag, as the events a drop sends to the scene: QDrag::exec() cannot be
/// driven from a test.
void dropClone(Scene *scene, const QPointF &from, const QPointF &to)
{
    const auto items = scene->selectedItems();
    QPointF center;
    int elements = 0;
    for (const auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            center += item->pos();
            ++elements;
        }
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    stream << from << center / static_cast<qreal>(elements);
    Serialization::serialize(items, stream, {.purpose = SerializationPurpose::InMemorySnapshot});

    QMimeData mimeData;
    mimeData.setData(MimeType::CloneDrag, itemData);
    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDrop);
    event.setMimeData(&mimeData);
    event.setScenePos(to);
    event.setProposedAction(Qt::CopyAction);
    event.setPossibleActions(Qt::CopyAction);
    QCoreApplication::sendEvent(scene, &event);
}

/// Sends \a command with \a params to \a handler, failing the running test on an error response.
template <typename Handler>
void sendMcp(Handler &handler, const QString &command, const QJsonObject &params)
{
    const QJsonObject response = handler.handleCommand(command, params, {});
    QVERIFY2(response.contains("result"), qPrintable(command + " failed"));
}

/// Applies one snapping path to the four selected elements, through the entry point the application uses; the MCP's
/// paths, named "mcp-", through its handlers on \a window, whose scene \a scene is.
void snapPathsApply(Scene *scene, const QString &operation, MainWindow *window)
{
    const QHash<QString, void (Scene::*)()> sceneActions{
        {"align-left", &Scene::alignLeft},
        {"align-right", &Scene::alignRight},
        {"align-top", &Scene::alignTop},
        {"align-bottom", &Scene::alignBottom},
        {"align-hcenter", &Scene::alignHorizontalCenter},
        {"align-vcenter", &Scene::alignVerticalCenter},
        {"distribute-h", &Scene::distributeHorizontally},
        {"distribute-v", &Scene::distributeVertically},
        {"rotate-right", &Scene::rotateRight},
        {"rotate-left", &Scene::rotateLeft},
        {"flip-h", &Scene::flipHorizontally},
        {"flip-v", &Scene::flipVertically},
    };
    if (const auto action = sceneActions.value(operation)) {
        (scene->*action)();
        return;
    }

    auto *first = TestUtils::elementsById(scene).constFirst();
    if (operation == "move") {
        // The first element alone, by a command as the property editor and MCP push one, its new place snapped as
        // theirs is.
        const QPointF oldPosition = first->pos();
        first->setPos(GraphicElement::snapToGrid(oldPosition + QPointF(37, 21)));
        scene->receiveCommand(new MoveCommand({first}, {oldPosition}, scene));
    } else if (operation == "nudge-right") {
        // The scene takes an arrow key as a nudge while no item has the keyboard focus, as in the application.
        QKeyEvent key(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
        QCoreApplication::sendEvent(scene, &key);
    } else if (operation == "nudge-shift-down") {
        QKeyEvent key(QEvent::KeyPress, Qt::Key_Down, Qt::ShiftModifier);
        QCoreApplication::sendEvent(scene, &key);
    } else if (operation == "drag") {
        // A drag of a selected element moves the whole selection, by a distance off the grid. It starts on the And: a
        // press on the switch would also toggle it.
        const QPointF from = TestUtils::elementsById(scene).at(1)->sceneBoundingRect().center();
        TestUtils::ScenePointer(scene).drag(from, from + QPointF(37, 21));
    } else if (operation == "drop") {
        dropFromPalette(scene, QPointF(101, 203));
    } else if (operation == "paste") {
        // Paste reads the system clipboard, which is why the class holds the clipboard resource lock.
        scene->copyAction();
        scene->pasteAction();
    } else if (operation == "duplicate") {
        scene->duplicateAction();
    } else if (operation == "clone-drop") {
        // A Ctrl-drag of the selection from the And, by a distance off the grid.
        const QPointF from = TestUtils::elementsById(scene).at(1)->sceneBoundingRect().center();
        dropClone(scene, from, from + QPointF(37, 21));
    } else if (operation == "add-without-drag") {
        // A double click on a palette entry, or a search's result, adds at the view's centre; the scene deletes the
        // mime data.
        const ElementLabel entry(ElementFactory::pixmap(ElementType::And), ElementType::And, {});
        scene->addItem(entry.mimeData(), QPointF(101, 203));
    } else if (operation == "split") {
        // A wire from the switch to the Led, added without a command, split at a point off the grid.
        auto *wire = new Connection();
        wire->setStartPort(first->outputPort());
        wire->setEndPort(TestUtils::elementsById(scene).at(2)->inputPort());
        scene->addItem(wire);
        scene->receiveCommand(new SplitCommand(wire, QPointF(101, 53), scene));
    } else if (operation == "mcp-create") {
        ElementHandler handler(window, nullptr);
        sendMcp(handler, "create_element", {{"type", "And"}, {"x", 101}, {"y", 203}});
    } else if (operation == "mcp-move") {
        ElementHandler handler(window, nullptr);
        sendMcp(handler, "move_element", {{"element_id", first->id()}, {"x", 101}, {"y", 203}});
    } else if (operation == "mcp-instantiate-ic") {
        ICHandler handler(window, nullptr);
        const QString icFile = TestUtils::fullAdderIcFile();
        sendMcp(handler, "instantiate_ic", {{"ic_name", icFile.left(icFile.size() - 6)}, {"x", 101}, {"y", 203}});
    } else {
        QFAIL(qPrintable("no snapping path " + operation));
    }
}

/// The snapping paths, each a golden in Tests/Integration/SnapPaths.
QStringList snapPathOperations()
{
    return {"add-without-drag", "align-bottom", "align-hcenter", "align-left", "align-right", "align-top",
            "align-vcenter", "clone-drop", "distribute-h", "distribute-v", "drag", "drop", "duplicate", "flip-h",
            "flip-v", "mcp-create", "mcp-instantiate-ic", "mcp-move", "move", "nudge-right", "nudge-shift-down",
            "paste", "rotate-left", "rotate-right", "split"};
}

QString snapPathsGoldensDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/SnapPaths/";
}

} // namespace

void TestSceneSnapshot::initTestCase()
{
    m_previousTheme = ThemeManager::theme();
    ThemeManager::setTheme(Theme::Light);
}

void TestSceneSnapshot::cleanupTestCase()
{
    ThemeManager::setTheme(m_previousTheme);
}

void TestSceneSnapshot::testSnapshotMatchesGolden_data()
{
    QTest::addColumn<QString>("path");
    for (const auto &group : circuitGroups()) {
        TestUtils::addFileRows(group.dir, {"*.panda"}, false, group.name + '/');
    }
}

void TestSceneSnapshot::testSnapshotMatchesGolden()
{
    QFETCH(QString, path);
    const QString missing = TestUtils::requireInputFile(path);
    QVERIFY2(missing.isEmpty(), qPrintable(missing));

    WorkSpace workspace;
    TestUtils::loadCircuit(workspace, path);
    if (QTest::currentTestFailed()) {
        return;
    }

    // The row is "<group>/<file>.panda"; its golden is "<group>/<file>.txt".
    const QString row(QTest::currentDataTag());
    const QString golden = sceneGoldensDir() + row.chopped(QStringLiteral(".panda").size()) + ".txt";
    const QString mismatch = TestUtils::compareWithGolden(golden, dumpScene(workspace.scene()));
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}

void TestSceneSnapshot::testEveryGoldenHasACircuit()
{
    // A golden whose circuit was removed or renamed would otherwise stay in the tree, compared with nothing.
    const auto groups = circuitGroups();
    const QDir goldens(sceneGoldensDir());
    QVERIFY2(goldens.exists(), qPrintable(goldens.path()));
    const auto groupNames = goldens.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    QCOMPARE(groupNames.size(), groups.size());

    for (const QString &groupName : groupNames) {
        const auto group = std::find_if(groups.cbegin(), groups.cend(), [&](const CircuitGroup &candidate) {
            return candidate.name == groupName;
        });
        QVERIFY2(group != groups.cend(), qPrintable(goldens.filePath(groupName) + " matches no input directory"));

        const auto files = QDir(goldens.filePath(groupName)).entryInfoList(QDir::Files, QDir::Name);
        for (const auto &golden : files) {
            QVERIFY2(golden.suffix() == "txt", qPrintable(golden.filePath() + " is not a golden"));
            const QString circuit = group->dir + golden.completeBaseName() + ".panda";
            QVERIFY2(QFileInfo::exists(circuit), qPrintable(golden.filePath() + " has no circuit " + circuit));
        }
    }
}

void TestSceneSnapshot::testWireGeometryAfterEdit_data()
{
    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("edit");

    const QString fixturesDir = QString(QUOTE(CURRENTDIR)) + "/Fixtures/";
    const auto files = QDir(fixturesDir).entryInfoList({"*.panda"}, QDir::Files, QDir::Name);
    if (files.isEmpty()) {
        // One row that fails, naming the directory, rather than no rows at all.
        QTest::newRow(qPrintable("missing fixtures " + fixturesDir)) << fixturesDir << QString();
    }
    for (const auto &file : files) {
        for (const char *edit : {"move", "rotate", "flip", "portcount", "resize", "rotate-body", "flip-body"}) {
            QTest::newRow(qPrintable(file.completeBaseName() + '/' + edit)) << file.absoluteFilePath() << QString(edit);
        }
    }
}

void TestSceneSnapshot::testWireGeometryAfterEdit()
{
    QFETCH(QString, path);
    QFETCH(QString, edit);
    const QString missing = TestUtils::requireInputFile(path);
    QVERIFY2(missing.isEmpty(), qPrintable(missing));

    // With rendering off, as the suite runs, a wire builds no path, so the switch is on from before the load.
    const TestUtils::ScopedRendering rendering;
    WorkSpace workspace;
    TestUtils::loadCircuit(workspace, path);
    if (QTest::currentTestFailed()) {
        return;
    }

    Scene *scene = workspace.scene();
    GraphicElement *target = editTarget(scene, edit);
    QVERIFY2(target, qPrintable(path + " has no element for " + edit));

    // Each edit goes through the entry point the application uses: a command, or the scene's action on the selection.
    if (edit == "move") {
        const QPointF oldPosition = target->pos();
        target->setPos(GraphicElement::snapToGrid(oldPosition + QPointF(40, 20)));
        scene->receiveCommand(new MoveCommand({target}, {oldPosition}, scene));
    } else if ((edit == "portcount") || (edit == "resize")) {
        scene->receiveCommand(new ChangePortSizeCommand({target}, target->inputSize() + 1, scene, true));
    } else {
        scene->clearSelection();
        target->setSelected(true);
        if (edit.startsWith("rotate")) {
            scene->rotateRight();
        } else {
            scene->flipHorizontally();
        }
    }

    // The row is "<fixture>/<edit>": the wires in "<row>.txt", the edited element in "<row>.ports.txt".
    const QString golden = QString(QUOTE(CURRENTDIR)) + "/Integration/WireGeometry/" + QTest::currentDataTag();
    const QString wireMismatch = TestUtils::compareWithGolden(golden + ".txt", dumpWireGeometry(scene));
    QVERIFY2(wireMismatch.isEmpty(), qPrintable(wireMismatch));
    const QString portMismatch = TestUtils::compareWithGolden(golden + ".ports.txt", dumpEditedElement(target));
    QVERIFY2(portMismatch.isEmpty(), qPrintable(portMismatch));
}

void TestSceneSnapshot::testSnapPathsMatchGolden_data()
{
    QTest::addColumn<QString>("operation");
    for (const QString &operation : snapPathOperations()) {
        QTest::newRow(qPrintable(operation)) << operation;
    }
}

void TestSceneSnapshot::testSnapPathsMatchGolden()
{
    // Each path acts on four selected elements off the grid, then is undone and redone: where every element is after
    // each step. The MCP's handlers act on a main window's scene.
    QFETCH(QString, operation);
    std::unique_ptr<WorkSpace> workspace;
    std::unique_ptr<MainWindow> window;
    Scene *scene = nullptr;
    if (operation.startsWith("mcp-")) {
        window = std::make_unique<MainWindow>();
        scene = window->currentTab()->scene();
    } else {
        workspace = std::make_unique<WorkSpace>();
        scene = workspace->scene();
    }
    snapPathsSetup(scene);
    QCOMPARE(scene->lastId(), kSnapPathsElements);
    scene->selectAll();

    QStringList sections{"initial\n" + snapPathsDump(scene)};
    snapPathsApply(scene, operation, window.get());
    if (QTest::currentTestFailed()) {
        return;
    }
    QCOMPARE(scene->undoStack()->count(), 1);
    sections << "after " + operation + '\n' + snapPathsDump(scene);
    scene->undoStack()->undo();
    sections << "undo\n" + snapPathsDump(scene);
    scene->undoStack()->redo();
    sections << "redo\n" + snapPathsDump(scene);

    const QString mismatch = TestUtils::compareWithGolden(snapPathsGoldensDir() + operation + ".txt", sections.join('\n'));
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}

void TestSceneSnapshot::testEverySnapPathGoldenHasAPath()
{
    // A golden whose path was removed or renamed would otherwise stay in the tree, compared with nothing.
    QStringList expected;
    for (const QString &operation : snapPathOperations()) {
        expected << operation + ".txt";
    }
    expected.sort();
    QCOMPARE(QDir(snapPathsGoldensDir()).entryList(QDir::Files, QDir::Name), expected);
}

void TestSceneSnapshot::testPropertyKindsMatchGolden()
{
    // Every creatable type with the property kinds its element offers the editor, in the order it lists them.
    QStringList lines;
    for (const ElementType type : TestUtils::dumpedTypes()) {
        if (!ElementFactory::hasCreator(type)) {
            lines << QString("%1 %2").arg(QLatin1String(TestUtils::kNoCreator), TestUtils::snapshotType(type));
            continue;
        }
        const std::unique_ptr<GraphicElement> element(ElementFactory::buildElement(type));
        QStringList kinds;
        for (const auto &property : element->editableProperties()) {
            kinds << TestUtils::snapshotPropertyKind(property.type);
        }
        lines << QString("T %1 props=%2").arg(TestUtils::snapshotType(type), kinds.join(','));
    }

    const QString golden = QString(QUOTE(CURRENTDIR)) + "/Integration/Properties/types.txt";
    const QString mismatch = TestUtils::compareWithGolden(golden, lines.join('\n') + '\n');
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}
