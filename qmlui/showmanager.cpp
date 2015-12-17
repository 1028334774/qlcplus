/*
  Q Light Controller Plus
  showmanager.cpp

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

#include "showmanager.h"
#include "track.h"
#include "show.h"
#include "doc.h"

ShowManager::ShowManager(QQuickView *view, Doc *doc, QObject *parent)
    : PreviewContext(view, doc, parent)
    , m_currentShow(NULL)
    , m_timeScale(5.0)
    , m_currentTime(0)
{
    qmlRegisterType<Track>("com.qlcplus.classes", 1, 0, "Track");
    qmlRegisterType<ShowFunction>("com.qlcplus.classes", 1, 0, "ShowFunction");

    siComponent = new QQmlComponent(m_view->engine(), QUrl("qrc:/ShowItem.qml"));
    if (siComponent->isError())
        qDebug() << siComponent->errors();
}

int ShowManager::currentShowID() const
{
    if (m_currentShow == NULL)
        return Function::invalidId();
    return m_currentShow->id();
}

void ShowManager::setCurrentShowID(int currentShowID)
{
    if (m_currentShow != NULL)
    {
        if (m_currentShow->id() == (quint32)currentShowID)
            return;
        disconnect(m_currentShow, SIGNAL(timeChanged(quint32)), this, SLOT(slotTimeChanged(quint32)));
    }

    m_currentShow = qobject_cast<Show*>(m_doc->function(currentShowID));
    emit currentShowIDChanged(currentShowID);
    if (m_currentShow != NULL)
    {
        connect(m_currentShow, SIGNAL(timeChanged(quint32)), this, SLOT(slotTimeChanged(quint32)));
        emit showDurationChanged(m_currentShow->totalDuration());
        emit showNameChanged(m_currentShow->name());
    }
    else
    {
        emit showDurationChanged(0);
        emit showNameChanged("");
    }
    emit tracksChanged();
}

float ShowManager::timeScale() const
{
    return m_timeScale;
}

void ShowManager::setTimeScale(float timeScale)
{
    if (m_timeScale == timeScale)
        return;

    m_timeScale = timeScale;
    emit timeScaleChanged(timeScale);
}

void ShowManager::addItem(QQuickItem *parent, int trackIdx, int startTime, quint32 functionID)
{
    // if no show is selected, then create a new one
    if (m_currentShow == NULL)
    {
        QString defaultName = QString("%1 %2").arg(tr("New Show")).arg(m_doc->nextFunctionID());
        m_currentShow = new Show(m_doc);
        m_currentShow->setName(defaultName);
        Function *f = qobject_cast<Function*>(m_currentShow);
        if (m_doc->addFunction(f) == false)
        {
            qDebug() << "Error in creating a new Show !";
            m_currentShow = NULL;
            return;
        }
        emit currentShowIDChanged(m_currentShow->id());
    }

    Track *selectedTrack = NULL;

    // if no Track index is provided, then add a new one
    if (trackIdx == -1)
    {
        selectedTrack = new Track();
        selectedTrack->setName(tr("Track %1").arg(m_currentShow->tracks().count() + 1));
        m_currentShow->addTrack(selectedTrack);
        trackIdx = m_currentShow->tracks().count() - 1;
        emit tracksChanged();
    }
    else
    {
        if (trackIdx >= m_currentShow->tracks().count())
        {
            qDebug() << "Track index out of bounds !" << trackIdx;
            return;
        }
        selectedTrack = m_currentShow->tracks().at(trackIdx);
    }

    // and now create the actual ShowFunction and the QML item
    Function *func = m_doc->function(functionID);
    if (func == NULL)
        return;

    ShowFunction *showFunc = selectedTrack->createShowFunction(functionID);
    showFunc->setStartTime(startTime);
    showFunc->setDuration(func->totalDuration());
    showFunc->setColor(ShowFunction::defaultColor(func->type()));

    QQuickItem *newItem = qobject_cast<QQuickItem*>(siComponent->create());

    newItem->setParentItem(parent);
    newItem->setProperty("trackIndex", trackIdx);
    newItem->setProperty("sfRef", QVariant::fromValue(showFunc));
    newItem->setProperty("funcRef", QVariant::fromValue(func));

    quint32 itemIndex = m_itemsMap.isEmpty() ? 0 : m_itemsMap.lastKey() + 1;
    quint32 itemID = trackIdx << 16 | itemIndex;
    m_itemsMap[itemID] = newItem;
}

bool ShowManager::checkAndMoveItem(ShowFunction *sf, int originalTrackIdx, int newTrackIdx, int newStartTime)
{
    if (m_currentShow == NULL || sf == NULL)
        return false;

    //qDebug() << Q_FUNC_INFO << "origIdx:" << originalTrackIdx << "newIdx:" << newTrackIdx << "time:" << newStartTime;

    Track *dstTrack = NULL;

    if (newTrackIdx >= m_currentShow->tracks().count())
    {
        // create a new track here
        dstTrack = new Track();
        dstTrack->setName(tr("Track %1").arg(m_currentShow->tracks().count() + 1));
        m_currentShow->addTrack(dstTrack);
        emit tracksChanged();
    }
    else
    {
        dstTrack = m_currentShow->tracks().at(newTrackIdx);

        bool overlapping = checkOverlapping(dstTrack, sf, newStartTime, sf->duration());
        if (overlapping == true)
            return false;
    }

    sf->setStartTime(newStartTime);

    // check if we need to move the ShowFunction to a different Track
    if (newTrackIdx != originalTrackIdx)
    {
        Track *srcTrack = m_currentShow->tracks().at(originalTrackIdx);
        srcTrack->showFunctions().takeAt(srcTrack->showFunctions().indexOf(sf));
        dstTrack->addShowFunction(sf);
    }

    return true;
}


QQmlListProperty<Track> ShowManager::tracks()
{
    m_tracksList.clear();
    if (m_currentShow)
        m_tracksList = m_currentShow->tracks();

    return QQmlListProperty<Track>(this, m_tracksList);
}

void ShowManager::resetView()
{
    QMapIterator<quint32, QQuickItem*> it(m_itemsMap);
    while(it.hasNext())
    {
        it.next();
        delete it.value();
    }
    m_itemsMap.clear();
}

void ShowManager::renderView(QQuickItem *parent)
{
    resetView();

    if (m_currentShow == NULL)
        return;

    int trkIdx = 0;

    foreach(Track *track, m_currentShow->tracks())
    {
        int itemIndex = 0;

        foreach(ShowFunction *sf, track->showFunctions())
        {
            Function *func = m_doc->function(sf->functionID());
            if (func == NULL)
                continue;

            QQuickItem *newItem = qobject_cast<QQuickItem*>(siComponent->create());

            newItem->setParentItem(parent);
            newItem->setProperty("trackIndex", trkIdx);
            newItem->setProperty("sfRef", QVariant::fromValue(sf));
            newItem->setProperty("funcRef", QVariant::fromValue(func));

            quint32 itemID = trkIdx << 16 | itemIndex;
            m_itemsMap[itemID] = newItem;
            itemIndex++;
        }

        trkIdx++;
    }
}

void ShowManager::enableFlicking(bool enable)
{
    QQuickItem *flickable = qobject_cast<QQuickItem*>(m_view->rootObject()->findChild<QObject *>("showItemsArea"));
    flickable->setProperty("interactive", enable);
}

int ShowManager::showDuration() const
{
    if (m_currentShow == NULL)
        return 0;

    return m_currentShow->totalDuration();
}

int ShowManager::currentTime() const
{
    return m_currentTime;
}

void ShowManager::setCurrentTime(int currentTime)
{
    if (m_currentTime == currentTime)
        return;

    m_currentTime = currentTime;
    emit currentTimeChanged(currentTime);
}

void ShowManager::playShow()
{
    if (m_currentShow == NULL)
        return;

    m_currentShow->start(m_doc->masterTimer(), false, m_currentTime);
    emit isPlayingChanged(true);
}

void ShowManager::stopShow()
{
    if (m_currentShow != NULL && m_currentShow->isRunning())
    {
        m_currentShow->stop();
        emit isPlayingChanged(false);
        return;
    }
    m_currentTime = 0;
    emit currentTimeChanged(m_currentTime);
}

bool ShowManager::isPlaying() const
{
    if (m_currentShow != NULL && m_currentShow->isRunning())
        return true;
    return false;
}

void ShowManager::slotTimeChanged(quint32 msec_time)
{
    m_currentTime = (int)msec_time;
    emit currentTimeChanged(m_currentTime);
}

bool ShowManager::checkOverlapping(Track *track, ShowFunction *sourceFunc,
                                   quint32 startTime, quint32 duration)
{
    if (track == NULL)
        return false;

    foreach(ShowFunction *sf, track->showFunctions())
    {
        if (sf == sourceFunc)
            continue;

        Function *func = m_doc->function(sf->functionID());
        if (func != NULL)
        {
            quint32 fst = sf->startTime();
            if ((startTime >= fst && startTime <= fst + sf->duration()) ||
                (fst >= startTime && fst <= startTime + duration))
            {
                return true;
            }
        }
    }

    return false;
}

void ShowManager::setShowName(QString showName)
{
    if (m_currentShow == NULL)
        return;

    if (m_currentShow->name() == showName)
        return;

    m_currentShow->setName(showName);
    emit showNameChanged(showName);
}

QString ShowManager::showName() const
{
    if (m_currentShow == NULL)
        return QString();

    return m_currentShow->name();
}

