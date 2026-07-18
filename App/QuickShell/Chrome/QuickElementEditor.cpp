// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickElementEditor.h"

#include <QDataStream>
#include <QKeySequence>

#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/DialogProvider.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

QuickElementEditor::QuickElementEditor(QObject *parent)
    : QObject(parent)
{
}

void QuickElementEditor::setCanvas(CanvasItem *canvas)
{
    if (m_canvas) {
        disconnect(m_canvas, &CanvasItem::selectionChanged, this, &QuickElementEditor::onSelectionChanged);
    }
    m_canvas = canvas;
    if (m_canvas) {
        connect(m_canvas, &CanvasItem::selectionChanged, this, &QuickElementEditor::onSelectionChanged);
    }
    onSelectionChanged();
}

void QuickElementEditor::onSelectionChanged()
{
    m_elements = m_canvas ? m_canvas->selectedElements() : QList<GraphicElement *>();
    refresh();
}

QString QuickElementEditor::elementTypeTitle() const
{
    return m_elements.isEmpty() ? QString() : ElementFactory::typeToTitleText(m_caps.elementType);
}

void QuickElementEditor::refresh()
{
    if (m_elements.isEmpty()) {
        m_caps = {};
        m_label.clear();
        m_color.clear();
        m_audio.clear();
        m_trigger.clear();
        m_inputSizeOptions.clear();
        m_outputSizeOptions.clear();
        m_outputValueOptions.clear();
        emit refreshed();
        return;
    }

    m_caps = computeCapabilities(m_elements);
    auto *firstElement = m_elements.constFirst();
    auto *firstInput = qobject_cast<GraphicElementInput *>(firstElement);

    m_label = m_caps.hasLabel ? firstElement->label() : QString();
    m_labelDirty = false;

    m_color = m_caps.hasColors ? firstElement->color() : QString();
    m_colorDirty = false;

    m_audio = m_caps.hasAudio ? firstElement->audio() : QString();
    m_audioDirty = false;

    m_volume = m_caps.hasVolume ? static_cast<int>(firstElement->volume() * 100.0f) : 100;
    m_volumeDirty = false;

    if (m_caps.hasFrequency) {
        // Buzzer uses audible tone range; Clock uses low oscillation range -- mirrors
        // ElementEditor::applyCapabilitiesToUi()'s identical branch exactly.
        const bool isBuzzer = (m_caps.hasSameType && m_caps.elementType == ElementType::Buzzer);
        m_frequencyMin = isBuzzer ? 20.0 : 0.1;
        m_frequencyMax = isBuzzer ? 20000.0 : 50.0;
        m_frequencyStep = isBuzzer ? 10.0 : 0.1;
        m_frequencyDecimals = isBuzzer ? 0 : 1;
        m_frequency = firstElement->frequency();
    }
    m_frequencyDirty = false;

    m_delaySteps = m_caps.hasDelay ? static_cast<int>(firstElement->delay() * 8.0) : 0;
    m_delayDirty = false;

    m_inputSizeOptions.clear();
    if (m_caps.canChangeInputSize) {
        if (m_caps.hasSameType && m_caps.elementType == ElementType::Mux) {
            // Mux: the user picks data inputs (2-8); select lines are computed as
            // ceil(log2(dataInputs)) -- option label shows data inputs, value carries total.
            for (int dataInputs = 2; dataInputs <= 8; ++dataInputs) {
                int selectLines = 1;
                while ((1 << selectLines) < dataInputs) {
                    ++selectLines;
                }
                const int totalInputs = dataInputs + selectLines;
                if (totalInputs >= m_caps.minimumInputs && totalInputs <= m_caps.maximumInputs) {
                    m_inputSizeOptions.append(SizeOption(QString::number(dataInputs), totalInputs));
                }
            }
        } else {
            for (int port = m_caps.minimumInputs; port <= m_caps.maximumInputs; ++port) {
                m_inputSizeOptions.append(SizeOption(QString::number(port), port));
            }
        }
    }
    m_inputSize = firstElement->inputSize();

    m_outputSizeOptions.clear();
    if (m_caps.canChangeOutputSize) {
        if (m_caps.hasSameType && m_caps.elementType == ElementType::Demux) {
            for (int n = m_caps.minimumOutputs; n <= m_caps.maximumOutputs; ++n) {
                m_outputSizeOptions.append(SizeOption(QString::number(n), n));
            }
        } else {
            for (int n : {2, 3, 4, 6, 8, 10, 12, 16}) {
                m_outputSizeOptions.append(SizeOption(QString::number(n), n));
            }
        }
    }
    m_outputSize = firstElement->outputSize();

    m_outputValueOptions.clear();
    if (m_caps.hasLatchedValue) {
        int maxOut = m_caps.maxCurrentOutputSize;
        if (maxOut == 1) {
            ++maxOut;
        }
        for (int val = 0; val < maxOut; ++val) {
            m_outputValueOptions.append(SizeOption(QString::number(val), val));
        }
    }
    m_outputValue = (m_caps.hasLatchedValue && firstInput) ? firstInput->outputValue() : 0;
    m_outputValueDirty = false;

    if (m_caps.hasOnlyInputs && firstInput) {
        // Qt::CheckState: 0 Unchecked, 1 PartiallyChecked, 2 Checked.
        m_lockedState = m_caps.sameCheckState ? (firstInput->isLocked() ? 2 : 0) : 1;
    } else {
        m_lockedState = 0;
    }
    m_lockedDirty = false;

    m_trigger = m_caps.hasTrigger ? firstElement->trigger().toString() : QString();
    m_triggerDirty = false;

    m_wirelessMode = 0;
    if (m_caps.hasWirelessMode) {
        if (auto *node = qobject_cast<Node *>(firstElement)) {
            m_wirelessMode = static_cast<int>(node->wirelessMode());
        }
    }
    m_wirelessModeDirty = false;

    emit refreshed();
}

void QuickElementEditor::apply()
{
    if (m_elements.isEmpty() || !m_canvas) {
        return;
    }

    // Reject label/mode changes that would create a duplicate wireless Tx channel -- mirrors
    // ElementEditor::apply()'s guard exactly. A Tx node's label is its channel name; two Tx
    // nodes sharing one label means one silently receives no signal.
    if (m_caps.hasWirelessMode) {
        for (auto *elm : std::as_const(m_elements)) {
            auto *node = qobject_cast<Node *>(elm);
            if (!node) {
                continue;
            }

            const QString candidateLabel = m_labelDirty ? m_label : node->label();
            const WirelessMode candidateMode = m_wirelessModeDirty ? static_cast<WirelessMode>(m_wirelessMode) : node->wirelessMode();
            if (candidateMode != WirelessMode::Tx || candidateLabel.isEmpty()) {
                continue;
            }

            for (auto *other : m_canvas->elements()) {
                if (other == elm) {
                    continue;
                }
                auto *otherNode = qobject_cast<Node *>(other);
                if (!otherNode || otherNode->wirelessMode() != WirelessMode::Tx) {
                    continue;
                }
                if (otherNode->label() == candidateLabel) {
                    Dialogs::provider()->choice(
                        tr("Duplicate Wireless Channel"),
                        tr("A Tx node with label \"%1\" already exists.\n"
                           "Each wireless channel must have a unique label.").arg(candidateLabel),
                        {DialogButton::Ok}, DialogButton::Ok);
                    refresh();
                    return;
                }
            }
        }
    }

    // Snapshot old state before any mutation below.
    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    for (auto *elm : std::as_const(m_elements)) {
        elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }

    // Collect connections that will be severed by a wireless mode change, before it happens --
    // setWirelessMode() hides the replaced port but does not delete connections itself.
    QList<QGraphicsItem *> wirelessConnsToDelete;
    if (m_caps.hasWirelessMode && m_wirelessModeDirty) {
        const auto newMode = static_cast<WirelessMode>(m_wirelessMode);
        for (auto *elm : std::as_const(m_elements)) {
            auto *node = qobject_cast<Node *>(elm);
            if (!node) {
                continue;
            }
            Port *port = (newMode == WirelessMode::Rx) ? static_cast<Port *>(node->inputPort())
                         : (newMode == WirelessMode::Tx) ? static_cast<Port *>(node->outputPort())
                         : nullptr;
            if (port) {
                for (auto *conn : port->connections()) {
                    wirelessConnsToDelete.append(static_cast<QGraphicsItem *>(conn));
                }
            }
        }
    }

    for (auto *elm : std::as_const(m_elements)) {
        applyProperty(elm);
    }

    const bool needsMacro = !wirelessConnsToDelete.isEmpty();
    if (needsMacro) {
        m_canvas->undoStack()->beginMacro(tr("Change wireless mode"));
    }

    m_canvas->receiveCommand(new CanvasUpdateCommand(m_elements, oldData, m_canvas));

    if (needsMacro) {
        m_canvas->receiveCommand(new CanvasDeleteItemsCommand(wirelessConnsToDelete, m_canvas));
        m_canvas->undoStack()->endMacro();
    }
}

void QuickElementEditor::applyProperty(GraphicElement *elm)
{
    if (m_labelDirty && m_caps.hasLabel) {
        elm->setLabel(m_label);
    }
    if (m_colorDirty && m_caps.hasColors) {
        elm->setColor(m_color);
    }
    if (m_frequencyDirty && m_caps.hasFrequency) {
        elm->setFrequency(m_frequency);
    }
    if (m_delayDirty && m_caps.hasDelay) {
        elm->setDelay(static_cast<double>(m_delaySteps) / 8.0);
    }
    if (m_audioDirty && m_caps.hasAudio) {
        elm->setAudio(m_audio);
    }
    if (m_volumeDirty && m_caps.hasVolume) {
        elm->setVolume(static_cast<float>(m_volume) / 100.0f);
    }
    if (m_triggerDirty && m_caps.hasTrigger) {
        elm->setTrigger(QKeySequence(m_trigger));
    }
    if (m_wirelessModeDirty && m_caps.hasWirelessMode) {
        if (auto *node = qobject_cast<Node *>(elm)) {
            node->setWirelessMode(static_cast<WirelessMode>(m_wirelessMode));
        }
    }
    if (m_lockedDirty && m_caps.hasOnlyInputs) {
        if (auto *input = qobject_cast<GraphicElementInput *>(elm)) {
            input->setLocked(m_lockedState == 2);
        }
    }
    if (m_outputValueDirty && m_caps.hasLatchedValue) {
        if (elm->elementType() == ElementType::InputRotary) {
            if (auto *inputRotary = qobject_cast<InputRotary *>(elm)) {
                inputRotary->setOn(true, m_outputValue);
            }
        } else if (auto *input = qobject_cast<GraphicElementInput *>(elm)) {
            input->setOn(static_cast<bool>(m_outputValue));
        }
    }
}

void QuickElementEditor::setLabel(const QString &value)
{
    if (!m_caps.hasLabel || value == m_label) {
        return;
    }
    m_label = value;
    m_labelDirty = true;
    emit refreshed();
    apply();
}

void QuickElementEditor::setColor(const QString &value)
{
    if (!m_caps.hasColors || value == m_color) {
        return;
    }
    m_color = value;
    m_colorDirty = true;
    emit refreshed();
    apply();
}

QList<ColorOption> QuickElementEditor::colorOptions()
{
    // Mirrors ElementEditor::fillColorComboBox() -- name is the internal English identifier
    // GraphicElement::setColor() expects; translatedName is display-only.
    return {
        ColorOption("White", tr("White"), "qrc:/Components/Output/Led/WhiteLed.svg"),
        ColorOption("Red", tr("Red"), "qrc:/Components/Output/Led/RedLed.svg"),
        ColorOption("Green", tr("Green"), "qrc:/Components/Output/Led/GreenLed.svg"),
        ColorOption("Blue", tr("Blue"), "qrc:/Components/Output/Led/BlueLed.svg"),
        ColorOption("Purple", tr("Purple"), "qrc:/Components/Output/Led/PurpleLed.svg"),
    };
}

void QuickElementEditor::setAudio(const QString &value)
{
    if (!m_caps.hasAudio || value == m_audio) {
        return;
    }
    m_audio = value;
    m_audioDirty = true;
    emit refreshed();
    apply();
}

QVariantList QuickElementEditor::audioOptions()
{
    // Mirrors ElementEditorUI's 8 fixed comboBoxAudio notes (C6..C7).
    return {"C6", "D6", "E6", "F6", "G6", "A6", "B6", "C7"};
}

void QuickElementEditor::setVolume(int value)
{
    if (!m_caps.hasVolume || value == m_volume) {
        return;
    }
    m_volume = value;
    m_volumeDirty = true;
    emit refreshed();
    apply();
}

void QuickElementEditor::setFrequency(double value)
{
    if (!m_caps.hasFrequency || qFuzzyCompare(value, m_frequency)) {
        return;
    }
    m_frequency = value;
    m_frequencyDirty = true;
    emit refreshed();
    apply();
}

void QuickElementEditor::setDelaySteps(int value)
{
    if (!m_caps.hasDelay || value == m_delaySteps) {
        return;
    }
    m_delaySteps = value;
    m_delayDirty = true;
    emit refreshed();
    apply();
}

void QuickElementEditor::setInputSize(int value)
{
    // Input/output size changes push their own CanvasChangePortSizeCommand, mirroring
    // ElementEditor::inputIndexChanged() -- NOT part of the general UpdateCommand apply()
    // pushes, since PropertyDescriptor::Type has no size-related member (port count is a
    // structural change, not a plain property write).
    if (!m_caps.canChangeInputSize || value == m_inputSize || m_elements.isEmpty() || !m_canvas) {
        return;
    }
    m_canvas->receiveCommand(new CanvasChangePortSizeCommand(m_elements, value, m_canvas, true));
    refresh();
}

void QuickElementEditor::setOutputSize(int value)
{
    if (!m_caps.canChangeOutputSize || value == m_outputSize || m_elements.isEmpty() || !m_canvas) {
        return;
    }
    m_canvas->receiveCommand(new CanvasChangePortSizeCommand(m_elements, value, m_canvas, false));
    refresh();
}

void QuickElementEditor::setOutputValue(int value)
{
    if (!m_caps.hasLatchedValue || value == m_outputValue) {
        return;
    }
    m_outputValue = value;
    m_outputValueDirty = true;
    emit refreshed();
    apply();
}

void QuickElementEditor::setLockedState(int value)
{
    if (!m_caps.hasOnlyInputs || value == m_lockedState) {
        return;
    }
    m_lockedState = value;
    m_lockedDirty = true;
    emit refreshed();
    apply();
}

void QuickElementEditor::setTrigger(const QString &value)
{
    if (!m_caps.hasTrigger) {
        return;
    }
    // Force uppercase so the stored key sequence matches Qt's key names across layouts --
    // mirrors ElementEditor::triggerChanged(). A single alphanumeric character only.
    const QString upper = value.toUpper();
    if (upper == m_trigger || upper.size() > 1) {
        return;
    }
    m_trigger = upper;
    m_triggerDirty = true;
    emit refreshed();
    apply();
}

void QuickElementEditor::setWirelessMode(int value)
{
    if (!m_caps.hasWirelessMode || value == m_wirelessMode) {
        return;
    }
    m_wirelessMode = value;
    m_wirelessModeDirty = true;
    emit refreshed();
    apply();
}

QVariantList QuickElementEditor::wirelessModeOptions()
{
    return {tr("None"), tr("Transmit (Tx)"), tr("Receive (Rx)")};
}

void QuickElementEditor::prepareContextMenu(GraphicElement *item)
{
    m_morphCandidates.clear();

    if (item && m_caps.canMorph) {
        // hasSameType here is the WHOLE selection's flag (matches addElementAction()'s own
        // "hasSameType && selectedElm->elementType() == type" guard exactly) -- a mixed-type
        // selection excludes nothing, since the exclusion only means "don't offer morphing to
        // the type everything already is."
        const bool hasSameType = m_caps.hasSameType;
        const auto addCandidate = [this, item, hasSameType](ElementType type) {
            if (hasSameType && item->elementType() == type) {
                return;
            }
            m_morphCandidates.append(MorphCandidate(
                static_cast<int>(type), ElementFactory::translatedName(type),
                QStringLiteral("qrc") + ElementMetadataRegistry::metadata(type).pixmapPath()));
        };

        // Mirrors ElementContextMenu::exec()'s per-ElementGroup switch exactly.
        switch (item->elementGroup()) {
        case ElementGroup::Gate:
            if (item->inputSize() == 1) {
                addCandidate(ElementType::Node);
                addCandidate(ElementType::Not);
            } else {
                addCandidate(ElementType::And);
                addCandidate(ElementType::Nand);
                addCandidate(ElementType::Nor);
                addCandidate(ElementType::Or);
                addCandidate(ElementType::Xnor);
                addCandidate(ElementType::Xor);
            }
            break;

        case ElementGroup::StaticInput:
        case ElementGroup::Input:
            addCandidate(ElementType::Clock);
            addCandidate(ElementType::InputButton);
            addCandidate(ElementType::InputGnd);
            addCandidate(ElementType::InputRotary);
            addCandidate(ElementType::InputSwitch);
            addCandidate(ElementType::InputVcc);
            break;

        case ElementGroup::Memory:
            // 2 inputs -> DLatch; 4 inputs -> DFF/TFF; 5 inputs -> JKFF/SRFF.
            if (item->inputSize() == 2) {
                addCandidate(ElementType::DLatch);
            } else if (item->inputSize() == 4) {
                addCandidate(ElementType::DFlipFlop);
                addCandidate(ElementType::TFlipFlop);
            } else if (item->inputSize() == 5) {
                addCandidate(ElementType::JKFlipFlop);
                addCandidate(ElementType::SRFlipFlop);
            }
            break;

        case ElementGroup::Output:
            if (item->elementType() == ElementType::Display7 || item->elementType() == ElementType::Display14
                || item->elementType() == ElementType::Display16) {
                addCandidate(ElementType::Display7);
                addCandidate(ElementType::Display14);
                addCandidate(ElementType::Display16);
            } else {
                addCandidate(ElementType::Buzzer);
                addCandidate(ElementType::Led);
            }
            break;

        case ElementGroup::IC:
        case ElementGroup::Mux:
        case ElementGroup::Other:
        case ElementGroup::Unknown:
            break;

        default:
            break;
        }
    }

    emit refreshed();
}

void QuickElementEditor::morphSelectionTo(int type)
{
    if (!m_canvas) {
        return;
    }
    m_canvas->morphSelectionTo(static_cast<ElementType>(type));
}
