// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QWidget>

class QListWidget;
class QLabel;
class GraphicElement;
class Scene;

class WarningsPanel : public QWidget
{
    Q_OBJECT
public:
    explicit WarningsPanel(QWidget *parent = nullptr);

    void setScene(Scene *scene);
    void refreshWarningsList();
    void retranslateUi();

signals:
    void warningElementActivated(int elementId);

private:
    QLabel *m_count;
    QListWidget *m_warnings;
    Scene *m_scene = nullptr;
};
