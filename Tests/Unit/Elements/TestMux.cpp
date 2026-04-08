// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestMux.h"

#include "App/Element/GraphicElements/Mux.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestMux::testMuxInputSize()
{
    WorkSpace workspace;
    auto *mux = new Mux;
    workspace.scene()->addItem(mux);
    QVERIFY(mux->inputSize() > 0);
}

void TestMux::testMuxSelection()
{
    WorkSpace workspace;
    auto *mux = new Mux;
    workspace.scene()->addItem(mux);
    QVERIFY(mux->inputSize() > 0);
}

void TestMux::testMuxPainting()
{
    WorkSpace workspace;
    auto *mux = new Mux;
    workspace.scene()->addItem(mux);
    QCOMPARE(mux->elementType(), ElementType::Mux);
}

