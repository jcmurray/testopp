/*
 * Copyright (c) 2011-2014 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "applicationui.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>

using namespace bb::cascades;

static ApplicationUI *s_btApp = NULL;

void btEvent(const int event, const char *bt_addr, const char *event_data)
{
    if (s_btApp) s_btApp->handleBtEvent(event, bt_addr, event_data);
}

void oppUpdateCallback(const char *bdaddr, uint32_t sent, uint32_t total)
{
    if (s_btApp) s_btApp->handleOppUpdateCallback(bdaddr, sent, total);
}

void oppCompleteCallback(const char *bdaddr)
{
    if (s_btApp) s_btApp->handleOppCompleteCallback(bdaddr);
}

void oppCancelledCallback(const char *bdaddr, bt_opp_reason_t reason)
{
    if (s_btApp) s_btApp->handleOppCancelledCallback(bdaddr, reason);
}

ApplicationUI::ApplicationUI(Application *app)
    : QObject(app)
    , _bt_initialised(false)
    , _targetBtAddress("34:BB:1F:3E:77:BC") // Q10
//  , _targetBtAddress("48:9D:24:AE:2E:59") // PPT
    , _fileToSend("test.zip")
    , _pathToFilesDirectory(QDir::currentPath().append("/app/public/files"))
    , _downloadFolderWatcher(new QFileSystemWatcher(this))
{
    s_btApp = this;

    _oppCallbacks.update = oppUpdateCallback;
    _oppCallbacks.complete = oppCompleteCallback;
    _oppCallbacks.cancelled = oppCancelledCallback;

    _translator = new QTranslator(this);
    _localeHandler = new LocaleHandler(this);

    bool res = QObject::connect(_localeHandler, SIGNAL(systemLanguageChanged()),
                                          this, SLOT(onSystemLanguageChanged()));
    Q_ASSERT(res);
    Q_UNUSED(res);

    onSystemLanguageChanged();

    _qml = QmlDocument::create("asset:///main.qml").parent(this);
    _root = _qml->createRootObject<AbstractPane>();
    _mainPage = _root->findChild<QObject*>("mainPage");

    _downloadFolderWatcher->addPath(QDir::currentPath().append("/shared/downloads"));

    // ============== Watch the downloads directory

    QObject::connect(_downloadFolderWatcher, SIGNAL(directoryChanged(const QString &)),
                                       this, SLOT(onDirectoryChanged(const QString &)));

    // ============== Message to be sent to page

    QObject::connect(       this, SIGNAL(message(QVariant)),
                       _mainPage,   SLOT(message(QVariant)));

    // ============== Hook up buttons

    QObject::connect(  _mainPage, SIGNAL(toggleBluetooth(bool)),
                            this, SLOT(onToggleBluetooth(bool)));

    QObject::connect(  _mainPage, SIGNAL(sendFile()),
                            this, SLOT(onSendFile()));

    // ============== Connection state to page

    QObject::connect(       this, SIGNAL(bluetoothInitialisedState(QVariant)),
                       _mainPage, SLOT(onBluetoothInitialisedState(QVariant)));


    Application::instance()->setScene(_root);
}

void ApplicationUI::onSystemLanguageChanged()
{
    QCoreApplication::instance()->removeTranslator(_translator);
    QString locale_string = QLocale().name();
    QString file_name = QString("testopp_%1").arg(locale_string);
    if (_translator->load(file_name, "app/native/qm")) {
        QCoreApplication::instance()->installTranslator(_translator);
    }
}

void ApplicationUI::onDirectoryChanged(const QString &path)
{
    qDebug() << "XXXX Directory changed" << path << endl;

    QDir downloads(path);
    QStringList nameFilters;
    nameFilters << "*.zip";
    QDir::Filters fileFilters = (QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot | QDir::Readable);
    QDir::SortFlags sortOrder = (QDir::Time | QDir::Reversed);

    QFileInfoList candidateFiles = downloads.entryInfoList(nameFilters, fileFilters, sortOrder);

    for (int i = 0; i < candidateFiles.count(); i++) {

        QString fileName = candidateFiles[i].fileName();
        QString filePath = candidateFiles[i].filePath();
        QFile receivedFile(filePath);

        emit message(QString("Received file %1").arg(fileName));

        QString program = "unzip";
        QStringList arguments;

        arguments << "-l";
        arguments << filePath;

        QProcess *unzip = new QProcess(this);

        unzip->setReadChannel(QProcess::StandardOutput);
        unzip->start(program, arguments);

        if (unzip->waitForStarted()) {          // TODO: not a good idea on a GUI thread but this is a test for PoC
            if (unzip->waitForFinished()) {     // TODO: not a good idea on a GUI thread but this is a test for PoC
                QByteArray result = unzip->readAllStandardOutput();
                QString unzipOutput(result);
                qDebug() << "XXXX Unzip Command completed\n" << unzipOutput << endl;
                emit message(unzipOutput);
            } else {
                qDebug() << "XXXX Unzip Command did not finish" << endl;
            }
        } else {
            qDebug() << "XXXX Unzip Command did not start" << endl;
        }

        if (receivedFile.remove()) {
            emit message(QString("Processed file %1").arg(fileName));
        } else {
            emit message(QString("Error removing file %1").arg(fileName));
        }
    }
}

void ApplicationUI::onToggleBluetooth(bool on)
{
    if (on) {
        initBluetooth();
    } else {
        deinitBluetooth();
    }
}

void ApplicationUI::onSendFile()
{
    if (btIsInitialised()) {
        QString testFilePath(_pathToFilesDirectory);

        //testFilePath.append("/").append(_fileToSend);
        //testFilePath = QString("/tmp/test.zip");
        testFilePath = QString("/accounts/1000/sharewith/bluetooth/test.zip");

        qDebug() << "XXXX Files directory    Path" << _pathToFilesDirectory << endl;
        qDebug() << "XXXX Test File    Path" << testFilePath << endl;

        if (bt_opp_send_file(_targetBtAddress.toLatin1().constData(), testFilePath.toLatin1().constData()) == EOK) {
            qDebug() << "XXXX bt_opp_send_file() OK" << endl;
            emit message(QString("Sending File %1 to %2").arg(_fileToSend).arg(_targetBtAddress));
        } else {
            qDebug() << "XXXX bt_opp_send_file() FAIL " << strerror(errno) << endl;
            emit message(QString("bt_opp_send_file() FAIL %1").arg(strerror(errno)));
        }

    } else {
        emit message("Bluetooth not initialised yet");
    }
}

void ApplicationUI::btInitialised(bool state)
{
    _bt_initialised = state;

    emit bluetoothInitialisedState(state);

}

bool ApplicationUI::btIsInitialised()
{
    return _bt_initialised;
}

void ApplicationUI::initBluetooth()
{
    if (bt_device_init(btEvent) == EOK) {
        qDebug() << "XXXX bt_device_init() OK" << endl;
    } else {
        qDebug() << "XXXX bt_device_init() FAIL " << strerror(errno) << endl;
        emit message(QString("bt_device_init() FAIL %1").arg(strerror(errno)));
        return;
    }

    if (bt_opp_init(&_oppCallbacks) == EOK) {
        qDebug() << "XXXX bt_opp_init() OK" << endl;
    } else {
        qDebug() << "XXXX bt_opp_init() FAIL " << strerror(errno) << endl;
        emit message(QString("bt_opp_init() FAIL %1").arg(strerror(errno)));
        return;
    }

    btInitialised(true);

}

void ApplicationUI::deinitBluetooth()
{
    qDebug() << "XXXX Calling bt_opp_deinit()" << endl;
    bt_opp_deinit();
    qDebug() << "XXXX Calling bt_device_deinit()" << endl;
    bt_device_deinit();

    btInitialised(false);
}

void ApplicationUI::handleBtEvent(const int event, const char *bt_addr, const char *event_data)
{
    Q_UNUSED(event)
    Q_UNUSED(bt_addr)
    Q_UNUSED(event_data)

    qDebug() << "XXXX handleBtEvent - event=" << btEventName(event) <<
                                 ", bt_addr=" << bt_addr <<
                              ", event_data=" << event_data << endl;
}

void ApplicationUI::handleOppUpdateCallback(const char *bdaddr, uint32_t sent, uint32_t total)
{
    Q_UNUSED(bdaddr)
    Q_UNUSED(sent)
    Q_UNUSED(total)

    qDebug() << "XXXX handleOppUpdateCallback - bdaddr=" << bdaddr <<
                                               ", sent=" << sent <<
                                              ", total=" << total << endl;

    emit message(QString("Transfer Update - sent %1 of %2").arg(sent).arg(total));
}

void ApplicationUI::handleOppCompleteCallback(const char *bdaddr)
{
    Q_UNUSED(bdaddr)

    qDebug() << "XXXX handleOppCompleteCallback - bdaddr=" << bdaddr << endl;

    emit message("Transfer Complete");
}

void ApplicationUI::handleOppCancelledCallback(const char *bdaddr, bt_opp_reason_t reason)
{
    Q_UNUSED(bdaddr)
    Q_UNUSED(reason)

    qDebug() << "XXXX handleOppCancelledCallback - bdaddr=" << bdaddr <<
                                                ", reason=" << oppReason(reason) << endl;
    emit message(QString("Transfer Complete - %1").arg(oppReason(reason)));
}

const char *ApplicationUI::btEventName(const int id)
{
    const event_names_t descriptions[] = {
            { BT_EVT_ACCESS_CHANGED, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_RADIO_SHUTDOWN, "BT_EVT_RADIO_SHUTDOWN" },
            { BT_EVT_RADIO_INIT, "BT_EVT_RADIO_INIT" },
            { BT_EVT_CONFIRM_NUMERIC_REQUEST, "BT_EVT_CONFIRM_NUMERIC_REQUEST" },
            { BT_EVT_PAIRING_COMPLETE, "BT_EVT_PAIRING_COMPLETE" },
            { BT_EVT_DEVICE_ADDED, "BT_EVT_DEVICE_ADDED" },
            { BT_EVT_DEVICE_DELETED, "BT_EVT_DEVICE_DELETED" },
            { BT_EVT_SERVICE_CONNECTED, "BT_EVT_SERVICE_CONNECTED" },
            { BT_EVT_SERVICE_DISCONNECTED, "BT_EVT_SERVICE_DISCONNECTED" },
            { BT_EVT_FAULT, "BT_EVT_FAULT" },
            { BT_EVT_DEVICE_CONNECTED, "BT_EVT_DEVICE_CONNECTED" },
            { BT_EVT_DEVICE_DISCONNECTED, "BT_EVT_DEVICE_DISCONNECTED" },
            { BT_EVT_NAME_UPDATED, "BT_EVT_NAME_UPDATED" },
            { BT_EVT_LE_DEVICE_CONNECTED, "BT_EVT_LE_DEVICE_CONNECTED" },
            { BT_EVT_LE_DEVICE_DISCONNECTED, "BT_EVT_LE_DEVICE_DISCONNECTED" },
            { BT_EVT_LE_NAME_UPDATED, "BT_EVT_LE_NAME_UPDATED" },
            { BT_EVT_SERVICES_UPDATED, "BT_EVT_SERVICES_UPDATED" },
            { BT_EVT_GATT_SERVICES_UPDATED, "BT_EVT_GATT_SERVICES_UPDATED" },
            { BT_EVT_LE_GATT_SERVICES_UPDATED, "BT_EVT_LE_GATT_SERVICES_UPDATED" },
            { BT_EVT_PAIRING_DELETED, "BT_EVT_PAIRING_DELETED" },
            { BT_EVT_PAIRING_STARTED, "BT_EVT_PAIRING_STARTED" },
            { BT_EVT_PAIRING_FAILED, "BT_EVT_PAIRING_FAILED" },
            { BT_EVT_UNDEFINED_EVENT, "BT_EVT_UNDEFINED_EVENT" }
    };

    for (int i = 0; i < (int)(sizeof(descriptions)/sizeof(event_names_t)); i++) {
        if (descriptions[i].id == id) {
            return descriptions[i].name;
        }
    }
    return "Unknown Event";
}

const char *ApplicationUI::oppReason(const bt_opp_reason_t reason)
{
    const opp_reason_names_t descriptions[] = {
            { BT_OPP_DEVICE_NOT_AVAILABLE, "BT_OPP_DEVICE_NOT_AVAILABLE" },
            { BT_OPP_TRANSFER_CANCELLED, "BT_OPP_TRANSFER_CANCELLED" },
            { BT_OPP_FAILED_TO_FIND_SERVICE, "BT_OPP_FAILED_TO_FIND_SERVICE" },
            { BT_OPP_TRANSFER_INTERRUPTED, "BT_OPP_TRANSFER_INTERRUPTED" }
    };

    for (int i = 0; i < (int)(sizeof(descriptions)/sizeof(opp_reason_names_t)); i++) {
        if (descriptions[i].reason == reason) {
            return descriptions[i].name;
        }
    }
    return "Unknown Reason";
}
