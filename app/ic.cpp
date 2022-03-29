// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ic.h"

#include "common.h"
#include "icmanager.h"
#include "icprototype.h"
#include "qneport.h"

#include <QGraphicsSceneMouseEvent>

IC::IC(QGraphicsItem *parent)
    : GraphicElement(ElementType::IC, ElementGroup::IC, 0, 0, 0, 0, parent)
{
    // qCDebug(zero) << "Creating box.";
    m_pixmapSkinName.append(":/basic/box.png");
    setHasLabel(true);
    setPixmap(m_pixmapSkinName[0], QRect(0, 0, 64, 64));
    setOutputsOnTop(true);
    setPortName("IC");
    // qCDebug(zero) << "Box done.";
}

IC::~IC()
{
    if (ICManager::instance() != nullptr) {
        ICPrototype *prototype = ICManager::instance()->getPrototype(m_file);
        if (prototype) {
            prototype->removeICObserver(this);
        }
    }
}

void IC::save(QDataStream &ds) const
{
    GraphicElement::save(ds);
    ds << m_file;
}

void IC::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version)
{
    GraphicElement::load(ds, portMap, version);
    if (version >= 1.2) {
        ds >> m_file;
    }
}

void IC::loadInputs(ICPrototype *prototype)
{
    setMaxInputSz(prototype->inputSize());
    setMinInputSz(prototype->inputSize());
    setInputSize(prototype->inputSize());
    qCDebug(three) << "IC" << m_file << "-> Inputs. min:" << minInputSz() << ", max:" << maxInputSz() << ", current:" << inputSize() << ", m_inputs:" << m_inputs.size();
    for (int inputIdx = 0; inputIdx < prototype->inputSize(); ++inputIdx) {
        QNEPort *in = input(inputIdx);
        in->setName(prototype->inputLabel(inputIdx));
        in->setRequired(prototype->isInputRequired(inputIdx));
        in->setDefaultValue(prototype->defaultInputValue(inputIdx));
        in->setValue(prototype->defaultInputValue(inputIdx));
    }
}

void IC::loadOutputs(ICPrototype *prototype)
{
    setMaxOutputSz(prototype->outputSize());
    setMinOutputSz(prototype->outputSize());
    setOutputSize(prototype->outputSize());
    for (int outputIdx = 0; outputIdx < prototype->outputSize(); ++outputIdx) {
        QNEPort *out = output(outputIdx);
        out->setName(prototype->outputLabel(outputIdx));
    }
    qCDebug(three) << "IC" << m_file << "-> Outputs. min:" << minOutputSz() << ", max:" << maxOutputSz() << ", current:" << outputSize() << ", m_outputs:" << m_outputs.size();
}

void IC::loadFile(const QString &fname)
{
    // qCDebug(zero) << "Opening IC:" << fname;
    ICPrototype *prototype = ICManager::instance()->getPrototype(fname);
    Q_ASSERT(prototype);
    m_file = prototype->fileName();
    setToolTip(m_file);
    prototype->insertICObserver(this);
    if (getLabel().isEmpty()) {
        setLabel(prototype->baseName().toUpper());
    }
    // Loading inputs
    loadInputs(prototype);
    // Loading outputs
    loadOutputs(prototype);
    updatePorts();
    // qCDebug(zero) << "IC loaded:" << fname;
}

QString IC::getFile() const
{
    return m_file;
}

ICPrototype *IC::getPrototype()
{
    return ICManager::instance()->getPrototype(m_file);
}

void IC::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    ICManager::instance()->openIC(m_file);
}

void IC::setSkin(bool defaultSkin, const QString &filename)
{
    if (defaultSkin) {
        m_pixmapSkinName[0] = ":/basic/box.png";
    } else {
        m_pixmapSkinName[0] = filename;
    }
    setPixmap(m_pixmapSkinName[0]);
}
