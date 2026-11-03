// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QVector>

#include "App/Core/Enums.h"

/**
 * \brief Whether \a a is at most \a b in the flat order of §2.1: Unknown is below every value, and the two levels and
 * Error are unordered among themselves.
 */
bool atMostInFlatOrder(Status a, Status b);

/**
 * \brief The value of a combinational gate of \a type on \a inputs, by the rule of §2.3 written out.
 * \details Each Unknown or Error input is read both ways, as Inactive and as Active, and every reading is computed at
 * levels. Where the readings agree the gate outputs that value. Where they disagree it outputs Unknown if an Unknown
 * input decides it, that is, some setting of the other open inputs makes its own level change the output; else Error
 * (IEEE 1164 ranks Unknown first). Written to be read, not to be fast: it enumerates the readings.
 */
Status referenceGate(ElementType type, const QVector<Status> &inputs);

/**
 * \brief The value of a D flip-flop on \a inputs, ordered data, clock, the clock's last level, and the stored output.
 * \details Stores the data on a rising edge, the clock having gone from Inactive to Active since it last sampled. A
 * clock whose last value and now are the same status is kept and has no edge, Unknown both times included.
 */
Status referenceDFlipFlop(const QVector<Status> &inputs);

/**
 * \brief The value of a D flip-flop whose data changes in the clock's round, on \a oldData and \a newData, ordered as
 * referenceDFlipFlop() orders its inputs but with the data given twice: the value is what the two data readings agree
 * on, and Unknown where the old and new data give different values (§2.6).
 */
Status referenceDFlipFlopChangingData(Status oldData, Status newData, Status clock, Status lastClock, Status stored);

/**
 * \brief The value of a D latch on \a inputs, ordered enable, data, and the stored output: transparent while enabled.
 */
Status referenceDLatch(const QVector<Status> &inputs);
