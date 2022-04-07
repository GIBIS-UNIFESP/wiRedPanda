#include "remotedeviceconfig.h"
#include "ui_remotedeviceconfig.h"

#include <QClipboard>
#include <QSettings>
#include <QInputDialog>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QHostInfo>
#include <QPixmap>
#include <QMenu>
#include <QAction>
#include <QMovie>

#include <iostream>

RemoteDeviceConfig::RemoteDeviceConfig(Editor *editor, QWidget *parent, GraphicElement *elm) : QDialog(parent), m_ui(new Ui::RemoteDeviceConfig), m_editor(editor), m_manager(new QNetworkAccessManager(this)) {
    m_ui->setupUi(this);

    setWindowTitle("Remote Device");
    setWindowFlags(Qt::Dialog);
    setModal(true);

    m_ui->stackedWidget->setCurrentIndex(0);

    QPixmap unavailable(":/remote/unavailable.png");
    m_ui->logo->setPixmap(unavailable);

    m_ui->progressBar->setMinimum(0);
    m_ui->progressBar->setMaximum(100);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    m_manager->setTransferTimeout(1500);
#endif

    connect(m_manager, &QNetworkAccessManager::finished,
            this, &RemoteDeviceConfig::connectionResponse);

    if (auto *remoteDevice = dynamic_cast<RemoteDevice*>(elm)) {
        m_elm = remoteDevice;
    }

    if (m_elm->getAuthToken() == "") {
        setupAuthScreen();
    } else if (m_elm->isInQueue()) {
        setupQueueScreen();
    } else {
        setupConfigScreen();
    }
}

RemoteDeviceConfig::~RemoteDeviceConfig() {
    delete m_manager;
    delete m_ui;
}

void RemoteDeviceConfig::setupQueueScreen() {
    m_ui->stackedWidget->setCurrentIndex(2);

    QMovie *movie = new QMovie(":/remote/loading.gif");
    m_ui->waiting_list_logo->setMovie(movie);
    movie->start();

    onQueueTimeRefresh();

    connect(&m_timer, &QTimer::timeout, this, &RemoteDeviceConfig::onQueueTimeRefresh);
    m_timer.start(1000);
}

void RemoteDeviceConfig::setupAuthScreen() {
    m_ui->stackedWidget->setCurrentIndex(0);

    for (auto option : m_elm->getOptions()) {
        m_ui->serviceSelector->addItem(QString::fromUtf8(option.getName().c_str()));
    }

    QString currentSelected = m_ui->serviceSelector->currentText();

    connect(m_ui->serviceSelector, &QComboBox::currentTextChanged, this, &RemoteDeviceConfig::comboboxItemChanged);
    connect(m_ui->connectButton, &QAbstractButton::clicked, this, &RemoteDeviceConfig::onTryToConnect);
    m_ui->connectButton->setDisabled(false);

    updateServiceInfo(currentSelected);
}

void RemoteDeviceConfig::onEditPortMapping(int row, int column) {
    QStringList items;

    if (column == 0) {
        const std::list<Pin>& availablePins = m_elm->getAvailablePins();
        std::list<Pin>::const_iterator it;
        for (it = availablePins.begin(); it != availablePins.end(); ++it) {
            items << QString::fromStdString(it->getName());
        }
    } else if (column == 1) {
        items << "INPUT";
        items << "OUTPUT";
    }

    QInputDialog input;
    input.setOptions(QInputDialog::UseListViewForComboBoxItems);
    input.setComboBoxItems(items);
    input.setWindowTitle("Choose action");

    if (input.exec())
    {
        QString inputValue = input.textValue();
        QTableWidgetItem* item = m_ui->tableWidget->item(row, column);

        if (!item) {
            item = new QTableWidgetItem();
            m_ui->tableWidget->setItem(row, column, item);
        }

        item->setText(inputValue);
    }
}

void RemoteDeviceConfig::onAddPin() {
    m_ui->tableWidget->insertRow(m_ui->tableWidget->currentRow() + 1);
}

void RemoteDeviceConfig::onRemovePin() {
    m_ui->tableWidget->removeRow(m_ui->tableWidget->currentRow());
}

bool RemoteDeviceConfig::savePortMapping() {
    m_elm->resetPortMapping();

    int rowsAmount = m_ui->tableWidget->rowCount();

    for (int row = 0; row<rowsAmount; row++) {
        QTableWidgetItem* portWidget = m_ui->tableWidget->item(row, 0);

        if (!portWidget) {
            QMessageBox messageBox;
            messageBox.information(0,"Error","Found rows with empty port name");
            messageBox.setFixedSize(500,200);
            return false;
        }

        QString portName = portWidget->text();

        QTableWidgetItem* typeWidget = m_ui->tableWidget->item(row, 1);

        if (!typeWidget) {
            QMessageBox messageBox;
            messageBox.information(0,"Error","Found rows with invalid type");
            messageBox.setFixedSize(500,200);
            return false;
        }

        QString typeName = typeWidget->text();

        Pin_Type type = Pin::convertTypeString(typeName.toStdString());

        if (!m_elm->mapPin(portName.toStdString(), static_cast<uint8_t>(type))) {
            QMessageBox messageBox;
            messageBox.information(0,"Error","Unable to map pin " + portName + " as " + typeName);
            messageBox.setFixedSize(500,200);
            return false;
        }

    }

    m_elm->sendIOInfo();
    m_elm->setupPorts();

    return true;
}
void RemoteDeviceConfig::onQueueTimeRefresh() {
    uint64_t timestamp = QDateTime::currentMSecsSinceEpoch() / 1000;

    long estimatedTime = static_cast<long>(m_elm->getQueueEstimatedEpoch() - timestamp);

    int hour = estimatedTime / 3600;
    int min = (estimatedTime % 3600) / 60;
    int secs = estimatedTime % 60;

    if (hour < 0)
        hour = 0;

    if (min < 0)
        min = 0;

    if (secs < 0)
        secs = 0;

    QString hourStr = QString::number(hour).rightJustified(2, '0');
    QString minStr = QString::number(min).rightJustified(2, '0');
    QString secsStr = QString::number(secs).rightJustified(2, '0');

    // changing separator from double dots sign to empty space makes a simple clock animation
    QString separator = ":";
    if (timestamp % 2 == 0)
        separator = " ";

    m_ui->timeEstimated->display(hourStr + separator + minStr + separator + secsStr);

    m_ui->estimatedTimeProgressBar->setMinimum(0);
    m_ui->estimatedTimeProgressBar->setMaximum(100);

    int estimatedSeconds = static_cast<int>(m_elm->getQueueEstimatedEpoch() - m_elm->getWaitingSince());

    if (estimatedSeconds <= 0)
        estimatedSeconds = 1;

    int secondsPassed = static_cast<int>(timestamp - m_elm->getWaitingSince());

    if (secondsPassed <= 0)
        secondsPassed = 0;

    m_ui->estimatedTimeProgressBar->setValue(static_cast<int>(secondsPassed * 100.0f / estimatedSeconds));

    QString queueInfoModel = "<html><head/><body><p><span style=\" font-size:12pt;\">You are currently on position </span><span style=\" font-size:12pt; font-weight:600;\">%1</span></p><p>Please be patient and you will receive at least %2 to use this device</p></body></html>";

    QString deviceAllowedTime = QDateTime::fromMSecsSinceEpoch(static_cast<uint32_t>(m_elm->getDeviceAllowedTime() * 1000), Qt::UTC).toString("mm:ss");

    QString str = queueInfoModel.arg(m_elm->getQueuePos()).arg(deviceAllowedTime);
    m_ui->queueInfoLbl->setText(str);

    // finally in
    if (!m_elm->isInQueue()) {
        if (m_elm->getDeviceId() != 0)
            setupConfigScreen();
        else
            close();
    }
}

void RemoteDeviceConfig::onTimeRefresh() {
    static uint16_t last_latency;

    uint64_t timestamp = QDateTime::currentMSecsSinceEpoch() / 1000;

    {
        uint64_t allowUntil = m_elm->getAllowUntil();
        long timeRemaining = static_cast<long>(allowUntil - timestamp);

        bool allowedTimeExceeded = false;

        // if time already exceeded just consider minTime
        if (timeRemaining < 0) {
            timeRemaining = 0;
            allowedTimeExceeded = true;
        }

        int min = timeRemaining / 60;
        int secs = timeRemaining % 60;

        // increases minWaitTime or availableAfterTime to the time left
        long avaliableAfterTime = m_elm->getAvailableAfterTime();

        min += avaliableAfterTime / 60;
        secs += avaliableAfterTime % 60;

        if (secs >= 60) {
            min += 1;
            secs -= 60;
        }

        if (min < 0)
            min = 0;

        if (secs < 0)
            secs = 0;

        QString minStr = QString::number(min).rightJustified(2, '0');
        QString secsStr = QString::number(secs).rightJustified(2, '0');

        // changing separator from double dots sign to empty space makes a simple clock animation
        QString separator = ":";
        if (timestamp % 2 == 0)
            separator = " ";

        m_ui->timeRemaining->display(minStr + separator + secsStr);

        timeRemaining += static_cast<long>(avaliableAfterTime);
        int percent = static_cast<int>(static_cast<float>(timeRemaining*100.0f)/static_cast<float>(m_elm->getTimeAllowed()));

        m_ui->progressBar->setValue(percent);

        QPalette progressBarPalette;

        if (allowedTimeExceeded) {
            progressBarPalette.setColor(QPalette::Highlight, Qt::yellow);
        } else {
            progressBarPalette.setColor(QPalette::Highlight, Qt::green);
        }

        m_ui->progressBar->setPalette(progressBarPalette);

        // Auto closes window when reaching zero seconds
        if (avaliableAfterTime <= 0)
            close();
    }

    QPalette sample_palette;
    sample_palette.setColor(QPalette::Window, Qt::white);
    if (m_elm->getLatency() <= 20) {
        sample_palette.setColor(QPalette::WindowText, Qt::darkGreen);
    } else if (m_elm->getLatency() > 20 && m_elm->getLatency() <= 50) {
        sample_palette.setColor(QPalette::WindowText, Qt::darkYellow);
    } else {
        sample_palette.setColor(QPalette::WindowText, Qt::darkRed);
    }

    static int warnedAlready = 0;

    if (warnedAlready < 2 && m_elm->getLatency() > 120 && last_latency > 120) {

        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setIcon(QMessageBox::Critical);
        msgBox->setText("Your connection latency is pretty bad, you will not be able to use the remote lab.");
        msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
        msgBox->show();
        msgBox->raise();

        warnedAlready = 2;
    }

    if (warnedAlready < 1 && m_elm->getLatency() > 80 && last_latency > 80) {

        QMessageBox *msgBox = new QMessageBox(this);
        msgBox->setIcon(QMessageBox::Warning);
        msgBox->setText("Looks like your connection is pretty unstable, you may not be able to use the remote lab.");
        msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
        msgBox->show();
        msgBox->raise();

        warnedAlready = 1;
    }

    last_latency = m_elm->getLatency();

    m_ui->ping->setAutoFillBackground(true);
    m_ui->ping->setPalette(sample_palette);
    m_ui->ping->setText("Ping: " + QString::number(m_elm->getLatency()) + "ms");
}

void RemoteDeviceConfig::setupConfigScreen() {
    m_ui->stackedWidget->setCurrentIndex(1);
    QString url = QString::fromStdString(m_elm->getCurrentOption().getUrl());

    std::cout << m_elm->getAuthToken() << std::endl;
    std::cout << url.toStdString() << std::endl;

    // Connect buttons
    connect(m_ui->buttonBox, &QDialogButtonBox::clicked, this, &RemoteDeviceConfig::onApplyConfig);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &RemoteDeviceConfig::onRejectConfig);

    m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QAction* addAction = new QAction("Add", this);
    QAction* removeAction = new QAction("Remove", this);

    QAction* copyAction = new QAction("Copy", this);

    connect(addAction, &QAction::triggered, this, &RemoteDeviceConfig::onAddPin);
    connect(removeAction, &QAction::triggered, this, &RemoteDeviceConfig::onRemovePin);

    connect(copyAction, &QAction::triggered, this, &RemoteDeviceConfig::onCopyToClipboard);

    addAction->setIcon(QIcon(":/toolbar/zoomIn.png"));
    removeAction->setIcon(QIcon(":/toolbar/zoomOut.png"));

    copyAction->setIcon(QIcon(":/toolbar/copy.png"));

    QMenu* alignMenu = new QMenu;
    alignMenu->addAction(addAction);
    alignMenu->addAction(removeAction);

    m_ui->pinOptions->setPopupMode(QToolButton::MenuButtonPopup);
    m_ui->pinOptions->setMenu(alignMenu);
    m_ui->pinOptions->setDefaultAction(addAction);

    m_ui->copyToClipboard->setDefaultAction(copyAction);

    connect(m_ui->tableWidget, &QTableWidget::cellDoubleClicked, this, &RemoteDeviceConfig::onEditPortMapping);

    m_ui->ping->setText("Ping: ...");
    m_elm->sendPing();

    onTimeRefresh();

    connect(&m_timer, &QTimer::timeout, this, &RemoteDeviceConfig::onTimeRefresh);
    m_timer.start(1000);

    m_ui->methodLabel->setText(QString::fromStdString(m_elm->getDeviceAuth().name));
    m_ui->methodTokenFrame->hide();

    if (m_elm->getDeviceMethod().compare("VirtualHere") == 0) {
        m_ui->content->setText(QString::fromStdString(m_elm->getDeviceAuth().token));
        m_ui->methodTokenFrame->show();
    }

    // pins

    m_ui->tableWidget->clearContents();

    const std::list<Pin>& mappedPins = m_elm->getMappedPins();
    std::list<Pin>::const_iterator it;
    int row = 0;
    for (it = mappedPins.begin(); it != mappedPins.end(); ++it) {
        m_ui->tableWidget->insertRow(row);
        QTableWidgetItem* portWidget = m_ui->tableWidget->item(row, 0);

        if (!portWidget) {
            portWidget = new QTableWidgetItem();
            m_ui->tableWidget->setItem(row, 0, portWidget);
        }

        portWidget->setText(QString::fromStdString(it->getName()));

        auto* typeWidget = m_ui->tableWidget->item(row, 1);

        if (!typeWidget) {
            typeWidget = new QTableWidgetItem();
            m_ui->tableWidget->setItem(row, 1, typeWidget);
        }

        QString type = "";
        switch(static_cast<int>(it->getType())) {
        case 1:  type = "INPUT";  break;
        case 2:  type = "OUTPUT"; break;
        default: type = "ERROR";  break;
        }

        typeWidget->setText(type);

        row++;
    }

    // retrieve service image
    {
        QNetworkRequest request;
        request.setUrl(QUrl(url+"logo"));
        request.setRawHeader("User-Agent", "WiRedPanda 1.0"); // TODO: replace 1.0 with APP_VERSION

        QNetworkReply* reply = m_manager->get(request);
        reply->setProperty("type", "image");
        reply->setProperty("req", "setInfoLogo");
    }

    // retrieve method image
    {
        QNetworkRequest request;
        request.setUrl(QUrl(url+"method"));
        request.setRawHeader("User-Agent", "WiRedPanda 1.0"); // TODO: replace 1.0 with APP_VERSION
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        QString postDataStr("token=" + QString::fromStdString(m_elm->getAuthToken()) + "&" + "deviceId=" + QString::number(m_elm->getDeviceId()));

        QNetworkReply* reply = m_manager->post(request, postDataStr.toUtf8());
        reply->setProperty("type", "image");
        reply->setProperty("req", "setMethod");
    }

    // TODO: Remove QProcess* memory should be freed
    if (m_elm->getDeviceMethod() == "VirtualHere") {
#ifdef _WIN32
        // start virtualhere
        {
            QStringList args;
            QProcess* process = new QProcess();
            process->setWorkingDirectory(QDir::currentPath());

            args << "-cvhui.ini";
            process->start("vhui32.exe", args);
        }
#endif
#ifdef _WIN64
        // start virtualhere
        {
            QStringList args;
            QProcess* process = new QProcess();
            process->setWorkingDirectory(QDir::currentPath());

            args << "-cvhui.ini";
            process->start("vhui64.exe", args);
        }
#endif
    }
}

void RemoteDeviceConfig::start() {
    exec();
}

void RemoteDeviceConfig::connectionResponse(QNetworkReply* reply) {
    QByteArray repliedData;
    repliedData = reply->readAll();

    if (reply->property("type") == "json") {
        QJsonDocument loadDoc(QJsonDocument::fromJson(repliedData));
        std::cout << "JSON " << static_cast<QString>(repliedData).toStdString() << std::endl;

        QJsonObject json = loadDoc.object();
        QVariantMap json_map = json.toVariantMap();

        if (reply->property("req") == "updateService") {
            QPixmap online(":/remote/online.png");
            QPixmap offline(":/remote/offline.png");

            QString versionStr = json["version"].toString();
            m_currentOption.setVersion(versionStr.toStdString());

            if (json["status"].toString() == "Online") {
                m_ui->status->setPixmap(online.scaled(m_ui->status->width(), m_ui->status->height(), Qt::KeepAspectRatio));
            } else {
                m_ui->status->setPixmap(offline.scaled(m_ui->status->width(), m_ui->status->height(), Qt::KeepAspectRatio));
            }

            m_ui->statusLabel->setText(json["status"].toString() != "" ? json["status"].toString() : "Offline");
            m_ui->uptimeLabel->setText("Uptime: " + json["uptime"].toString());

            m_ui->deviceSelector->clear();
            m_ui->methodSelector->clear();

            int totalAmount = 0;
            int avaliableAmount = 0;

            {
                QVariantMap devicesAmount = json_map["devices"].toMap();
                QStringList key_list = devicesAmount.keys();
                for (int i = 0; i < key_list.count(); ++i) {
                    QString key = key_list.at(i);
                    int deviceId = devicesAmount[key].toInt();
                    m_ui->deviceSelector->addItem(QString ("%1").arg(key), QVariant::fromValue(deviceId));
                }
            }

            {
                QVariantMap devicesAmount = json_map["devicesAmount"].toMap();
                QStringList key_list = devicesAmount.keys();
                for (int i = 0; i < key_list.count(); ++i) {
                    QString key = key_list.at(i);
                    int amount = devicesAmount[key].toInt();
                    totalAmount += amount;
                }
            }

            {
                QVariantMap devicesAvailable = json_map["devicesAvailable"].toMap();
                QStringList key_list = devicesAvailable.keys();
                for (int i = 0; i < key_list.count(); ++i) {
                    QString key = key_list.at(i);
                    int amount = devicesAvailable[key].toInt();
                    avaliableAmount += amount;
                }
            }

            {
                QVariantMap methods = json_map["methods"].toMap();
                QStringList key_list = methods.keys();
                for (int i = 0; i < key_list.count(); ++i) {
                    QString key = key_list.at(i);
                    int methodId = methods[key].toInt();
                    m_ui->methodSelector->addItem(QString ("%1").arg(key), QVariant::fromValue(methodId));
                }
            }

            m_ui->lcdTotal->display(totalAmount);
            m_ui->lcdAmount->display(avaliableAmount);
        } else if (reply->property("req") == "connect") {
            m_ui->connectButton->setDisabled(false);

            if (json["reply"] != "ok") {
                QMessageBox messageBox;
                messageBox.critical(0,"Error",json["msg"].toString());
                messageBox.setFixedSize(500,200);
                return;
            }

            QString host = json["host"].toString();

            if (host == "0.0.0.0")
                host = reply->url().toString();

            QHostInfo info = QHostInfo::fromName(host);

            if (info.error() != QHostInfo::NoError) {
                std::cout << info.errorString().toStdString() << std::endl;
                QMessageBox messageBox;
                messageBox.critical(0,"Error","Unable to resolve hostname !");
                messageBox.setFixedSize(500,200);
                close();
                return;
            }

            host = info.addresses().first().toString();

            uint8_t deviceTypeId = m_ui->deviceSelector->currentData().toUInt();
            uint8_t methodId = m_ui->methodSelector->currentData().toUInt();

            if (!m_elm->connectTo(host.toStdString(), json["port"].toInt(), json["token"].toString().toStdString(), deviceTypeId, methodId)) {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","Connection failure !");
                messageBox.setFixedSize(500,200);
                close();
                return;
            }

            close();
        }
    }

    if (reply->property("type") == "image") {
        QString req = reply->property("req").toString();
        std::cout << "IMAGE req=" << req.toStdString() << std::endl;

        if (req == "setLogo") {
            QPixmap pic;
            pic.loadFromData(repliedData);

            if (!pic.isNull()) {
                m_ui->logo->setPixmap(pic.scaled(m_ui->logo->width(), m_ui->logo->height(), Qt::KeepAspectRatio));
            } else {
                QPixmap unavailable(":/remote/unavailable.png");
                m_ui->logo->setPixmap(unavailable);
            }
        } else if (req == "setInfoLogo") {
            QPixmap pic;
            pic.loadFromData(repliedData);

            if (!pic.isNull()) {
                m_ui->infoLogo->setPixmap(pic.scaled(m_ui->infoLogo->width(), m_ui->infoLogo->height(), Qt::KeepAspectRatio));
            } else {
                QPixmap unavailable(":/remote/unavailable.png");
                m_ui->infoLogo->setPixmap(unavailable);
            }
        } else if (req == "setMethod") {
            QPixmap pic;
            pic.loadFromData(repliedData);

            if (!pic.isNull()) {
                m_ui->methodImg->setPixmap(pic.scaled(m_ui->methodImg->width(), m_ui->methodImg->height(), Qt::KeepAspectRatio));
            } else {
                QPixmap unavailable(":/remote/unavailable.png");
                m_ui->methodImg->setPixmap(unavailable);
            }
        }
    }
}

void RemoteDeviceConfig::updateServiceInfo(const QString &str) {
    auto &options = m_elm->getOptions();
    auto it = options.begin();

    while (it != options.end()) {
        if (it->getName() != str.toStdString())
            it++;
        else
            break;
    }

    if (it != options.end()) {
        m_currentOption = (*it);
        QString url = QString::fromStdString(it->getUrl());

        // retrieve service information
        {
            QNetworkRequest request;
            request.setUrl(QUrl(url));
            request.setRawHeader("User-Agent", "WiRedPanda 1.0"); // TODO: replace with APP_VERSION

            QNetworkReply* reply = m_manager->get(request);
            reply->setProperty("type", "json");
            reply->setProperty("req", "updateService");
        }

        // retrieve service image
        {
            QNetworkRequest request;
            request.setUrl(QUrl(url+"logo"));
            request.setRawHeader("User-Agent", "WiRedPanda 1.0"); // TODO: replace with APP_VERSION

            QNetworkReply* reply = m_manager->get(request);
            reply->setProperty("type", "image");
            reply->setProperty("req", "setLogo");
        }

        // change to loading
        QPixmap unavailable(":/remote/unavailable.png");
        QPixmap searching(":/remote/searching.png");

        m_ui->logo->setPixmap(unavailable);
        m_ui->status->setPixmap(searching.scaled(m_ui->status->width(), m_ui->status->height(), Qt::KeepAspectRatio));
        m_ui->deviceSelector->clear();
        m_ui->statusLabel->setText("...");
        m_ui->lcdTotal->display(0);
        m_ui->lcdAmount->display(0);
    } else {
        QMessageBox messageBox;
        messageBox.critical(0,"Error","Any available services could be found");
        messageBox.setFixedSize(500,200);
        close();
    }
}

void RemoteDeviceConfig::onTryToConnect() {
    // verify if the version is compatible before sending connection request

    m_ui->connectButton->setDisabled(true);

    const QString versionStr = QString::fromStdString(m_currentOption.getVersion());
    const int majorVersion = versionStr.split(".").first().toInt();

    if (MAJOR_REMOTE_VERSION < majorVersion) {
        QMessageBox messageBox;
        messageBox.critical(0,"Version mismatch","It seems your version is currently outdated, the version required for this domain is (" + versionStr + "). Please download the newest version.");
        return;
    } else if (MAJOR_REMOTE_VERSION > majorVersion) {
        QMessageBox messageBox;
        messageBox.critical(0,"Version mismatch","It seems that the domain is using an outdated version, please contact the administrator.");
        return;
    }

    QNetworkRequest request;
    request.setUrl(QUrl(QString::fromStdString(m_currentOption.getUrl()) + "login"));
    request.setRawHeader("User-Agent", "WiRedPanda 1.0"); // TODO: replace with APP_VERSION
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QString encodedPasswd = m_ui->passwdInput->text();
    Auth_Method currentAuthMethod = m_currentOption.getAuthMethod();
    if (currentAuthMethod != Auth_Method::Plain) {
        QCryptographicHash::Algorithm algorithm;
        switch(currentAuthMethod) {
        case Auth_Method::MD5:
            algorithm = QCryptographicHash::Algorithm::Md5;
            break;
        case Auth_Method::SHA1:
            algorithm = QCryptographicHash::Algorithm::Sha1;
            break;
        case Auth_Method::SHA256:
            algorithm = QCryptographicHash::Algorithm::Sha256;
            break;
        default:
            algorithm = QCryptographicHash::Algorithm::Sha256;
            std::cerr << "ERROR: Unable to evaluate the correct encoding method, sending as SHA256." << std::endl;
            break;
        }

        QCryptographicHash hashAlgorithm(algorithm);
        hashAlgorithm.addData(encodedPasswd.toStdString().c_str(), encodedPasswd.toStdString().length());
        encodedPasswd = hashAlgorithm.result().toHex();
    }

    m_elm->setCurrentOption(m_currentOption);

    QString postDataStr("login=" + m_ui->loginInput->text() + "&" + "passwd=" + encodedPasswd);

    // when logging in, it might take more time
    // once LDAP service can be quite slow
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    m_manager->setTransferTimeout(20000);
#endif
    QNetworkReply* reply = m_manager->post(request, postDataStr.toUtf8());
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    m_manager->setTransferTimeout(1000);
#endif

    reply->setProperty("type", "json");
    reply->setProperty("req", "connect");
}

void RemoteDeviceConfig::comboboxItemChanged(QString currentSelected) {
    updateServiceInfo(currentSelected);
}

void RemoteDeviceConfig::onApplyConfig(QAbstractButton* btn)
{
    if (btn->text() == "Apply") {
        std::cout << "APPLIED!" << std::endl;
        if (savePortMapping()) {
            close();
        }
    }
}

void RemoteDeviceConfig::onRejectConfig()
{
    std::cout << "REJECTED!" << std::endl;
}

void RemoteDeviceConfig::onCopyToClipboard()
{
    QClipboard *clipboard = QGuiApplication::clipboard();

    clipboard->clear();
    clipboard->setText(QString::fromStdString(m_elm->getDeviceAuth().token));
}

void RemoteDeviceConfig::on_disconnectBtn_clicked()
{
    m_elm->disconnect();

    QMessageBox messageBox;
    messageBox.information(0, "Info", "Disconnected successfully!");
    close();
}

void RemoteDeviceConfig::on_leaveBtn_clicked()
{
    m_elm->disconnect();

    QMessageBox messageBox;
    messageBox.information(0,"Info","You have left the queue!");
    close();
}
