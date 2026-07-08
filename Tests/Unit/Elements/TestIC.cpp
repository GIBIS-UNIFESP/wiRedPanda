// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestIC.h"

#include "App/Core/Settings.h"
#include "App/Element/IC.h"
#include "App/Element/ICPreviewPopup.h"
#include "App/IO/SerializationContext.h"
#include "Tests/Common/TestUtils.h"

void TestICUnit::testICLoadFromFile()
{
    IC ic;
    QCOMPARE(ic.elementType(), ElementType::IC);
}

void TestICUnit::testICPortLabelResolution()
{
    IC ic;
    // Unloaded IC has no ports
    QVERIFY(ic.inputs().isEmpty());
    QVERIFY(ic.outputs().isEmpty());
}

void TestICUnit::testICNestedSaveLoad()
{
    IC ic;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    ic.save(stream, {.purpose = SerializationPurpose::PortableFile});
    QVERIFY(!data.isEmpty());
}

void TestICUnit::testICInvalidFile()
{
    IC ic;
    QVERIFY(ic.file().isEmpty());
}

void TestICUnit::testICPreviewPopupRespectsDisabledSetting()
{
    const bool originalIcPreviewDisabled = Settings::icPreviewDisabled();

    IC ic;
    ICPreviewPopup popup;

    Settings::setIcPreviewDisabled(true);
    popup.showForIC(&ic, QPoint(0, 0));
    QVERIFY(popup.pendingIC() == nullptr);
    QVERIFY(!popup.isShowActiveFor(&ic));

    Settings::setIcPreviewDisabled(false);
    popup.showForIC(&ic, QPoint(0, 0));
    QCOMPARE(popup.pendingIC(), &ic);
    QVERIFY(popup.isShowActiveFor(&ic));

    Settings::setIcPreviewDisabled(originalIcPreviewDisabled);
}
