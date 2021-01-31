/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef IC_H
#define IC_H

#include "graphicelement.h"
#include "simulationcontroller.h"

class Editor;

class ICPrototype;

//!
//! \brief The IC class
//!
class IC : public GraphicElement
{
    Q_OBJECT

    friend class CodeGenerator;

public:
    IC(QGraphicsItem *parent = nullptr);
    ~IC() override;

    //!
    //! \brief save saves the IC's internal file to the given data stream
    //!
    void save(QDataStream &ds) const override;
    void load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap, double version) override;
    //!
    //! \brief loadFile is used when an IC is dragged into the editor
    //!
    void loadFile(const QString &fname);
    //! \brief getFile returns this->m_file
    QString getFile() const;
    bool setFile(const QString &newFileName);
    ICPrototype *getPrototype();
    // WARNING - TODO: getElements is unimplemented
    QVector<GraphicElement *> getElements() const;
    //!
    //! \brief setSkin changes the skin (or icon) of an IC in the editor
    //! \param defaultSkin, if true, makes the IC display its default skin.
    //! \param filename is the IC's new skin if defaultSkin is false.
    //!
    void setSkin(bool defaultSkin, const QString &filename) override;

private:
    Editor *editor;
    QString m_file;

    void loadInputs(ICPrototype *prototype);

    void loadOutputs(ICPrototype *prototype);

    /* QGraphicsItem interface */
protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
};

#endif /* IC_H */
