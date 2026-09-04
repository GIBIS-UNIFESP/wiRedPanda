// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief QuickExerciseController: QML-facing presenter driving the circuit-exercise overlay.
 */

#pragma once

#include <QObject>
#include <QPointer>
#include <QQmlEngine>
#include <QString>

class CanvasItem;
class ExerciseEngine;
struct ExerciseStep;

/**
 * \class QuickExerciseController
 * \brief Copy-and-adapted, Widgets-free presenter for the Quick chrome's exercise overlay.
 *
 * \details Wraps a real, shared ExerciseEngine (App/Exercise/ExerciseEngine.h) -- the same
 * class MainWindow drives on the Widgets side -- translating its signals into QML-bindable
 * properties, the way ExerciseOverlay's setupUi()/onStepChanged()/onExerciseCompleted() drive
 * real QLabel/QPushButton widgets there. setCanvas() is called on every tab switch (mirrors
 * MainWindow.cpp's tab-switch rebind, itself guarded on "only while an exercise is active" --
 * see its own doc comment) and start() is what actually launches an exercise (mirrors
 * MainWindow::startExercise()).
 */
class QuickExerciseController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("QuickExerciseController is only ever exposed via AppController.exercise")

    // FINAL: QuickExerciseController is never subclassed, matching every other presenter in
    // this Quick chrome since the AOT-compilation refactor (project_qml_aot_compilation_fusion_style_pin).
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged FINAL)
    Q_PROPERTY(bool completed READ isCompleted NOTIFY completedChanged FINAL)
    Q_PROPERTY(QString stepCounterText READ stepCounterText NOTIFY stepChanged FINAL)
    Q_PROPERTY(QString instruction READ instruction NOTIFY stepChanged FINAL)
    Q_PROPERTY(QString hint READ hint NOTIFY stepChanged FINAL)
    Q_PROPERTY(bool hintVisible READ isHintVisible NOTIFY hintVisibleChanged FINAL)
    Q_PROPERTY(QString hintButtonText READ hintButtonText NOTIFY hintVisibleChanged FINAL)
    Q_PROPERTY(bool prevEnabled READ isPrevEnabled NOTIFY stepChanged FINAL)
    Q_PROPERTY(bool nextEnabled READ isNextEnabled NOTIFY stepChanged FINAL)
    Q_PROPERTY(QString nextButtonText READ nextButtonText NOTIFY stepChanged FINAL)

public:
    /// \a engine is owned by the caller (QuickAppController, mirroring MainWindow owning
    /// m_exerciseEngine separately from m_exerciseOverlay) and must outlive this presenter.
    explicit QuickExerciseController(ExerciseEngine *engine, QObject *parent = nullptr);

    /// Records \a canvas as "the current tab's canvas" and, if an exercise is currently active,
    /// immediately rebinds the engine to it too -- mirrors MainWindow.cpp's tab-switch handler,
    /// which only calls ExerciseEngine::setScene() again while m_exerciseEngine->isActive().
    /// A canvas recorded while no exercise is active is simply remembered for the next start().
    void setCanvas(CanvasItem *canvas);

    [[nodiscard]] bool isActive() const;
    [[nodiscard]] bool isCompleted() const { return m_completed; }
    [[nodiscard]] QString stepCounterText() const;
    /// The completion message once completed (mirrors ExerciseOverlay::onExerciseCompleted()'s
    /// "Exercise complete! Well done." text swap), the current step's instruction otherwise.
    [[nodiscard]] QString instruction() const;
    [[nodiscard]] QString hint() const { return m_hint; }
    [[nodiscard]] bool isHintVisible() const { return m_hintVisible; }
    [[nodiscard]] QString hintButtonText() const;
    [[nodiscard]] bool isPrevEnabled() const { return m_prevEnabled; }
    [[nodiscard]] bool isNextEnabled() const { return m_nextEnabled; }
    [[nodiscard]] QString nextButtonText() const { return m_nextButtonText; }

    /// Loads \a resourcePath and starts the exercise against the last canvas passed to
    /// setCanvas(). Returns false (and starts nothing) if the resource fails to load. Mirrors
    /// MainWindow::startExercise().
    Q_INVOKABLE bool start(const QString &resourcePath);
    Q_INVOKABLE void toggleHint();
    Q_INVOKABLE void goToPreviousStep();
    /// Advances one step (observe steps' Next button), or -- once completed -- acts as the
    /// relabeled Close button, mirroring ExerciseOverlay::onExerciseCompleted()'s rewiring of
    /// the Next button's own click handler.
    Q_INVOKABLE void advanceStep();
    /// Stops the active exercise. Mirrors ExerciseOverlay::closeRequested()'s handler.
    Q_INVOKABLE void close();

signals:
    void activeChanged();
    void completedChanged();
    /// Covers stepCounterText/instruction/hint/prevEnabled/nextEnabled/nextButtonText together
    /// -- mirrors ExerciseOverlay::onStepChanged() updating all of its labels/buttons from one
    /// engine signal.
    void stepChanged();
    void hintVisibleChanged();

private:
    void onStepChanged(int step, int total, const ExerciseStep &data);
    void onExerciseCompleted();
    void onExerciseStopped();
    void onRetranslated();

    ExerciseEngine *m_engine;
    QPointer<CanvasItem> m_canvas;

    int m_step = 0;
    int m_total = 0;
    QString m_instruction;
    QString m_hint;
    bool m_hintVisible = false;
    bool m_prevEnabled = false;
    bool m_nextEnabled = false;
    QString m_nextButtonText;
    bool m_completed = false;
};
