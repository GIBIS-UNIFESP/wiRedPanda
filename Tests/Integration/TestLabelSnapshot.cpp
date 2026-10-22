// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestLabelSnapshot.h"

#include <functional>
#include <memory>
#include <utility>

#include <QFileInfo>
#include <QTest>
#include <QTransform>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

// A named namespace: the port layout golden, in the same unity batch, has helpers of the same names.
namespace LabelSnapshot {

QString goldenPath()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/LabelSnapshot/labels.txt";
}

/// Off the origin and off the multiples of 64, so a translation lost or applied twice shows.
constexpr QPointF kPlacement(200, -120);

QString describe(const GraphicElement *element)
{
    return QString("  rot=%1 flipX=%2 flipY=%3 label=%4")
        .arg(TestUtils::snapshotNumber(element->rotation()), QString::number(element->isFlippedX()),
             QString::number(element->isFlippedY()), TestUtils::snapshotTransform(SnapshotAccess::labelSceneTransform(element)));
}

/// One configuration in the sixteen orientations, given its ports and then oriented, then oriented first where that
/// differs.
QStringList describeAll(Scene *scene, const QString &header, const TestUtils::ElementBuilder &build,
                        const std::function<void(GraphicElement *)> &applyPorts)
{
    return TestUtils::describeInOrientations(scene, header, build, applyPorts, describe, "  oriented first: same",
                                             "  oriented first:");
}

/// Places \a element and gives it a label, then adds it to \a scene.
std::unique_ptr<GraphicElement> placed(std::unique_ptr<GraphicElement> element, Scene *scene)
{
    element->setPos(kPlacement);
    element->setLabel("Label");
    scene->addItem(element.get());
    return element;
}

QString dumpLabels()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    QStringList lines;

    for (const ElementType type : TestUtils::dumpedTypes()) {
        if (!ElementFactory::hasCreator(type)) {
            lines << QString("%1 %2").arg(TestUtils::kNoCreator, TestUtils::snapshotType(type));
            continue;
        }

        const std::unique_ptr<GraphicElement> probe(ElementFactory::buildElement(type));
        if (!probe->hasLabel()) {
            // Listed rather than skipped, so a type that gains or loses a label changes the golden.
            lines << QString("T %1 no label").arg(TestUtils::snapshotType(type));
            continue;
        }

        // The fewest and the most ports: a TruthTable's label stands below a body that grows with them.
        QList<std::pair<int, int>> counts{{probe->minInputSize(), probe->minOutputSize()}};
        if ((probe->maxInputSize() != probe->minInputSize()) || (probe->maxOutputSize() != probe->minOutputSize())) {
            counts.append({probe->maxInputSize(), probe->maxOutputSize()});
        }

        const TestUtils::ElementBuilder build = [type](Scene *target) {
            return placed(std::unique_ptr<GraphicElement>(ElementFactory::buildElement(type)), target);
        };
        for (const auto &[inputs, outputs] : std::as_const(counts)) {
            const QString in = (type == ElementType::Demux) ? QString("derived") : QString::number(inputs);
            const QString header = QString("T %1 in=%2 out=%3")
                                       .arg(TestUtils::snapshotType(type), in, QString::number(outputs));
            lines << describeAll(scene, header, build, [inputCount = inputs, outputCount = outputs](GraphicElement *element) {
                TestUtils::applyPortCounts(element, inputCount, outputCount);
            });
        }
    }

    // An IC's definition takes the place of a port count, so the second order loads it into an IC already oriented,
    // as a reload of a changed file does.
    const TestUtils::ElementBuilder buildIc = [](Scene *target) { return placed(std::make_unique<IC>(), target); };
    const QFileInfo icFile(TestUtils::fullAdderIcFile());
    const auto loadIc = [&icFile](GraphicElement *element) {
        static_cast<IC *>(element)->loadFile(icFile.filePath(), icFile.absolutePath());
    };
    lines << describeAll(scene, "T IC file=" + icFile.fileName(), buildIc, loadIc);

    return lines.join('\n') + '\n';
}

} // namespace LabelSnapshot

void TestLabelSnapshot::testLabelGeometryMatchesGolden()
{
    const QString mismatch = TestUtils::compareWithGolden(LabelSnapshot::goldenPath(), LabelSnapshot::dumpLabels());
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}
