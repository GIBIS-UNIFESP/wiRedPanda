# Qt Code-Based UI Best Practices - wiRedPanda

## Design Principles

### 1. Separation of Concerns
```cpp
class MainWindow : public QMainWindow {
private:
    // UI creation methods
    void setupUi();
    void createMenus();
    void createToolbars();
    void createLayouts();
    
    // Event handling methods
    void connectSignals();
    void setupEventFilters();
    
    // Business logic methods
    void updateState();
    void validateInput();
};
```

### 2. Widget Ownership and Memory Management
```cpp
class DialogBase : public QDialog {
private:
    // Stack-allocated widgets (recommended for simple cases)
    QLabel m_titleLabel;
    QPushButton m_okButton;
    
    // Heap-allocated widgets (for dynamic/conditional widgets)
    std::unique_ptr<QTableWidget> m_dynamicTable;
    
    // Parent-owned widgets (Qt's parent-child system)
    QVBoxLayout *m_mainLayout; // owned by parent widget
    
public:
    DialogBase(QWidget *parent = nullptr) : QDialog(parent) {
        // Stack widgets need parent assignment
        m_titleLabel.setParent(this);
        m_okButton.setParent(this);
        
        // Heap widgets with parent
        m_dynamicTable = std::make_unique<QTableWidget>(this);
        
        setupUi();
    }
};
```

### 3. Layout Builder Pattern
```cpp
// Utility class for cleaner layout creation
class LayoutBuilder {
public:
    static QVBoxLayout* vbox(std::initializer_list<QWidget*> widgets, 
                            int spacing = -1, const QMargins &margins = {}) {
        auto *layout = new QVBoxLayout;
        if (spacing >= 0) layout->setSpacing(spacing);
        if (!margins.isNull()) layout->setContentsMargins(margins);
        
        for (auto *widget : widgets) {
            layout->addWidget(widget);
        }
        return layout;
    }
    
    static QHBoxLayout* hbox(std::initializer_list<QWidget*> widgets,
                            int spacing = -1, const QMargins &margins = {}) {
        auto *layout = new QHBoxLayout;
        if (spacing >= 0) layout->setSpacing(spacing);
        if (!margins.isNull()) layout->setContentsMargins(margins);
        
        for (auto *widget : widgets) {
            layout->addWidget(widget);
        }
        return layout;
    }
    
    static QFormLayout* form(std::initializer_list<std::pair<QString, QWidget*>> pairs) {
        auto *layout = new QFormLayout;
        for (const auto &pair : pairs) {
            layout->addRow(pair.first, pair.second);
        }
        return layout;
    }
    
    static QGridLayout* grid() {
        return new QGridLayout;
    }
};

// Usage example
void setupButtonRow() {
    auto *buttonLayout = LayoutBuilder::hbox({
        &m_okButton, &m_cancelButton, &m_applyButton
    }, 6, QMargins(8, 4, 8, 4));
}
```

### 4. Widget Factory Functions
```cpp
namespace WidgetFactory {
    // Consistent button creation
    inline QPushButton* createButton(const QString &text, 
                                    const QString &tooltip = {},
                                    const QIcon &icon = {}) {
        auto *button = new QPushButton(text);
        if (!tooltip.isEmpty()) button->setToolTip(tooltip);
        if (!icon.isNull()) button->setIcon(icon);
        return button;
    }
    
    // Standardized line edits
    inline QLineEdit* createLineEdit(const QString &placeholder,
                                    QValidator *validator = nullptr,
                                    const QString &tooltip = {}) {
        auto *edit = new QLineEdit;
        if (!placeholder.isEmpty()) edit->setPlaceholderText(placeholder);
        if (validator) edit->setValidator(validator);
        if (!tooltip.isEmpty()) edit->setToolTip(tooltip);
        return edit;
    }
    
    // Common spinbox configurations
    inline QSpinBox* createSpinBox(int min, int max, int value, 
                                  const QString &suffix = {}) {
        auto *spinBox = new QSpinBox;
        spinBox->setRange(min, max);
        spinBox->setValue(value);
        if (!suffix.isEmpty()) spinBox->setSuffix(suffix);
        return spinBox;
    }
}
```

## Code Organization Patterns

### 1. Modular UI Setup
```cpp
void MainWindow::setupUi() {
    // Order matters for some widgets (e.g., central widget before dock widgets)
    createCentralWidget();
    createMenuBar();
    createToolBars();
    createStatusBar();
    createDockWidgets();
    
    // Apply layouts and configure relationships
    configureLayouts();
    applyStyles();
    
    // Connect signals after all widgets exist
    connectSignals();
    
    // Final configuration
    restoreGeometry();
    updateWindowTitle();
}
```

### 2. Widget Configuration Methods
```cpp
void ElementEditor::configurePropertyTable() {
    m_propertyTable = new QTableWidget(0, 2, this);
    
    // Headers
    m_propertyTable->setHorizontalHeaderLabels({tr("Property"), tr("Value")});
    m_propertyTable->horizontalHeader()->setStretchLastSection(true);
    m_propertyTable->verticalHeader()->hide();
    
    // Behavior
    m_propertyTable->setAlternatingRowColors(true);
    m_propertyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_propertyTable->setSortingEnabled(false);
    
    // Size policies
    m_propertyTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}
```

### 3. Signal Connection Patterns
```cpp
void MainWindow::connectSignals() {
    // Use lambda for simple connections
    connect(&m_newAction, &QAction::triggered, [this]() {
        createNewTab();
    });
    
    // Member function connections
    connect(&m_saveAction, &QAction::triggered, this, &MainWindow::save);
    
    // Cross-widget connections
    connect(m_workspace, &WorkSpace::circuitModified,
            this, &MainWindow::updateWindowTitle);
    
    // Custom signal connections with parameters
    connect(m_elementList, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::elementSelectionChanged);
}
```

## Layout Management Best Practices

### 1. Responsive Layouts
```cpp
void MainWindow::createResponsiveLayout() {
    // Main splitter for resizable areas
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_mainSplitter->setChildrenCollapsible(false);
    
    // Left panel (fixed minimum width)
    auto *leftPanel = createLeftPanel();
    leftPanel->setMinimumWidth(200);
    leftPanel->setMaximumWidth(300);
    
    // Center area (expandable)
    auto *centerArea = createCenterArea();
    
    // Right panel (optional, collapsible)
    auto *rightPanel = createRightPanel();
    rightPanel->setMaximumWidth(250);
    
    m_mainSplitter->addWidget(leftPanel);
    m_mainSplitter->addWidget(centerArea);
    m_mainSplitter->addWidget(rightPanel);
    
    // Set proportional sizes
    m_mainSplitter->setStretchFactor(0, 0); // left: fixed
    m_mainSplitter->setStretchFactor(1, 1); // center: expand
    m_mainSplitter->setStretchFactor(2, 0); // right: fixed
    
    setCentralWidget(m_mainSplitter);
}
```

### 2. Form Layout Best Practices
```cpp
void SettingsDialog::createFormLayout() {
    auto *formLayout = new QFormLayout;
    formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    formLayout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
    formLayout->setLabelAlignment(Qt::AlignRight);
    
    // Group related fields
    formLayout->addRow(tr("Application Settings"), createSeparatorLine());
    formLayout->addRow(tr("Language:"), m_languageCombo);
    formLayout->addRow(tr("Theme:"), m_themeCombo);
    
    formLayout->addRow("", nullptr); // spacer
    
    formLayout->addRow(tr("Editor Settings"), createSeparatorLine());
    formLayout->addRow(tr("Grid Size:"), m_gridSizeSpinBox);
    formLayout->addRow(tr("Auto-save:"), m_autoSaveCheckBox);
    
    setLayout(formLayout);
}

QFrame* SettingsDialog::createSeparatorLine() {
    auto *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}
```

### 3. Complex Layout Composition
```cpp
void BeWavedDolphin::setupComplexLayout() {
    // Top-level vertical layout
    auto *mainLayout = new QVBoxLayout;
    
    // Toolbar area
    auto *toolbarLayout = LayoutBuilder::hbox({
        m_playButton, m_pauseButton, m_stopButton,
        createVerticalSeparator(),
        m_zoomInButton, m_zoomOutButton, m_zoomFitButton
    }, 4);
    
    // Content area with splitter
    m_contentSplitter = new QSplitter(Qt::Vertical);
    
    // Wave display area  
    auto *waveArea = createWaveDisplayArea();
    
    // Control panel
    auto *controlPanel = createControlPanel();
    
    m_contentSplitter->addWidget(waveArea);
    m_contentSplitter->addWidget(controlPanel);
    m_contentSplitter->setStretchFactor(0, 3); // wave area gets more space
    m_contentSplitter->setStretchFactor(1, 1);
    
    // Assemble main layout
    mainLayout->addLayout(toolbarLayout);
    mainLayout->addWidget(m_contentSplitter);
    
    setLayout(mainLayout);
}
```

## Error Handling and Validation

### 1. Widget State Validation
```cpp
class FormValidator {
public:
    static bool validateRequiredFields(const std::vector<QWidget*> &widgets) {
        for (auto *widget : widgets) {
            if (auto *lineEdit = qobject_cast<QLineEdit*>(widget)) {
                if (lineEdit->text().trimmed().isEmpty()) {
                    lineEdit->setStyleSheet("border: 2px solid red;");
                    lineEdit->setFocus();
                    return false;
                }
                lineEdit->setStyleSheet(""); // clear error style
            }
        }
        return true;
    }
    
    static void clearValidationStyles(const std::vector<QWidget*> &widgets) {
        for (auto *widget : widgets) {
            widget->setStyleSheet("");
        }
    }
};
```

### 2. Safe Widget Access
```cpp
template<typename T>
T* findChildWidget(QWidget *parent, const QString &name) {
    auto *child = parent->findChild<T*>(name);
    if (!child) {
        qWarning() << "Widget not found:" << name;
        return nullptr;
    }
    return child;
}

// Usage
void updateDisplay() {
    if (auto *label = findChildWidget<QLabel>(this, "statusLabel")) {
        label->setText("Updated");
    }
}
```

## Performance Considerations

### 1. Lazy Widget Creation
```cpp
class TabWidget : public QTabWidget {
private:
    std::vector<std::function<QWidget*()>> m_tabFactories;
    std::vector<QWidget*> m_createdTabs;
    
public:
    void addLazyTab(const QString &title, std::function<QWidget*()> factory) {
        // Add placeholder tab
        addTab(new QWidget, title);
        m_tabFactories.push_back(factory);
        m_createdTabs.push_back(nullptr);
    }
    
protected:
    void tabInserted(int index) override {
        connect(this, &QTabWidget::currentChanged, [this](int index) {
            // Create widget only when tab is first accessed
            if (index < m_createdTabs.size() && !m_createdTabs[index]) {
                auto *widget = m_tabFactories[index]();
                m_createdTabs[index] = widget;
                removeTab(index);
                insertTab(index, widget, tabText(index));
                setCurrentIndex(index);
            }
        });
        QTabWidget::tabInserted(index);
    }
};
```

### 2. Efficient Updates
```cpp
class PropertyEditor : public QWidget {
private:
    QTimer m_updateTimer;
    bool m_needsUpdate = false;
    
public:
    PropertyEditor(QWidget *parent = nullptr) : QWidget(parent) {
        // Batch updates to avoid excessive redraws
        m_updateTimer.setSingleShot(true);
        m_updateTimer.setInterval(50); // 50ms delay
        connect(&m_updateTimer, &QTimer::timeout, 
                this, &PropertyEditor::performUpdate);
    }
    
    void requestUpdate() {
        m_needsUpdate = true;
        m_updateTimer.start();
    }
    
private:
    void performUpdate() {
        if (!m_needsUpdate) return;
        
        // Batch all UI updates here
        updatePropertyList();
        updateValueDisplays();
        
        m_needsUpdate = false;
    }
};
```

## Testing Support

### 1. Testable UI Code
```cpp
class TestableDialog : public QDialog {
private:
    // Expose widgets for testing via accessors
    QLineEdit *m_nameEdit = nullptr;
    QPushButton *m_okButton = nullptr;
    
public:
    // Test accessors
    QLineEdit* nameEdit() const { return m_nameEdit; }
    QPushButton* okButton() const { return m_okButton; }
    
    // Testable validation
    bool validateForm() const {
        return !m_nameEdit->text().isEmpty();
    }
    
    // Simulate user actions
    void simulateOkClick() {
        if (validateForm()) {
            accept();
        }
    }
};

// Test code can access widgets directly
void testDialog() {
    TestableDialog dialog;
    dialog.nameEdit()->setText("Test Name");
    QVERIFY(dialog.validateForm());
    dialog.simulateOkClick();
    QCOMPARE(dialog.result(), QDialog::Accepted);
}
```

### 2. Widget State Capture
```cpp
struct WidgetState {
    QString text;
    bool enabled;
    bool visible;
    QSize size;
    
    static WidgetState capture(QWidget *widget) {
        WidgetState state;
        state.enabled = widget->isEnabled();
        state.visible = widget->isVisible();
        state.size = widget->size();
        
        if (auto *lineEdit = qobject_cast<QLineEdit*>(widget)) {
            state.text = lineEdit->text();
        }
        // Add other widget types as needed
        
        return state;
    }
};
```

## Cross-Platform Considerations

### 1. Platform-Specific Adaptations
```cpp
void MainWindow::applyPlatformStyle() {
#ifdef Q_OS_MAC
    // macOS specific styling
    setUnifiedTitleAndToolBarOnMac(true);
    m_toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
#elif defined(Q_OS_WIN)
    // Windows specific styling  
    m_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);
#else
    // Linux/other platforms
    m_toolbar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
#endif
}
```

### 2. Responsive Design
```cpp
void MainWindow::adjustForScreenSize() {
    const auto screenGeometry = QApplication::primaryScreen()->geometry();
    const bool isSmallScreen = screenGeometry.width() < 1200;
    
    if (isSmallScreen) {
        // Compact layout for smaller screens
        m_sidebar->hide();
        m_toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    } else {
        // Full layout for larger screens
        m_sidebar->show();
        m_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
}
```

These patterns ensure maintainable, scalable, and robust code-based UI implementation for wiRedPanda.