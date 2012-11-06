/*
  Q Light Controller
  sceneitems.h

  Copyright (C) Heikki Junnila

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


#ifndef SCENEITEMS_H
#define SCENEITEMS_H

#include <QGraphicsItem>
#include <QFont>
#include <QObject>

#include "chaser.h"

/*********************************************************************
 * Scene Header class. Clickable time line header
 *********************************************************************/
class SceneHeaderItem :  public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    SceneHeaderItem(int);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setTimeScale(int val);
    int getTimeScale();
    int getTimeStep();

signals:
    void itemClicked(QGraphicsSceneMouseEvent *);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    int width;
    int timeStep;
    int timeScale;
};

/***************************************************************************
 * Scene Cursor class. Cursor which marks the time position in a scene
 ***************************************************************************/
class SceneCursorItem : public QGraphicsItem
{
public:
    SceneCursorItem(int h);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setTime(quint32 t);
    quint32 getTime();
private:
    int height;
    quint32 m_time;
};

/****************************************************************************
 * Track class. This is only a visual item to fullfill the multi track view
 ****************************************************************************/
class TrackItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    TrackItem(int number);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    int getTrackNumber();

private:
    QString trackName;
    int trackNumber;
    QFont m_font;
};

/*********************************************************************
 * Sequence Item. Clickable and draggable object identifying a chaser
 *********************************************************************/
class SequenceItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    SequenceItem(Chaser *seq);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setTimeScale(int val);
    Chaser *getChaser();

signals:
    void itemDropped(QGraphicsSceneMouseEvent *, SequenceItem *);
    //void itemSelected(bool status);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    //void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

protected slots:
    void slotSequenceChanged(quint32);

private:
    QColor color;
    /* Reference to the actual Chaser object which holds the sequence steps */
    Chaser *chaser;
    /* width of the graphics object. Recalculated every time a chaser step  changes */
    int seq_width;
    /* horizontal scale to adapt width to the current time line */
    int timeScale;
};

#endif
