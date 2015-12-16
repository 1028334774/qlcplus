/*
  Q Light Controller Plus
  showmanager.h

  Copyright (c) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef SHOWMANAGER_H
#define SHOWMANAGER_H

#include <QObject>
#include <QQuickItem>

#include "previewcontext.h"

class Doc;
class Show;
class Track;
class Function;
class ShowFunction;

class ShowManager : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(int currentShowID READ currentShowID WRITE setCurrentShowID NOTIFY currentShowIDChanged)
    Q_PROPERTY(QString showName READ showName WRITE setShowName NOTIFY showNameChanged)
    Q_PROPERTY(float timeScale READ timeScale WRITE setTimeScale NOTIFY timeScaleChanged)
    Q_PROPERTY(int showDuration READ showDuration NOTIFY showDurationChanged)
    Q_PROPERTY(QQmlListProperty<Track> tracks READ tracks NOTIFY tracksChanged)

public:
    explicit ShowManager(QQuickView *view, Doc *doc, QObject *parent = 0);

    /** Return the ID of the Show Function being edited */
    int currentShowID() const;

    /** Set the ID of the Show Function to edit */
    void setCurrentShowID(int currentShowID);

    /** Return the name of the Show Function being edited */
    QString showName() const;

    /** Set the name of the Show Function to edit */
    void setShowName(QString showName);

    /** Return the current time scale of the Show Manager timeline */
    float timeScale() const;

    /** Set the time scale of the Show Manager timeline */
    void setTimeScale(float timeScale);

    Q_INVOKABLE void addItem(QQuickItem *parent, int trackIdx, int startTime, quint32 functionID);

    QQmlListProperty<Track> tracks();

    Q_INVOKABLE void resetView();

    Q_INVOKABLE void renderView(QQuickItem *parent);

    /** Return the current Show total duration in milliseconds */
    int showDuration() const;

signals:
    void currentShowIDChanged(int currentShowID);
    void showNameChanged(QString showName);
    void timeScaleChanged(float timeScale);
    void showDurationChanged(int showDuration);
    void tracksChanged();

private:
    /** A reference to the Show Function being edited */
    Show *m_currentShow;

    /** The current time scale of the Show Manager timeline */
    float m_timeScale;

    /** A list of references to the selected Show Tracks */
    QList <Track*> m_tracksList;

    /** Pre-cached QML component for quick item creation */
    QQmlComponent *siComponent;
};

#endif // SHOWMANAGER_H
