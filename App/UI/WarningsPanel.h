// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <QString>

class QListWidget;
class QLabel;
class GraphicElement;
class Scene;

class WarningsPanel : public QWidget
{
    Q_OBJECT
public:
    explicit WarningsPanel(QWidget *parent = nullptr, bool summaryOnly = false);

    void setScene(Scene *scene);
    void showElementWarnings(GraphicElement *element);
    void refreshWarningsList();

signals:
    void warningElementActivated(GraphicElement *element);

private:
    QLabel *m_header;
    QLabel *m_description;
    QLabel *m_count;
    QLabel *m_emptyState;
    QListWidget *m_currentWarnings;
    QListWidget *m_warnings;
    Scene *m_scene = nullptr;
    bool m_summaryOnly = false;
};
