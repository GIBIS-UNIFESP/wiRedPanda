// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Core/Application.h"

#include <array>

#include <QElapsedTimer>
#include <QFontDatabase>
#include <QMutex>

#include "App/Core/Common.h"

#ifdef HAVE_SENTRY
#include "thirdparty/sentry/include/sentry.h"
#endif

namespace {

/// Substrings of throw messages we deliberately drop on the floor.
/// A message belongs here only if it has BOTH of:
///   1. no UX recovery path the user can take when it fires, and
///   2. no actionable signal for us — either the throw is a pure defensive net
///      whose root causes are addressed elsewhere, or it's environmental and
///      not a wiRedPanda bug.
/// Keep this list short. When a new pattern is added, document the Sentry
/// issue ID(s) it suppresses next to the entry so future cleanup can verify
/// the pattern is still load-bearing.
constexpr std::array kSentryDenyPatterns {
    // Defensive nets whose Cluster D root causes were closed in the 5.0.2 cycle
    // (A14/A15/C5/C7/C8). If they fire on a current build it's already too late
    // to act on them: the scene is in a divergent state, the message box has
    // already shown, and the user's next save autosave-rolls a corrupt file.
    "One or more items was not found on the scene",
    "One or more elements was not found on the scene",
    "One or more elements were not found on scene",
};

#ifdef HAVE_SENTRY
/// Deduplicates identical Sentry events within a cooldown window and drops
/// known-noise messages outright.  Returns true if the event should be sent.
bool shouldSendToSentry(const QString &message)
{
    if (Application::isSentryDenyMessage(message)) {
        return false;
    }

    static QString lastMessage;
    static QElapsedTimer timer;
    static QMutex mutex;

    QMutexLocker lock(&mutex);

    constexpr qint64 cooldownMs = 5000;

    if (message == lastMessage && timer.isValid() && timer.elapsed() < cooldownMs) {
        return false;
    }

    lastMessage = message;
    timer.start();
    return true;
}
#endif

} // namespace

Application::ExceptionPresenter Application::s_exceptionPresenter;

void Application::registerBundledFonts()
{
    // Both weights are embedded (not just Regular, with the Bold face left to Qt's own
    // synthesis) because synthetic-bold advance widths are computed by the platform's font
    // engine -- DirectWrite, CoreText, FreeType each embolden slightly differently -- so a
    // synthesized bold label's on-screen geometry isn't actually identical across platforms.
    // TestICFixtureLayout's overlap checks caught exactly this: clean on Linux (FreeType) but
    // several fixtures failed on Windows (DirectWrite) with real, non-suppressible px overlaps.
    // A real embedded bold face's glyph outlines/advance widths are baked into the file, so
    // every platform lays out the identical bold glyphs -- only sub-pixel hinting/rasterization
    // differences remain.
    //
    // The if-failure branches below (LCOV_EXCL_LINE) stay uncovered on purpose: nothing can
    // corrupt the embedded resource to make addApplicationFont() fail from a test.
    if (QFontDatabase::addApplicationFont(QStringLiteral(":/Fonts/NotoSans-Regular.ttf")) == -1) { // LCOV_EXCL_LINE
        qWarning() << "Failed to register bundled font: NotoSans-Regular.ttf"; // LCOV_EXCL_LINE
    }
    if (QFontDatabase::addApplicationFont(QStringLiteral(":/Fonts/NotoSans-Bold.ttf")) == -1) { // LCOV_EXCL_LINE
        qWarning() << "Failed to register bundled font: NotoSans-Bold.ttf"; // LCOV_EXCL_LINE
    }
}

void Application::setExceptionPresenter(ExceptionPresenter presenter)
{
    s_exceptionPresenter = std::move(presenter);
}

bool Application::isSentryDenyMessage(const QString &message)
{
    for (const auto *pattern : kSentryDenyPatterns) {
        if (message.contains(QLatin1String(pattern))) {
            return true;
        }
    }
    return false;
}

ExceptionInfo Application::makeExceptionInfo(const std::exception &e)
{
    ExceptionInfo info;
    info.what = QString::fromUtf8(e.what());
    if (const auto *pandaEx = dynamic_cast<const Pandaception *>(&e)) {
        info.englishMessage = pandaEx->englishMessage();
        const char *f = pandaEx->file();
        info.file = f ? QString::fromUtf8(f) : QString();
        info.line = pandaEx->line();
    } else {
        info.englishMessage = info.what;
    }
    return info;
} // LCOV_EXCL_LINE -- compiler-generated cleanup for the returned ExceptionInfo's exception-unwind path, never taken

void Application::handleException(const ExceptionInfo &info, const QObject *context)
{
    if (Application::interactiveMode && s_exceptionPresenter) {
        s_exceptionPresenter(info.what, context);
    }

#ifdef HAVE_SENTRY
    sentry_set_tag("app.interactive", interactiveMode ? "true" : "false");

    if (!shouldSendToSentry(info.englishMessage)) {
        return;
    }

    sentry_value_t event_ = sentry_value_new_event();
    sentry_value_set_by_key(event_, "level", sentry_value_new_string("warning"));

    sentry_value_t exc = sentry_value_new_exception(
        "Exception", info.englishMessage.toStdString().c_str());

    sentry_value_t mechanism = sentry_value_new_object();
    sentry_value_set_by_key(mechanism, "type", sentry_value_new_string("generic"));
    sentry_value_set_by_key(mechanism, "handled", sentry_value_new_bool(1));
    if (!info.file.isEmpty()) {
        // Include the throw-site location so Sentry shows where the
        // exception originated rather than the catch block.
        sentry_value_set_by_key(mechanism, "description",
            sentry_value_new_string(
                QStringLiteral("%1:%2").arg(info.file).arg(info.line).toStdString().c_str()));
    }
    sentry_value_set_by_key(exc, "mechanism", mechanism);

    sentry_value_set_stacktrace(exc, NULL, 0);
    sentry_event_add_exception(event_, exc);
    sentry_capture_event(event_);
#endif
}
