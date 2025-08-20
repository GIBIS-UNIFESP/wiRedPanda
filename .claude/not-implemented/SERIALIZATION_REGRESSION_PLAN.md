# wiRedPanda Serialization Regression Testing Plan

## Version History & File Format Analysis

### **Discovered Version History**
Based on git tags and code analysis, wiRedPanda has evolved through major format changes:

#### **Major File Format Versions**
- **v1.x Series**: `v1.8-beta`, `v1.9-beta`
- **v2.x Series**: `v2.3` through `v2.7.0`
- **v3.x Series**: `v3.0.0`, `v3.0.1`
- **v4.x Series**: `v4.0.0-rc` through `v4.2.0` (current)

### **File Format Evolution Evidence**

#### **File Format Header Evolution**
```cpp
// Current format (v4.1+): Binary header with version
const quint32 MAGIC_HEADER_CIRCUIT = 0x57504346; // "WPCF"
const quint32 MAGIC_HEADER_WAVEFORM = 0x57505746; // "WPWF"

// Legacy format (pre-v4.1): Text-based metadata
// Files start with application name + version string
```

#### **Critical Version Breakpoints**
From serialization.cpp analysis:

1. **Version 1.1**: Clock frequency support
2. **Version 1.2**: Label support introduced
3. **Version 1.3**: Dynamic port sizing
4. **Version 1.4**: Canvas rectangle saving
5. **Version 1.6-1.7**: Display element enhancements
6. **Version 1.9**: Keyboard trigger support
7. **Version 2.4**: Audio/buzzer element support
8. **Version 2.7**: Pixmap skin names
9. **Version 3.0**: Dolphin filename support
10. **Version 3.1**: Element locking, colors
11. **Version 3.3**: Filename format changes
12. **Version 4.0**: **MAJOR FORMAT CHANGE** - Warning shown for older files
13. **Version 4.01**: Priority system
14. **Version 4.1**: **NEW BINARY FORMAT** - Complete serialization rewrite
15. **Version 4.2**: Truth table enhancements

## Comprehensive Regression Test Strategy

### **Test Framework Architecture**

```cpp
class TestSerializationRegression : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Version-specific regression tests
    void testVersion1x_Legacy();
    void testVersion2x_BasicFeatures();
    void testVersion3x_EnhancedFeatures();
    void testVersion4x_ModernFormat();

    // Format transition tests
    void testVersion4_0_BreakingChanges();
    void testVersion4_1_BinaryFormat();
    void testVersion4_2_LatestFeatures();

    // Cross-version compatibility
    void testForwardCompatibility();
    void testBackwardCompatibility();
    void testFileFormatMigration();

    // Error handling tests
    void testCorruptedLegacyFiles();
    void testUnsupportedVersions();
    void testMalformedHeaders();

private:
    // Test file generation and validation
    struct VersionTestFile {
        QVersionNumber version;
        QString filename;
        QString description;
        QList<ElementType> expectedElements;
        bool shouldLoadSuccessfully;
        bool shouldShowWarning;
    };

    void createLegacyTestFiles();
    void validateFileLoading(const VersionTestFile& test);
    bool verifyCircuitIntegrity(Scene* scene, const VersionTestFile& test);

    WorkSpace* m_workspace;
    QList<VersionTestFile> m_testFiles;
};
```

### **Test File Matrix**

#### **Generated Legacy Test Files**
```cpp
const QList<VersionTestFile> TEST_FILES = {
    // Version 1.x - Early format
    {VERSION("1.1"), "test_v1_1_basic_clock.panda", "Basic circuit with clock",
     {ElementType::Clock, ElementType::Led}, true, true},
    {VERSION("1.2"), "test_v1_2_with_labels.panda", "Circuit with element labels",
     {ElementType::And, ElementType::InputButton}, true, true},
    {VERSION("1.3"), "test_v1_3_dynamic_ports.panda", "Elements with dynamic port sizing",
     {ElementType::And, ElementType::Or}, true, true},

    // Version 2.x - Feature expansion
    {VERSION("2.3"), "test_v2_3_standard_elements.panda", "Standard logic elements",
     {ElementType::And, ElementType::Or, ElementType::Not}, true, true},
    {VERSION("2.4"), "test_v2_4_audio_support.panda", "Circuit with audio elements",
     {ElementType::Buzzer, ElementType::AudioBox}, true, true},
    {VERSION("2.7"), "test_v2_7_pixmap_skins.panda", "Elements with custom skins",
     {ElementType::Led, ElementType::InputButton}, true, true},

    // Version 3.x - Enhanced features
    {VERSION("3.0"), "test_v3_0_dolphin_support.panda", "Circuit with waveform file",
     {ElementType::Clock, ElementType::DFlipFlop}, true, true},
    {VERSION("3.1"), "test_v3_1_locked_elements.panda", "Circuit with locked elements",
     {ElementType::InputSwitch, ElementType::Led}, true, true},

    // Version 4.x - Modern format
    {VERSION("4.0"), "test_v4_0_modern_format.panda", "First modern format version",
     {ElementType::And, ElementType::Or, ElementType::IC}, true, false},
    {VERSION("4.1"), "test_v4_1_binary_format.panda", "New binary format",
     {ElementType::All}, true, false},
    {VERSION("4.2"), "test_v4_2_truth_tables.panda", "Enhanced truth tables",
     {ElementType::TruthTable, ElementType::And}, true, false},

    // Error cases
    {VERSION("0.9"), "test_v0_9_unsupported.panda", "Unsupported old version",
     {}, false, false},
    {VERSION("5.0"), "test_v5_0_future.panda", "Future version file",
     {}, false, false}
};
```

### **Test Implementation**

#### **Legacy File Generation**
```cpp
void TestSerializationRegression::createLegacyTestFiles() {
    for (const auto& testFile : TEST_FILES) {
        if (testFile.version >= VERSION("4.1")) {
            createModernFormatFile(testFile);
        } else {
            createLegacyFormatFile(testFile);
        }
    }
}

void TestSerializationRegression::createLegacyFormatFile(const VersionTestFile& test) {
    QFile file(test.filename);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);

    // Write legacy header (pre-4.1 format)
    QString appName = QString("wiRedPanda %1").arg(test.version.toString());
    stream << appName;

    // Create simple test circuit based on version capabilities
    if (test.version >= VERSION("1.2")) {
        // Add elements with labels
        addElementWithLabel(stream, test.expectedElements, test.version);
    } else {
        // Basic elements only
        addBasicElements(stream, test.expectedElements, test.version);
    }

    // Version-specific features
    if (test.version >= VERSION("2.4") && test.expectedElements.contains(ElementType::Buzzer)) {
        addAudioElements(stream, test.version);
    }

    if (test.version >= VERSION("3.0")) {
        addDolphinReference(stream, test.version);
    }
}

void TestSerializationRegression::createModernFormatFile(const VersionTestFile& test) {
    QFile file(test.filename);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);

    // Write modern binary header (4.1+ format)
    stream.setVersion(QDataStream::Qt_5_12);
    stream << Serialization::MAGIC_HEADER_CIRCUIT;
    stream << test.version;

    // Add comprehensive test circuit
    addModernCircuitElements(stream, test.expectedElements, test.version);
}
```

#### **Version-Specific Test Cases**

```cpp
void TestSerializationRegression::testVersion1x_Legacy() {
    // Test early wiRedPanda files (1.1-1.9)
    for (const auto& test : m_testFiles) {
        if (test.version.majorVersion() == 1) {
            validateFileLoading(test);

            // Verify version 1.x specific behaviors
            if (test.version >= VERSION("1.1")) {
                verifyClockSupport();
            }
            if (test.version >= VERSION("1.2")) {
                verifyLabelSupport();
            }
            if (test.version >= VERSION("1.3")) {
                verifyDynamicPortSizing();
            }
        }
    }
}

void TestSerializationRegression::testVersion4_0_BreakingChanges() {
    // Test the major format transition at v4.0
    VersionTestFile v3_test = findTestFile(VERSION("3.1"));
    VersionTestFile v4_test = findTestFile(VERSION("4.0"));

    // Load v3.1 file - should show compatibility warning
    loadFileAndExpectWarning(v3_test, "version < 4.0");

    // Load v4.0 file - should load cleanly
    loadFileAndExpectSuccess(v4_test);

    // Verify elements maintain functionality across version boundary
    verifyCircuitBehaviorEquivalence(v3_test, v4_test);
}

void TestSerializationRegression::testVersion4_1_BinaryFormat() {
    // Test the binary format transition at v4.1
    VersionTestFile oldFormat = findTestFile(VERSION("4.0"));
    VersionTestFile newFormat = findTestFile(VERSION("4.1"));

    // Both should load successfully but use different parsers
    QVERIFY(loadFile(oldFormat));   // Uses legacy text parser
    QVERIFY(loadFile(newFormat));   // Uses binary parser

    // Verify equivalent circuits produce same simulation results
    verifySimulationEquivalence(oldFormat, newFormat);
}
```

#### **Comprehensive Validation**

```cpp
void TestSerializationRegression::validateFileLoading(const VersionTestFile& test) {
    qDebug() << "Testing file:" << test.filename << "Version:" << test.version.toString();

    m_workspace = new WorkSpace();

    try {
        // Attempt to load the file
        m_workspace->load(test.filename);

        if (test.shouldLoadSuccessfully) {
            QVERIFY2(true, qPrintable(QString("File %1 loaded successfully").arg(test.filename)));

            // Verify circuit integrity
            QVERIFY(verifyCircuitIntegrity(m_workspace->scene(), test));

            // Verify simulation functionality
            QVERIFY(verifySimulationWorks(m_workspace->simulation()));

            // Check for expected warnings
            if (test.shouldShowWarning && test.version < VERSION("4.0")) {
                verifyCompatibilityWarningShown();
            }

        } else {
            QFAIL(qPrintable(QString("File %1 should not have loaded").arg(test.filename)));
        }

    } catch (const std::exception& e) {
        if (test.shouldLoadSuccessfully) {
            QFAIL(qPrintable(QString("Failed to load %1: %2").arg(test.filename, e.what())));
        } else {
            QVERIFY2(true, qPrintable(QString("Expected failure for %1: %2").arg(test.filename, e.what())));
        }
    }

    delete m_workspace;
}

bool TestSerializationRegression::verifyCircuitIntegrity(Scene* scene, const VersionTestFile& test) {
    // Verify expected elements are present
    auto elements = scene->elements();

    for (ElementType expectedType : test.expectedElements) {
        bool found = false;
        for (auto* element : elements) {
            if (getElementType(element) == expectedType) {
                found = true;
                break;
            }
        }
        QVERIFY2(found, qPrintable(QString("Expected element type %1 not found").arg(static_cast<int>(expectedType))));
    }

    // Verify connections are intact
    auto connections = scene->connections();
    for (auto* conn : connections) {
        QVERIFY(conn->startPort() != nullptr);
        QVERIFY(conn->endPort() != nullptr);
    }

    // Version-specific integrity checks
    if (test.version >= VERSION("1.2")) {
        verifyLabelsPreserved(elements);
    }

    if (test.version >= VERSION("2.7")) {
        verifyPixmapSkinsPreserved(elements);
    }

    if (test.version >= VERSION("3.1")) {
        verifyElementLockingPreserved(elements);
    }

    return true;
}
```

### **Error Condition Testing**

```cpp
void TestSerializationRegression::testCorruptedLegacyFiles() {
    // Test handling of corrupted files from each major version

    // Corrupted v1.x file
    createCorruptedFile("corrupted_v1.panda", VERSION("1.5"), CorruptionType::TruncatedHeader);
    expectLoadFailure("corrupted_v1.panda", "Invalid file format");

    // Corrupted v2.x file
    createCorruptedFile("corrupted_v2.panda", VERSION("2.4"), CorruptionType::MissingElements);
    expectLoadFailure("corrupted_v2.panda", "Missing element data");

    // Corrupted v3.x file
    createCorruptedFile("corrupted_v3.panda", VERSION("3.1"), CorruptionType::InvalidConnections);
    expectLoadFailure("corrupted_v3.panda", "Invalid connection");

    // Corrupted v4.x binary file
    createCorruptedFile("corrupted_v4.panda", VERSION("4.1"), CorruptionType::BadMagicHeader);
    expectLoadFailure("corrupted_v4.panda", "Invalid file format");
}

void TestSerializationRegression::testUnsupportedVersions() {
    // Test files claiming to be from unsupported versions

    // Very old version (should be rejected)
    createTestFile("ancient_v0_5.panda", VERSION("0.5"));
    expectLoadFailure("ancient_v0_5.panda", "Unsupported version");

    // Future version (should show warning but may work)
    createTestFile("future_v5_0.panda", VERSION("5.0"));
    expectLoadWithWarning("future_v5_0.panda", "out of date");
}
```

### **Performance & Stress Testing**

```cpp
void TestSerializationRegression::testLargeFileRegression() {
    // Test large files from each major version to ensure no performance regression

    for (const auto& version : {VERSION("2.7"), VERSION("3.1"), VERSION("4.0"), VERSION("4.2")}) {
        QString filename = QString("large_circuit_v%1.panda").arg(version.toString());
        createLargeTestCircuit(filename, version, 1000); // 1000 elements

        auto startTime = std::chrono::high_resolution_clock::now();
        QVERIFY(loadFile(filename));
        auto endTime = std::chrono::high_resolution_clock::now();

        auto loadTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        qDebug() << "Load time for" << filename << ":" << loadTime.count() << "ms";

        // Ensure load time is reasonable (adjust thresholds as needed)
        QVERIFY2(loadTime.count() < 5000, "File loading took too long");
    }
}
```

## Expected Results & Coverage Impact

### **Backward Compatibility Validation**
- **100% compatibility** with v4.x series
- **Graceful degradation** for v3.x series (warnings shown)
- **Best effort loading** for v2.x series
- **Legacy support** for v1.x series where possible

### **Error Handling Verification**
- **Robust parsing** of malformed legacy files
- **Clear error messages** for unsupported formats
- **No crashes** on corrupted input
- **Graceful fallbacks** for missing features

### **Coverage Improvements**
- **Serialization module**: 95%+ coverage (currently low)
- **Version handling**: 100% coverage of all version branches
- **Error paths**: Complete coverage of exception handling
- **File format parsing**: Full coverage of legacy and modern formats

### **Real-World Benefits**
- **User confidence**: Files from any version will load reliably
- **Data preservation**: No lost work from format migrations
- **Upgrade safety**: Users can upgrade without fear of data loss
- **Developer confidence**: Changes won't break existing files

This comprehensive regression testing strategy ensures wiRedPanda maintains perfect backward compatibility while preventing serialization regressions that could cause user data loss.
