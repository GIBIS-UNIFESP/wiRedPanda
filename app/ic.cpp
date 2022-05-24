// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ic.h"

#include "common.h"
#include "icmanager.h"
#include "icprototype.h"
#include "qneport.h"

#include <QGraphicsSceneMouseEvent>

namespace
{
int id = qRegisterMetaType<IC>();
}

IC::IC(QGraphicsItem *parent)
    : GraphicElement(ElementType::IC, ElementGroup::IC, 0, 0, 0, 0, parent)
{
    // qCDebug(zero) << "Creating box.";
    m_defaultSkins.append(":/basic/box.png");

    m_label->setPos(30, 64);
    m_label->setRotation(90);

    setPixmap(m_defaultSkins[0], QRect(0, 0, 64, 64));
    setHasLabel(true);
    setPortName("IC");
    setToolTip(m_translatedName);
    // qCDebug(zero) << "Box done.";
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
    stream << m_file;
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
    qCDebug(three) << "IC" << m_file << "-> Inputs. min:" << minInputSize() << ", max:" << maxInputSize() << ", current:" << inputSize() << ", m_inputs:" << m_inputs.size();
    for (int inputIndex = 0; inputIndex < prototype->inputSize(); ++inputIndex) {
        QNEPort *in = input(inputIndex);
        in->setName(prototype->inputLabel(inputIndex));
        in->setRequired(prototype->isInputRequired(inputIndex));
        in->setDefaultValue(prototype->defaultInputValue(inputIndex));
        in->setValue(static_cast<signed char>(prototype->defaultInputValue(inputIndex)));
    }
}

void IC::loadOutputs(ICPrototype *prototype)
{
    setMaxOutputSize(prototype->outputSize());
    setMinOutputSize(prototype->outputSize());
    setOutputSize(prototype->outputSize());
    for (int outputIndex = 0; outputIndex < prototype->outputSize(); ++outputIndex) {
        QNEPort *out = output(outputIndex);
        out->setName(prototype->outputLabel(outputIndex));
    }
    qCDebug(three) << "IC" << m_file << "-> Outputs. min:" << minOutputSize() << ", max:" << maxOutputSize() << ", current:" << outputSize() << ", m_outputs:" << m_outputs.size();
}

void IC::loadFile(const QString &fileName)
{
    // qCDebug(zero) << "Opening IC:" << fileName;
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
    // qCDebug(zero) << "IC loaded:" << fileName;
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

void IC::setSkin(const bool defaultSkin, const QString &fileName)
{
    m_defaultSkins[0] = (defaultSkin) ? ":/basic/box.png" : fileName;
    setPixmap(m_defaultSkins[0]);
}
