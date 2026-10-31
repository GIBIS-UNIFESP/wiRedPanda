// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElement.h"

#include <cmath>
#include <memory>
#include <optional>

#include <QCoreApplication>
#include <QDirIterator>
#include <QFile>
#include <QFontDatabase>
#include <QFontInfo>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSimpleTextItem>
#include <QLineEdit>
#include <QMetaEnum>
#include <QPainter>
#include <QPixmap>
#include <QScopeGuard>
#include <QSet>
#include <QStyleOptionGraphicsItem>
#include <QUndoStack>
#include <QXmlStreamReader>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display16.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Line.h"
#include "App/Element/GraphicElements/Text.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

namespace {

/// Sends a real double-click mouse event to \a scene at \a scenePos, exactly as the view would.
void sendDoubleClick(Scene *scene, const QPointF &scenePos)
{
    QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMouseDoubleClick);
    event.setScenePos(scenePos);
    event.setButton(Qt::LeftButton);
    event.setButtons(Qt::LeftButton);
    QCoreApplication::sendEvent(scene, &event);
}

/// Finds the QLineEdit hosted by InlineLabelEditor's QGraphicsProxyWidget, if one is active.
QLineEdit *findInlineEditor(Scene *scene)
{
    for (auto *item : scene->items()) {
        if (auto *proxy = qgraphicsitem_cast<QGraphicsProxyWidget *>(item)) {
            if (auto *lineEdit = qobject_cast<QLineEdit *>(proxy->widget())) {
                return lineEdit;
            }
        }
    }
    return nullptr;
}

} // namespace

void TestElement::testElementPaintSelection()
{
    And andGate;
    andGate.setSelected(true);
    QVERIFY(andGate.isSelected());

    andGate.setSelected(false);
    QVERIFY(!andGate.isSelected());
}

void TestElement::testElementSaveLoad()
{
    And gate;
    gate.setRotation(90);

    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    gate.save(writeStream, {.purpose = SerializationPurpose::PortableFile});
    QVERIFY(!data.isEmpty());
}

void TestElement::testElementSkin()
{
    Led led;
    QVERIFY(led.hasColors());
}

void TestElement::testElementAppearance()
{
    Led led;
    led.setColor("Red");
    QCOMPARE(led.color(), QString("Red"));
}

void TestElement::testElementHelpTextIsItsName()
{
    // An element's help text is its type's name, here in English with no translator loaded; it carries no tooltip or
    // object name of its own.
    And andGate;
    QCOMPARE(andGate.helpText(), QStringLiteral("And"));
    QCOMPARE(andGate.translatedName(), QStringLiteral("And"));
    QVERIFY(andGate.toolTip().isEmpty());
    QVERIFY(andGate.objectName().isEmpty());
}

void TestElement::testElementDoubleClick()
{
    And andGate;
    QCOMPARE(andGate.elementType(), ElementType::And);
}

void TestElement::testTextElement()
{
    WorkSpace workspace;
    auto *text = new Text;
    workspace.scene()->addItem(text);
    QCOMPARE(text->elementType(), ElementType::Text);
    QVERIFY(!text->boundingRect().isNull());
}

void TestElement::testLineElement()
{
    WorkSpace workspace;
    auto *line = new Line;
    workspace.scene()->addItem(line);
    QCOMPARE(line->elementType(), ElementType::Line);
    QVERIFY(!line->boundingRect().isNull());
}

void TestElement::testDisplay14Paint()
{
    WorkSpace workspace;
    auto *disp = new Display14;
    workspace.scene()->addItem(disp);
    disp->setColor("Green");

    // Set some segments to Active to exercise paint() branches
    for (int i = 0; i < disp->inputSize(); ++i) {
        disp->inputPort(i)->setStatus(Status::Active);
    }
    disp->refresh();

    // Render to pixmap to trigger paint()
    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    disp->paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY2(TestUtils::pixmapHasInk(pixmap), "Display14 paint() must draw visible pixels");
}

void TestElement::testDisplay16Paint()
{
    WorkSpace workspace;
    auto *disp = new Display16;
    workspace.scene()->addItem(disp);
    disp->setColor("Blue");

    for (int i = 0; i < disp->inputSize(); ++i) {
        disp->inputPort(i)->setStatus(Status::Active);
    }
    disp->refresh();

    QPixmap pixmap(128, 128);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    disp->paint(&painter, &option, nullptr);
    painter.end();

    QVERIFY2(TestUtils::pixmapHasInk(pixmap), "Display16 paint() must draw visible pixels");
}

void TestElement::testInlineDoubleClickRenamesLabelWithUndo()
{
    WorkSpace workspace;
    auto *led = new Led;
    led->setLabel("Original");
    workspace.scene()->addItem(led);

    sendDoubleClick(workspace.scene(), led->mapToScene(led->boundingRect().center()));

    QLineEdit *editor = findInlineEditor(workspace.scene());
    QVERIFY2(editor, "Double-clicking a labelable element must spawn an inline QLineEdit");
    QCOMPARE(editor->text(), QString("Original"));

    editor->setText("Renamed");
    QTest::keyClick(editor, Qt::Key_Return);

    QCOMPARE(led->label(), QString("Renamed"));
    QCOMPARE(workspace.scene()->undoStack()->count(), 1);

    workspace.scene()->undoStack()->undo();
    QCOMPARE(led->label(), QString("Original"));

    workspace.scene()->undoStack()->redo();
    QCOMPARE(led->label(), QString("Renamed"));
}

void TestElement::testInlineDoubleClickEscapeCancelsWithoutUndo()
{
    WorkSpace workspace;
    auto *led = new Led;
    led->setLabel("Original");
    workspace.scene()->addItem(led);

    sendDoubleClick(workspace.scene(), led->mapToScene(led->boundingRect().center()));

    QLineEdit *editor = findInlineEditor(workspace.scene());
    QVERIFY(editor);

    editor->setText("Should not stick");
    QTest::keyClick(editor, Qt::Key_Escape);

    QCOMPARE(led->label(), QString("Original"));
    QCOMPARE(workspace.scene()->undoStack()->count(), 0);
}

void TestElement::testInlineDoubleClickIgnoredWithoutLabel()
{
    // And has no hasLabel() support -- double-click must fall through to default Qt
    // behavior and must not spawn an inline editor.
    WorkSpace workspace;
    auto *gate = new And;
    QVERIFY(!gate->hasLabel());
    workspace.scene()->addItem(gate);

    sendDoubleClick(workspace.scene(), gate->mapToScene(gate->boundingRect().center()));

    QVERIFY(!findInlineEditor(workspace.scene()));
}

namespace TestElementDetail {

/// Whether \a text's own painting, without its child items (the label), draws below its 64x64 picture, where the
/// empty-state hint goes.
bool drawsHint(Text *text)
{
    QPixmap pixmap(400, 160);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QStyleOptionGraphicsItem option;
    text->paint(&painter, &option, nullptr);
    painter.end();
    return TestUtils::pixmapHasInk(pixmap.copy(0, 64, 400, 96));
}

} // namespace TestElementDetail

void TestElement::testTextEmptyStateHintTogglesWithLabelContent()
{
    using TestElementDetail::drawsHint;
    WorkSpace workspace;
    auto *text = new Text;
    workspace.scene()->addItem(text);

    // A fresh Text starts empty, so the hint must be drawn immediately.
    QVERIFY(drawsHint(text));

    text->setLabel("Now has content");
    QVERIFY(!drawsHint(text));

    text->setLabel(""); // clearing it back out must draw the hint again
    QVERIFY(drawsHint(text));

    text->setLabel("Second time");
    QVERIFY(!drawsHint(text));
}

void TestElement::testTextEmptyStateHintFollowsALanguageChange()
{
    // The hint is part of the body, whose width follows the hint's in the current language
    WorkSpace workspace;
    auto *text = new Text;
    workspace.scene()->addItem(text);
    const qreal english = text->body().body.width();

    // The main window has each scene retranslate at a language change, and the scene tells its elements.
    TestUtils::TagEveryStringTranslator tagging;
    QCoreApplication::installTranslator(&tagging);
    auto removeTagging = qScopeGuard([&tagging] { QCoreApplication::removeTranslator(&tagging); });
    workspace.scene()->retranslateUi();
    QVERIFY2(text->body().body.width() > english, "the tagged hint is longer");
    QCOMPARE(text->boundingRect(), text->measureBoundingRect());

    removeTagging.dismiss();
    QCoreApplication::removeTranslator(&tagging);
    workspace.scene()->retranslateUi();
    QCOMPARE(text->body().body.width(), english);
}

void TestElement::testLabelFontIsAFamilyThePlatformHas()
{
    // A label asks for a family the platform has under that name: one it lacks is matched through Qt's table of family
    // aliases, which Qt builds on first use (costly on macOS, where "Sans Serif" is no family)
    And gate;
    gate.setLabel("label");
    QFont font;
    for (auto *child : gate.childItems()) {
        if (auto *text = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(child); text && (text->text() == "label")) {
            font = text->font();
        }
    }
    QVERIFY2(QFontDatabase::families().contains(font.family()), qPrintable(font.family()));
    QCOMPARE(QFontInfo(font).family(), font.family());
    QVERIFY(font.bold());
}

void TestElement::testBundledPicturesDrawTheirTextAsOutlines()
{
    // A bundled picture's text is drawn as outlines: Qt SVG reads a font-family such as "sans-serif" as one family's
    // name, which no platform has, so the text took whatever face Qt fell back to and made Qt build its table of family
    // aliases, as a label's missing family did
    int pictures = 0;
    QDirIterator it(":/", {"*.svg"}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = it.next();
        QFile file(path);
        QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(path));
        QXmlStreamReader xml(&file);
        while (!xml.atEnd()) {
            if (xml.readNext() == QXmlStreamReader::StartElement) {
                const auto name = xml.name();
                QVERIFY2((name != u"text") && (name != u"tspan") && (name != u"textPath") && (name != u"flowRoot"),
                         qPrintable(path));
            }
        }
        QVERIFY2(!xml.hasError(), qPrintable(path + ": " + xml.errorString()));
        ++pictures;
    }
    QVERIFY(pictures > 100);
}

void TestElement::testSceneEventSwallowsCtrlClickPress()
{
    // Element::sceneEvent() swallows Ctrl+click presses so the scene's rubber-band
    // selection logic can handle them, instead of the element intercepting the press and
    // starting its own move. Called directly (via friendship) rather than through a real
    // QTest::mousePress(..., Qt::ControlModifier, ...): this sandbox's offscreen/X11 setup does
    // not reliably reproduce the requested keyboard modifier on the resulting scene event (a
    // real Shift-modifier request was observed arriving as Control at the scene level), making
    // that route non-deterministic for this specific guard.
    And gate;

    QGraphicsSceneMouseEvent pressEvent(QEvent::GraphicsSceneMousePress);
    pressEvent.setButton(Qt::LeftButton);
    pressEvent.setButtons(Qt::LeftButton);
    pressEvent.setModifiers(Qt::ControlModifier);
    QVERIFY2(gate.sceneEvent(&pressEvent), "Ctrl+click press must be swallowed (return true)");

    QGraphicsSceneMouseEvent releaseEvent(QEvent::GraphicsSceneMouseRelease);
    releaseEvent.setButton(Qt::LeftButton);
    releaseEvent.setModifiers(Qt::ControlModifier);
    QVERIFY2(gate.sceneEvent(&releaseEvent), "Ctrl+click release must also be swallowed");
}

void TestElement::testColorCycleFunctionsDefaultToWhiteForNonColoredElement()
{
    // The base Element::color() always returns an empty string; previousColor()/
    // nextColor() must fall back to "White" (not an empty/garbage result) for an element type
    // that never overrides color(), e.g. And.
    And gate;
    QCOMPARE(gate.color(), QString());
    QCOMPARE(gate.previousColor(), QString("White"));
    QCOMPARE(gate.nextColor(), QString("White"));
}

void TestElement::testColorNameToIndexUnknownColorDefaultsToZero()
{
    QCOMPARE(Element::colorNameToIndex("NotARealColor"), 0);
}

void TestElement::testIsValidPropagatesErrorStatusToConnectedOutputs()
{
    // An element with an unconnected, required input is invalid; isValid() must then mark every
    // connection on its outputs -- and the port at the connection's far end -- Status::Error, so
    // the visual chain shows where validity breaks.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *gate = new And;
    auto *led = new Led;
    builder.add(gate, led);
    builder.connect(gate, 0, led, 0);

    QVERIFY(!gate->isValid()); // both inputs are unconnected and required by default

    Connection *conn = gate->outputPort(0)->connections().constFirst();
    QCOMPARE(conn->status(), Status::Error);
    QCOMPARE(led->inputPort(0)->status(), Status::Error);
}

void TestElement::testBaseSettersAreNoOpsForUnsupportedProperties()
{
    // Element's default setColor()/setAudio()/setVolume()/setFrequency()/setDelay() are
    // no-ops for element types that don't override them -- confirm the matching getters stay at
    // their fixed defaults afterward.
    And gate;
    gate.setColor("Red");
    QCOMPARE(gate.color(), QString());

    gate.setAudio("some/path.wav");
    QCOMPARE(gate.audio(), QString());

    gate.setVolume(0.5f);
    QCOMPARE(gate.volume(), 0.0f);

    gate.setFrequency(42.0);
    QCOMPARE(gate.frequency(), 0.0);

    gate.setDelay(42.0);
    QCOMPARE(gate.delay(), 0.0);
}

void TestElement::testBlobNameDefaultsToEmptyForNonIcElement()
{
    And gate;
    QVERIFY(gate.blobName().isEmpty());
}

void TestElement::testBodiesDrawnInCodeOfferNoAppearance()
{
    // A body drawn in code is sized by its ports and never shows a custom appearance, so its type offers none
    QStringList drawnInCode;
    const QMetaEnum types = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < types.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(types.value(i));
        if ((type == ElementType::Unknown) || !ElementFactory::hasCreator(type)) {
            continue;
        }
        const std::unique_ptr<Element> element(ElementFactory::buildElement(type));
        if (SnapshotAccess::pixmapIdentity(element.get()) == "footprint") {
            drawnInCode << ElementFactory::typeToText(type);
            QVERIFY2(!element->canChangeAppearance(), qPrintable(ElementFactory::typeToText(type)));
        }
    }
    drawnInCode.sort();
    QCOMPARE(drawnInCode, QStringList({"Demux", "IC", "Mux", "TruthTable"}));
}

void TestElement::testTheNameFollowsTheLanguage()
{
    // Made before a translator is installed, the element names itself in the language current when asked.
    And gate;
    TestUtils::TagEveryStringTranslator tagging;
    QCoreApplication::installTranslator(&tagging);
    const QString tagged = gate.translatedName();
    const QString taggedHelp = gate.helpText();
    QCoreApplication::removeTranslator(&tagging);

    QCOMPARE(tagged, QStringLiteral("[tr And]"));
    QCOMPARE(taggedHelp, tagged);
    QCOMPARE(gate.translatedName(), QStringLiteral("And"));
}

void TestElement::testSnapToGridKeepsAPositionBeyondIntRange()
{
    // A file may hold any finite position, and every move, paste and drop snaps it: a position past int's range snaps
    // in floating point, a position on the grid unchanged.
    const QPointF snapped = Element::snapToGrid(QPointF(-5e153, 1e10), 8);
    QCOMPARE(snapped.y(), 1e10);
    QVERIFY(qFuzzyCompare(snapped.x(), -5e153));

    // Positions within int's range snap as they always have, halves away from zero
    QCOMPARE(Element::snapToGrid(QPointF(12, -12), 8), QPointF(16, -16));
    QCOMPARE(Element::snapToGrid(QPointF(11.9, -3.9), 8), QPointF(8, 0));

    // A small negative value snaps to +0, as an int did, not -0, which equals it but saves as other bytes
    const QPointF nearOrigin = Element::snapToGrid(QPointF(-3.9, -0.5), 8);
    QVERIFY(!std::signbit(nearOrigin.x()));
    QVERIFY(!std::signbit(nearOrigin.y()));
}

void TestElement::testPortUpdateKeepsFieldsAttributesLeaveUnset()
{
    // A field an element's attributes leave unset keeps what the port holds, so a value set elsewhere (a wireless
    // node's requirement, set by its mode) survives an update. Each port of every type, at every legal port count, is
    // given a value of its own in every field its attributes leave unset, and must still hold it after the update.
    struct Seeded {
        Port *port;
        QString where;
        std::optional<bool> required;
        std::optional<Status> defaultStatus;
        std::optional<QString> name;
    };

    QStringList changed;
    int seededBesideAttributes = 0;
    int seededWithoutAttributes = 0;
    for (const auto type : TestUtils::dumpedTypes()) {
        if (type == ElementType::IC) {
            continue;
        }
        const std::unique_ptr<Element> probe(ElementFactory::buildElement(type));
        for (int inputs = probe->minInputSize(); inputs <= probe->maxInputSize(); ++inputs) {
            for (int outputs = probe->minOutputSize(); outputs <= probe->maxOutputSize(); ++outputs) {
                const std::unique_ptr<Element> element(ElementFactory::buildElement(type));
                TestUtils::applyPortCounts(element.get(), inputs, outputs);
                if ((element->inputSize() != inputs) || (element->outputSize() != outputs)) {
                    continue; // A Demux derives its inputs from its outputs
                }
                const QString counts = QString("%1 in=%2 out=%3").arg(ElementFactory::typeToText(type)).arg(inputs).arg(outputs);

                QList<Seeded> seeded;
                const auto seed = [&](Port *port, const QString &where, const std::optional<Element::PortAttributes> &attributes) {
                    Seeded entry{port, counts + ' ' + where, {}, {}, {}};
                    if (!attributes || !attributes->required) {
                        entry.required = !port->isRequired();
                        port->setRequired(*entry.required);
                    }
                    if (!attributes || !attributes->defaultStatus) {
                        entry.defaultStatus = (port->defaultValue() == Status::Active) ? Status::Inactive : Status::Active;
                        port->setDefaultStatus(*entry.defaultStatus);
                    }
                    if (!attributes || (!attributes->name && !attributes->derivedName)) {
                        entry.name = "seeded " + where;
                        port->setName(*entry.name);
                    }
                    if (attributes && (entry.required || entry.defaultStatus || entry.name)) {
                        ++seededBesideAttributes;
                    } else if (!attributes) {
                        ++seededWithoutAttributes;
                    }
                    seeded.append(entry);
                };
                for (int index = 0; index < element->inputSize(); ++index) {
                    seed(element->inputPort(index), QString("in %1").arg(index), element->inputAttributes(index));
                }
                for (int index = 0; index < element->outputSize(); ++index) {
                    seed(element->outputPort(index), QString("out %1").arg(index), element->outputAttributes(index));
                }

                element->updatePortsProperties();

                for (const auto &entry : std::as_const(seeded)) {
                    if (entry.required && (entry.port->isRequired() != *entry.required)) {
                        changed << entry.where + ": required";
                    }
                    if (entry.defaultStatus && (entry.port->defaultValue() != *entry.defaultStatus)) {
                        changed << entry.where + ": default";
                    }
                    if (entry.name && (entry.port->name() != *entry.name)) {
                        changed << entry.where + ": name";
                    }
                }
            }
        }
    }

    QVERIFY2(changed.isEmpty(), qPrintable(changed.join('\n')));
    // Ports whose element sets some fields and leaves others unset take the attributes' branch of the update, and ports
    // with no attributes at all take the other
    QVERIFY(seededBesideAttributes > 0);
    QVERIFY(seededWithoutAttributes > 0);
}

void TestElement::testEveryPortNameHasItsSource()
{
    // Generated code names signals from a port's source name, whatever the interface language. Every named port of
    // every type, at every legal port count, has one, and untranslated its name is that source; under a translator
    // installed after the element is built, only the 40 segment names change, derived from their descriptions when
    // asked, and no source does. An IC's pins are named from its circuit (plan 4).
    QStringList wrong;
    QSet<QString> translatedNames;
    TestUtils::TagEveryStringTranslator tagging;
    for (const auto type : TestUtils::dumpedTypes()) {
        if (type == ElementType::IC) {
            continue;
        }
        const std::unique_ptr<Element> probe(ElementFactory::buildElement(type));
        for (int inputs = probe->minInputSize(); inputs <= probe->maxInputSize(); ++inputs) {
            for (int outputs = probe->minOutputSize(); outputs <= probe->maxOutputSize(); ++outputs) {
                const std::unique_ptr<Element> element(ElementFactory::buildElement(type));
                TestUtils::applyPortCounts(element.get(), inputs, outputs);
                if ((element->inputSize() != inputs) || (element->outputSize() != outputs)) {
                    continue; // A Demux derives its inputs from its outputs
                }
                const QString counts = QString("%1 in=%2 out=%3").arg(ElementFactory::typeToText(type)).arg(inputs).arg(outputs);

                for (const auto *port : element->allPorts()) {
                    const QString where = counts + (port->isInput() ? " in " : " out ") + QString::number(port->index());
                    const QString name = port->name();
                    const QString source = port->sourceName();
                    if (!name.isEmpty() && source.isEmpty()) {
                        wrong << where + ": no source for " + name;
                    } else if (name != source) {
                        wrong << where + ": untranslated name " + name + " is not its source " + source;
                    }

                    QCoreApplication::installTranslator(&tagging);
                    const QString taggedName = port->name();
                    const QString taggedSource = port->sourceName();
                    QCoreApplication::removeTranslator(&tagging);
                    if (taggedSource != source) {
                        wrong << where + ": source " + source + " follows the translator to " + taggedSource;
                    }
                    if (taggedName != name) {
                        translatedNames.insert(ElementFactory::typeToText(type) + ' ' + taggedName);
                    }
                }
            }
        }
    }

    QVERIFY2(wrong.isEmpty(), qPrintable(wrong.join('\n')));
    // The 7-segment display's 8 inputs, the 14-segment's 15 and the 16-segment's 17
    QCOMPARE(translatedNames.size(), 40);
}
