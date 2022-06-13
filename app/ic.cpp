// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ic.h"

#include "common.h"
#include "globalproperties.h"
#include "icmanager.h"
#include "icprototype.h"
#include "qneport.h"

#include <QFileInfo>
#include <QGraphicsSceneMouseEvent>

namespace
{
int id = qRegisterMetaType<IC>();
}

IC::IC(QGraphicsItem *parent)
    : GraphicElement(ElementType::IC, ElementGroup::IC, 0, 0, 0, 0, parent)
{
    if (GlobalProperties::skipInit) {
        return;
    }

    m_defaultSkins << m_pixmapPath;
    m_alternativeSkins = m_defaultSkins;
    setPixmap(0);

    m_label->setPos(30, 64);
    m_label->setRotation(90);

    setHasLabel(true);
    setPortName("IC");
    setToolTip(m_translatedName);
}

IC::~IC()
{
    if (auto *prototype = ICManager::prototype(m_file)) {
        prototype->removeICObserver(this);
    }
}

void IC::save(QDataStream &stream) const
{
    GraphicElement::save(stream);
    stream << QFileInfo(m_file).fileName();
}

void IC::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const double version)
{
    GraphicElement::load(stream, portMap, version);
    if (version >= 1.2) {
        stream >> m_file;
    }
}

void IC::loadInputs(ICPrototype *prototype)
{
    setMaxInputSize(prototype->inputSize());
    setMinInputSize(prototype->inputSize());
    setInputSize(prototype->inputSize());
    qCDebug(three) << tr("IC") << m_file << tr("-> Inputs. min:") << minInputSize() << tr(", max:") << maxInputSize() << tr(", current:") << inputSize() << tr(", m_inputs:") << m_inputPorts.size();
    for (int inputIndex = 0; inputIndex < prototype->inputSize(); ++inputIndex) {
        QNEPort *portIn = inputPort(inputIndex);
        portIn->setName(prototype->inputLabel(inputIndex));
        portIn->setRequired(prototype->isInputRequired(inputIndex));
        portIn->setDefaultValue(prototype->defaultInputValue(inputIndex));
        portIn->setValue(prototype->defaultInputValue(inputIndex));
    }
}

void IC::loadOutputs(ICPrototype *prototype)
{
    setMaxOutputSize(prototype->outputSize());
    setMinOutputSize(prototype->outputSize());
    setOutputSize(prototype->outputSize());
    for (int outputIndex = 0; outputIndex < prototype->outputSize(); ++outputIndex) {
        QNEPort *portOut = outputPort(outputIndex);
        portOut->setName(prototype->outputLabel(outputIndex));
    }
    qCDebug(three) << tr("IC") << m_file << tr("-> Outputs. min:") << minOutputSize() << tr(", max:") << maxOutputSize() << tr(", current:") << outputSize() << tr(", m_outputs:") << m_outputPorts.size();
}

void IC::loadFile(const QString &fileName)
{
    ICPrototype *prototype = ICManager::prototype(fileName);
    m_file = prototype->fileName();
    setToolTip(m_file);
    prototype->insertICObserver(this);
    if (label().isEmpty()) {
        setLabel(prototype->baseName().toUpper());
    }
    // Loading inputs
    loadInputs(prototype);
    // Loading outputs
    loadOutputs(prototype);
    updatePorts();
}

QString IC::file() const
{
    return m_file;
}

ICPrototype *IC::prototype()
{
    return ICManager::prototype(m_file);
}

void IC::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    emit ICManager::instance().openIC(m_file);
}
