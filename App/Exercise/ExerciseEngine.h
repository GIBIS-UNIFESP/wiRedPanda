// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>

#include <QObject>
#include <QVector>

#include "App/Core/StepEngineCore.h"
#include "App/Exercise/ExerciseStep.h"

class CanvasItem;
class GraphicElement;

class ExerciseEngine : public QObject
{
    Q_OBJECT

public:
    explicit ExerciseEngine(QObject *parent = nullptr);

    /// Loads an exercise from a Qt resource path (e.g. ":/Exercises/basic-and-gate.json").
    /// Returns false and leaves the engine inactive if the resource is missing or malformed.
    bool loadFromResource(const QString &resourcePath);

    QString exerciseId()    const { return m_core.id(); }
    QString exerciseTitle() const { return m_core.title(); }

    /// Binds the engine to \a canvas. Disconnects the previous canvas if any. Pass nullptr to
    /// detach without binding a new canvas.
    ///
    /// Reuses CanvasItem::undoStack()'s indexChanged signal as the "circuit changed" trigger --
    /// the exact signal QuickMinimap already uses for the identical purpose. ExerciseStep
    /// validation (requiredElements/requiredConnections) is entirely structural, never live
    /// simulation values, so a structural-edit signal is exactly the right trigger.
    ///
    /// Defined in a separate translation unit (App/QuickShell/Chrome/QuickExerciseEngineBinding.cpp,
    /// compiled only into wiredpanda_quick), not ExerciseEngine.cpp: this class is shared,
    /// whole-archived Layer 1 code compiled once into wiredpanda_lib, which is deliberately built
    /// with no Qt Quick dependency at all (see CMakeLists.txt's QT_LIBS/QUICK_LIBS split) --
    /// CanvasItem.h (a real QQuickItem) can't be included from anything wiredpanda_lib compiles.
    /// m_connectFn/m_disconnectFn/m_elementsFn (std::function, fully type-erased) are what let
    /// this shared class stay ignorant of CanvasItem's concrete type: setCanvas() assigns
    /// CanvasItem-capturing closures from within its own Quick-only TU; every other method
    /// (start()/stop()/markCompleted()/validation) only ever calls through the type-erased
    /// std::function members, never a concrete CanvasItem* directly.
    void setCanvas(CanvasItem *canvas);

    int  currentStep() const { return m_core.currentStep(); }
    int  totalSteps()  const { return m_core.totalSteps(); }
    bool isActive()    const { return m_core.isActive(); }

    /// Returns the data for the current step. Caller must check isActive() first.
    const ExerciseStep &currentStepData() const;

    void start();
    void stop();
    void goToPreviousStep();

    /// Manually advances one step (used by the overlay Next/Finish button for observe steps).
    void advanceStep();

    /// Re-translates the title and all steps' instruction/hint text for the current
    /// language, preserving currentStep()/isActive(). No-op if never loaded or inactive.
    /// Emits retranslated() (never stepChanged()) so listeners can't replay step-entry
    /// side effects like MainWindow's click-target handling.
    void retranslate();

signals:
    void stepChanged(int step, int total, const ExerciseStep &data);
    void stepCompleted(int step);
    void exerciseCompleted();
    void exerciseStopped();
    void retranslated();

private slots:
    void onCircuitChanged();

private:
    /// Returns true when the current step's requirements are satisfied by the current scene.
    /// Always returns false for observe steps (both requirement vectors empty).
    bool validateCurrentStep() const;

    bool validateElements(const QVector<ExerciseElementRequirement> &reqs) const;
    bool validateConnections(const QVector<ExerciseConnectionRequirement> &reqs) const;

    void emitCurrentStep();
    void markCompleted();

    /// Shared "advance the core, then react" tail for advanceStep() and onCircuitChanged(),
    /// which differ only in their guard.
    void performAdvance();

    /// Returns the bound scene's/canvas's elements(), or an empty vector if neither is bound.
    /// A thin, type-agnostic wrapper around m_elementsFn -- see its own doc comment.
    QVector<GraphicElement *> currentElements() const;

    StepEngineCore<ExerciseStep> m_core;

    /// Type-erased "connect/disconnect the currently-bound canvas's change signal" and "read
    /// its current elements" -- assigned by setCanvas() (compiled in its own Quick-only TU).
    /// Each closure captures a *local* QPointer<CanvasItem> by value (not a class member) for
    /// the usual destroyed-out-from-under-us safety net, without this class ever storing that
    /// type as a member -- deliberately: a stored QPointer<CanvasItem> member would need
    /// CanvasItem's complete type wherever it's ever assigned, but this class is shared,
    /// whole-archived Layer 1 code compiled once into wiredpanda_lib, deliberately built with
    /// no Qt Quick dependency at all (see CMakeLists.txt's QT_LIBS/QUICK_LIBS split) --
    /// CanvasItem.h (a real QQuickItem) can't be included from anything wiredpanda_lib compiles.
    /// Every other method (start()/stop()/markCompleted()/currentElements()) calls through
    /// these std::function members only, never a concrete CanvasItem* directly.
    std::function<void()> m_connectFn;
    std::function<void()> m_disconnectFn;
    std::function<QVector<GraphicElement *>()> m_elementsFn;
};
