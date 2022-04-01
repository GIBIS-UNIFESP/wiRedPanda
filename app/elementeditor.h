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

    bool eventFilter(QObject *obj, QEvent *event) override;
    void changeTriggerAction();
    void contextMenu(QPoint screenPos);
    void disable();
    void fillColorComboBox();
    void renameAction();
    void retranslateUi();
    void setEditor(Editor *value);
    void setScene(Scene *s);
    void updateElementSkin();

signals:
    void sendCommand(QUndoCommand *cmd);

private:
    void apply();
    void defaultSkin();
    void inputIndexChanged(int idx);
    void inputLocked(const bool value);
    void outputIndexChanged(const QString &idx);
    void outputValueChanged(const QString &idx);
    void selectionChanged();
    void setCurrentElements(const QVector<GraphicElement *> &elms);
    void triggerChanged(const QString &cmd);
    void updateSkins();

    Ui::ElementEditor *m_ui;
    Editor *m_editor;
    QString m_manyAudios;
    QString m_manyColors;
    QString m_manyFreq;
    QString m_manyIS;
    QString m_manyLabels;
    QString m_manyOS;
    QString m_manyOV;
    QString m_manyTriggers;
    QString m_skinName;
    QVector<GraphicElement *> m_elements;
    Scene *m_scene;
    bool m_canChangeInputSize, m_canChangeOutputSize;
    bool m_defaultSkin;
    bool m_hasAnyProperty, m_hasLabel, m_hasColors, m_hasFrequency, m_hasAudio;
    bool m_hasElements;
    bool m_hasOnlyInputs;
    bool m_hasSameAudio;
    bool m_hasSameInputSize, m_hasSameOutputSize, m_hasSameOutputValue;
    bool m_hasSameLabel, m_hasSameColors, m_hasSameFrequency;
    bool m_hasSameTrigger, m_canMorph, m_hasSameType;
    bool m_hasTrigger, m_hasRotation, m_canChangeSkin;
    bool m_updatingSkin;
};

