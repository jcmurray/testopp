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

    // ============== Message to be sent to page

    QObject::connect(       this, SIGNAL(message(QVariant)),
                       _mainPage,   SLOT(message(QVariant)));

    // ============== Hook up buttons

    QObject::connect(  _mainPage, SIGNAL(toggleBluetooth(QVariant)),
                            this, SLOT(onToggleBluetooth(QVariant)));

    QObject::connect(  _mainPage, SIGNAL(task2Signal()),
                            this, SLOT(onTask2Signal()));

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

void ApplicationUI::onToggleBluetooth(const QVariant &on)
{
    if (on.toBool()) {
        initBluetooth();
    } else {
        deinitBluetooth();
    }
}

void ApplicationUI::onTask2Signal()
{}

void ApplicationUI::btInitialised(bool state)
{
    _bt_initialised = state;

    emit bluetoothInitialisedState(state);

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

    // TODO:
}

void ApplicationUI::handleOppUpdateCallback(const char *bdaddr, uint32_t sent, uint32_t total)
{
    Q_UNUSED(bdaddr)
    Q_UNUSED(sent)
    Q_UNUSED(total)

    qDebug() << "XXXX handleOppUpdateCallback - bdaddr=" << bdaddr <<
                                               ", sent=" << sent <<
                                              ", total=" << total << endl;

    // TODO:
}

void ApplicationUI::handleOppCompleteCallback(const char *bdaddr)
{
    Q_UNUSED(bdaddr)

    qDebug() << "XXXX handleOppCompleteCallback - bdaddr=" << bdaddr << endl;

    // TODO:
}

void ApplicationUI::handleOppCancelledCallback(const char *bdaddr, bt_opp_reason_t reason)
{
    Q_UNUSED(bdaddr)
    Q_UNUSED(reason)

    qDebug() << "XXXX handleOppCancelledCallback - bdaddr=" << bdaddr <<
                                                ", reason=" << reason << endl;
    // TODO:
}

const char *ApplicationUI::btEventName(const int id)
{
    const event_names_t descriptions[] = {
            { BT_EVT_ACCESS_CHANGED, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_RADIO_SHUTDOWN, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_RADIO_INIT, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_CONFIRM_NUMERIC_REQUEST, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_PAIRING_COMPLETE, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_DEVICE_ADDED, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_DEVICE_DELETED, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_SERVICE_CONNECTED, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_SERVICE_DISCONNECTED, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_LE_DEVICE_CONNECTED, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_LE_DEVICE_DISCONNECTED, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_LE_NAME_UPDATED, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_LE_GATT_SERVICES_UPDATED, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_FAULT, "BT_EVT_ACCESS_CHANGED" },
            { BT_EVT_UNDEFINED_EVENT, "BT_EVT_ACCESS_CHANGED" },
            { -1, NULL }
    };
    int i;
    for (i = 0; descriptions[i].id != -1; i++) {
        if (descriptions[i].id == id) {
            return descriptions[i].name;
        }
    }
    return "Unknown Event";
}
