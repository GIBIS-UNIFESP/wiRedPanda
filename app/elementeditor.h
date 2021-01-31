/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ELEMENTEDITOR_H
#define ELEMENTEDITOR_H

#include <QWidget>

#include "graphicelement.h"

class QUndoCommand;
class Scene;

namespace Ui
{
class ElementEditor;
}

class Editor;

class ElementEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ElementEditor(QWidget *parent = nullptr);
    ~ElementEditor() override;

    void setScene(Scene *s);
    void contextMenu(QPoint screenPos);
    void renameAction();
    void changeTriggerAction();
    void retranslateUi();
    /*
     *  void renameAction( const QVector< GraphicElement *> &element );
     *  void changeColorAction( const QVector<GraphicElement *> &element );
     */

    void fillColorComboBox();
    void updateElementSkin();

    void setEditor(Editor *value);

    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
    void sendCommand(QUndoCommand *cmd);


private slots:

    void selectionChanged();

    void inputIndexChanged(int index);
    void triggerChanged(const QString &arg1);
    void defaultSkin();

private:
    void setCurrentElements(const QVector<GraphicElement *> &element);
    void apply();

    Ui::ElementEditor *ui;
    QVector<GraphicElement *> m_elements;
    Scene *scene;
    Editor *editor;
    bool hasAnyProperty, hasLabel, hasColors, hasFrequency, hasAudio;
    bool canChangeInputSize, hasTrigger, hasRotation, canChangeSkin;
    bool hasSameLabel, hasSameColors, hasSameFrequency;
    bool hasSameInputSize, hasSameTrigger, canMorph, hasSameType;
    bool hasSameAudio;
    bool hasElements;

    QString _manyLabels;
    QString _manyColors;
    QString _manyIS;
    QString _manyFreq;
    QString _manyTriggers;
    QString _manyAudios;
    QString m_skinName;
    bool m_defaultSkin;
};

#endif /* ELEMENTEDITOR_H */
