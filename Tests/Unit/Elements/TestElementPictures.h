// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/// The picture library: each version of a file loaded once and shared, its oriented variants built from its source.
class TestElementPictures : public QObject
{
    Q_OBJECT

private slots:
    void testElementsShowingOneFileShareItsPicture();
    void testAFileChangedOnDiskIsANewVersion();
    void testVariantsAreBuiltFromTheSourceReadAtLoad();
};
