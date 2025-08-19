// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QHash>

class ArduinoTemplates
{
public:
    // Template generation methods
    static QString generateDFlipFlopTemplate(const QHash<QString, QString> &vars);
    static QString generateJKFlipFlopTemplate(const QHash<QString, QString> &vars);
    static QString generateSRFlipFlopTemplate(const QHash<QString, QString> &vars);
    static QString generateTFlipFlopTemplate(const QHash<QString, QString> &vars);
    static QString generateClockTemplate(const QHash<QString, QString> &vars);
    static QString generateCombinationalTemplate(const QString &operation, const QHash<QString, QString> &vars);
    
    // Specialized templates
    static QString generateInterruptHandlerTemplate(const QHash<QString, QString> &vars);
    static QString generateTimerISRTemplate(const QHash<QString, QString> &vars);
    static QString generateDebounceTemplate(const QHash<QString, QString> &vars);
    
    // Complete code section templates
    static QString generateSetupSectionTemplate(const QHash<QString, QString> &vars);
    static QString generateLoopSectionTemplate(const QHash<QString, QString> &vars);
    static QString generateIncludesSectionTemplate(const QHash<QString, QString> &vars);
    
private:
    // Template substitution helper
    static QString substituteVariables(const QString &templateStr, const QHash<QString, QString> &vars);
    
    // Template constants
    static const QString DFLIPFLOP_TEMPLATE;
    static const QString JKFLIPFLOP_TEMPLATE;
    static const QString SRFLIPFLOP_TEMPLATE;
    static const QString TFLIPFLOP_TEMPLATE;
    static const QString INTERRUPT_HANDLER_TEMPLATE;
    static const QString TIMER_ISR_TEMPLATE;
    static const QString DEBOUNCE_TEMPLATE;
    static const QString SETUP_SECTION_TEMPLATE;
    static const QString LOOP_SECTION_TEMPLATE;
    static const QString INCLUDES_SECTION_TEMPLATE;
};
