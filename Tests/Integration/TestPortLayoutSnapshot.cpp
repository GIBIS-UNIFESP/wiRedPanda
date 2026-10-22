// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestPortLayoutSnapshot.h"

#include <functional>
#include <memory>
#include <utility>

#include <QCoreApplication>
#include <QFileInfo>
#include <QMetaEnum>
#include <QTest>
#include <QTransform>
#include <QTranslator>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

namespace {

QString goldenPath()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/PortLayoutSnapshot/layouts.txt";
}

/// Tags every translated string, so a name that goes through QCoreApplication::translate() shows it.
class TaggingTranslator : public QTranslator
{
public:
    QString translate(const char *context, const char *sourceText, const char *disambiguation, int n) const override
    {
        Q_UNUSED(disambiguation)
        Q_UNUSED(n)
        return QString("[%1|%2]").arg(QString::fromUtf8(context), QString::fromUtf8(sourceText));
    }

    bool isEmpty() const override { return false; }
};

/// A requested port count; inputs is -1 where they follow the outputs (a Demux's select lines).
struct PortCounts {
    int inputs;
    int outputs;
};

/// Every legal port count of \a probe's type: each count from the minimum to the maximum, except a Mux's inputs, which
/// are what the element editor offers, 2 to 8 data inputs with their select lines.
QList<PortCounts> legalCounts(const GraphicElement &probe)
{
    QList<int> inputs;
    if (probe.elementType() == ElementType::Demux) {
        inputs << -1;
    } else if (probe.elementType() == ElementType::Mux) {
        for (int data = 2; data <= 8; ++data) {
            int selectLines = 1;
            while ((1 << selectLines) < data) {
                ++selectLines;
            }
            if ((data + selectLines >= probe.minInputSize()) && (data + selectLines <= probe.maxInputSize())) {
                inputs << data + selectLines;
            }
        }
    } else {
        for (int count = probe.minInputSize(); count <= probe.maxInputSize(); ++count) {
            inputs << count;
        }
    }

    QList<PortCounts> counts;
    for (const int in : std::as_const(inputs)) {
        for (int out = probe.minOutputSize(); out <= probe.maxOutputSize(); ++out) {
            counts.append({in, out});
        }
    }
    return counts;
}

QString portName(const Port *port)
{
    return QString(port->isInput() ? "i" : "o") + QString::number(port->index());
}

QString rectText(const QRectF &rect)
{
    return TestUtils::snapshotPoint(rect.topLeft()) + ',' + TestUtils::snapshotPoint({rect.width(), rect.height()});
}

/// What the layout produced in one orientation: the element, then each port. A port on an element that keeps its
/// graphic upright is turned by its transform.
QStringList describe(const GraphicElement *element)
{
    // A Text's bounds take in its text, whose width follows the platform's fonts, so only its body is written.
    const QString bounds = (element->elementType() == ElementType::Text)
                               ? rectText(element->GraphicElement::boundingRect()) + "+text"
                               : rectText(element->boundingRect());
    QStringList lines;
    lines << QString("  rot=%1 flipX=%2 flipY=%3 in=%4 out=%5 bounds=%6 pix=%7")
                 .arg(TestUtils::snapshotNumber(element->rotation()), QString::number(element->isFlippedX()),
                      QString::number(element->isFlippedY()), QString::number(element->inputSize()),
                      QString::number(element->outputSize()), bounds, SnapshotAccess::pixmapIdentity(element));
    for (const auto *port : element->allPorts()) {
        lines << QString("    %1 name=%2 pos=%3 scene=%4 xf=%5 required=%6 default=%7")
                     .arg(portName(port), TestUtils::snapshotQuoted(port->name()),
                          TestUtils::snapshotPoint(port->pos()), TestUtils::snapshotPoint(port->scenePos()),
                          TestUtils::snapshotTransform(port->transform()), QString::number(port->isRequired()),
                          TestUtils::snapshotStatus(port->defaultValue()));
    }
    return lines;
}

/// One configuration in the sixteen orientations, in both orders, then its port names under the tagging translator.
QStringList describeAll(Scene *scene, const QString &header, const TestUtils::ElementBuilder &build,
                        const std::function<void(GraphicElement *)> &applyPorts, TaggingTranslator &tagging)
{
    QStringList lines = TestUtils::describeInOrientations(scene, header, build, applyPorts, describe,
                                                          "  oriented first: same", "  oriented first:");

    QCoreApplication::installTranslator(&tagging);
    const auto tagged = build(scene);
    applyPorts(tagged.get());
    QStringList names;
    for (const auto *port : tagged->allPorts()) {
        names << portName(port) + '=' + TestUtils::snapshotQuoted(port->name());
    }
    scene->removeItem(tagged.get());
    QCoreApplication::removeTranslator(&tagging);
    lines << "  tagged names: " + names.join(' ');
    return lines;
}

QString dumpLayouts()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    TaggingTranslator tagging;
    QStringList lines;

    for (const ElementType type : TestUtils::dumpedTypes()) {
        if (!ElementFactory::hasCreator(type)) {
            lines << QString("%1 %2").arg(TestUtils::kNoCreator, TestUtils::snapshotType(type));
            continue;
        }

        const TestUtils::ElementBuilder build = [type](Scene *target) {
            std::unique_ptr<GraphicElement> element(ElementFactory::buildElement(type));
            target->addItem(element.get());
            return element;
        };
        const std::unique_ptr<GraphicElement> probe(ElementFactory::buildElement(type));
        for (const auto &counts : legalCounts(*probe)) {
            const QString inputs = (counts.inputs < 0) ? QString("derived") : QString::number(counts.inputs);
            const QString header = QString("T %1 in=%2 out=%3")
                                       .arg(TestUtils::snapshotType(type), inputs, QString::number(counts.outputs));
            lines << describeAll(scene, header, build, [counts](GraphicElement *element) {
                TestUtils::applyPortCounts(element, counts.inputs, counts.outputs);
            }, tagging);
        }
    }

    // An IC's definition takes the place of a port count, so the second order loads it into an IC already oriented,
    // as a reload of a changed file does.
    const TestUtils::ElementBuilder buildIc = [](Scene *target) {
        auto ic = std::make_unique<IC>();
        target->addItem(ic.get());
        return std::unique_ptr<GraphicElement>(std::move(ic));
    };
    const QFileInfo icFile(TestUtils::fullAdderIcFile());
    const auto loadIc = [&icFile](GraphicElement *element) {
        static_cast<IC *>(element)->loadFile(icFile.filePath(), icFile.absolutePath());
    };
    lines << describeAll(scene, "T IC file=" + icFile.fileName(), buildIc, loadIc, tagging);

    return lines.join('\n') + '\n';
}

} // namespace

void TestPortLayoutSnapshot::initTestCase()
{
    m_previousTheme = ThemeManager::theme();
    ThemeManager::setTheme(Theme::Light);
}

void TestPortLayoutSnapshot::cleanupTestCase()
{
    ThemeManager::setTheme(m_previousTheme);
}

void TestPortLayoutSnapshot::testLayoutMatchesGolden()
{
    const QString mismatch = TestUtils::compareWithGolden(goldenPath(), dumpLayouts());
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}
