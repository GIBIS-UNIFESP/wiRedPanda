// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QVector>

class QDataStream;
class QFile;
class QSaveFile;
class WaveformGrid;

namespace DolphinSerializer {

struct WaveformData {
    int inputPorts = 0;
    int columns    = 0;
    QVector<int> values;
};

void saveBinary(QDataStream &stream, const WaveformGrid *grid, int inputPorts);
WaveformData loadBinary(QDataStream &stream, int maxInputPorts);
void saveCSV(QSaveFile &file, const WaveformGrid *grid);
WaveformData loadCSV(QFile &file, int maxInputPorts);

} // namespace DolphinSerializer
