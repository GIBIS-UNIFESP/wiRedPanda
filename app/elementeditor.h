/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "graphicelement.h"

#include <QWidget>

class Editor;
class QUndoCommand;
class Scene;

namespace Ui
{
class ElementEditor;
}

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

    // void renameAction(const QVector<GraphicElement *> &element);
    // void changeColorAction(const QVector<GraphicElement *> &element);

    void fillColorComboBox();
    void updateElementSkin();

    void setEditor(Editor *value);

    bool eventFilter(QObject *obj, QEvent *event) override;

    void disable();
signals:
    void sendCommand(QUndoCommand *cmd);

private slots:

    void selectionChanged();
    void inputIndexChanged(int idx);
    void outputIndexChanged(const QString &idx);
    void outputValueChanged(const QString &idx);
    void inputLocked(const bool value);
    void triggerChanged(const QString &cmd);
    void defaultSkin();

private:
    void setCurrentElements(const QVector<GraphicElement *> &elms);
    void updateSkins();
    void apply();

    Ui::ElementEditor *m_ui;
    QVector<GraphicElement *> m_elements;
    Scene *m_scene;
    Editor *m_editor;
    bool m_hasAnyProperty, m_hasLabel, m_hasColors, m_hasFrequency, m_hasAudio;
    bool m_canChangeInputSize, m_canChangeOutputSize;
    bool m_hasTrigger, m_hasRotation, m_canChangeSkin;
    bool m_hasSameLabel, m_hasSameColors, m_hasSameFrequency;
    bool m_hasSameInputSize, m_hasSameOutputSize, m_hasSameOutputValue;
    bool m_hasSameTrigger, m_canMorph, m_hasSameType;
    bool m_hasSameAudio;
    bool m_hasElements;
    bool m_hasOnlyInputs;

    QString m_manyLabels;
    QString m_manyColors;
    QString m_manyIS;
    QString m_manyOS;
    QString m_manyOV;
    QString m_manyFreq;
    QString m_manyTriggers;
    QString m_manyAudios;
    QString m_skinName;
    bool m_updatingSkin;
    bool m_defaultSkin;
};

