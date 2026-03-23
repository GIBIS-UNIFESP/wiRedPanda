// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Named version predicates for file-format compatibility checks.
 *
 * Centralises "what changed in each version" so that load/save code reads
 * as self-documenting prose rather than raw version comparisons.  Each
 * predicate is a single-line wrapper around Versions::V_X_Y constants.
 */

#pragma once

#include <QVersionNumber>

#include "App/Versions.h"

namespace VersionInfo {

/// V1.1: Clock element added.
inline bool hasClock(const QVersionNumber &v) { return v >= Versions::V_1_1; }

/// V1.2: Labels added to elements; IC filename field.
inline bool hasLabels(const QVersionNumber &v) { return v >= Versions::V_1_2; }

/// V1.3: Port-size constraints stored in file.
inline bool hasPortSizes(const QVersionNumber &v) { return v >= Versions::V_1_3; }

/// V1.4: Scene rectangle stored in file.
inline bool hasSceneRect(const QVersionNumber &v) { return v >= Versions::V_1_4; }

/// V1.6: Display7 first color support.
inline bool hasDisplay7Color(const QVersionNumber &v) { return v >= Versions::V_1_6; }

/// V1.7: Display7 additional color support.
inline bool hasDisplay7ExtColor(const QVersionNumber &v) { return v >= Versions::V_1_7; }

/// V1.9: Keyboard trigger sequences added.
inline bool hasTrigger(const QVersionNumber &v) { return v >= Versions::V_1_9; }

/// V2.4: Audio element support (Buzzer, AudioBox).
inline bool hasAudio(const QVersionNumber &v) { return v >= Versions::V_2_4; }

/// V2.7: Pixmap skin names stored in file.
inline bool hasSkinNames(const QVersionNumber &v) { return v >= Versions::V_2_7; }

/// V3.0: Dolphin waveform filename stored in file header.
inline bool hasDolphinFilename(const QVersionNumber &v) { return v >= Versions::V_3_0; }

/// V3.1: Lock state for input elements; color for some display/button types.
inline bool hasLockState(const QVersionNumber &v) { return v >= Versions::V_3_1; }

/// V3.3: "none" sentinel normalised to empty string for dolphin filename.
inline bool hasDolphinSentinelFix(const QVersionNumber &v) { return v >= Versions::V_3_3; }

/// V4.0.1: Unused priority field in old format.
inline bool hasUnusedPriority(const QVersionNumber &v) { return v >= Versions::V_4_0_1; }

/// V4.1: Format changed from flat binary to keyed QMap; port serial IDs; rotation fix.
inline bool hasQMapFormat(const QVersionNumber &v) { return v >= Versions::V_4_1; }

/// V4.2: TruthTable output data stored in file.
inline bool hasTruthTableData(const QVersionNumber &v) { return v >= Versions::V_4_2; }

/// V4.3: Clock delay format corrected.
inline bool hasDelayFix(const QVersionNumber &v) { return v >= Versions::V_4_3; }

} // namespace VersionInfo

