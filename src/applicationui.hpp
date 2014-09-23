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

#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_

#include <errno.h>

#include <QObject>
#include <QFile>
#include <QFileSystemWatcher>

#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/Application>
#include <bb/cascades/LocaleHandler>
#include <btapi/btdevice.h>
#include <btapi/btopp.h>

typedef struct _event_names_t {
    int id;
    const char *name;
} event_names_t;

typedef struct _opp_reason_names_t {
    bt_opp_reason_t reason;
    const char *name;
} opp_reason_names_t;

void btEvent(const int event, const char *bt_addr, const char *event_data);
void oppUpdateCallback(const char *bdaddr, uint32_t sent, uint32_t total);
void oppCompleteCallback(const char *bdaddr);
void oppCancelledCallback(const char *bdaddr, bt_opp_reason_t reason);

using namespace bb::cascades;

namespace bb
{
    namespace cascades
    {
        class LocaleHandler;
    }
}

class QTranslator;

class ApplicationUI
    : public QObject
{
    Q_OBJECT

public:
    ApplicationUI(bb::cascades::Application *app);
    virtual ~ApplicationUI() { }

    void handleBtEvent(const int event, const char *bt_addr, const char *event_data);
    void handleOppUpdateCallback(const char *bdaddr, uint32_t sent, uint32_t total);
    void handleOppCompleteCallback(const char *bdaddr);
    void handleOppCancelledCallback(const char *bdaddr, bt_opp_reason_t reason);

private slots:
    void onSystemLanguageChanged();
    void onToggleBluetooth(bool on);
    void onSendFile();
    void onDirectoryChanged(const QString &path);

signals:
    void message(const QVariant &text);
    void bluetoothInitialisedState(const QVariant &state);

private:
    void initBluetooth();
    void deinitBluetooth();
    void btInitialised(bool state);
    bool btIsInitialised();
    const char *btEventName(const int id);
    const char *oppReason(const bt_opp_reason_t reason);

    QTranslator *_translator;
    LocaleHandler *_localeHandler;
    QmlDocument *_qml;
    AbstractPane *_root;
    QObject *_mainPage;
    bool _bt_initialised;
    bt_opp_callbacks_t _oppCallbacks;
    QString _targetBtAddress;
    QString _fileToSend;
    QString _pathToFilesDirectory;
    QFileSystemWatcher *_downloadFolderWatcher;
};

#endif /* ApplicationUI_HPP_ */
