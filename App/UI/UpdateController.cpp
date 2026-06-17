// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/UpdateController.h"

#include <QCheckBox>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>

#include "App/Core/Application.h"
#include "App/Core/Settings.h"
#include "App/Core/UpdateChecker.h"
#include "App/Versions.h"

UpdateController::UpdateController(QWidget *parent)
    : QObject(parent)
    , m_parent(parent)
{
}

void UpdateController::checkForUpdates()
{
    if (!Application::interactiveMode) {
        return;
    }

    auto *updateChecker = new UpdateChecker(this);
    connect(updateChecker, &UpdateChecker::updateAvailable, this, &UpdateController::showUpdateDialog);
    updateChecker->checkForUpdates();
}

void UpdateController::showUpdateDialog(const QString &latestVersion, const QUrl &downloadUrl, const QUrl &releaseUrl)
{
    QDialog dialog(m_parent);
    dialog.setWindowTitle(tr("Update Available"));
    dialog.setWindowModality(Qt::WindowModal);

    auto *layout = new QVBoxLayout(&dialog);

    const bool hasDirectDownload = downloadUrl.isValid() && !downloadUrl.isEmpty();
    auto *label = new QLabel(
        (hasDirectDownload
             ? tr("<b>wiRedPanda %1 is available.</b><br><br>"
                  "You are currently running version %2.<br>"
                  "Click <b>Download</b> to save the new version to your computer.")
             : tr("<b>wiRedPanda %1 is available.</b><br><br>"
                  "You are currently running version %2.<br>"
                  "Visit the release page to download the new version."))
            .arg(latestVersion, APP_VERSION),
        &dialog);
    label->setTextFormat(Qt::RichText);
    label->setWordWrap(true);
    layout->addWidget(label);

    auto *skipCheckBox = new QCheckBox(tr("Don't notify me about this version again"), &dialog);
    layout->addWidget(skipCheckBox);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    auto *downloadButton = buttonBox->addButton(tr("Download"), QDialogButtonBox::AcceptRole);
    connect(downloadButton, &QPushButton::clicked, &dialog, [&dialog] { dialog.accept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttonBox);

    const bool accepted = dialog.exec() == QDialog::Accepted;

    if (skipCheckBox->isChecked()) {
        Settings::setUpdateCheckSkippedVersion(latestVersion);
    }

    // The check date is recorded by UpdateChecker::onReplyFinished — the
    // single writer — so no dialog outcome needs to touch it here.
    if (accepted) {
        if (hasDirectDownload) {
            downloadUpdate(latestVersion, downloadUrl);
        } else {
            QDesktopServices::openUrl(releaseUrl);
        }
    }
}

void UpdateController::downloadUpdate(const QString &latestVersion, const QUrl &url)
{
    const QString fileName = url.fileName();
    const QString savePath = QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).filePath(fileName);

    auto *progress = new QProgressDialog(tr("Downloading wiRedPanda %1…").arg(latestVersion), tr("Cancel"), 0, 100, m_parent);
    progress->setWindowTitle(tr("Downloading Update"));
    progress->setWindowModality(Qt::WindowModal);
    progress->setMinimumDuration(0);
    progress->setValue(0);

    auto *network = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = network->get(request);

    connect(reply, &QNetworkReply::downloadProgress, progress, [progress](qint64 received, qint64 total) {
        if (total > 0) {
            progress->setValue(static_cast<int>(received * 100 / total));
        }
    });

    connect(progress, &QProgressDialog::canceled, reply, &QNetworkReply::abort);

    connect(reply, &QNetworkReply::finished, this, [this, reply, progress, savePath] {
        progress->close();
        progress->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            if (reply->error() != QNetworkReply::OperationCanceledError) {
                QMessageBox::warning(m_parent, tr("Download Failed"), tr("Could not download the update:\n%1").arg(reply->errorString()));
            }
            reply->deleteLater();
            return;
        }

        QFile file(savePath);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(m_parent, tr("Download Failed"), tr("Could not save the file:\n%1").arg(savePath));
            reply->deleteLater();
            return;
        }
        file.write(reply->readAll());
        file.close();
        reply->deleteLater();

        QMessageBox::information(m_parent, tr("Download Complete"),
            tr("wiRedPanda has been downloaded to:\n%1").arg(savePath));
    });
}
