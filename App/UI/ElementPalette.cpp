// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/ElementPalette.h"

#include <QDir>
#include <QIcon>
#include <QLayout>
#include <QLineEdit>
#include <QMimeData>
#include <QRegularExpression>
#include <QScrollArea>

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementLabel.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/UI/MainWindowUI.h"

ElementPalette::ElementPalette(MainWindowUi *ui, QObject *parent)
    : QObject(parent)
    , m_ui(ui)
{
    connect(m_ui->lineEditSearch, &QLineEdit::textChanged,   this, &ElementPalette::onSearchTextChanged);
    connect(m_ui->lineEditSearch, &QLineEdit::returnPressed, this, &ElementPalette::onSearchReturnPressed);
}

void ElementPalette::populate()
{
    setupTabIcons();

    const int ioTabIndex = getTabIndex("io");
    if (ioTabIndex != -1) {
        m_ui->tabElements->setCurrentIndex(ioTabIndex);
    } else {
        m_ui->tabElements->setCurrentIndex(0);
    }

    populateMenu(m_ui->verticalSpacer_InOut,       {"InputVcc", "InputGnd", "InputButton", "InputSwitch", "InputRotary", "Clock", "Led", "Display7", "Display14", "Display16", "Buzzer", "AudioBox"}, m_ui->scrollAreaWidgetContents_InOut->layout());
    populateMenu(m_ui->verticalSpacer_Gates,        {"And", "Or", "Not", "Nand", "Nor", "Xor", "Xnor", "Node"},                                                                                       m_ui->scrollAreaWidgetContents_Gates->layout());
    populateMenu(m_ui->verticalSpacer_Combinational,{"TruthTable", "Mux", "Demux"},                                                                                                                   m_ui->scrollAreaWidgetContents_Combinational->layout());
    populateMenu(m_ui->verticalSpacer_Memory,       {"DLatch", "SRLatch", "DFlipFlop", "JKFlipFlop", "TFlipFlop"},                                                                                    m_ui->scrollAreaWidgetContents_Memory->layout());
    populateMenu(m_ui->verticalSpacer_Misc,         {"Text", "Line"},                                                                                                                                  m_ui->scrollAreaWidgetContents_Misc->layout());
}

void ElementPalette::updateICList(const QFileInfo &currentFile)
{
    // Remove the expanding spacer before inserting items so new labels don't push it out of place.
    m_ui->scrollAreaWidgetContents_IC->layout()->removeItem(m_ui->verticalSpacer_IC);

    // Clear all existing IC labels from both the IC panel and the search panel.
    const auto items = m_ui->scrollAreaWidgetContents_IC->findChildren<ElementLabel *>();
    for (auto *item : items) {
        item->deleteLater();
    }

    const auto items2 = m_ui->scrollAreaWidgetContents_Search->findChildren<ElementLabel *>();
    for (auto *item : items2) {
        if (item->elementType() == ElementType::IC) {
            item->deleteLater();
        }
    }

    if (currentFile.exists()) {
        QDir directory(currentFile.absoluteDir());
        // Enumerate all .panda files in the same directory — they are candidate ICs.
        QStringList files = directory.entryList({"*.panda", "*.PANDA"}, QDir::Files);
        // Exclude the project file itself and hidden/autosave files.
        files.removeAll(currentFile.fileName());
        for (int i = static_cast<int>(files.size()) - 1; i >= 0; --i) {
            if (files.at(i).at(0) == '.') {
                files.removeAt(i);
            }
        }

        for (const QString &file : std::as_const(files)) {
            const QPixmap pixmap(":/Components/Logic/ic-panda.svg");

            auto *item = new ElementLabel(pixmap, ElementType::IC, file, m_ui->scrollAreaWidgetContents_IC);
            m_ui->scrollAreaWidgetContents_IC->layout()->addWidget(item);

            auto *item2 = new ElementLabel(pixmap, ElementType::IC, file, m_ui->scrollAreaWidgetContents_Search);
            m_ui->scrollAreaWidgetContents_Search->layout()->addWidget(item2);
        }
    }

    m_ui->scrollAreaWidgetContents_IC->layout()->addItem(m_ui->verticalSpacer_IC);
}

void ElementPalette::retranslateLabels()
{
    const auto items = m_ui->tabElements->findChildren<ElementLabel *>();
    for (auto *item : items) {
        item->updateName();
    }
}

void ElementPalette::updateTheme()
{
    const int memoryTabIndex = getTabIndex("memory");
    if (memoryTabIndex != -1) {
        m_ui->tabElements->setTabIcon(memoryTabIndex, QIcon(DFlipFlop::pixmapPath()));
    }

    const auto labels = m_ui->memory->findChildren<ElementLabel *>();
    for (auto *label : labels) {
        label->updateTheme();
    }
}

void ElementPalette::onSearchTextChanged(const QString &text)
{
    m_ui->scrollAreaWidgetContents_Search->layout()->removeItem(m_ui->verticalSpacer_Search);

    const int searchTabIndex = getTabIndex("search");

    if (text.isEmpty()) {
        // Restore the normal tab bar and return to the tab the user was on before typing.
        m_ui->tabElements->tabBar()->show();
        m_ui->tabElements->setCurrentIndex(m_lastTabIndex);
        if (searchTabIndex != -1) {
            m_ui->tabElements->setTabEnabled(searchTabIndex, false);
        }
        m_lastTabIndex = -1;
    } else {
        // On first keystroke, remember which tab was active so we can restore it.
        if (m_lastTabIndex == -1) {
            m_lastTabIndex = m_ui->tabElements->currentIndex();
        }

        // Hide the tab bar so only unified search results are visible.
        m_ui->tabElements->tabBar()->hide();
        if (searchTabIndex != -1) {
            m_ui->tabElements->setCurrentIndex(searchTabIndex);
            m_ui->tabElements->setTabEnabled(searchTabIndex, true);
        }

        const auto allItems = m_ui->scrollArea_Search->findChildren<ElementLabel *>();

        // First pass: match by object name (e.g. "label_and") — prioritises exact type hits.
        QRegularExpression regex1(QString("^label_.*%1.*").arg(text), QRegularExpression::CaseInsensitiveOption);
        auto searchResults = m_ui->scrollArea_Search->findChildren<ElementLabel *>(regex1);

        // Second pass: match by human-readable translated element name.
        QRegularExpression regex2(QString(".*%1.*").arg(text), QRegularExpression::CaseInsensitiveOption);
        for (auto *item : allItems) {
            if (regex2.match(item->name()).hasMatch() && !searchResults.contains(item)) {
                searchResults.append(item);
            }
        }

        // Third pass: also search IC file names (all share object name "label_ic").
        const auto ics = m_ui->scrollArea_Search->findChildren<ElementLabel *>("label_ic");
        for (auto *ic : ics) {
            if (regex2.match(ic->icFileName()).hasMatch()) {
                searchResults.append(ic);
            }
        }

        for (auto *item : allItems) {
            item->setHidden(true);
        }
        for (auto *item : std::as_const(searchResults)) {
            item->setVisible(true);
        }
    }

    m_ui->scrollAreaWidgetContents_Search->layout()->addItem(m_ui->verticalSpacer_Search);
}

void ElementPalette::onSearchReturnPressed()
{
    const auto allLabels = m_ui->scrollArea_Search->findChildren<ElementLabel *>();

    // Emit the first visible result so MainWindow can add it to the active scene.
    for (auto *label : std::as_const(allLabels)) {
        if (label->isVisible()) {
            emit addElementRequested(label->mimeData());
            m_ui->lineEditSearch->clear();
            return;
        }
    }
}

void ElementPalette::populateMenu(QSpacerItem *spacer, const QStringList &names, QLayout *layout)
{
    layout->removeItem(spacer);

    // Each element type gets two labels: one in its category panel and a
    // duplicate in the search panel so search can find everything in one place.
    QWidget *const container = layout->parentWidget();
    for (const auto &name : names) {
        const auto type   = ElementFactory::textToType(name);
        const auto pixmap = ElementFactory::pixmap(type);
        layout->addWidget(new ElementLabel(pixmap, type, name, container));
        m_ui->scrollAreaWidgetContents_Search->layout()->addWidget(new ElementLabel(pixmap, type, name, m_ui->scrollAreaWidgetContents_Search));
    }

    layout->addItem(spacer);
}

void ElementPalette::setupTabIcons()
{
    // Lookup by object name so reordering tabs in the UI doesn't break icon assignment.
    const int ioTabIndex            = getTabIndex("io");
    const int gatesTabIndex         = getTabIndex("gates");
    const int combinationalTabIndex = getTabIndex("combinational");
    const int memoryTabIndex        = getTabIndex("memory");
    const int icTabIndex            = getTabIndex("ic");
    const int miscTabIndex          = getTabIndex("misc");
    const int searchTabIndex        = getTabIndex("search");

    if (ioTabIndex != -1)            m_ui->tabElements->setTabIcon(ioTabIndex,            QIcon(":/Components/Input/buttonOff.svg"));
    if (gatesTabIndex != -1)         m_ui->tabElements->setTabIcon(gatesTabIndex,          QIcon(":/Components/Logic/xor.svg"));
    if (combinationalTabIndex != -1) m_ui->tabElements->setTabIcon(combinationalTabIndex,  QIcon(":/Components/Logic/truthtable-rotated.svg"));
    if (memoryTabIndex != -1)        m_ui->tabElements->setTabIcon(memoryTabIndex,         QIcon(DFlipFlop::pixmapPath()));
    if (icTabIndex != -1)            m_ui->tabElements->setTabIcon(icTabIndex,             QIcon(":/Components/Logic/ic-panda.svg"));
    if (miscTabIndex != -1)          m_ui->tabElements->setTabIcon(miscTabIndex,           QIcon(":/Components/Misc/text.png"));
    // The search tab is virtual; enabled only when the user types in the search box.
    if (searchTabIndex != -1)        m_ui->tabElements->setTabEnabled(searchTabIndex, false);
}

int ElementPalette::getTabIndex(const QString &objectName) const
{
    for (int i = 0; i < m_ui->tabElements->count(); ++i) {
        QWidget *tabWidget = m_ui->tabElements->widget(i);
        if (tabWidget && tabWidget->objectName() == objectName) {
            return i;
        }
    }
    return -1;
}

