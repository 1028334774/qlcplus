/*
  Q Light Controller Plus
  track.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

#include "track.h"
#include "scene.h"
#include "doc.h"

#define KXMLQLCTrackID        "ID"
#define KXMLQLCTrackName      "Name"
#define KXMLQLCTrackSceneID   "SceneID"
#define KXMLQLCTrackIsMute    "isMute"

#define KXMLQLCTrackFunctions "Functions"

Track::Track(quint32 sceneID)
    : m_id(Track::invalidId())
    , m_sceneID(sceneID)
    , m_isMute(false)

{
    setName(tr("New Track"));
}

Track::~Track()
{

}

/****************************************************************************
 * ID
 ****************************************************************************/

void Track::setId(quint32 id)
{
    m_id = id;
}

quint32 Track::id() const
{
    return m_id;
}

quint32 Track::invalidId()
{
    return UINT_MAX;
}

/****************************************************************************
 * Name
 ****************************************************************************/

void Track::setName(const QString& name)
{
    m_name = name;
    emit changed(this->id());
}

QString Track::name() const
{
    return m_name;
}

/*********************************************************************
 * Scene
 *********************************************************************/
void Track::setSceneID(quint32 id)
{
    m_sceneID = id;
}

quint32 Track::getSceneID()
{
    return m_sceneID;
}

/*********************************************************************
 * Mute state
 *********************************************************************/
void Track::setMute(bool state)
{
    m_isMute = state;
}

bool Track::isMute()
{
    return m_isMute;
}

/*********************************************************************
 * Sequences
 *********************************************************************/

ShowFunction* Track::createShowFunction(quint32 id)
{
    ShowFunction *func = new ShowFunction();
    func->setFunctionID(id);
    m_functions.append(func);

    return func;
}

bool Track::addShowFunction(ShowFunction *func)
{
    if (func == NULL || func->functionID() == Function::invalidId())
        return false;

    m_functions.append(func);

    return true;
}

bool Track::removeShowFunction(ShowFunction *function)
{
    if (m_functions.contains(function) == false)
        return false;

    ShowFunction *func = m_functions.takeAt(m_functions.indexOf(function));
    delete func;

    return true;
}

QList <ShowFunction *> Track::showFunctions() const
{
    return m_functions;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/
bool Track::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement tag;

    Q_ASSERT(doc != NULL);

    /* Track entry */
    tag = doc->createElement(KXMLQLCTrack);
    tag.setAttribute(KXMLQLCTrackID, this->id());
    tag.setAttribute(KXMLQLCTrackName, this->name());
    if (m_sceneID != Scene::invalidId())
        tag.setAttribute(KXMLQLCTrackSceneID, m_sceneID);
    tag.setAttribute(KXMLQLCTrackIsMute, m_isMute);

    /* Save the list of Functions if any is present */
    if (m_functions.isEmpty() == false)
    {
        foreach(ShowFunction *func, showFunctions())
            func->saveXML(doc, &tag);
    }

    wksp_root->appendChild(tag);

    return true;
}

bool Track::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCTrack)
    {
        qWarning() << Q_FUNC_INFO << "Track node not found";
        return false;
    }

    bool ok = false;
    QXmlStreamAttributes attrs = root.attributes();
    quint32 id = attrs.value(KXMLQLCTrackID).toString().toUInt(&ok);
    if (ok == false)
    {
        qWarning() << "Invalid Track ID:" << attrs.value(KXMLQLCTrackID).toString();
        return false;
    }
    // Assign the ID to myself
    m_id = id;

    if (attrs.hasAttribute(KXMLQLCTrackName) == true)
        m_name = attrs.value(KXMLQLCTrackName).toString();

    if (attrs.hasAttribute(KXMLQLCTrackSceneID))
    {
        ok = false;
        id = attrs.value(KXMLQLCTrackSceneID).toString().toUInt(&ok);
        if (ok == false)
        {
            qWarning() << "Invalid Scene ID:" << attrs.value(KXMLQLCTrackSceneID).toString();
            return false;
        }
        m_sceneID = id;
    }

    ok = false;
    bool mute = attrs.value(KXMLQLCTrackIsMute).toString().toInt(&ok);
    if (ok == false)
    {
        qWarning() << "Invalid Mute flag:" << root.attribute(KXMLQLCTrackIsMute);
        return false;
    }
    m_isMute = mute;

    /* look for show functions */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLShowFunction)
        {
            ShowFunction *newFunc = new ShowFunction();
            newFunc->loadXML(tag);
            if (addShowFunction(newFunc) == false)
                delete newFunc;
        }
        /* LEGACY code: to be removed */
        else if (root.name() == KXMLQLCTrackFunctions)
        {
            QString strvals = root.readElementText();
            if (strvals.isEmpty() == false)
            {
                QStringList varray = strvals.split(",");
                for (int i = 0; i < varray.count(); i++)
                    createShowFunction(QString(varray.at(i)).toUInt());
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Track tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}
