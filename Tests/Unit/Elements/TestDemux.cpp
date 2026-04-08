// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestDemux.h"

#include "App/Element/GraphicElements/Demux.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestDemux::testDemuxOutputSize()
{
    WorkSpace workspace;
    auto *demux = new Demux;
    workspace.scene()->addItem(demux);
    int defaultSize = demux->outputSize();
    QVERIFY(defaultSize > 0);
}

void TestDemux::testDemuxRouting()
{
    WorkSpace workspace;
    auto *demux = new Demux;
    workspace.scene()->addItem(demux);
    QVERIFY(demux->inputSize() > 0);
}

void TestDemux::testDemuxPainting()
{
    WorkSpace workspace;
    auto *demux = new Demux;
    workspace.scene()->addItem(demux);
    QCOMPARE(demux->elementType(), ElementType::Demux);
}

