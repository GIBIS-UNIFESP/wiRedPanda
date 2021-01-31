/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ELEMENTEDITOR_H
#define ELEMENTEDITOR_H

#include <QWidget>

#include "graphicelement.h"
#include "scene.h"

class QUndoCommand;

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

signals:
    void sendCommand(QUndoCommand *cmd);

private slots:

    void selectionChanged();

    void on_lineEditElementLabel_editingFinished();

    void on_comboBoxInputSz_currentIndexChanged(int index);

    void on_doubleSpinBoxFrequency_editingFinished();

    void on_comboBoxColor_currentIndexChanged(int index);

    void on_lineEditTrigger_textChanged(const QString &arg1);

    void on_lineEditTrigger_editingFinished();

    bool eventFilter(QObject *obj, QEvent *event) override;

    void on_comboBoxAudio_currentIndexChanged(int);

    void on_pushButtonChangeSkin_clicked();

    void on_pushButtonDefaultSkin_clicked();

private:
    void setCurrentElements(const QVector<GraphicElement *> &element);
    void apply();

    Ui::ElementEditor * m_ui;
    QVector<GraphicElement *> m_elements;
    Scene *m_scene;
    Editor *m_editor;
    bool m_hasAnyProperty, m_hasLabel, m_hasColors, m_hasFrequency, m_hasAudio;
    bool m_canChangeInputSize, m_hasTrigger, m_hasRotation, m_canChangeSkin;
    bool m_hasSameLabel, m_hasSameColors, m_hasSameFrequency;
    bool m_hasSameInputSize, m_hasSameTrigger, m_canMorph, m_hasSameType;
    bool m_hasSameAudio;
    bool m_hasElements;

    QString m_manyLabels;
    QString m_manyColors;
    QString m_manyIS;
    QString m_manyFreq;
    QString m_manyTriggers;
    QString m_manyAudios;
    QString m_skinName;
    bool m_defaultSkin;
};

#endif /* ELEMENTEDITOR_H */
