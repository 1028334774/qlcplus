/*
  Q Light Controller Plus
  fixtureremap.h

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

#ifndef FIXTUREREMAP_H
#define FIXTUREREMAP_H

#include <QDialog>
#include <QList>

#include "ui_fixtureremap.h"

class Doc;
class RemapWidget;

struct RemapInfo
{
    QTreeWidgetItem *source;
    QTreeWidgetItem *target;
};

class FixtureRemap : public QDialog, public Ui_FixtureRemap
{
    Q_OBJECT
    Q_DISABLE_COPY(FixtureRemap)
    
public:
    explicit FixtureRemap(Doc* doc, QWidget *parent = 0);
    ~FixtureRemap();
    
private:
    Doc* m_doc;
    Doc* m_targetDoc;
    RemapWidget *remapWidget;
    QList <RemapInfo> m_remapList;

protected:
    void fillFixturesTree(Doc *doc, QTreeWidget *tree);

protected slots:
    void slotAddTargetFixture();
    void slotRemoveTargetFixture();
    void slotAddRemap();
    void slotUpdateConnections();

    /** Callback for OK button clicks */
    void accept();

};

#endif // FIXTUREREMAP_H
