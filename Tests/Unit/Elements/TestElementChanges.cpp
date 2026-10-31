// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementChanges.h"

#include <functional>
#include <utility>

#include <QBitArray>
#include <QDataStream>
#include <QFile>
#include <QHash>
#include <QImage>
#include <QKeySequence>
#include <QMetaEnum>
#include <QTemporaryDir>
#include <QTest>

#include "App/Core/Common.h"
#include "App/Element/BodyMeasure.h"
#include "App/Element/ElementChange.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInput.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/InputButton.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/Text.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Scene.h"
#include "App/Versions.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

namespace ElementChangesTestDetail {

/// Returns a report of \a what changed of element \a id as "<id> <changes>".
QString report(const int id, const ElementChanges what)
{
    return QStringLiteral("%1 %2").arg(id).arg(what.toInt());
}

/// Writes down each change a scene reports of an element.
class Recorder
{
public:
    /// Starts writing down what \a scene reports.
    explicit Recorder(Scene &scene)
    {
        QObject::connect(&scene, &Scene::elementChanged, &scene, [this](const int id, const ElementChanges what) {
            reports << std::pair{id, what};
        });
    }

    /// Returns the reports, in order, as report() writes them.
    QStringList written() const
    {
        QStringList lines;
        for (const auto &[id, what] : reports) {
            lines << report(id, what);
        }
        return lines;
    }

    /// Returns what the reports about element \a id add up to.
    ElementChanges reportedFor(const int id) const
    {
        ElementChanges all;
        for (const auto &[reportedId, what] : reports) {
            if (reportedId == id) {
                all |= what;
            }
        }
        return all;
    }

    /// Returns how many reports were about element \a id.
    qsizetype countFor(const int id) const
    {
        qsizetype count = 0;
        for (const auto &entry : reports) {
            count += (entry.first == id) ? 1 : 0;
        }
        return count;
    }

    QList<std::pair<int, ElementChanges>> reports;
};

/// A change the element editor, a command or a file can make to an element, and what it is reported as at least.
struct Operation {
    QString name;
    ElementChanges reportedAs;
    /// Makes the change to \a element; returns false, having changed nothing, when the element offers no such change.
    std::function<bool(Element &element, Scene &scene)> apply;
};

/// The sub-circuit every IC starts with.
QString icFile()
{
    return TestUtils::cpuComponentsDir() + "level1_d_latch.panda";
}

/// The sub-circuit the IC operation loads in its place.
QString otherIcFile()
{
    return TestUtils::cpuComponentsDir() + "level1_sr_latch.panda";
}

/// Returns a new element of \a type, in no scene; an IC holds a sub-circuit, as one in a circuit does.
Element *build(const ElementType type)
{
    auto *element = ElementFactory::buildElement(type);
    if (auto *ic = qobject_cast<IC *>(element)) {
        ic->loadFile(icFile(), TestUtils::cpuComponentsDir());
    }
    return element;
}

/// Writes a picture of \a size to \a path, for a custom appearance, and returns the path.
QString writeImage(const QString &path, const QSize size)
{
    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::red);
    return image.save(path) ? path : QString();
}

/// Returns the port count after \a current within [\a min, \a max]: one more, or one fewer at the maximum.
int otherCount(const int current, const int min, const int max)
{
    return (current < max) ? current + 1 : qMax(min, current - 1);
}

/// Returns \a element saved as an undo snapshot saves it.
QByteArray snapshot(const Element &element)
{
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    element.save(out, {.purpose = SerializationPurpose::InMemorySnapshot});
    return bytes;
}

/// Loads \a bytes into \a element as the undo of a property edit does.
void load(Element &element, Scene &scene, const QByteArray &bytes)
{
    QDataStream in(bytes);
    QHash<quint64, Port *> portMap;
    auto context = scene.deserializationContext(portMap, FormatRev::current, SerializationPurpose::InMemorySnapshot);
    element.load(in, context);
}

/// Every change an element can take, each to apply to a fresh element; \a image is a custom appearance, \a audio a
/// sound other than the audio box's default.
QList<Operation> operations(const QString &image, const QString &audio)
{
    using Change = ElementChange;
    return {
        {"move", Change::Placement, [](Element &element, Scene &) {
             element.setPos(element.pos() + QPointF(64, 32));
             return true;
         }},
        {"rotate", Change::Placement, [](Element &element, Scene &) {
             element.setRotation(90);
             return true;
         }},
        {"flip horizontally", Change::Placement, [](Element &element, Scene &) {
             element.setFlippedX(true);
             return true;
         }},
        {"flip vertically", Change::Placement, [](Element &element, Scene &) {
             element.setFlippedY(true);
             return true;
         }},
        {"label", Change::Label, [](Element &element, Scene &) {
             if (!element.hasLabel()) {
                 return false;
             }
             element.setLabel(QStringLiteral("Changed"));
             return true;
         }},
        {"trigger", Change::Parameter | Change::Label, [](Element &element, Scene &) {
             if (!element.hasTrigger()) {
                 return false;
             }
             element.setTrigger(QKeySequence(QStringLiteral("B")));
             return true;
         }},
        {"colour", Change::Parameter, [](Element &element, Scene &) {
             if (!element.hasColors()) {
                 return false;
             }
             element.setColor(element.nextColor());
             return true;
         }},
        {"frequency", Change::Parameter, [](Element &element, Scene &) {
             if (!element.hasFrequency()) {
                 return false;
             }
             element.setFrequency(element.frequency() * 2 + 1);
             return true;
         }},
        {"delay", Change::Parameter, [](Element &element, Scene &) {
             if (!element.hasDelay()) {
                 return false;
             }
             element.setDelay(0.25);
             return true;
         }},
        {"audio file", Change::Parameter, [audio](Element &element, Scene &) {
             if (!element.hasAudioBox()) {
                 return false;
             }
             element.setAudio(audio);
             return true;
         }},
        {"volume", Change::Parameter, [](Element &element, Scene &) {
             if (!element.hasVolume()) {
                 return false;
             }
             element.setVolume(0.25F);
             return true;
         }},
        {"appearance", Change::Parameter, [image](Element &element, Scene &) {
             if (!element.canChangeAppearance()) {
                 return false;
             }
             element.setAppearance(false, image);
             return true;
         }},
        {"default appearance", Change::Parameter, [](Element &element, Scene &) {
             if (!element.canChangeAppearance()) {
                 return false;
             }
             element.setAppearance(true, {});
             return true;
         }},
        {"appearance of a state", Change::Parameter, [image](Element &element, Scene &) {
             if (!element.canChangeAppearance()) {
                 return false;
             }
             element.setAppearanceAt(0, image);
             return true;
         }},
        {"input count", Change::Ports, [](Element &element, Scene &) {
             // A demultiplexer's inputs follow its outputs, so setting them changes nothing.
             const int before = element.inputSize();
             element.setInputSize(otherCount(before, element.minInputSize(), element.maxInputSize()));
             return element.inputSize() != before;
         }},
        {"output count", Change::Ports, [](Element &element, Scene &) {
             const int before = element.outputSize();
             element.setOutputSize(otherCount(before, element.minOutputSize(), element.maxOutputSize()));
             return element.outputSize() != before;
         }},
        {"lock", Change::Parameter, [](Element &element, Scene &) {
             auto *input = qobject_cast<ElementInput *>(&element);
             if (!input) {
                 return false;
             }
             input->setLocked(!input->isLocked());
             return true;
         }},
        {"level", Change::Parameter, [](Element &element, Scene &) {
             // A switch and a rotary switch save where they are set; a button's press and a clock's tick are momentary.
             if (auto *inputSwitch = qobject_cast<InputSwitch *>(&element)) {
                 inputSwitch->setOn(!inputSwitch->isOn());
                 return true;
             }
             if (auto *rotary = qobject_cast<InputRotary *>(&element)) {
                 rotary->setOn(true, 1);
                 return true;
             }
             return false;
         }},
        {"truth table bit", Change::Parameter, [](Element &element, Scene &) {
             auto *truthTable = qobject_cast<TruthTable *>(&element);
             if (!truthTable) {
                 return false;
             }
             truthTable->setKeyBit(0, !truthTable->key().testBit(0));
             return true;
         }},
        {"truth table", Change::Parameter, [](Element &element, Scene &) {
             auto *truthTable = qobject_cast<TruthTable *>(&element);
             if (!truthTable) {
                 return false;
             }
             QBitArray key = truthTable->key();
             key.setBit(1, !key.testBit(1));
             truthTable->setkey(key);
             return true;
         }},
        {"wireless mode", Change::Parameter | Change::Ports, [](Element &element, Scene &) {
             auto *node = qobject_cast<Node *>(&element);
             if (!node) {
                 return false;
             }
             node->setWirelessMode(WirelessMode::Tx);
             return true;
         }},
        {"sub-circuit", Change::Parameter | Change::Ports, [](Element &element, Scene &) {
             auto *ic = qobject_cast<IC *>(&element);
             if (!ic) {
                 return false;
             }
             ic->loadFile(otherIcFile(), TestUtils::cpuComponentsDir());
             return true;
         }},
        {"blob name", Change::Parameter, [](Element &element, Scene &) {
             auto *ic = qobject_cast<IC *>(&element);
             if (!ic) {
                 return false;
             }
             ic->setBlobName(QStringLiteral("embedded"));
             return true;
         }},
        {"load", Change::Placement | Change::Parameter | Change::Label | Change::Ports, [](Element &element, Scene &scene) {
             load(element, scene, snapshot(element));
             return true;
         }},
    };
}

/// Returns "<type>: <what>", to say where a check failed.
QString where(const Element &element, const QString &what)
{
    return QStringLiteral("%1: %2").arg(ElementFactory::typeToText(element.elementType()), what);
}

/// Returns the bounds Qt is given and the measured ones, to say how they differ.
QString bounds(const Element &element)
{
    const auto text = [](const QRectF &rect) {
        return QStringLiteral("(%1, %2, %3 x %4)").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
    };
    return QStringLiteral("bounds %1, measured %2").arg(text(element.boundingRect()), text(element.measureBoundingRect()));
}

} // namespace ElementChangesTestDetail

// --- Coverage ---

void TestElementChanges::testEveryChangeOfEveryTypeIsReported()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString image = ElementChangesTestDetail::writeImage(dir.filePath("appearance.png"), QSize(96, 80));
    QVERIFY(!image.isEmpty());
    const QString audio = dir.filePath("other.wav");
    QVERIFY(QFile::copy(QStringLiteral(":/Components/Output/Audio/wiredpanda.wav"), audio));
    const auto operations = ElementChangesTestDetail::operations(image, audio);
    QHash<QString, int> applied;

    const QMetaEnum types = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < types.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(types.value(i));
        if ((type == ElementType::Unknown) || !ElementFactory::hasCreator(type)) {
            continue;
        }

        Scene scene;
        ElementChangesTestDetail::Recorder recorder(scene);

        for (const auto &operation : operations) {
            // Each change is made to a fresh element, in a scene as the editor's are.
            auto *element = ElementChangesTestDetail::build(type);
            scene.addItem(element);
            QVERIFY(element->boundingRect() == element->measureBoundingRect());
            recorder.reports.clear();

            const QString where = ElementChangesTestDetail::where(*element, operation.name);
            if (operation.apply(*element, scene)) {
                ++applied[operation.name];
                const ElementChanges reported = recorder.reportedFor(element->id());
                QVERIFY2(reported.testFlags(operation.reportedAs),
                         qPrintable(QStringLiteral("%1 was reported as %2, not as %3")
                                        .arg(where).arg(reported.toInt()).arg(operation.reportedAs.toInt())));
                QVERIFY2(element->boundingRect() == element->measureBoundingRect(),
                         qPrintable(QStringLiteral("%1: %2").arg(where, ElementChangesTestDetail::bounds(*element))));
                // The body the element keeps is the one it has now.
                QVERIFY2(element->body() == BodyMeasure::installed().measure(*element), qPrintable(where + ": body"));
            }
            delete element;
        }
    }

    // No operation went unexercised.
    for (const auto &operation : operations) {
        QVERIFY2(applied.value(operation.name) > 0, qPrintable(operation.name + " applies to no type"));
    }
}

// --- Reporting ---

void TestElementChanges::testEachChangeIsReportedOnce()
{
    Scene scene;
    ElementChangesTestDetail::Recorder recorder(scene);
    auto *gate = new And;
    auto *clock = new Clock;
    auto *demux = new Demux;
    scene.addItem(gate);
    scene.addItem(clock);
    scene.addItem(demux);
    const int gateId = gate->id();
    const int clockId = clock->id();
    const int demuxId = demux->id();

    // A rotatable element turns its picture and its transform: one report.
    recorder.reports.clear();
    gate->setRotation(90);
    QCOMPARE(recorder.countFor(gateId), 1);
    QVERIFY(recorder.reportedFor(gateId).testFlag(ElementChange::Placement));

    // An element whose picture stays upright turns its ports: one report.
    recorder.reports.clear();
    clock->setRotation(90);
    QCOMPARE(recorder.countFor(clockId), 1);
    QVERIFY(recorder.reportedFor(clockId).testFlag(ElementChange::Placement));

    // A port count lays every port out again: one report, of the ports.
    recorder.reports.clear();
    gate->setInputSize(5);
    QCOMPARE(recorder.countFor(gateId), 1);
    QVERIFY(recorder.reportedFor(gateId).testFlag(ElementChange::Ports));

    // A demultiplexer's outputs bring the select lines with them: one report.
    recorder.reports.clear();
    demux->setOutputSize(4);
    QCOMPARE(recorder.countFor(demuxId), 1);
    QVERIFY(recorder.reportedFor(demuxId).testFlag(ElementChange::Ports));

    // A load sets everything a file holds: one report.
    recorder.reports.clear();
    ElementChangesTestDetail::load(*gate, scene, ElementChangesTestDetail::snapshot(*gate));
    QCOMPARE(recorder.countFor(gateId), 1);

    // A label below a gate is not part of its bounds: the report is of the label alone.
    recorder.reports.clear();
    gate->setLabel(QStringLiteral("Gate"));
    QCOMPARE(recorder.written(), QStringList{ElementChangesTestDetail::report(gateId, ElementChange::Label)});

    // A move is of the placement alone.
    recorder.reports.clear();
    gate->setPos(gate->pos() + QPointF(32, 0));
    QCOMPARE(recorder.written(), QStringList{ElementChangesTestDetail::report(gateId, ElementChange::Placement)});

    // Nothing of the other elements was reported.
    QCOMPARE(recorder.countFor(clockId), 0);
    QCOMPARE(recorder.countFor(demuxId), 0);
}

void TestElementChanges::testBoundsThatMoveAreReportedAsBody()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString image = ElementChangesTestDetail::writeImage(dir.filePath("large.png"), QSize(96, 80));
    QVERIFY(!image.isEmpty());

    Scene scene;
    ElementChangesTestDetail::Recorder recorder(scene);
    auto *text = new Text;
    auto *input = new InputSwitch;
    auto *demux = new Demux;
    scene.addItem(text);
    scene.addItem(input);
    scene.addItem(demux);

    // A text's label is part of its bounds.
    recorder.reports.clear();
    QRectF before = text->boundingRect();
    text->setLabel(QStringLiteral("A label long enough to be wider than the hint it replaces"));
    QCOMPARE(recorder.reportedFor(text->id()).toInt(), (ElementChange::Label | ElementChange::Body).toInt());
    QVERIFY(text->boundingRect() != before);
    QVERIFY(text->boundingRect() == text->measureBoundingRect());

    // A larger custom appearance grows the picture.
    recorder.reports.clear();
    before = input->boundingRect();
    input->setAppearance(false, image);
    QCOMPARE(recorder.reportedFor(input->id()).toInt(), (ElementChange::Parameter | ElementChange::Body).toInt());
    QVERIFY(input->boundingRect() != before);
    QVERIFY(input->boundingRect().contains(QRectF(0, 0, 96, 80)));
    QVERIFY(input->boundingRect() == input->measureBoundingRect());

    // More outputs make a demultiplexer's body taller.
    recorder.reports.clear();
    before = demux->boundingRect();
    demux->setOutputSize(8);
    QVERIFY(recorder.reportedFor(demux->id()).testFlags(ElementChange::Ports | ElementChange::Body));
    QVERIFY(demux->boundingRect().height() > before.height());
    QVERIFY(demux->boundingRect() == demux->measureBoundingRect());
}

void TestElementChanges::testThePictureAloneIsNotReported()
{
    Scene scene;
    ElementChangesTestDetail::Recorder recorder(scene);

    // A theme and a refresh redraw the picture; a picture of the same size moves no bounds, so nothing is reported.
    const QMetaEnum types = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < types.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(types.value(i));
        if ((type == ElementType::Unknown) || !ElementFactory::hasCreator(type)) {
            continue;
        }
        auto *element = ElementChangesTestDetail::build(type);
        scene.addItem(element);
        recorder.reports.clear();

        element->updateTheme();
        element->refresh();
        const QString where = ElementChangesTestDetail::where(*element, QStringLiteral("theme and refresh"));
        QVERIFY2(recorder.countFor(element->id()) == 0, qPrintable(where));
        QVERIFY2(element->boundingRect() == element->measureBoundingRect(),
                 qPrintable(QStringLiteral("%1: %2").arg(where, ElementChangesTestDetail::bounds(*element))));
    }

    // A button's press and a clock's tick are momentary: neither is saved, and neither is reported.
    auto *button = new InputButton;
    auto *clock = new Clock;
    scene.addItem(button);
    scene.addItem(clock);
    recorder.reports.clear();
    button->setOn(true);
    button->setOn(false);
    clock->setOn(true);
    clock->setOn(false);
    QCOMPARE(recorder.countFor(button->id()), 0);
    QCOMPARE(recorder.countFor(clock->id()), 0);
}

void TestElementChanges::testAChangeThatThrowsIsStillReported()
{
    Scene scene;
    ElementChangesTestDetail::Recorder recorder(scene);
    auto *gate = new And;
    scene.addItem(gate);
    const int gateId = gate->id();

    // A snapshot cut short is read up to where it ends: what was read before the throw changed the element.
    QByteArray bytes = ElementChangesTestDetail::snapshot(*gate);
    bytes.chop(bytes.size() / 2);
    recorder.reports.clear();
    bool threw = false;
    try {
        ElementChangesTestDetail::load(*gate, scene, bytes);
    } catch (const Pandaception &) {
        threw = true;
    }
    QVERIFY(threw);
    QCOMPARE(recorder.countFor(gateId), 1);
    QVERIFY(gate->boundingRect() == gate->measureBoundingRect());
}

// --- Bounds ---

void TestElementChanges::testElementsOutOfASceneMeasureTheirBoundsLive()
{
    const QString longLabel = QStringLiteral("A label long enough to be wider than the hint it replaces");

    // Out of a scene, the bounds are measured whenever they are asked for.
    auto *text = new Text;
    const QRectF unlabelled = text->boundingRect();
    text->setLabel(longLabel);
    QVERIFY(text->boundingRect() != unlabelled);
    QVERIFY(text->boundingRect() == text->measureBoundingRect());

    // In a scene they are the ones measured as it entered, brought up to date by each change.
    Scene scene;
    scene.addItem(text);
    QVERIFY(text->boundingRect() == text->measureBoundingRect());
    text->setLabel({});
    QVERIFY(text->boundingRect() == unlabelled);

    // Out of it again, they are measured live again.
    scene.removeItem(text);
    text->setLabel(longLabel);
    QVERIFY(text->boundingRect() != unlabelled);
    QVERIFY(text->boundingRect() == text->measureBoundingRect());
    delete text;

    // A port moved behind the element's back shows the difference: in a scene the bounds Qt has hold still until a
    // change tells Qt, out of one they follow at once.
    auto *gate = new And;
    scene.addItem(gate);
    const QRectF held = gate->boundingRect();
    gate->inputPort(0)->setPos(gate->inputPort(0)->pos() - QPointF(48, 0));
    QCOMPARE(gate->boundingRect(), held);
    QVERIFY(gate->measureBoundingRect() != held);
    gate->updatePortsProperties();
    QVERIFY(gate->boundingRect() == gate->measureBoundingRect());
    scene.removeItem(gate);
    gate->inputPort(0)->setPos(gate->inputPort(0)->pos() - QPointF(48, 0));
    QVERIFY(gate->boundingRect() == gate->measureBoundingRect());
    QVERIFY(gate->boundingRect() != held);
    delete gate;
}
