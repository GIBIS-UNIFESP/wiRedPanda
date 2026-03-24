// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QVersionNumber>

namespace Versions {

inline const QVersionNumber V_1_1 = QVersionNumber(1, 1);
inline const QVersionNumber V_1_2 = QVersionNumber(1, 2);
inline const QVersionNumber V_1_3 = QVersionNumber(1, 3);
inline const QVersionNumber V_1_4 = QVersionNumber(1, 4);
inline const QVersionNumber V_1_6 = QVersionNumber(1, 6);
inline const QVersionNumber V_1_7 = QVersionNumber(1, 7);
inline const QVersionNumber V_1_9 = QVersionNumber(1, 9);
inline const QVersionNumber V_2_4 = QVersionNumber(2, 4);
inline const QVersionNumber V_2_7 = QVersionNumber(2, 7);
inline const QVersionNumber V_3_0 = QVersionNumber(3, 0);
inline const QVersionNumber V_3_1 = QVersionNumber(3, 1);
inline const QVersionNumber V_3_3 = QVersionNumber(3, 3);
inline const QVersionNumber V_4_0 = QVersionNumber(4, 0);
inline const QVersionNumber V_4_0_1 = QVersionNumber(4, 0, 1);
inline const QVersionNumber V_4_1 = QVersionNumber(4, 1);
inline const QVersionNumber V_4_2 = QVersionNumber(4, 2);
inline const QVersionNumber V_4_3 = QVersionNumber(4, 3);
inline const QVersionNumber V_4_4 = QVersionNumber(4, 4);
inline const QVersionNumber V_4_5 = QVersionNumber(4, 5);

} // namespace Versions

namespace AppVersion {

inline const QVersionNumber current = QVersionNumber::fromString(APP_VERSION).normalized();

} // namespace AppVersion

