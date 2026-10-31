// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestElementBodySnapshot.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QMetaEnum>
#include <QPainter>
#include <QPainterPath>
#include <QSet>
#include <QStyleOptionGraphicsItem>
#include <QTemporaryDir>
#include <QTest>

#include "App/Core/ThemeManager.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInput.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/IC.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"

// A named namespace: a unity build can put this file in one translation unit with other snapshots' helpers.
namespace ElementBodySnapshotDetail {

/// The space around an element's extent in its atlas cell.
constexpr int kMargin = 4;

/// How far a mask reaches past what it covers.
constexpr int kMaskPadding = 2;

/// The side of a Text's picture, past which its bounds follow the hint and the label.
constexpr int kTextPictureSide = 64;

/// The corner radius of the selection outline every element draws over its bounds.
constexpr int kSelectionRadius = 5;

QString goldenDir()
{
    return QString(QUOTE(CURRENTDIR)) + "/Integration/BodySnapshot/";
}

/// Where the atlases live that 32-bit x86 draws apart from every other platform.
QString i386GoldenDir()
{
    return goldenDir() + "i386/";
}

/**
 * \brief The golden atlas named \a name for this platform.
 * \details Debian's 32-bit x86 Qt does its floating point on the x87 unit, whose rounding differs from SSE2's, so its
 * rasteriser draws a few antialiased pixels apart (one per cell on the Mux's and Demux's round-joined corners, where the
 * same Qt on amd64 does not). Such an atlas has its own golden in i386GoldenDir(), compared there as exactly as the
 * shared one is everywhere else. Regenerating on 32-bit x86 rewrites those and every shared atlas it draws within the
 * tolerance, so the shared ones are regenerated on a 64-bit platform, and a new 32-bit x86 one is the run's actual
 * image of an atlas that fails there.
 */
QString atlasGolden(const QString &name)
{
#ifdef Q_PROCESSOR_X86_32
    if (const QString own = i386GoldenDir() + name; QFileInfo::exists(own)) {
        return own;
    }
#endif
    return goldenDir() + name;
}

/// An IC takes its body from its definition.
QString icFile()
{
    return TestUtils::fullAdderIcFile();
}

/// One state an element is put in before it is oriented.
struct BodyState {
    QString name;
    std::function<void(Element *)> apply;
};

/// A type, or an IC from one source, and the states its body is pinned in.
struct BodyCase {
    QString name;
    std::function<std::unique_ptr<Element>()> build;
    QList<BodyState> states;
    bool themed = false; ///< Whether its picture follows the theme, so it is pinned in both.
};

/// Custom appearances of another size than the built-in pictures, asymmetric so that every orientation shows.
class CustomPictures
{
public:
    CustomPictures()
    {
        QImage image(96, 48, QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.fillRect(0, 0, 96, 48, QColor(40, 120, 200));
        painter.fillRect(8, 8, 40, 32, QColor(240, 200, 40));
        painter.fillRect(56, 8, 32, 16, QColor(200, 40, 80));
        painter.end();
        m_png = m_dir.filePath("custom.png");
        image.save(m_png, "PNG");

        m_svg = m_dir.filePath("custom.svg");
        QFile svg(m_svg);
        if (svg.open(QIODevice::WriteOnly)) {
            svg.write("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"80\" height=\"40\" viewBox=\"0 0 80 40\">"
                      "<rect x=\"1\" y=\"1\" width=\"78\" height=\"38\" fill=\"#33aa77\" stroke=\"#112233\" stroke-width=\"2\"/>"
                      "<circle cx=\"20\" cy=\"20\" r=\"10\" fill=\"#ffcc00\"/>"
                      "<rect x=\"50\" y=\"8\" width=\"20\" height=\"10\" fill=\"#cc2244\"/>"
                      "</svg>");
        }
    }

    bool isValid() const { return m_dir.isValid() && QFileInfo::exists(m_png) && QFileInfo::exists(m_svg); }
    const QString &png() const { return m_png; }
    const QString &svg() const { return m_svg; }

private:
    QTemporaryDir m_dir;
    QString m_png;
    QString m_svg;
};

const QStringList &colours()
{
    static const QStringList list{"White", "Red", "Green", "Blue", "Purple"};
    return list;
}

QString onOff(const bool on)
{
    return on ? QStringLiteral("on") : QStringLiteral("off");
}

/**
 * \brief Drives \a element's inputs from the bits of \a pattern, port 0 the lowest, and redraws it as the simulation
 * does.
 * \details Each input is wired to a source in the element's scene, which owns them: a VCC where its bit is set, a GND
 * elsewhere. An input reads its wire wherever the element is turned or moved, while one without a wire goes back to
 * its default status each time its ports are laid out or moved.
 */
void driveInputs(Element *element, const quint32 pattern)
{
    CircuitBuilder builder(qobject_cast<Scene *>(element->scene()));
    for (int index = 0; index < element->inputSize(); ++index) {
        auto *source = ElementFactory::buildElement(((pattern >> index) & 1U) ? ElementType::InputVcc : ElementType::InputGnd);
        builder.add(source);
        builder.connect(source, 0, element, index);
    }
    element->refresh();
}

/// Has \a node's simulation see \a status on its input, which colours its wireless indicator.
void driveNode(Node *node, const Status status)
{
    const std::unique_ptr<Element> source(ElementFactory::buildElement(ElementType::InputVcc));
    source->initSimulationVectors(0, 1);
    source->setOutputValue(status);
    node->initSimulationVectors(1, 1);
    node->connectPredecessor(0, source.get(), 0);
    node->updateLogic();
    node->connectPredecessor(0, nullptr, 0);
}

QList<BodyState> inputCounts(const QList<int> &counts)
{
    QList<BodyState> states;
    for (const int count : counts) {
        states.append({QString("in=%1").arg(count), [count](Element *e) { e->setInputSize(count); }});
    }
    return states;
}

QList<BodyState> customAppearances(const CustomPictures &custom)
{
    return {{"custom.png", [&custom](Element *e) { e->setAppearance(false, custom.png()); }},
            {"custom.svg", [&custom](Element *e) { e->setAppearance(false, custom.svg()); }}};
}

QList<BodyState> defaultOnly()
{
    return {{"default", [](Element *) {}}};
}

QList<BodyState> switchable(const CustomPictures &custom)
{
    return {{"off", [](Element *e) { static_cast<ElementInput *>(e)->setOn(false); }},
            {"on", [](Element *e) { static_cast<ElementInput *>(e)->setOn(true); }},
            {"on custom.png", [&custom](Element *e) {
                 static_cast<ElementInput *>(e)->setOn(true);
                 e->setAppearance(false, custom.png());
             }}};
}

QList<BodyState> ledStates(const CustomPictures &custom)
{
    QList<BodyState> states;
    for (const auto &colour : colours()) {
        for (const bool on : {false, true}) {
            states.append({QString("in=1 %1 %2").arg(colour, onOff(on)), [colour, on](Element *e) {
                               e->setColor(colour);
                               driveInputs(e, on ? 1U : 0U);
                           }});
        }
    }
    for (int inputs = 2; inputs <= 4; ++inputs) {
        for (quint32 code = 0; code < (1U << inputs); ++code) {
            states.append({QString("in=%1 code=%2").arg(inputs).arg(code), [inputs, code](Element *e) {
                               e->setInputSize(inputs);
                               driveInputs(e, code);
                           }});
        }
    }
    states.append({"in=1 Red on custom.png", [&custom](Element *e) {
                       e->setColor("Red");
                       driveInputs(e, 1U);
                       e->setAppearance(false, custom.png());
                   }});
    return states;
}

QList<BodyState> displayStates(const int inputs, const CustomPictures &custom)
{
    const quint32 all = (1U << inputs) - 1U;
    const QList<std::pair<QString, quint32>> patterns{{"none", 0U}, {"even", 0x55555555U & all}, {"odd", 0xAAAAAAAAU & all}};
    QList<BodyState> states;
    for (const auto &colour : colours()) {
        states.append({colour + " all", [colour, all](Element *e) {
                           e->setColor(colour);
                           driveInputs(e, all);
                       }});
    }
    for (const auto &[name, pattern] : patterns) {
        states.append({"Red " + name, [pattern](Element *e) {
                           e->setColor("Red");
                           driveInputs(e, pattern);
                       }});
    }
    states.append({"Red all custom.svg", [&custom, all](Element *e) {
                       e->setColor("Red");
                       driveInputs(e, all);
                       e->setAppearance(false, custom.svg());
                   }});
    return states;
}

/// An audio output's input is wired: one that is not is not valid, and stays silent whatever its input reads.
QList<BodyState> audioStates()
{
    return {{"stopped", [](Element *e) { driveInputs(e, 0U); }}, {"playing", [](Element *e) { driveInputs(e, 1U); }}};
}

QList<BodyState> rotaryStates(const CustomPictures &custom)
{
    QList<BodyState> states;
    QList<std::pair<int, int>> positions;
    for (int outputs = 2; outputs <= 16; ++outputs) {
        positions.append({outputs, outputs - 1});
    }
    positions.append({16, 0});
    for (const auto &[outputs, position] : std::as_const(positions)) {
        states.append({QString("out=%1 at=%2").arg(outputs).arg(position), [outputs, position](Element *e) {
                           e->setOutputSize(outputs);
                           static_cast<ElementInput *>(e)->setOn(true, position);
                       }});
    }
    states.append({"out=4 at=1 custom.png", [&custom](Element *e) {
                       e->setOutputSize(4);
                       static_cast<ElementInput *>(e)->setOn(true, 1);
                       e->setAppearance(false, custom.png());
                   }});
    return states;
}

QList<BodyState> muxStates()
{
    QList<BodyState> states;
    for (int data = 2; data <= 8; ++data) {
        int selectLines = 1;
        while ((1 << selectLines) < data) {
            ++selectLines;
        }
        const int inputs = data + selectLines;
        states.append({QString("data=%1 in=%2").arg(data).arg(inputs), [inputs](Element *e) { e->setInputSize(inputs); }});
    }
    return states;
}

QList<BodyState> demuxStates()
{
    QList<BodyState> states;
    for (int outputs = 2; outputs <= 8; ++outputs) {
        states.append({QString("out=%1").arg(outputs), [outputs](Element *e) { e->setOutputSize(outputs); }});
    }
    return states;
}

QList<BodyState> truthTableStates()
{
    QList<BodyState> states;
    for (int inputs = 2; inputs <= 8; ++inputs) {
        states.append({QString("in=%1 out=1").arg(inputs), [inputs](Element *e) { TestUtils::applyPortCounts(e, inputs, 1); }});
    }
    for (int outputs = 2; outputs <= 8; ++outputs) {
        states.append({QString("in=8 out=%1").arg(outputs), [outputs](Element *e) { TestUtils::applyPortCounts(e, 8, outputs); }});
    }
    return states;
}

QList<BodyState> nodeStates()
{
    QList<BodyState> states{{"none", [](Element *) {}}};
    for (const auto mode : {WirelessMode::Tx, WirelessMode::Rx}) {
        const QString modeName = (mode == WirelessMode::Tx) ? "Tx" : "Rx";
        states.append({modeName, [mode](Element *e) { static_cast<Node *>(e)->setWirelessMode(mode); }});
        for (const auto status : {Status::Inactive, Status::Active, Status::Error}) {
            states.append({modeName + ' ' + TestUtils::snapshotStatus(status), [mode, status](Element *e) {
                               auto *node = static_cast<Node *>(e);
                               node->setWirelessMode(mode);
                               driveNode(node, status);
                           }});
        }
    }
    return states;
}

QList<BodyState> textStates()
{
    return {{"empty", [](Element *) {}}, {"labelled", [](Element *e) { e->setLabel("Notes"); }}};
}

std::function<std::unique_ptr<Element>()> builderOf(const ElementType type)
{
    return [type] { return std::unique_ptr<Element>(ElementFactory::buildElement(type)); };
}

/// Every case: each creatable type but IC under its own name, then an IC from a file and an embedded one.
QList<BodyCase> bodyCases(const CustomPictures &custom)
{
    QList<BodyCase> cases;
    const auto add = [&cases](const ElementType type, const QList<BodyState> &states, const bool themed = false) {
        cases.append({ElementFactory::typeToText(type), builderOf(type), states, themed});
    };

    for (const auto type : {ElementType::And, ElementType::Or, ElementType::Nand, ElementType::Nor, ElementType::Xor, ElementType::Xnor}) {
        add(type, inputCounts({2, 8}) + ((type == ElementType::And) ? customAppearances(custom) : QList<BodyState>{}));
    }
    add(ElementType::Not, defaultOnly());
    add(ElementType::InputVcc, defaultOnly());
    add(ElementType::InputGnd, defaultOnly());
    add(ElementType::InputButton, switchable(custom));
    add(ElementType::InputSwitch, switchable(custom));
    add(ElementType::Clock, switchable(custom));
    add(ElementType::InputRotary, rotaryStates(custom));
    add(ElementType::Led, ledStates(custom));
    add(ElementType::Display7, displayStates(8, custom));
    add(ElementType::Display14, displayStates(15, custom));
    add(ElementType::Display16, displayStates(17, custom));
    add(ElementType::Buzzer, audioStates());
    add(ElementType::AudioBox, audioStates());
    for (const auto type : {ElementType::DLatch, ElementType::SRLatch, ElementType::DFlipFlop, ElementType::JKFlipFlop,
                            ElementType::SRFlipFlop, ElementType::TFlipFlop}) {
        add(type, defaultOnly(), true);
    }
    add(ElementType::Mux, muxStates());
    add(ElementType::Demux, demuxStates());
    add(ElementType::TruthTable, truthTableStates());
    add(ElementType::Node, nodeStates());
    add(ElementType::Line, defaultOnly());
    add(ElementType::Text, textStates());

    const auto loadIc = [](IC *ic) { ic->loadFile(icFile(), QFileInfo(icFile()).absolutePath()); };
    cases.append({"IC-file",
                  [loadIc] {
                      auto ic = std::make_unique<IC>();
                      loadIc(ic.get());
                      return std::unique_ptr<Element>(std::move(ic));
                  },
                  defaultOnly()});
    cases.append({"IC-embedded",
                  [] {
                      auto ic = std::make_unique<IC>();
                      QFile file(icFile());
                      if (file.open(QIODevice::ReadOnly)) {
                          ic->loadFromBlob(file.readAll(), QFileInfo(icFile()).absolutePath());
                      }
                      ic->setBlobName("full_adder");
                      return std::unique_ptr<Element>(std::move(ic));
                  },
                  defaultOnly()});
    return cases;
}

/// The themes a case is pinned in, by the suffix of its golden.
QList<std::pair<Theme, QString>> themesOf(const BodyCase &bodyCase)
{
    QList<std::pair<Theme, QString>> themes{{Theme::Light, QString()}};
    if (bodyCase.themed) {
        themes.append({Theme::Dark, QStringLiteral("_dark")});
    }
    return themes;
}

/// The element of \a bodyCase in \a state and \a orientation, in \a scene.
std::unique_ptr<Element> configure(Scene *scene, const BodyCase &bodyCase, const BodyState &state, const TestUtils::Orientation &orientation)
{
    auto element = bodyCase.build();
    scene->addItem(element.get());
    state.apply(element.get());
    TestUtils::orient(element.get(), orientation);
    return element;
}

QString rectText(const QRectF &rect)
{
    return TestUtils::snapshotPoint(rect.topLeft()) + ',' + TestUtils::snapshotPoint({rect.width(), rect.height()});
}

QString transformText(const QTransform &t)
{
    return QStringList({TestUtils::snapshotNumber(t.m11()), TestUtils::snapshotNumber(t.m12()), TestUtils::snapshotNumber(t.m21()),
                        TestUtils::snapshotNumber(t.m22()), TestUtils::snapshotNumber(t.dx()), TestUtils::snapshotNumber(t.dy())})
        .join(',');
}

/// The child items that draw with the element's body, which the presenters leave none of: the ports and the label are
/// left out, the port-layout and label goldens pinning them, and so is a child that draws nothing.
QList<QGraphicsItem *> drawingChildren(const Element *element)
{
    QList<QGraphicsItem *> children;
    const auto items = element->childItems();
    for (auto *child : items) {
        if (child->isVisible() && (child->type() != Port::Type) && (child != SnapshotAccess::label(element))
            && !child->boundingRect().isEmpty()) {
            children.append(child);
        }
    }
    return children;
}

/// Whether \a element's bounds follow the platform's font: a Text's take its hint's and its label's.
bool boundsFollowFont(const Element *element)
{
    return element->elementType() == ElementType::Text;
}

/// The geometry of \a element, on one line, with a line per child item that draws. Each transform is the item's whole
/// one, its rotation and flips, as the scene applies it to the element at the origin.
QStringList describe(const QString &state, const Element *element)
{
    QStringList lines;
    lines << QString("  %1 rot=%2 flipX=%3 flipY=%4 bounds=%5 centre=%6 xf=%7 pix=%8")
                 .arg(state,
                      TestUtils::snapshotNumber(element->rotation()),
                      QString::number(element->isFlippedX()),
                      QString::number(element->isFlippedY()),
                      boundsFollowFont(element) ? QStringLiteral("font") : rectText(element->boundingRect()),
                      TestUtils::snapshotPoint(element->pixmapCenter()),
                      transformText(element->sceneTransform()),
                      SnapshotAccess::pixmapIdentity(element));
    for (const auto *child : drawingChildren(element)) {
        const bool text = boundsFollowFont(element);
        lines << QString("    child z=%1 pos=%2 xf=%3 size=%4")
                     .arg(TestUtils::snapshotNumber(child->zValue()),
                          TestUtils::snapshotPoint(child->pos()),
                          transformText(child->sceneTransform()),
                          text ? QStringLiteral("font") : TestUtils::snapshotPoint({child->boundingRect().width(), child->boundingRect().height()}));
    }
    return lines;
}

/// The scene rectangle \a element draws in: its bounds and its child items'. A Text's are its picture's, the rest
/// following this platform's font.
QRectF extent(const Element *element)
{
    if (boundsFollowFont(element)) {
        return element->sceneTransform().mapRect(QRectF(0, 0, kTextPictureSide, kTextPictureSide));
    }
    QRectF rect = element->sceneBoundingRect();
    for (const auto *child : drawingChildren(element)) {
        rect |= child->sceneBoundingRect();
    }
    return rect;
}

/// Masks what this platform's font decides in \a element's cell \a cell of the atlas \a painter draws on, \a offset
/// taking the scene to the atlas. That is all of a Text's cell but its picture's inside: its bounds hold its hint and its
/// label, so the selection outline drawn over them, rounded corners and all, falls where the font puts it, on any side
/// of the picture and up to the corners' radius into it. The bundled pictures draw their letters as outlines, which
/// need no mask.
void maskText(QPainter &painter, const Element *element, const QTransform &offset, const QRect &cell)
{
    if (!boundsFollowFont(element)) {
        return;
    }
    const qreal inset = kSelectionRadius + kMaskPadding;
    const QRectF picture = QRectF(0, 0, kTextPictureSide, kTextPictureSide).adjusted(inset, inset, -inset, -inset);
    QPainterPath masked;
    masked.addRect(cell);
    QPainterPath kept;
    kept.addRect((element->sceneTransform() * offset).mapRect(picture));
    painter.fillPath(masked.subtracted(kept), QColor::fromRgb(TestUtils::kMaskColour));
}

/// Paints \a element and its child items that draw onto \a painter, \a offset taking the scene to the image, as a
/// view does with antialiasing on (fast mode off). A Text's hint, which its body draws, is this platform's font, which
/// maskText() covers.
void drawBody(QPainter &painter, Element *element, const QTransform &offset)
{
    QStyleOptionGraphicsItem option;
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter.setTransform(element->sceneTransform() * offset);
    option.exposedRect = element->boundingRect();
    element->paint(&painter, &option, nullptr);
    for (auto *child : boundsFollowFont(element) ? QList<QGraphicsItem *>() : drawingChildren(element)) {
        painter.setTransform(child->sceneTransform() * offset);
        option.exposedRect = child->boundingRect();
        child->paint(&painter, &option, nullptr);
    }
    painter.restore();
}

/// The atlas of \a bodyCase: a row per state and selection (unselected above selected), a column per orientation,
/// text masked.
QImage drawAtlas(const BodyCase &bodyCase)
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    const auto orientations = TestUtils::orientations();

    std::vector<std::unique_ptr<Element>> elements;
    QSizeF largest;
    for (const auto &state : bodyCase.states) {
        for (const auto &orientation : orientations) {
            elements.push_back(configure(scene, bodyCase, state, orientation));
            largest = largest.expandedTo(extent(elements.back().get()).size());
        }
    }

    const QSize cell(static_cast<int>(std::ceil(largest.width())) + 2 * kMargin, static_cast<int>(std::ceil(largest.height())) + 2 * kMargin);
    QImage atlas(cell.width() * static_cast<int>(orientations.size()), cell.height() * static_cast<int>(bodyCase.states.size()) * 2,
                 QImage::Format_ARGB32_Premultiplied);
    atlas.fill(Qt::white);

    QPainter painter(&atlas);
    for (qsizetype index = 0; index < static_cast<qsizetype>(elements.size()); ++index) {
        Element *element = elements.at(static_cast<std::size_t>(index)).get();
        const auto row = static_cast<int>(index / orientations.size());
        const auto column = static_cast<int>(index % orientations.size());
        for (const bool selected : {false, true}) {
            element->setSelected(selected);
            const QPoint corner(column * cell.width(), (row * 2 + (selected ? 1 : 0)) * cell.height());
            const QPointF origin = extent(element).topLeft();
            const QTransform offset = QTransform::fromTranslate(corner.x() + kMargin - origin.x(), corner.y() + kMargin - origin.y());
            // Clipped to the cell: a selected Text's outline follows its font-sized bounds past its picture
            painter.save();
            painter.setClipRect(QRect(corner, cell));
            drawBody(painter, element, offset);
            maskText(painter, element, offset, QRect(corner, cell));
            painter.restore();
        }
    }
    painter.end();

    for (auto &element : elements) {
        scene->removeItem(element.get());
    }
    return atlas;
}

} // namespace ElementBodySnapshotDetail

using namespace ElementBodySnapshotDetail;

void TestElementBodySnapshot::testEveryTypeHasACase()
{
    const CustomPictures custom;
    QVERIFY(custom.isValid());
    QSet<QString> named;
    for (const auto &bodyCase : bodyCases(custom)) {
        named.insert(bodyCase.name);
    }

    const QMetaEnum types = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < types.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(types.value(i));
        if (TestUtils::leftOutOfDumps(type)) {
            continue;
        }
        QVERIFY2(named.contains(ElementFactory::typeToText(type)), types.key(i));
    }
    QVERIFY(named.contains("IC-file"));
    QVERIFY(named.contains("IC-embedded"));

    // Each 32-bit x86 atlas is a case's, in a theme it is pinned in, and still differs from the shared one: one equal
    // to it would hide that its renderer now draws alike
    QSet<QString> atlases;
    for (const auto &bodyCase : bodyCases(custom)) {
        for (const auto &[theme, suffix] : themesOf(bodyCase)) {
            atlases.insert(bodyCase.name + suffix + ".png");
        }
    }
    const QStringList own = QDir(i386GoldenDir()).entryList(QDir::Files);
    for (const QString &name : own) {
        QVERIFY2(atlases.contains(name), qPrintable(name));
        const QImage shared(goldenDir() + name);
        const QImage i386(i386GoldenDir() + name);
        QVERIFY2(!shared.isNull() && !i386.isNull(), qPrintable(name));
        QVERIFY2(shared.convertToFormat(QImage::Format_ARGB32) != i386.convertToFormat(QImage::Format_ARGB32), qPrintable(name));
    }

}

void TestElementBodySnapshot::testGeometryMatchesGolden()
{
    TestUtils::ScopedThemeOverride themeOverride;
    const CustomPictures custom;
    QVERIFY(custom.isValid());

    QStringList lines;
    for (const auto &bodyCase : bodyCases(custom)) {
        for (const auto &[theme, suffix] : themesOf(bodyCase)) {
            ThemeManager::setTheme(theme);
            WorkSpace workspace;
            Scene *scene = workspace.scene();
            lines << "T " + bodyCase.name + suffix;
            for (const auto &state : bodyCase.states) {
                for (const auto &orientation : TestUtils::orientations()) {
                    const auto element = configure(scene, bodyCase, state, orientation);
                    lines << describe(state.name, element.get());
                    scene->removeItem(element.get());
                }
            }
        }
    }

    const QString mismatch = TestUtils::compareWithGolden(goldenDir() + "bodies.txt", lines.join('\n') + '\n');
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}

void TestElementBodySnapshot::testPixelsMatchGoldens_data()
{
    QTest::addColumn<QString>("caseName");
    QTest::addColumn<int>("theme");
    QTest::addColumn<QString>("suffix");

    const CustomPictures custom;
    for (const auto &bodyCase : bodyCases(custom)) {
        for (const auto &[theme, suffix] : themesOf(bodyCase)) {
            QTest::newRow(qPrintable(bodyCase.name + suffix)) << bodyCase.name << static_cast<int>(theme) << suffix;
        }
    }
}

void TestElementBodySnapshot::testPixelsMatchGoldens()
{
    QFETCH(QString, caseName);
    QFETCH(int, theme);
    QFETCH(QString, suffix);

    TestUtils::ScopedThemeOverride themeOverride;
    ThemeManager::setTheme(static_cast<Theme>(theme));
    const CustomPictures custom;
    QVERIFY(custom.isValid());

    const auto cases = bodyCases(custom);
    const auto found = std::find_if(cases.cbegin(), cases.cend(), [&caseName](const BodyCase &c) { return c.name == caseName; });
    QVERIFY(found != cases.cend());

    const QString mismatch = TestUtils::compareWithGoldenImage(atlasGolden(caseName + suffix + ".png"), drawAtlas(*found));
    QVERIFY2(mismatch.isEmpty(), qPrintable(mismatch));
}
