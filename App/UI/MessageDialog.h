// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 *  \brief Thin wrapper around KMessageBox / QMessageBox that compiles with or
 *  without KDE Frameworks. Centralizes the conditional so call sites stay clean.
 *
 *  KDE builds get native-styled KDE dialogs (proper button labels, KNotify
 *  integration). Non-KDE builds fall back to plain QMessageBox.
 *
 *  Argument order follows the KMessageBox convention: (parent, text, title).
 */

#pragma once

#include <QString>

class QWidget;

#ifdef USE_KDE_FRAMEWORKS
#  include <KMessageBox>
#else
#  include <QMessageBox>
#endif

namespace MessageDialog {

inline void error(QWidget *parent, const QString &text, const QString &title = {})
{
#ifdef USE_KDE_FRAMEWORKS
    KMessageBox::error(parent, text, title);
#else
    QMessageBox::critical(parent, title, text);
#endif
}

inline void information(QWidget *parent, const QString &text, const QString &title = {})
{
#ifdef USE_KDE_FRAMEWORKS
    KMessageBox::information(parent, text, title);
#else
    QMessageBox::information(parent, title, text);
#endif
}

/// Single-OK warning. Maps to KMessageBox::error on KDE (no single-button warning exists).
inline void warning(QWidget *parent, const QString &text, const QString &title = {})
{
#ifdef USE_KDE_FRAMEWORKS
    KMessageBox::error(parent, text, title);
#else
    QMessageBox::warning(parent, title, text);
#endif
}

/// Yes/No question. Returns true on Yes/PrimaryAction.
inline bool questionYesNo(QWidget *parent, const QString &text, const QString &title = {})
{
#ifdef USE_KDE_FRAMEWORKS
    // KF6 dropped KStandardGuiItem::yes()/no(); construct items inline.
    return KMessageBox::questionTwoActions(parent, text, title,
                                            KGuiItem(QStringLiteral("&Yes")),
                                            KGuiItem(QStringLiteral("&No")))
           == KMessageBox::PrimaryAction;
#else
    return QMessageBox::question(parent, title, text,
                                  QMessageBox::Yes | QMessageBox::No)
           == QMessageBox::Yes;
#endif
}

} // namespace MessageDialog
