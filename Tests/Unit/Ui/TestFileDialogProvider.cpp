// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestFileDialogProvider.h"

#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QPushButton>
#include <QTemporaryDir>

#include "App/UI/FileDialogProvider.h"
#include "Tests/Common/TestUtils.h"

namespace {

// RealFileDialogProvider wraps QFileDialog's real, blocking dialogs -- under the offscreen QPA
// platform used by this test suite, Qt has no native picker to fall back on, so
// QFileDialog::getOpenFileName()/exec() show Qt's own non-native widget dialog. accept()/
// reject()/done() are protected on QFileDialog, so drive it the same way a real user would:
// click its own accept/reject button (found via its QDialogButtonBox). The accept button's
// *role* stays AcceptRole regardless of dialog mode, but Qt's non-native file dialog does not
// tag it with the QDialogButtonBox::Open/Save *standard-button* enum (confirmed empirically: a
// button(QDialogButtonBox::Save) lookup found nothing on a save-mode dialog, hanging exec()
// with the button never clicked) -- so look it up by role, not by standard-button identity.
QAbstractButton *findFileDialogButton(QDialogButtonBox *buttonBox, QDialogButtonBox::ButtonRole role)
{
    for (auto *button : buttonBox->buttons()) {
        if (buttonBox->buttonRole(button) == role) {
            return button;
        }
    }
    return nullptr;
}

TestUtils::AutoDismisser dismisserForFileDialogButton(QDialogButtonBox::ButtonRole role)
{
    return TestUtils::AutoDismisser([role](QWidget *w) {
        auto *dialog = qobject_cast<QFileDialog *>(w);
        if (!dialog) return false;
        auto *buttonBox = dialog->findChild<QDialogButtonBox *>();
        if (!buttonBox) return false;
        auto *button = findFileDialogButton(buttonBox, role);
        if (!button) return false;
        // Force-enabled: Qt's own accept-button auto-enable/disable bookkeeping
        // (QFileDialogPrivate::_q_updateOkButton()) looks the button up by its
        // QDialogButtonBox::Open/Save *standard-button* identity, which (see above) this
        // dialog's button never has -- confirmed empirically that this leaves the button
        // permanently disabled on a fraction of runs (never re-enabled, hanging exec()
        // forever) regardless of a valid file having been selected. That auto-enable logic
        // is Qt's own concern, not RealFileDialogProvider's; forcing the state here drives
        // the dialog the same way a user clicking an already-enabled button would, without
        // depending on that unrelated, racy internal bookkeeping.
        button->setEnabled(true);
        // A real synthetic mouse click, not QAbstractButton::click() -- confirmed empirically
        // that click() on this dialog's Cancel button left enough internal state dangling to
        // make a *later* real QFileDialog in the same process hang in exec() with its own
        // accept button correctly found+clicked (see testRealGetSaveFileNameReturnsPathAndFilter
        // 's identical finding for the accept side).
        QTest::mouseClick(button, Qt::LeftButton);
        return true;
    });
}

} // namespace

void TestFileDialogProviderUnit::testFileDialogCreation()
{
    // FileDialogProvider is abstract — verify global accessor works
    QVERIFY(FileDialogs::provider() != nullptr);
}

void TestFileDialogProviderUnit::testRealGetOpenFileNameReturnsSelectedPath()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString filePath = dir.filePath("pick_me.panda");
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.close();

    RealFileDialogProvider provider;

    TestUtils::AutoDismisser dismisser([&](QWidget *w) {
        auto *dialog = qobject_cast<QFileDialog *>(w);
        if (!dialog) return false;
        dialog->selectFile(filePath);
        auto *buttonBox = dialog->findChild<QDialogButtonBox *>();
        if (!buttonBox) return false;
        auto *button = findFileDialogButton(buttonBox, QDialogButtonBox::AcceptRole);
        if (!button) return false;
        // See dismisserForFileDialogButton()'s comment: force-enabled rather than gated on
        // isEnabled(), since Qt's own auto-enable bookkeeping for this button is unreliable here.
        button->setEnabled(true);
        QTest::mouseClick(button, Qt::LeftButton);
        return true;
    });

    const QString result = provider.getOpenFileName(nullptr, QStringLiteral("Pick a file"), dir.path(), QStringLiteral("*"));

    QCOMPARE(result, filePath);
}

void TestFileDialogProviderUnit::testRealGetOpenFileNameReturnsEmptyOnCancel()
{
    RealFileDialogProvider provider;

    auto dismisser = dismisserForFileDialogButton(QDialogButtonBox::RejectRole);

    const QString result = provider.getOpenFileName(nullptr, QStringLiteral("Pick a file"), QString(), QStringLiteral("*"));

    QVERIFY(result.isEmpty());
}

void TestFileDialogProviderUnit::testRealGetSaveFileNameReturnsPathAndFilter()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString filePath = dir.filePath("save_me.panda");

    RealFileDialogProvider provider;

    TestUtils::AutoDismisser dismisser([&](QWidget *w) {
        auto *dialog = qobject_cast<QFileDialog *>(w);
        if (!dialog) return false;
        dialog->selectFile(filePath);
        auto *buttonBox = dialog->findChild<QDialogButtonBox *>();
        if (!buttonBox) return false;
        auto *button = findFileDialogButton(buttonBox, QDialogButtonBox::AcceptRole);
        if (!button) return false;
        // See dismisserForFileDialogButton()'s comment: force-enabled rather than gated on
        // isEnabled(), since Qt's own auto-enable bookkeeping for this button is unreliable here.
        button->setEnabled(true);
        QTest::mouseClick(button, Qt::LeftButton);
        return true;
    });

    const FileDialogResult result = provider.getSaveFileName(nullptr, QStringLiteral("Save a file"), dir.path(),
                                                               QStringLiteral("Panda files (*.panda)"));

    QCOMPARE(result.fileName, filePath);
    QCOMPARE(result.selectedFilter, QStringLiteral("Panda files (*.panda)"));
}

void TestFileDialogProviderUnit::testRealGetSaveFileNameReturnsEmptyOnCancel()
{
    RealFileDialogProvider provider;

    auto dismisser = dismisserForFileDialogButton(QDialogButtonBox::RejectRole);

    const FileDialogResult result = provider.getSaveFileName(nullptr, QStringLiteral("Save a file"), QString(), QStringLiteral("*"));

    QVERIFY(result.fileName.isEmpty());
}
