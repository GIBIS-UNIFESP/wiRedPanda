// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QLoggingCategory>
#include <stdexcept>

Q_DECLARE_LOGGING_CATEGORY(zero)
Q_DECLARE_LOGGING_CATEGORY(one)
Q_DECLARE_LOGGING_CATEGORY(two)
Q_DECLARE_LOGGING_CATEGORY(three)
Q_DECLARE_LOGGING_CATEGORY(four)
Q_DECLARE_LOGGING_CATEGORY(five)

#undef qCDebug // to add noquote() and nospace()

#if QT_VERSION < QT_VERSION_CHECK(6, 3, 0)
#define qCDebug(category, ...) \
    for (bool qt_category_enabled = category().isDebugEnabled(); qt_category_enabled; qt_category_enabled = false) \
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC, category().categoryName()).debug(__VA_ARGS__).noquote().nospace()
#else
#define qCDebug(category, ...) QT_MESSAGE_LOGGER_COMMON(category, QtDebugMsg).debug(__VA_ARGS__).noquote().nospace()
#endif

class Comment
{
public:
    static void setVerbosity(const int verbosity);
};

class Pandaception : public std::runtime_error
{
public:
    explicit Pandaception(const QString &message);

private:
    Q_DISABLE_COPY(Pandaception)

};

class GraphicElement;

class Common
{
public:
    static QVector<GraphicElement *> sortGraphicElements(QVector<GraphicElement *> elements);
    static int calculatePriority(GraphicElement *elm, QHash<GraphicElement *, bool> &beingVisited, QHash<GraphicElement *, int> &priorities);
};
