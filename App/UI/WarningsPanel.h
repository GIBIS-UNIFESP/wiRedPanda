// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <QString>

class QTextEdit;
class QListWidget;
class QLabel;
class QPushButton;
class GraphicElement;
class Scene;

class WarningsPanel : public QWidget
{
    Q_OBJECT
public:
    explicit WarningsPanel(QWidget *parent = nullptr);

    void setScene(Scene *scene);
    void showElementWarnings(GraphicElement *element);
    void refreshErrorList();
    void addLogEntry(const QString &entry);

signals:
    void errorElementActivated(GraphicElement *element);

private:
    QLabel *m_header;
    QListWidget *m_currentWarnings;
    QPushButton *m_errorsToggle;
    QListWidget *m_errors;
    QTextEdit *m_log;
    Scene *m_scene = nullptr;
};
