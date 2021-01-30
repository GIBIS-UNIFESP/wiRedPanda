// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ic.h"

#include <iostream>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QPointF>
#include <QProcess>

#include "globalproperties.h"
#include "icmanager.h"
#include "icnotfoundexception.h"
#include "icprototype.h"
#include "inputswitch.h"
#include "nodes/qneconnection.h"
#include "serializationfunctions.h"

IC::IC(QGraphicsItem *parent)
    : GraphicElement(ElementType::IC, ElementGroup::IC, 0, 0, 0, 0, parent)
{
    pixmapSkinName.append(":/basic/box.png");
    setHasLabel(true);
    setPixmap(pixmapSkinName[0], QRect(0, 0, 64, 64));
    setOutputsOnTop(true);
    setPortName("IC");
}

IC::~IC()
{
    ICPrototype *prototype = ICManager::instance()->getPrototype(m_file);
    if (prototype) {
        prototype->removeICObserver(this);
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
    COMMENT("IC " << m_file.toStdString() << " -> Inputs. min: " << minInputSz() << ", max: " << maxInputSz() << ", current: " << inputSize()
                  << ", m_inputs: " << m_inputs.size(),
            0);
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
        QNEPort *in = output(outputIdx);
        in->setName(prototype->outputLabel(outputIdx));
    }
    COMMENT("IC " << m_file.toStdString() << " -> Outputs. min: " << minOutputSz() << ", max: " << maxOutputSz() << ", current: " << outputSize()
                  << ", m_outputs: " << m_outputs.size(),
            0);
}

void IC::loadFile(QString fname)
{
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
}

QString IC::getFile() const
{
    return m_file;
}

bool IC::setFile(QString newFileName)
{
    COMMENT("Updating ic name.", 0);
    if (!ICManager::instance()->updatePrototypeFilePathName(m_file, newFileName)) {
        return false;
    }
    m_file = newFileName;
    return true;
}

ICPrototype *IC::getPrototype()
{
    return ICManager::instance()->getPrototype(m_file);
}

void IC::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QMessageBox msgBox;
        /*    msgBox.setParent(  ); */
        msgBox.setLocale(QLocale::Portuguese);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setText(tr("Do you want to load this file?<br>%1").arg(m_file));
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setDefaultButton(QMessageBox::Yes);
        if (msgBox.exec() == QMessageBox::Yes) {
            auto *wPanda = new QProcess(scene());
            QStringList args;
            args << m_file;
            wPanda->start(QCoreApplication::applicationFilePath(), args);
        }
    }
}

void IC::setSkin(bool defaultSkin, QString filename)
{
    if (defaultSkin) {
        pixmapSkinName[0] = ":/basic/box.png";
    } else {
        pixmapSkinName[0] = filename;
    }
    setPixmap(pixmapSkinName[0]);
}
