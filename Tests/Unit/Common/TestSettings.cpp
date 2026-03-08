// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Common/TestSettings.h"

#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QTemporaryDir>

#include "App/Core/Settings.h"
#include "Tests/Common/TestUtils.h"

void TestSettings::initTestCase()
{
    // Set up test-specific QSettings paths
    // This prevents test settings from interfering with real application settings
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                       QDir::tempPath() + "/wiredpanda_tests");
}

void TestSettings::init()
{
    // Clear test settings before each test
    QSettings testSettings(m_testOrganization, m_testApplication);
    testSettings.clear();
    testSettings.sync();
}

void TestSettings::cleanup()
{
    // Clean up after each test
    QSettings testSettings(m_testOrganization, m_testApplication);
    testSettings.clear();
    testSettings.sync();
}

// ============================================================
// Basic Operations Tests
// ============================================================

void TestSettings::testSettingsFileName()
{
    // Get the settings file path
    QString fileName = Settings::fileName();
    QVERIFY(!fileName.isEmpty());

    // Verify it's a valid path
    QFileInfo fileInfo(fileName);
    QVERIFY(!fileInfo.absolutePath().isEmpty());
}

void TestSettings::testSettingsSetAndGet()
{
    // Set values of different types
    Settings::setValue("test/string", QString("hello"));
    Settings::setValue("test/integer", 42);
    Settings::setValue("test/boolean", true);

    // Get values back
    QCOMPARE(Settings::value("test/string").toString(), QString("hello"));
    QCOMPARE(Settings::value("test/integer").toInt(), 42);
    QCOMPARE(Settings::value("test/boolean").toBool(), true);
}

void TestSettings::testSettingsContains()
{
    // Set a value
    Settings::setValue("test/exists", "value");

    // Verify it exists
    QVERIFY(Settings::contains("test/exists"));

    // Verify non-existent key
    QVERIFY(!Settings::contains("test/notexists"));
}

void TestSettings::testSettingsRemove()
{
    // Set a value
    Settings::setValue("test/toremove", "value");
    QVERIFY(Settings::contains("test/toremove"));

    // Remove it
    Settings::remove("test/toremove");

    // Verify it's gone
    QVERIFY(!Settings::contains("test/toremove"));
}

void TestSettings::testSettingsGetNonExistent()
{
    // Get non-existent key
    QVariant result = Settings::value("test/nonexistent");

    // Should return empty/default variant
    QVERIFY(!result.isValid() || result.toString().isEmpty());
}

void TestSettings::testSettingsOverwrite()
{
    // Set initial value
    Settings::setValue("test/overwrite", "first");
    QCOMPARE(Settings::value("test/overwrite").toString(), QString("first"));

    // Overwrite with new value
    Settings::setValue("test/overwrite", "second");
    QCOMPARE(Settings::value("test/overwrite").toString(), QString("second"));
}

// ============================================================
// Data Types Tests
// ============================================================

void TestSettings::testSettingsString()
{
    QString testString = "Hello, World! With Unicode: ñ, ü, 中文";

    Settings::setValue("test/string", testString);
    QCOMPARE(Settings::value("test/string").toString(), testString);
}

void TestSettings::testSettingsInteger()
{
    int testInt = 12345;
    qint64 testLongInt = 9876543210LL;

    Settings::setValue("test/int", testInt);
    Settings::setValue("test/longint", testLongInt);

    QCOMPARE(Settings::value("test/int").toInt(), testInt);
    QCOMPARE(Settings::value("test/longint").toLongLong(), testLongInt);
}

void TestSettings::testSettingsBoolean()
{
    Settings::setValue("test/true", true);
    Settings::setValue("test/false", false);

    QCOMPARE(Settings::value("test/true").toBool(), true);
    QCOMPARE(Settings::value("test/false").toBool(), false);
}

void TestSettings::testSettingsByteArray()
{
    QByteArray testArray = "Binary data \x00\x01\x02";
    QVariant variant = testArray;

    Settings::setValue("test/bytearray", variant);
    QByteArray result = Settings::value("test/bytearray").toByteArray();

    QCOMPARE(result, testArray);
}

void TestSettings::testSettingsStringList()
{
    QStringList testList = {"one", "two", "three", "four"};
    QVariant variant = testList;

    Settings::setValue("test/stringlist", variant);
    QStringList result = Settings::value("test/stringlist").toStringList();

    QCOMPARE(result, testList);
}

void TestSettings::testSettingsEmptyStringList()
{
    QStringList emptyList;

    Settings::setValue("test/emptylist", emptyList);
    QStringList result = Settings::value("test/emptylist").toStringList();

    QVERIFY(result.isEmpty());
}

void TestSettings::testSettingsNullVariant()
{
    // Get non-existent setting without default
    QVariant result = Settings::value("test/null/key");

    // Non-existent key without default should return invalid QVariant
    QVERIFY(!result.isValid());
}

// ============================================================
// Real-World Settings Tests
// ============================================================

void TestSettings::testThemePersistence()
{
    // Simulate theme selection (0 = light, 1 = dark)
    Settings::setValue("test/theme", 1);

    QCOMPARE(Settings::value("test/theme").toInt(), 1);

    // Change theme
    Settings::setValue("test/theme", 0);
    QCOMPARE(Settings::value("test/theme").toInt(), 0);
}

void TestSettings::testLanguagePersistence()
{
    QStringList languages = {"en", "pt_BR", "es", "fr", "de"};

    for (const QString &lang : languages) {
        Settings::setValue("test/language", lang);
        QCOMPARE(Settings::value("test/language").toString(), lang);
    }
}

void TestSettings::testRecentFilesPersistence()
{
    QStringList recentFiles = {
        "/path/to/file1.panda",
        "/path/to/file2.panda",
        "/path/to/file3.panda"
    };

    Settings::setValue("test/recentFileList", recentFiles);
    QStringList result = Settings::value("test/recentFileList").toStringList();

    QCOMPARE(result, recentFiles);
}

void TestSettings::testAutosaveFilesPersistence()
{
    const QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QStringList autosaveFiles = {
        tmpDir + "/.circuit1.XXXXX.panda",
        tmpDir + "/.circuit2.XXXXX.panda"
    };

    Settings::setValue("test/autosaveFile", autosaveFiles);
    QStringList result = Settings::value("test/autosaveFile").toStringList();

    QCOMPARE(result.count(), 2);
    QVERIFY(result.contains(tmpDir + "/.circuit1.XXXXX.panda"));
}

void TestSettings::testWindowGeometryPersistence()
{
    QByteArray geometry = "test_geometry_data";
    QByteArray state = "test_state_data";

    Settings::setValue("test/MainWindow/geometry", geometry);
    Settings::setValue("test/MainWindow/windowState", state);

    QCOMPARE(Settings::value("test/MainWindow/geometry").toByteArray(), geometry);
    QCOMPARE(Settings::value("test/MainWindow/windowState").toByteArray(), state);
}

void TestSettings::testBooleanSettings()
{
    // Simulate various boolean application settings
    Settings::setValue("test/fastMode", true);
    Settings::setValue("test/labelsUnderIcons", false);
    Settings::setValue("test/hideV4Warning", true);

    QCOMPARE(Settings::value("test/fastMode").toBool(), true);
    QCOMPARE(Settings::value("test/labelsUnderIcons").toBool(), false);
    QCOMPARE(Settings::value("test/hideV4Warning").toBool(), true);
}

void TestSettings::testHierarchicalKeys()
{
    // Test hierarchical key structure with "/"
    Settings::setValue("MainWindow/geometry", "geom");
    Settings::setValue("MainWindow/windowState", "state");
    Settings::setValue("Splitter/geometry", "splitter_geom");
    Settings::setValue("Splitter/state", "splitter_state");

    // Verify all can be accessed
    QVERIFY(Settings::contains("MainWindow/geometry"));
    QVERIFY(Settings::contains("MainWindow/windowState"));
    QVERIFY(Settings::contains("Splitter/geometry"));
    QVERIFY(Settings::contains("Splitter/state"));

    // Remove one
    Settings::remove("MainWindow/geometry");
    QVERIFY(!Settings::contains("MainWindow/geometry"));

    // Others still exist
    QVERIFY(Settings::contains("MainWindow/windowState"));
    QVERIFY(Settings::contains("Splitter/geometry"));
}

void TestSettings::testWarningFlagsPersistence()
{
    // Test warning suppression flags
    Settings::setValue("hideV4Warning", QString("true"));

    QString result = Settings::value("hideV4Warning").toString();
    QCOMPARE(result, QString("true"));

    // Remove warning flag
    Settings::remove("hideV4Warning");
    QVERIFY(!Settings::contains("hideV4Warning"));
}

// ============================================================
// Edge Cases Tests
// ============================================================

void TestSettings::testMultipleReads()
{
    // Set a value
    Settings::setValue("test/multiread", "same_value");

    // Read multiple times - should get same value
    QCOMPARE(Settings::value("test/multiread").toString(), QString("same_value"));
    QCOMPARE(Settings::value("test/multiread").toString(), QString("same_value"));
    QCOMPARE(Settings::value("test/multiread").toString(), QString("same_value"));
}

void TestSettings::testClearAllSettings()
{
    // Set multiple values
    Settings::setValue("test/clear/1", "value1");
    Settings::setValue("test/clear/2", "value2");
    Settings::setValue("test/clear/3", "value3");

    // Verify they exist
    QVERIFY(Settings::contains("test/clear/1"));
    QVERIFY(Settings::contains("test/clear/2"));
    QVERIFY(Settings::contains("test/clear/3"));

    // Remove all
    Settings::remove("test/clear/1");
    Settings::remove("test/clear/2");
    Settings::remove("test/clear/3");

    // Verify they're gone
    QVERIFY(!Settings::contains("test/clear/1"));
    QVERIFY(!Settings::contains("test/clear/2"));
    QVERIFY(!Settings::contains("test/clear/3"));
}

void TestSettings::testVeryLongKey()
{
    // Create a very long key name
    QString longKey = "test/" + QString("a").repeated(500);

    Settings::setValue(longKey, "value");
    QCOMPARE(Settings::value(longKey).toString(), QString("value"));
}

void TestSettings::testVeryLargeValue()
{
    // Create a very large value
    QString largeValue = QString("x").repeated(10000);

    Settings::setValue("test/large", largeValue);
    QCOMPARE(Settings::value("test/large").toString(), largeValue);
}

