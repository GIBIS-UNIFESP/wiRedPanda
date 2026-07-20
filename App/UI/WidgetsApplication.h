// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief WidgetsApplication: the real QApplication wiredpanda's App/Main.cpp constructs.
 */

#pragma once

#include <QApplication>

/**
 * \class WidgetsApplication
 * \brief Custom QApplication that wraps event dispatch with exception handling.
 *
 * \details Overrides notify() to catch std::exception objects thrown during event processing
 * and report them via Application::handleException() (App/Core/Application.h — the
 * framework-agnostic runtime-flags/exception-safety class both apps share; this class is only
 * the Widgets-specific application object, split out so Application itself never needs
 * Qt6::Widgets). Registers the real QMessageBox-showing exception presenter at construction —
 * see the .cpp.
 */
class WidgetsApplication : public QApplication
{
    Q_OBJECT

public:
    /**
     * \brief Constructs the application with command-line arguments.
     * \param argc Argument count (passed by reference as required by QApplication).
     * \param argv Argument vector.
     */
    WidgetsApplication(int &argc, char **argv);

    /// Destructor.
    ~WidgetsApplication() override = default;

    /// \reimp Dispatches \a event to \a receiver, catching and reporting exceptions.
    bool notify(QObject *receiver, QEvent *event) override;

private:
    Q_DISABLE_COPY(WidgetsApplication)
};
