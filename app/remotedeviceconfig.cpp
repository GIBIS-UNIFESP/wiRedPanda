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

RemoteDeviceConfig::RemoteDeviceConfig( Editor *editor, QWidget *parent, GraphicElement *elm ) : QDialog( parent ), ui( new Ui::RemoteDeviceConfig ), editor( editor ), manager(new QNetworkAccessManager(this)) {
  ui->setupUi( this );

  setWindowTitle( "Remote Device" );
  setWindowFlags( Qt::Dialog );
  setModal( true );
  QSettings settings( QSettings::IniFormat, QSettings::UserScope,
                      QApplication::organizationName( ), QApplication::applicationName( ) );

  ui->stackedWidget->setCurrentIndex(0);

  QPixmap unavailable( ":/remote/unavailable.png" );
  ui->logo->setPixmap(unavailable);

  ui->progressBar->setMinimum(0);
  ui->progressBar->setMaximum(100);

  manager->setTransferTimeout(1500);

  connect(manager, &QNetworkAccessManager::finished,
          this, &RemoteDeviceConfig::connectionResponse);

  if (RemoteDevice* remoteDevice = dynamic_cast<RemoteDevice*>(elm)) {
    this->elm = remoteDevice;
  }

  if (this->elm->getAuthToken() == "") {
    setupAuthScreen();
  } else if (this->elm->isInQueue()) {
    setupQueueScreen();
  } else {
    setupConfigScreen();
  }
}

RemoteDeviceConfig::~RemoteDeviceConfig() {
    delete manager;
    delete ui;
}

void RemoteDeviceConfig::setupQueueScreen() {
    ui->stackedWidget->setCurrentIndex(2);

    QMovie *movie = new QMovie(":/remote/loading.gif");
    ui->waiting_list_logo->setMovie(movie);
    movie->start();

    onQueueTimeRefresh();

    connect(&this->timer, &QTimer::timeout, this, &RemoteDeviceConfig::onQueueTimeRefresh);
    this->timer.start(1000);
}

void RemoteDeviceConfig::setupAuthScreen() {
    ui->stackedWidget->setCurrentIndex(0);

    for (auto option : this->elm->getOptions()) {
      ui->serviceSelector->addItem(QString::fromUtf8(option.getName().c_str()));
    }

    QString currentSelected = ui->serviceSelector->currentText();

    connect(ui->serviceSelector, SIGNAL(currentTextChanged(QString)), this, SLOT(comboboxItemChanged(QString)));
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(onTryToConnect()));
    ui->connectButton->setDisabled(false);

    updateServiceInfo(currentSelected);
}

void RemoteDeviceConfig::onEditPortMapping(int row, int column) {
    QStringList items;

    if (column == 0) {
        const std::list<Pin>& availablePins = elm->getAvailablePins();
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
        QTableWidgetItem* item = ui->tableWidget->item(row, column);

        if (!item) {
            item = new QTableWidgetItem();
            ui->tableWidget->setItem(row, column, item);
        }

        item->setText(inputValue);
    }
}

void RemoteDeviceConfig::onAddPin() {
    ui->tableWidget->insertRow(ui->tableWidget->currentRow()+1);
}

void RemoteDeviceConfig::onRemovePin() {
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());
}

bool RemoteDeviceConfig::savePortMapping() {
    elm->resetPortMapping();

    int rowsAmount = ui->tableWidget->rowCount();

    for (int row = 0; row<rowsAmount; row++) {
        QTableWidgetItem* portWidget = ui->tableWidget->item(row, 0);

        if (!portWidget) {
            QMessageBox messageBox;
            messageBox.information(0,"Error","Found rows with empty port name");
            messageBox.setFixedSize(500,200);
            return false;
        }

        QString portName = portWidget->text();

        QTableWidgetItem* typeWidget = ui->tableWidget->item(row, 1);

        if (!typeWidget) {
            QMessageBox messageBox;
            messageBox.information(0,"Error","Found rows with invalid type");
            messageBox.setFixedSize(500,200);
            return false;
        }

        QString typeName = typeWidget->text();

        PIN_TYPE type = Pin::convertTypeString(typeName.toStdString());

        if (!elm->mapPin(portName.toStdString(), type)) {
            QMessageBox messageBox;
            messageBox.information(0,"Error","Unable to map pin " + portName + " as " + typeName);
            messageBox.setFixedSize(500,200);
            return false;
        }

    }

    elm->sendIOInfo();
    elm->setupPorts();

    return true;
}
void RemoteDeviceConfig::onQueueTimeRefresh() {
    uint64_t timestamp = QDateTime::currentSecsSinceEpoch();

    long estimatedTime = static_cast<long>(elm->getQueueEstimatedEpoch() - timestamp);

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

    ui->timeEstimated->display(hourStr + separator + minStr + separator + secsStr);

    ui->estimatedTimeProgressBar->setMinimum(0);
    ui->estimatedTimeProgressBar->setMaximum(100);

    int estimatedSeconds = static_cast<int>(elm->getQueueEstimatedEpoch() - elm->getWaitingSince());

    if (estimatedSeconds <= 0)
        estimatedSeconds = 1;

    int secondsPassed = static_cast<int>(timestamp - elm->getWaitingSince());

    if (secondsPassed <= 0)
        secondsPassed = 0;

    ui->estimatedTimeProgressBar->setValue(static_cast<int>(secondsPassed*100.0f/estimatedSeconds));

    QString queueInfoModel = "<html><head/><body><p><span style=\" font-size:12pt;\">You are currently on position </span><span style=\" font-size:12pt; font-weight:600;\">%1</span></p><p>Please be patient and you will receive at least %2 to use this device</p></body></html>";

    QString deviceAllowedTime = QDateTime::fromSecsSinceEpoch(static_cast<uint32_t>(elm->getDeviceAllowedTime()), Qt::UTC).toString("mm:ss");

    QString str = queueInfoModel.arg(elm->getQueuePos()).arg(deviceAllowedTime);
    ui->queueInfoLbl->setText(str);

    // finally in
    if (!elm->isInQueue()) {
        if (elm->getDeviceId() != 0)
            setupConfigScreen();
        else
            this->close();
    }
}

void RemoteDeviceConfig::onTimeRefresh() {
    static uint16_t last_latency;

    uint64_t timestamp = QDateTime::currentSecsSinceEpoch();

    {
        uint64_t allowUntil = elm->getAllowUntil();
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
        long avaliableAfterTime = elm->getAvailableAfterTime();

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

        ui->timeRemaining->display(minStr + separator + secsStr);

        timeRemaining += static_cast<long>(avaliableAfterTime);
        int percent = static_cast<int>(static_cast<float>(timeRemaining*100.0f)/static_cast<float>(elm->getTimeAllowed()));

        ui->progressBar->setValue(percent);

        QPalette progressBarPalette;

        if (allowedTimeExceeded) {
            progressBarPalette.setColor(QPalette::Highlight, Qt::yellow);
        } else {
            progressBarPalette.setColor(QPalette::Highlight, Qt::green);
        }

        ui->progressBar->setPalette(progressBarPalette);

        // Auto closes window when reaching zero seconds
        if (avaliableAfterTime <= 0)
            this->close();
    }

    QPalette sample_palette;
    sample_palette.setColor(QPalette::Window, Qt::white);
    if (elm->getLatency() <= 20) {
        sample_palette.setColor(QPalette::WindowText, Qt::darkGreen);
    } else if (elm->getLatency() > 20 && elm->getLatency() <= 50) {
        sample_palette.setColor(QPalette::WindowText, Qt::darkYellow);
    } else {
        sample_palette.setColor(QPalette::WindowText, Qt::darkRed);
    }

    static int warnedAlready = 0;

    if (warnedAlready < 2 && elm->getLatency() > 120 && last_latency > 120) {

        QMessageBox *msgBox = new QMessageBox( this );
        msgBox->setIcon( QMessageBox::Critical );
        msgBox->setText("Your connection latency is pretty bad, you will not be able to use the remote lab.");
        msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
        msgBox->show();
        msgBox->raise();

        warnedAlready = 2;
    }

    if (warnedAlready < 1 && elm->getLatency() > 80 && last_latency > 80) {

        QMessageBox *msgBox = new QMessageBox( this );
        msgBox->setIcon( QMessageBox::Warning );
        msgBox->setText("Looks like your connection is pretty unstable, you may not be able to use the remote lab.");
        msgBox->setAttribute(Qt::WA_DeleteOnClose); // delete pointer after close
        msgBox->show();
        msgBox->raise();

        warnedAlready = 1;
    }

    last_latency = elm->getLatency();

    ui->ping->setAutoFillBackground(true);
    ui->ping->setPalette(sample_palette);
    ui->ping->setText("Ping: " + QString::number(elm->getLatency()) + "ms");
}

void RemoteDeviceConfig::setupConfigScreen() {
    ui->stackedWidget->setCurrentIndex(1);
    QString url = QString::fromStdString(this->elm->getCurrentOption().getUrl());

    std::cout << this->elm->getAuthToken() << std::endl;
    std::cout << url.toStdString() << std::endl;

    // Connect buttons
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &RemoteDeviceConfig::onApplyConfig);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &RemoteDeviceConfig::onRejectConfig);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

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

    ui->pinOptions->setPopupMode(QToolButton::MenuButtonPopup);
    ui->pinOptions->setMenu(alignMenu);
    ui->pinOptions->setDefaultAction(addAction);

    ui->copyToClipboard->setDefaultAction(copyAction);

    connect(ui->tableWidget, &QTableWidget::cellDoubleClicked, this, &RemoteDeviceConfig::onEditPortMapping);

    ui->ping->setText("Ping: ...");
    elm->sendPing();

    onTimeRefresh();

    connect(&this->timer, &QTimer::timeout, this, &RemoteDeviceConfig::onTimeRefresh);
    this->timer.start(1000);

    ui->methodLabel->setText(QString::fromStdString(elm->getDeviceAuth().name));
    ui->methodTokenFrame->hide();

    if (elm->getDeviceMethod().compare("VirtualHere") == 0) {
        ui->content->setText(QString::fromStdString(elm->getDeviceAuth().token));
        ui->methodTokenFrame->show();
    }

    // pins

    ui->tableWidget->clearContents();

    const std::list<Pin>& mappedPins = elm->getMappedPins();
    std::list<Pin>::const_iterator it;
    int row = 0;
    for (it = mappedPins.begin(); it != mappedPins.end(); ++it) {
        ui->tableWidget->insertRow(row);
        QTableWidgetItem* portWidget = ui->tableWidget->item(row, 0);

        if (!portWidget) {
            portWidget = new QTableWidgetItem();
            ui->tableWidget->setItem(row, 0, portWidget);
        }

        portWidget->setText(QString::fromStdString(it->getName()));

        QTableWidgetItem* typeWidget = ui->tableWidget->item(row, 1);

        if (!typeWidget) {
            typeWidget = new QTableWidgetItem();
            ui->tableWidget->setItem(row, 1, typeWidget);
        }

        QString type = "";
        switch(it->getType()) {
            case 1:
                type = "INPUT";
                break;
            case 2:
                type = "OUTPUT";
                break;
            default:
                type = "ERROR";
                break;
        }

        typeWidget->setText(type);

        row++;
    }

    // retrieve service image
    {
        QNetworkRequest request;
        request.setUrl(QUrl(url+"logo"));
        request.setRawHeader("User-Agent", "WiredPanda 1.0");

        QNetworkReply* reply = manager->get(request);
        reply->setProperty("type", "image");
        reply->setProperty("req", "setInfoLogo");
    }

    // retrieve method image
    {
        QNetworkRequest request;
        request.setUrl(QUrl(url+"method"));
        request.setRawHeader("User-Agent", "WiredPanda 1.0");
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        QString postDataStr("token=" + QString::fromStdString(elm->getAuthToken()) + "&" + "deviceId=" + QString::number(elm->getDeviceId()));

        QNetworkReply* reply = manager->post(request, postDataStr.toUtf8());
        reply->setProperty("type", "image");
        reply->setProperty("req", "setMethod");
    }

    // TODO: Remove QProcess* memory should be freed
    if (elm->getDeviceMethod() == "VirtualHere") {
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
        std::cout << "JSON " << static_cast<QString>(repliedData).toStdString( ) << std::endl;

        QJsonObject json = loadDoc.object();
        QVariantMap json_map = json.toVariantMap();

        if (reply->property("req") == "updateService") {
            QPixmap online( ":/remote/online.png" );
            QPixmap offline( ":/remote/offline.png" );

            QString versionStr = json["version"].toString();
            currentOption.setVersion(versionStr.toStdString());

            if (json["status"].toString() == "Online") {
                ui->status->setPixmap(online.scaled(ui->status->width(), ui->status->height(), Qt::KeepAspectRatio));
            } else {
                ui->status->setPixmap(offline.scaled(ui->status->width(), ui->status->height(), Qt::KeepAspectRatio));
            }

            ui->statusLabel->setText(json["status"].toString() != "" ? json["status"].toString() : "Offline");
            ui->uptimeLabel->setText("Uptime: " + json["uptime"].toString());

            ui->deviceSelector->clear();
            ui->methodSelector->clear();

            int totalAmount = 0;
            int avaliableAmount = 0;

            {
                QVariantMap devicesAmount = json_map["devices"].toMap();
                QStringList key_list = devicesAmount.keys();
                for(int i=0; i < key_list.count(); ++i) {
                    QString key = key_list.at(i);
                    int deviceId = devicesAmount[key].toInt();
                    ui->deviceSelector->addItem(QString ("%1").arg(key), QVariant::fromValue(deviceId));
                }
            }

            {
                QVariantMap devicesAmount = json_map["devicesAmount"].toMap();
                QStringList key_list = devicesAmount.keys();
                for(int i=0; i < key_list.count(); ++i) {
                    QString key = key_list.at(i);
                    int amount = devicesAmount[key].toInt();
                    totalAmount += amount;
                }
            }

            {
                QVariantMap devicesAvailable = json_map["devicesAvailable"].toMap();
                QStringList key_list = devicesAvailable.keys();
                for(int i=0; i < key_list.count(); ++i) {
                    QString key = key_list.at(i);
                    int amount = devicesAvailable[key].toInt();
                    avaliableAmount += amount;
                }
            }

            {
                QVariantMap methods = json_map["methods"].toMap();
                QStringList key_list = methods.keys();
                for(int i=0; i < key_list.count(); ++i) {
                    QString key = key_list.at(i);
                    int methodId = methods[key].toInt();
                    ui->methodSelector->addItem(QString ("%1").arg(key), QVariant::fromValue(methodId));
                }
            }

            ui->lcdTotal->display(totalAmount);
            ui->lcdAmount->display(avaliableAmount);
        } else if (reply->property("req") == "connect") {
            ui->connectButton->setDisabled(false);

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

            if (info.error() != QHostInfo::NoError ){
                std::cout << info.errorString().toStdString() << std::endl;
                QMessageBox messageBox;
                messageBox.critical(0,"Error","Unable to resolve hostname !");
                messageBox.setFixedSize(500,200);
                this->close();
                return;
            }

            host = info.addresses().first().toString();

            uint8_t deviceTypeId = ui->deviceSelector->currentData().toUInt();
            uint8_t methodId = ui->methodSelector->currentData().toUInt();

            if (!elm->connectTo(host.toStdString(), json["port"].toInt(), json["token"].toString().toStdString(), deviceTypeId, methodId)) {
                QMessageBox messageBox;
                messageBox.critical(0,"Error","Connection failure !");
                messageBox.setFixedSize(500,200);
                this->close();
                return;
            }

            this->close();
        }
    }

    if(reply->property("type") == "image") {
        QString req = reply->property("req").toString();
        std::cout << "IMAGE req=" << req.toStdString() << std::endl;

        if (req == "setLogo") {
            QPixmap pic;
            pic.loadFromData(repliedData);

            if (!pic.isNull()) {
                ui->logo->setPixmap(pic.scaled(ui->logo->width(), ui->logo->height(), Qt::KeepAspectRatio));
            } else {
                QPixmap unavailable( ":/remote/unavailable.png" );
                ui->logo->setPixmap(unavailable);
            }
        } else if (req == "setInfoLogo") {
            QPixmap pic;
            pic.loadFromData(repliedData);

            if (!pic.isNull()) {
                ui->infoLogo->setPixmap(pic.scaled(ui->infoLogo->width(), ui->infoLogo->height(), Qt::KeepAspectRatio));
            } else {
                QPixmap unavailable( ":/remote/unavailable.png" );
                ui->infoLogo->setPixmap(unavailable);
            }
        } else if (req == "setMethod") {
            QPixmap pic;
            pic.loadFromData(repliedData);

            if (!pic.isNull()) {
                ui->methodImg->setPixmap(pic.scaled(ui->methodImg->width(), ui->methodImg->height(), Qt::KeepAspectRatio));
            } else {
                QPixmap unavailable( ":/remote/unavailable.png" );
                ui->methodImg->setPixmap(unavailable);
            }
        }
    }
}

void RemoteDeviceConfig::updateServiceInfo(QString str) {
    auto& options = this->elm->getOptions();
    auto it = options.begin();

    while(it != options.end()) {
        if (it->getName() != str.toStdString())
            it++;
        else
            break;
    }

    if (it != options.end()) {
        currentOption = (*it);
        QString url = QString::fromStdString(it->getUrl());

        // retrieve service information
        {
            QNetworkRequest request;
            request.setUrl(QUrl(url));
            request.setRawHeader("User-Agent", "WiredPanda 1.0");

            QNetworkReply* reply = manager->get(request);
            reply->setProperty("type", "json");
            reply->setProperty("req", "updateService");
        }

        // retrieve service image
        {
            QNetworkRequest request;
            request.setUrl(QUrl(url+"logo"));
            request.setRawHeader("User-Agent", "WiredPanda 1.0");

            QNetworkReply* reply = manager->get(request);
            reply->setProperty("type", "image");
            reply->setProperty("req", "setLogo");
        }

        // change to loading
        QPixmap unavailable( ":/remote/unavailable.png" );
        QPixmap searching( ":/remote/searching.png" );

        ui->logo->setPixmap(unavailable);
        ui->status->setPixmap(searching.scaled(ui->status->width(), ui->status->height(), Qt::KeepAspectRatio));
        ui->deviceSelector->clear();
        ui->statusLabel->setText("...");
        ui->lcdTotal->display(0);
        ui->lcdAmount->display(0);
    } else {
        QMessageBox messageBox;
        messageBox.critical(0,"Error","Any available services could be found");
        messageBox.setFixedSize(500,200);
        this->close();
    }
}

void RemoteDeviceConfig::onTryToConnect() {
    // verify if the version is compatible before sending connection request

    ui->connectButton->setDisabled(true);

    QString versionStr = QString::fromStdString(currentOption.getVersion());
    // gets first dot position
    int firstDotIndex = versionStr.indexOf(".");
    QStringRef majorVersionStr(&versionStr, 1, firstDotIndex-1);
    int majorVersion = majorVersionStr.toInt();

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
    request.setUrl(QUrl(QString::fromStdString(currentOption.getUrl())+"login"));
    request.setRawHeader("User-Agent", "WiredPanda 1.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QString encodedPasswd = ui->passwdInput->text();
    AUTH_METHOD currentAuthMethod = currentOption.getAuthMethod();
    if (currentAuthMethod != AUTH_METHOD::PLAIN) {
        QCryptographicHash::Algorithm algorithm;
        switch(currentAuthMethod) {
            case AUTH_METHOD::MD5:
                algorithm = QCryptographicHash::Algorithm::Md5;
                break;
            case AUTH_METHOD::SHA1:
                algorithm = QCryptographicHash::Algorithm::Sha1;
                break;
            case AUTH_METHOD::SHA256:
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

    elm->setCurrentOption(currentOption);

    QString postDataStr("login=" + ui->loginInput->text() + "&" + "passwd=" + encodedPasswd);

    // when logging in, it might take more time
    // once LDAP service can be quite slow
    manager->setTransferTimeout(20000);
    QNetworkReply* reply = manager->post(request, postDataStr.toUtf8());
    manager->setTransferTimeout(1000);

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
            this->close();
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
    clipboard->setText(QString::fromStdString(elm->getDeviceAuth().token));
}

void RemoteDeviceConfig::on_disconnectBtn_clicked()
{
    elm->disconnect();

    QMessageBox messageBox;
    messageBox.information(0,"Info","Disconnected successfully!");
    this->close();
}

void RemoteDeviceConfig::on_leaveBtn_clicked()
{
    elm->disconnect();

    QMessageBox messageBox;
    messageBox.information(0,"Info","You have left the queue!");
    this->close();
}
