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

ApplicationUI::ApplicationUI(Application *app)
    : QObject(app)
    , _pathToFilesDirectory(QDir::currentPath().append("/app/public/files"))
    , _downloadFolderWatcher(new QFileSystemWatcher(this))
{
    s_btApp = this;
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

    // ============== Connection state to page

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

        delete unzip;

        if (receivedFile.remove()) {
            emit message(QString("Processed file %1").arg(fileName));
        } else {
            emit message(QString("Error removing file %1").arg(fileName));
        }
    }
}
