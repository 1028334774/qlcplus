/*
  Q Light Controller
  fadechannel.h

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef CHANNELSGROUP_H
#define CHANNELSGROUP_H

#include <QObject>

#include "scenevalue.h"

class QDomDocument;
class QDomElement;

#define KXMLQLCChannelsGroup "ChannelsGroup"

class ChannelsGroup : public QObject
{
    Q_OBJECT

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    /** Create a new ChannelsGroup with empty/invalid values */
    ChannelsGroup(Doc* parent);

    /** Copy constructor */
    ChannelsGroup(Doc* parent, const ChannelsGroup* chg);

    /** destroy this ChannelsGroup */
    ~ChannelsGroup();

signals:
    /** Emitted whenever a channels group's properties are changed */
    void changed(quint32 id);

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    static bool loader(const QDomElement& root, Doc* doc);

    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

    bool loadXML(const QDomElement& root);

    /************************************************************************
     * ID
     ************************************************************************/
public:
    /** Set a channels group's id (only Doc is allowed to do this!) */
    void setId(quint32 id);

    /** Get a channels group's unique id */
    quint32 id() const;

    /** Get an invalid channels group id */
    static quint32 invalidId();

private:
    quint32 m_id;

    /************************************************************************
     * Name
     ************************************************************************/
public:
    /** Set the name of a channels group */
    void setName(const QString& name);

    /** Get the name of a channels group */
    QString name() const;

    /************************************************************************
     * Channels
     ************************************************************************/
    /** Empty the current values of this channels group */
    void resetList();

    /** Add a channel to this channels group */
    bool addChannel(quint32 fxid, quint32 channel);

    /** Returns the current list of channels of this group */
    QList <SceneValue> getChannels() const;
    /*********************************************************************
     * Status
     *********************************************************************/
public:
    /**
     * Get the channels group instance's status info for Fixture Manager
     *
     * @return A sort-of HTML-RTF-gibberish for Fixture Manager
     */
    QString status(Doc *doc) const;

private:
    QString m_name;

    uchar m_masterValue;
    QList <SceneValue> m_channels;
};

#endif
