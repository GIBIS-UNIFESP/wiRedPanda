#pragma once

#include "editor.h"
#include "remotedevice.h"

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
class RemoteDeviceConfig;
}

class RemoteDeviceConfig : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteDeviceConfig(Editor *editor, QWidget *parent = nullptr, GraphicElement *elm = nullptr);
    ~RemoteDeviceConfig() override;

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

    Ui::RemoteDeviceConfig *m_ui;
    Editor *m_editor;
    QNetworkAccessManager *m_manager;
    QTimer m_timer;
    RemoteDevice *m_elm;
    RemoteLabOption m_currentOption;
};
