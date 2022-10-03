#include "remoteconfig.h"
#include "ui_remoteconfig.h"

#include <QAction>
#include <QClipboard>
#include <QCryptographicHash>
#include <QHostInfo>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMovie>
#include <QPixmap>
#include <QSettings>
#include <iostream>

RemoteConfig::RemoteConfig(QWidget *parent, GraphicElement *elm)
    : QDialog(parent),
      m_ui(new Ui::RemoteConfig),
      m_manager(new QNetworkAccessManager(this))
{
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

    connect(m_manager, &QNetworkAccessManager::finished, this, &RemoteConfig::connectionResponse);

    if (auto *remoteDevice = dynamic_cast<Remote *>(elm)) {
        m_remote = remoteDevice;
    }

    if (m_remote->authToken().isEmpty()) {
        setupAuthScreen();
    } else if (m_remote->isInQueue()) {
        setupQueueScreen();
    } else {
        setupConfigScreen();
    }
}

RemoteConfig::~RemoteConfig()
{
    delete m_manager;
    delete m_ui;
}

void RemoteConfig::setupQueueScreen()
{
    m_ui->stackedWidget->setCurrentIndex(2);

    auto *movie = new QMovie(":/remote/loading.gif");
    m_ui->waiting_list_logo->setMovie(movie);
    movie->start();

    onQueueTimeRefresh();

    connect(&m_timer, &QTimer::timeout, this, &RemoteConfig::onQueueTimeRefresh);
    m_timer.start(1000);
}

void RemoteConfig::setupAuthScreen()
{
    m_ui->stackedWidget->setCurrentIndex(0);

    for (const auto &option : m_remote->options()) {
        m_ui->serviceSelector->addItem(option.name());
    }

    QString currentSelected = m_ui->serviceSelector->currentText();

    connect(m_ui->serviceSelector, &QComboBox::currentTextChanged, this, &RemoteConfig::comboboxItemChanged);
    connect(m_ui->connectButton, &QAbstractButton::clicked, this, &RemoteConfig::onTryToConnect);
    m_ui->connectButton->setDisabled(false);

    updateServiceInfo(currentSelected);
}

void RemoteConfig::onEditPortMapping(int row, int column)
{
    QStringList items;

    if (column == 0) {
        for (auto pin : m_remote->availablePins()) {
            items << pin.name();
        }
    } else if (column == 1) {
        items << "INPUT";
        items << "OUTPUT";
    }

    QInputDialog input;
    input.setOptions(QInputDialog::UseListViewForComboBoxItems);
    input.setComboBoxItems(items);
    input.setWindowTitle("Choose action");

    if (input.exec()) {
        QString inputValue = input.textValue();
        QTableWidgetItem *item = m_ui->tableWidget->item(row, column);

        if (!item) {
            item = new QTableWidgetItem();
            m_ui->tableWidget->setItem(row, column, item);
        }

        item->setText(inputValue);
    }
}

void RemoteConfig::onAddPin()
{
    m_ui->tableWidget->insertRow(m_ui->tableWidget->currentRow() + 1);
}

void RemoteConfig::onRemovePin()
{
    m_ui->tableWidget->removeRow(m_ui->tableWidget->currentRow());
}

bool RemoteConfig::savePortMapping()
{
    m_remote->resetPortMapping();

    int rowsAmount = m_ui->tableWidget->rowCount();

    for (int row = 0; row < rowsAmount; row++) {
        QTableWidgetItem *portWidget = m_ui->tableWidget->item(row, 0);

        if (!portWidget) {
            QMessageBox messageBox;
            messageBox.information(nullptr, "Error", "Found rows with empty port name");
            messageBox.setFixedSize(500, 200);
            return false;
        }

        QString portName = portWidget->text();

        QTableWidgetItem *typeWidget = m_ui->tableWidget->item(row, 1);

        if (!typeWidget) {
            QMessageBox messageBox;
            messageBox.information(nullptr, "Error", "Found rows with invalid type");
            messageBox.setFixedSize(500, 200);
            return false;
        }

        QString typeName = typeWidget->text();

        Pin_Type type = Pin::convertTypeString(typeName);

        if (!m_remote->mapPin(portName, static_cast<uint8_t>(type))) {
            QMessageBox messageBox;
            messageBox.information(nullptr, "Error", "Unable to map pin " + portName + " as " + typeName);
            messageBox.setFixedSize(500, 200);
            return false;
        }
    }

    m_remote->sendIOInfo();
    m_remote->setupPorts();

    return true;
}

void RemoteConfig::onQueueTimeRefresh()
{
    uint64_t timestamp = QDateTime::currentMSecsSinceEpoch() / 1000;

    long estimatedTime = static_cast<long>(m_remote->queueEstimatedEpoch() - timestamp);

    int hour = estimatedTime / 3600;
    int min = (estimatedTime % 3600) / 60;
    int secs = estimatedTime % 60;

    if (hour < 0) {
        hour = 0;
    }
    if (min < 0) {
        min = 0;
    }
    if (secs < 0) {
        secs = 0;
    }

    QString hourStr = QString::number(hour).rightJustified(2, '0');
    QString minStr = QString::number(min).rightJustified(2, '0');
    QString secsStr = QString::number(secs).rightJustified(2, '0');

    // changing separator from double dots sign to empty space makes a simple clock animation
    QString separator = ":";
    if (timestamp % 2 == 0) {
        separator = " ";
    }
    m_ui->timeEstimated->display(hourStr + separator + minStr + separator + secsStr);

    m_ui->estimatedTimeProgressBar->setMinimum(0);
    m_ui->estimatedTimeProgressBar->setMaximum(100);

    int estimatedSeconds = static_cast<int>(m_remote->queueEstimatedEpoch() - m_remote->waitingSince());

    if (estimatedSeconds <= 0) {
        estimatedSeconds = 1;
    }
    int secondsPassed = static_cast<int>(timestamp - m_remote->waitingSince());

    if (secondsPassed <= 0) {
        secondsPassed = 0;
    }
    m_ui->estimatedTimeProgressBar->setValue(static_cast<int>(secondsPassed * 100.0f / estimatedSeconds));

    QString queueInfoModel =
            R"(<html><head/><body><p><span style=" font-size:12pt;">You are currently on position </span><span style=" font-size:12pt; font-weight:600;">%1</span></p><p>Please be patient and you will receive at least %2 to use this device</p></body></html>)";
    QString deviceAllowedTime = QDateTime::fromMSecsSinceEpoch(static_cast<uint32_t>(m_remote->deviceAllowedTime() * 1000), Qt::UTC).toString("mm:ss");

    QString str = queueInfoModel.arg(m_remote->queuePos()).arg(deviceAllowedTime);
    m_ui->queueInfoLbl->setText(str);

    // finally in
    if (!m_remote->isInQueue()) {
        if (m_remote->deviceId() == 0) {
            close();
        }

        setupConfigScreen();
    }
}

void RemoteConfig::onTimeRefresh()
{
    static uint16_t last_latency;

    uint64_t timestamp = QDateTime::currentMSecsSinceEpoch() / 1000;

    {
        uint64_t allowUntil = m_remote->allowUntil();
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
        long avaliableAfterTime = m_remote->availableAfterTime();

        min += avaliableAfterTime / 60;
        secs += avaliableAfterTime % 60;

        if (secs >= 60) {
            min += 1;
            secs -= 60;
        }

        if (min < 0) {
            min = 0;
        }

        if (secs < 0) {
            secs = 0;
        }

        QString minStr = QString::number(min).rightJustified(2, '0');
        QString secsStr = QString::number(secs).rightJustified(2, '0');

        // changing separator from double dots sign to empty space makes a simple clock animation
        QString separator = ":";
        if (timestamp % 2 == 0) {
            separator = " ";
        }

        m_ui->timeRemaining->display(minStr + separator + secsStr);

        timeRemaining += static_cast<long>(avaliableAfterTime);
        int percent = static_cast<int>(static_cast<float>(timeRemaining * 100.0f) / static_cast<float>(m_remote->timeAllowed()));

        m_ui->progressBar->setValue(percent);

        QPalette progressBarPalette;
        progressBarPalette.setColor(QPalette::Highlight, allowedTimeExceeded ? Qt::yellow : Qt::green);

        m_ui->progressBar->setPalette(progressBarPalette);

        // Auto closes window when reaching zero seconds
        if (avaliableAfterTime <= 0) {
            close();
        }
    }

    QPalette sample_palette;
    sample_palette.setColor(QPalette::Window, Qt::white);
    if (m_remote->latency() <= 20) {
        sample_palette.setColor(QPalette::WindowText, Qt::darkGreen);
    } else if (m_remote->latency() > 20 && m_remote->latency() <= 50) {
        sample_palette.setColor(QPalette::WindowText, Qt::darkYellow);
    } else {
        sample_palette.setColor(QPalette::WindowText, Qt::darkRed);
    }

    static int warnedAlready = 0;

    if (warnedAlready < 2 && m_remote->latency() > 120 && last_latency > 120) {
        auto *msgBox = new QMessageBox(this);
        msgBox->setIcon(QMessageBox::Critical);
        msgBox->setText("Your connection latency is pretty bad, you will not be able to use the remote lab.");
        msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
        msgBox->show();
        msgBox->raise();

        warnedAlready = 2;
    }

    if (warnedAlready < 1 && m_remote->latency() > 80 && last_latency > 80) {
        auto *msgBox = new QMessageBox(this);
        msgBox->setIcon(QMessageBox::Warning);
        msgBox->setText("Looks like your connection is pretty unstable, you may not be able to use the remote lab.");
        msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
        msgBox->show();
        msgBox->raise();

        warnedAlready = 1;
    }

    last_latency = m_remote->latency();

    m_ui->ping->setAutoFillBackground(true);
    m_ui->ping->setPalette(sample_palette);
    m_ui->ping->setText("Ping: " + QString::number(m_remote->latency()) + "ms");
}

void RemoteConfig::setupConfigScreen()
{
    m_ui->stackedWidget->setCurrentIndex(1);
    QString url = m_remote->currentOption().url();

    qDebug() << m_remote->authToken();
    qDebug() << url;

    // Connect buttons
    connect(m_ui->buttonBox, &QDialogButtonBox::clicked, this, &RemoteConfig::onApplyConfig);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &RemoteConfig::onRejectConfig);

    m_ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *addAction = new QAction("Add", this);
    auto *removeAction = new QAction("Remove", this);

    auto *copyAction = new QAction("Copy", this);

    connect(addAction, &QAction::triggered, this, &RemoteConfig::onAddPin);
    connect(removeAction, &QAction::triggered, this, &RemoteConfig::onRemovePin);

    connect(copyAction, &QAction::triggered, this, &RemoteConfig::onCopyToClipboard);

    addAction->setIcon(QIcon(":/toolbar/zoomIn.png"));
    removeAction->setIcon(QIcon(":/toolbar/zoomOut.png"));

    copyAction->setIcon(QIcon(":/toolbar/copy.png"));

    auto *alignMenu = new QMenu;
    alignMenu->addAction(addAction);
    alignMenu->addAction(removeAction);

    m_ui->pinOptions->setPopupMode(QToolButton::MenuButtonPopup);
    m_ui->pinOptions->setMenu(alignMenu);
    m_ui->pinOptions->setDefaultAction(addAction);

    m_ui->copyToClipboard->setDefaultAction(copyAction);

    connect(m_ui->tableWidget, &QTableWidget::cellDoubleClicked, this, &RemoteConfig::onEditPortMapping);

    m_ui->ping->setText("Ping: ...");
    m_remote->sendPing();

    onTimeRefresh();

    connect(&m_timer, &QTimer::timeout, this, &RemoteConfig::onTimeRefresh);
    m_timer.start(1000);

    m_ui->methodLabel->setText(m_remote->deviceAuth().m_name);
    m_ui->methodTokenFrame->hide();

    if (m_remote->deviceMethod() == "VirtualHere") {
        m_ui->content->setText(m_remote->deviceAuth().m_token);
        m_ui->methodTokenFrame->show();
    }

    // pins

    m_ui->tableWidget->clearContents();

    const auto &mappedPins = m_remote->mappedPins();
    int row = 0;
    for (auto pin : mappedPins) {
        m_ui->tableWidget->insertRow(row);
        QTableWidgetItem *portWidget = m_ui->tableWidget->item(row, 0);

        if (!portWidget) {
            portWidget = new QTableWidgetItem();
            m_ui->tableWidget->setItem(row, 0, portWidget);
        }

        portWidget->setText(pin.name());

        auto *typeWidget = m_ui->tableWidget->item(row, 1);

        if (!typeWidget) {
            typeWidget = new QTableWidgetItem();
            m_ui->tableWidget->setItem(row, 1, typeWidget);
        }

        QString type = "";
        switch (static_cast<int>(pin.type())) {
        case 1: type = "INPUT"; break;
        case 2: type = "OUTPUT"; break;
        default: type = "ERROR"; break;
        }

        typeWidget->setText(type);

        row++;
    }

    // retrieve service image
    {
        QNetworkRequest request;
        request.setUrl(QUrl(url + "logo"));
        request.setRawHeader("User-Agent", "WiRedPanda 1.0"); // TODO: replace 1.0 with APP_VERSION

        QNetworkReply *reply = m_manager->get(request);
        reply->setProperty("type", "image");
        reply->setProperty("req", "setInfoLogo");
    }

    // retrieve method image
    {
        QNetworkRequest request;
        request.setUrl(QUrl(url + "method"));
        request.setRawHeader("User-Agent", "WiRedPanda 1.0"); // TODO: replace 1.0 with APP_VERSION
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        QString postDataStr("token=" + m_remote->authToken() + "&" + "deviceId=" + QString::number(m_remote->deviceId()));

        QNetworkReply *reply = m_manager->post(request, postDataStr.toUtf8());
        reply->setProperty("type", "image");
        reply->setProperty("req", "setMethod");
    }

    // TODO: Remove QProcess* memory should be freed
    if (m_remote->deviceMethod() == "VirtualHere") {
#ifdef _WIN32
        // start virtualhere
        {
            QStringList args;
            QProcess *process = new QProcess();
            process->setWorkingDirectory(QDir::currentPath());

            args << "-cvhui.ini";
            process->start("vhui32.exe", args);
        }
#endif
#ifdef _WIN64
        // start virtualhere
        {
            QStringList args;
            QProcess *process = new QProcess();
            process->setWorkingDirectory(QDir::currentPath());

            args << "-cvhui.ini";
            process->start("vhui64.exe", args);
        }
#endif
    }
}

void RemoteConfig::start()
{
    exec();
}

void RemoteConfig::connectionResponse(QNetworkReply *reply)
{
    QByteArray repliedData;
    repliedData = reply->readAll();

    if (reply->property("type") == "json") {
        QJsonDocument loadDoc(QJsonDocument::fromJson(repliedData));
        qDebug() << "JSON " << static_cast<QString>(repliedData);

        QJsonObject json = loadDoc.object();
        QVariantMap json_map = json.toVariantMap();

        if (reply->property("req") == "updateService") {
            QPixmap online(":/remote/online.png");
            QPixmap offline(":/remote/offline.png");

            QString versionStr = json["version"].toString();
            m_currentOption.setVersion(versionStr);

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
                    const QString &key = key_list.at(i);
                    int deviceId = devicesAmount[key].toInt();
                    m_ui->deviceSelector->addItem(QString("%1").arg(key), QVariant::fromValue(deviceId));
                }
            }

            {
                QVariantMap devicesAmount = json_map["devicesAmount"].toMap();
                QStringList key_list = devicesAmount.keys();
                for (int i = 0; i < key_list.count(); ++i) {
                    const QString &key = key_list.at(i);
                    int amount = devicesAmount[key].toInt();
                    totalAmount += amount;
                }
            }

            {
                QVariantMap devicesAvailable = json_map["devicesAvailable"].toMap();
                QStringList key_list = devicesAvailable.keys();
                for (int i = 0; i < key_list.count(); ++i) {
                    const QString &key = key_list.at(i);
                    int amount = devicesAvailable[key].toInt();
                    avaliableAmount += amount;
                }
            }

            {
                QVariantMap methods = json_map["methods"].toMap();
                QStringList key_list = methods.keys();
                for (int i = 0; i < key_list.count(); ++i) {
                    const QString &key = key_list.at(i);
                    int methodId = methods[key].toInt();
                    m_ui->methodSelector->addItem(QString("%1").arg(key), QVariant::fromValue(methodId));
                }
            }

            m_ui->lcdTotal->display(totalAmount);
            m_ui->lcdAmount->display(avaliableAmount);
        } else if (reply->property("req") == "connect") {
            m_ui->connectButton->setDisabled(false);

            if (json["reply"] != "ok") {
                QMessageBox messageBox;
                messageBox.critical(nullptr, "Error", json["msg"].toString());
                messageBox.setFixedSize(500, 200);
                return;
            }

            QString host = json["host"].toString();

            if (host == "0.0.0.0") {
                host = reply->url().toString();
            }
            QHostInfo info = QHostInfo::fromName(host);

            if (info.error() != QHostInfo::NoError) {
                qDebug() << info.errorString();
                QMessageBox messageBox;
                messageBox.critical(nullptr, "Error", "Unable to resolve hostname !");
                messageBox.setFixedSize(500, 200);
                close();
                return;
            }

            host = info.addresses().first().toString();

            uint8_t deviceTypeId = m_ui->deviceSelector->currentData().toUInt();
            uint8_t methodId = m_ui->methodSelector->currentData().toUInt();

            if (!m_remote->connectTo(host, json["port"].toInt(), json["token"].toString(), deviceTypeId, methodId)) {
                QMessageBox messageBox;
                messageBox.critical(nullptr, "Error", "Connection failure!");
                messageBox.setFixedSize(500, 200);
                close();
                return;
            }

            close();
        }
    }

    if (reply->property("type") == "image") {
        QString req = reply->property("req").toString();
        qDebug() << "IMAGE req=" << req;

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

void RemoteConfig::updateServiceInfo(const QString &str)
{
    const auto &options = m_remote->options();
    RemoteLabOption *option_ = nullptr;

    for (auto option : options) {
        if (option.name() == str) {
            option_ = &option;
            break;
        }
    }

    if (!option_) {
        QMessageBox messageBox;
        messageBox.critical(nullptr, "Error", "Any available services could be found");
        messageBox.setFixedSize(500, 200);
        close();
        return;
    }

    m_currentOption = *option_;
    QString url = option_->url();

    // retrieve service information
    {
        QNetworkRequest request;
        request.setUrl(QUrl(url));
        request.setRawHeader("User-Agent", "WiRedPanda 1.0"); // TODO: replace with APP_VERSION

        QNetworkReply *reply = m_manager->get(request);
        reply->setProperty("type", "json");
        reply->setProperty("req", "updateService");
    }

    // retrieve service image
    {
        QNetworkRequest request;
        request.setUrl(QUrl(url + "logo"));
        request.setRawHeader("User-Agent", "WiRedPanda 1.0"); // TODO: replace with APP_VERSION

        QNetworkReply *reply = m_manager->get(request);
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
}

void RemoteConfig::onTryToConnect()
{
    // verify if the version is compatible before sending connection request

    m_ui->connectButton->setDisabled(true);

    const QString versionStr = m_currentOption.version();
    const int majorVersion = versionStr.split(".").first().toInt();

    if (majorVersion != major_remote_version) {
        throw std::runtime_error("Version mismatch, the version required for this domain is (" + versionStr.toStdString() + ").");
    }

    QNetworkRequest request;
    request.setUrl(QUrl(m_currentOption.url() + "login"));
    request.setRawHeader("User-Agent", "WiRedPanda 1.0"); // TODO: replace with APP_VERSION
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QString encodedPasswd = m_ui->passwdInput->text();
    Auth_Method currentAuthMethod = m_currentOption.authMethod();
    if (currentAuthMethod != Auth_Method::Plain) {
        QCryptographicHash::Algorithm algorithm;
        switch (currentAuthMethod) {
        case Auth_Method::MD5: algorithm = QCryptographicHash::Algorithm::Md5; break;
        case Auth_Method::SHA1: algorithm = QCryptographicHash::Algorithm::Sha1; break;
        case Auth_Method::SHA256: algorithm = QCryptographicHash::Algorithm::Sha256; break;
        default:
            algorithm = QCryptographicHash::Algorithm::Sha256;
            qDebug() << "ERROR: Unable to evaluate the correct encoding method, sending as SHA256.";
            break;
        }

        QCryptographicHash hashAlgorithm(algorithm);
        hashAlgorithm.addData(encodedPasswd.toUtf8(), encodedPasswd.length());
        encodedPasswd = hashAlgorithm.result().toHex();
    }

    m_remote->setCurrentOption(m_currentOption);

    QString postDataStr("login=" + m_ui->loginInput->text() + "&" + "passwd=" + encodedPasswd);

    // when logging in, it might take more time
    // once LDAP service can be quite slow
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    m_manager->setTransferTimeout(20000);
#endif
    QNetworkReply *reply = m_manager->post(request, postDataStr.toUtf8());
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    m_manager->setTransferTimeout(1000);
#endif

    reply->setProperty("type", "json");
    reply->setProperty("req", "connect");
}

void RemoteConfig::comboboxItemChanged(const QString &currentSelected)
{
    updateServiceInfo(currentSelected);
}

void RemoteConfig::onApplyConfig(QAbstractButton *btn)
{
    if (btn->text() == "Apply") {
        qDebug() << "APPLIED!";
        if (savePortMapping()) {
            close();
        }
    }
}

void RemoteConfig::onRejectConfig()
{
    qDebug() << "REJECTED!";
}

void RemoteConfig::onCopyToClipboard()
{
    QClipboard *clipboard = QGuiApplication::clipboard();

    clipboard->clear();
    clipboard->setText(m_remote->deviceAuth().m_token);
}

void RemoteConfig::on_disconnectBtn_clicked()
{
    m_remote->disconnect();

    QMessageBox messageBox;
    messageBox.information(nullptr, "Info", "Disconnected successfully!");
    close();
}

void RemoteConfig::on_leaveBtn_clicked()
{
    m_remote->disconnect();

    QMessageBox messageBox;
    messageBox.information(nullptr, "Info", "You have left the queue!");
    close();
}
