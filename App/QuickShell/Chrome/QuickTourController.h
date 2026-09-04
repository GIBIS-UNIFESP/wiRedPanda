// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickTourController: QML-facing presenter driving the guided-interface-tour overlay.
 */

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QStringList>

#include "App/Tour/TourEngine.h"

/**
 * \class QuickTourController
 * \brief Copy-and-adapted, Widgets-free presenter for the Quick chrome's guided-tour overlay.
 *
 * \details Owns a real TourEngine directly (unlike QuickExerciseController's externally-owned
 * ExerciseEngine): TourEngine has zero Scene/Widgets coupling, so there's no CanvasItem-binding
 * split to route around.
 *
 * Target resolution (mapping a step's target id to a highlight rect) and click dispatch
 * (activating a step's click ids, e.g. switching a palette tab) are NOT done here: both need
 * the actual Main.qml item tree (toolbar buttons, palette tabs, element editor), which only
 * Main.qml itself has by id -- see TourOverlay.qml's own doc comment for where that logic
 * actually lives. This class only exposes the *data* (currentTarget as a plain string,
 * clicksRequested as a signal carrying the id list) for that QML-side logic to act on.
 */
class QuickTourController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("QuickTourController is only ever exposed via AppController.tour")

    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged FINAL)
    Q_PROPERTY(QString stepCounterText READ stepCounterText NOTIFY stepChanged FINAL)
    Q_PROPERTY(QString title READ title NOTIFY stepChanged FINAL)
    Q_PROPERTY(QString body READ body NOTIFY stepChanged FINAL)
    Q_PROPERTY(QString currentTarget READ currentTarget NOTIFY stepChanged FINAL)
    Q_PROPERTY(bool prevEnabled READ isPrevEnabled NOTIFY stepChanged FINAL)
    Q_PROPERTY(QString nextButtonText READ nextButtonText NOTIFY stepChanged FINAL)

public:
    explicit QuickTourController(QObject *parent = nullptr);

    [[nodiscard]] bool isActive() const { return m_engine.isActive(); }
    [[nodiscard]] QString stepCounterText() const;
    [[nodiscard]] QString title() const { return m_title; }
    [[nodiscard]] QString body() const { return m_body; }
    [[nodiscard]] QString currentTarget() const { return m_currentTarget; }
    [[nodiscard]] bool isPrevEnabled() const { return m_prevEnabled; }
    [[nodiscard]] QString nextButtonText() const { return m_nextButtonText; }

    /// Loads \a resourcePath and starts the tour. Returns false (and starts nothing) if the
    /// resource fails to load. Mirrors MainWindow::startTour().
    Q_INVOKABLE bool start(const QString &resourcePath);
    Q_INVOKABLE void goToPreviousStep();
    Q_INVOKABLE void advanceStep();
    /// Stops the active tour. Mirrors TourOverlay::closeRequested()'s handler.
    Q_INVOKABLE void close();

signals:
    void activeChanged();
    /// Covers stepCounterText/title/body/currentTarget/prevEnabled/nextButtonText together --
    /// mirrors TourOverlay::onStepChanged() updating everything from one engine signal.
    void stepChanged();
    /// Emitted first, before stepChanged(), carrying the new step's click ids -- mirrors
    /// MainWindow's click-before-target-resolution ordering. This is a real emission-order
    /// guarantee: onEngineStepChanged() (below) emits this signal and returns from its QML-side
    /// Connections handler -- a direct, same-thread connection, so fully synchronous -- before
    /// calling applyStepData(), which updates currentTarget and emits stepChanged(). By the time
    /// any QML binding reads currentTarget, this step's clicks (e.g. a palette tab switch) have
    /// already been dispatched.
    void clicksRequested(const QStringList &ids);

private:
    /// Updates every stepChanged()-covered field from \a data and emits stepChanged(). Shared by
    /// onEngineStepChanged() (a real step transition) and the retranslated() handler (text-only
    /// refresh, same step) -- both need the same field set recomputed.
    void applyStepData(int step, int total, const TourStep &data);
    /// Connected to TourEngine::stepChanged() -- the only path that should replay a step's click
    /// ids (TourEngine::retranslate()'s own doc comment: retranslated() is deliberately a
    /// separate signal specifically so listeners "can't replay step-entry side effects like
    /// MainWindow's click-target handling").
    void onEngineStepChanged(int step, int total, const TourStep &data);
    void onTourFinished();

    TourEngine m_engine;

    int m_step = 0;
    int m_total = 0;
    QString m_title;
    QString m_body;
    QString m_currentTarget;
    bool m_prevEnabled = false;
    QString m_nextButtonText;
};
