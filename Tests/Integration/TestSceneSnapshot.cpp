// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestSceneSnapshot.h"

#include <algorithm>
#include <tuple>
#include <utility>

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QTest>

#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
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
