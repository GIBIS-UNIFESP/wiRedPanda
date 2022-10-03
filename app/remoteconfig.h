#pragma once

#include "remote.h"
#include "workspace.h"

#include <QAbstractButton>
#include <QDialog>
#include <QLocalSocket>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextStream>

constexpr int major_remote_version = 1;
constexpr int minor_remote_version = 0;

namespace Ui
{
class RemoteConfig;
}

class RemoteConfig : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteConfig(QWidget *parent = nullptr, GraphicElement *elm = nullptr);
    ~RemoteConfig() override;

    static QString version() {
        return QString("%1.%2").arg(QString::number(major_remote_version), QString::number(minor_remote_version));
    }

    bool savePortMapping();
    void setupAuthScreen();
    void setupConfigScreen();
    void setupQueueScreen();
    void start();
    void updateServiceInfo(const QString &str);

private:
    void comboboxItemChanged(const QString &currentSelected);
    void connectionResponse(QNetworkReply *reply);
    void onAddPin();
    void onApplyConfig(QAbstractButton *btn);
    void onCopyToClipboard();
    void onEditPortMapping(int row, int column);
    void onQueueTimeRefresh();
    void onRejectConfig();
    void onRemovePin();
    void onTimeRefresh();
    void onTryToConnect();
    void on_disconnectBtn_clicked();
    void on_leaveBtn_clicked();

    Ui::RemoteConfig *m_ui;
    QNetworkAccessManager *m_manager;
    QTimer m_timer;
    Remote *m_remote;
    RemoteLabOption m_currentOption;
};
