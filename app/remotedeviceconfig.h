#pragma once

#include "editor.h"
#include "remotedevice.h"

#include <QDialog>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAbstractButton>
#include <QLocalSocket>

#define MAJOR_REMOTE_VERSION 1
#define MINOR_REMOTE_VERSION 0

namespace Ui
{
class RemoteDeviceConfig;
}

class RemoteDeviceConfig : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteDeviceConfig(Editor *editor, QWidget *parent = nullptr, GraphicElement *elm = nullptr);
    ~RemoteDeviceConfig();

    static QString version() {
        return QString("%1.%2").arg(QString::number(MAJOR_REMOTE_VERSION), QString::number(MINOR_REMOTE_VERSION));
    }

    bool savePortMapping();
    void setupAuthScreen();
    void setupConfigScreen();
    void setupQueueScreen();
    void start();
    void updateServiceInfo(const QString &str);


private:
    void comboboxItemChanged(QString arg);
    void connectionResponse(QNetworkReply* reply);
    void onAddPin();
    void onApplyConfig(QAbstractButton* btn);
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
