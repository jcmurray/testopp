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

ApplicationUI::ApplicationUI(Application *app)
    : QObject(app)
{
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

    QObject::connect(  _mainPage, SIGNAL(task1Signal()),
                            this, SLOT(onTask1Signal()));

    QObject::connect(  _mainPage, SIGNAL(task2Signal()),
                            this, SLOT(onTask2Signal()));

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

void ApplicationUI::onTask1Signal()
{}

void ApplicationUI::onTask2Signal()
{}
