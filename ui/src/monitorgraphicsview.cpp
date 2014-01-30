/*
  Q Light Controller Plus
  monitorgraphicsview.cpp

  Copyright (C) Massimo Callegari

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

#include "monitorgraphicsview.h"
#include "monitorfixtureitem.h"
#include "qlcfixturemode.h"
#include "fixture.h"
#include "doc.h"

MonitorGraphicsView::MonitorGraphicsView(Doc *doc, QWidget *parent)
    : QGraphicsView(parent)
    , m_doc(doc)
    , m_gridEnabled(true)
{
    m_scene = new QGraphicsScene();
    m_scene->setSceneRect(this->rect());
    //setSceneRect(parent->rect());
    setScene(m_scene);

    m_gridSize = QSize(5, 5);

    updateGrid();
}

void MonitorGraphicsView::setGridSize(QSize size)
{
    m_gridSize = size;
    updateGrid();
}

void MonitorGraphicsView::setGridMetrics(float value)
{
    m_unitValue = value;
    QHashIterator <quint32, MonitorFixtureItem*> it(m_fixtures);
    while (it.hasNext() == true)
    {
        it.next();
        updateFixtureSize(it.key());
    }
}

QList<quint32> MonitorGraphicsView::fixturesID() const
{
    return m_fixtures.keys();
}

void MonitorGraphicsView::updateFixtureSize(quint32 id)
{
    Fixture *fxi = m_doc->fixture(id);
    if (fxi == NULL || m_fixtures.contains(id) == false)
        return;

    const QLCFixtureMode *mode = fxi->fixtureMode();
    int width = mode->physical().width();
    int height = mode->physical().height();

    MonitorFixtureItem *item = m_fixtures[id];
    item->setSize(QSize((width * m_cellPixels) / m_unitValue, (height * m_cellPixels) / m_unitValue));
}

void MonitorGraphicsView::addFixtureItem(quint32 id)
{
    if (id == Fixture::invalidId() || m_fixtures.contains(id) == true)
        return;

    MonitorFixtureItem *item = new MonitorFixtureItem(m_doc, id);
    m_fixtures[id] = item;
    m_scene->addItem(item);
    updateFixtureSize(id);
}

void MonitorGraphicsView::updateGrid()
{
    for (int i = 0; i < m_gridItems.count(); i++)
        m_scene->removeItem((QGraphicsItem *)m_gridItems.at(i));

    if (m_gridEnabled == true)
    {
        m_xOffset = 0;
        m_yOffset = 0;
        int xInc = this->width() / m_gridSize.width();
        int yInc = this->height() / m_gridSize.height();
        if (yInc < xInc)
        {
            m_cellPixels = yInc;
            m_xOffset = (this->width() - (m_cellPixels * m_gridSize.width())) / 2;
        }
        else if (xInc < yInc)
        {
            m_cellPixels = xInc;
            m_yOffset = (this->height() - (m_cellPixels * m_gridSize.height())) / 2;
        }
        int xPos = m_xOffset;
        int yPos = m_yOffset;
        for (int i = 0; i < m_gridSize.width() + 1; i++)
        {
            QGraphicsLineItem *item = m_scene->addLine(xPos, m_yOffset, xPos, this->height() - m_yOffset, QPen( QColor(150, 150, 150, 255) ));
            item->setZValue(0);
            xPos += m_cellPixels;
            m_gridItems.append(item);
        }

        for (int i = 0; i < m_gridSize.height() + 1; i++)
        {
            QGraphicsLineItem *item = m_scene->addLine(m_xOffset, yPos, this->width() - m_xOffset, yPos, QPen( QColor(150, 150, 150, 255) ));
            item->setZValue(0);
            yPos += m_cellPixels;
            m_gridItems.append(item);
        }
    }
}

void MonitorGraphicsView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    updateGrid();
}
