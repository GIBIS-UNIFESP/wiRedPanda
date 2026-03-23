// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Custom QApplication subclass with exception handling and main-window access.
 */

#pragma once

#include <QApplication>

#include "App/UI/MainWindow.h"

/**
 * \class Application
 * \brief Custom QApplication that wraps event dispatch with exception handling.
 *
 * \details Overrides notify() to catch Pandaception and std::exception objects
 * thrown during event processing and display appropriate error dialogs.
 * Also provides typed access to the main window instance.
 */
class Application : public QApplication
{
    Q_OBJECT

public:
    /**
     * \brief Constructs the application with command-line arguments.
     * \param argc Argument count (passed by reference as required by QApplication).
     * \param argv Argument vector.
     */
    Application(int &argc, char **argv);

    /**
     * \brief Returns the application instance cast to Application*.
     * \return Pointer to the Application instance, or nullptr if not available.
     */
    static Application *instance();

    /// Destructor.
    ~Application() override = default;

    // --- Event Handling ---

    /// \reimp Dispatches \a event to \a receiver, catching and reporting exceptions.
    bool notify(QObject *receiver, QEvent *event) override;

    // --- Main Window Access ---

    /// Returns the application's main window.
    MainWindow *mainWindow() const;

    /**
     * \brief Sets the application's main window to \a mainWindow.
     * \param mainWindow Pointer to the MainWindow instance.
     */
    void setMainWindow(MainWindow *mainWindow);

    /// When false, suppresses informational dialogs (e.g. version-mismatch warnings).
    inline static bool interactiveMode = true;

private:
    Q_DISABLE_COPY(Application)

    // --- Members ---

    MainWindow *m_mainWindow = nullptr;
};

