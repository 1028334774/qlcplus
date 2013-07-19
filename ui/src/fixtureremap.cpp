/*
  Q Light Controller Plus
  fixtureremap.cpp

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

#include <QMessageBox>
#include <QScrollBar>
#include <QDir>

#include "qlcfixturedef.h"
#include "fixtureremap.h"
#include "remapwidget.h"
#include "qlcchannel.h"
#include "addfixture.h"
#include "scenevalue.h"
#include "scene.h"
#include "doc.h"

#define KColumnName         0
#define KColumnAddress      1
#define KColumnUniverse     2
#define KColumnID           3
#define KColumnChIdx        4

FixtureRemap::FixtureRemap(Doc *doc, QWidget *parent)
    : QDialog(parent)
    , m_doc(doc)
{
    Q_ASSERT(doc != NULL);

    setupUi(this);

    connect(m_addButton, SIGNAL(clicked()),
            this, SLOT(slotAddTargetFixture()));
    connect(m_removeButton, SIGNAL(clicked()),
            this, SLOT(slotRemoveTargetFixture()));
    connect(m_remapButton, SIGNAL(clicked()),
            this, SLOT(slotAddRemap()));

    connect(m_sourceTree->verticalScrollBar(), SIGNAL(sliderMoved(int)),
            this, SLOT(slotUpdateConnections()));

    connect(m_sourceTree, SIGNAL(clicked(QModelIndex)),
            this, SLOT(slotUpdateConnections()));
    connect(m_sourceTree, SIGNAL(expanded(QModelIndex)),
            this, SLOT(slotUpdateConnections()));
    connect(m_sourceTree, SIGNAL(collapsed(QModelIndex)),
            this, SLOT(slotUpdateConnections()));

    connect(m_targetTree->verticalScrollBar(), SIGNAL(sliderMoved(int)),
            this, SLOT(slotUpdateConnections()));
    connect(m_targetTree, SIGNAL(clicked(QModelIndex)),
            this, SLOT(slotUpdateConnections()));
    connect(m_targetTree, SIGNAL(expanded(QModelIndex)),
            this, SLOT(slotUpdateConnections()));
    connect(m_targetTree, SIGNAL(collapsed(QModelIndex)),
            this, SLOT(slotUpdateConnections()));

    remapWidget = new RemapWidget(m_sourceTree, m_targetTree, this);
    remapWidget->show();
    m_remapLayout->addWidget(remapWidget);

    m_targetDoc = new Doc(this);
    /* Load user fixtures first so that they override system fixtures */
    m_targetDoc->fixtureDefCache()->load(QLCFixtureDefCache::userDefinitionDirectory());
    m_targetDoc->fixtureDefCache()->load(QLCFixtureDefCache::systemDefinitionDirectory());

    m_targetProjectLabel->setText(QString("%1%2%3").arg(m_doc->getWorkspacePath()).arg(QDir::separator()).arg("remapped_project.qxw"));

    fillFixturesTree(m_doc, m_sourceTree);
}

FixtureRemap::~FixtureRemap()
{
    delete m_targetDoc;
}

void FixtureRemap::fillFixturesTree(Doc *doc, QTreeWidget *tree)
{
    tree->clear();
    //tree->header()->setResizeMode(QHeaderView::ResizeToContents);
    tree->setIconSize(QSize(24, 24));
    tree->setAllColumnsShowFocus(true);

    foreach(Fixture *fxi, doc->fixtures())
    {
        QTreeWidgetItem *topItem = NULL;
        quint32 uni = fxi->universe();
        for (int i = 0; i < tree->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* tItem = tree->topLevelItem(i);
            quint32 tUni = tItem->text(KColumnUniverse).toUInt();
            if (tUni == uni)
            {
                topItem = tItem;
                break;
            }
        }

        // Haven't found this universe node ? Create it.
        if (topItem == NULL)
        {
            topItem = new QTreeWidgetItem(tree);
            topItem->setText(KColumnName, tr("Universe %1").arg(uni + 1));
            topItem->setText(KColumnUniverse, QString::number(uni));
            topItem->setExpanded(true);
        }

        quint32 baseAddr = fxi->address();
        QTreeWidgetItem *fItem = new QTreeWidgetItem(topItem);
        fItem->setText(KColumnName, fxi->name());
        fItem->setIcon(KColumnName, fxi->getIconFromType(fxi->type()));
        fItem->setText(KColumnAddress, QString("%1 - %2").arg(baseAddr + 1).arg(baseAddr + fxi->channels()));
        fItem->setText(KColumnUniverse, QString::number(uni));
        fItem->setText(KColumnID, QString::number(fxi->id()));

        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel* channel = fxi->channel(c);
            QTreeWidgetItem *item = new QTreeWidgetItem(fItem);
            item->setText(KColumnName, QString("%1:%2").arg(c + 1)
                          .arg(channel->name()));
            item->setIcon(KColumnName, channel->getIconFromGroup(channel->group()));
            item->setText(KColumnUniverse, QString::number(uni));
            item->setText(KColumnID, QString::number(fxi->id()));
            item->setText(KColumnChIdx, QString::number(c));
        }
    }

    tree->resizeColumnToContents(KColumnName);
}

void FixtureRemap::slotAddTargetFixture()
{
    AddFixture af(this, m_targetDoc);
    if (af.exec() == QDialog::Rejected)
        return;

    QString name = af.name();
    quint32 address = af.address();
    quint32 universe = af.universe();
    quint32 channels = af.channels();
    const QLCFixtureDef* fixtureDef = af.fixtureDef();
    const QLCFixtureMode* mode = af.mode();
    int gap = af.gap();

    for(int i = 0; i < af.amount(); i++)
    {
        QString modname;

        /* If an empty name was given use the model instead */
        if (name.simplified().isEmpty())
        {
            if (fixtureDef != NULL)
                name = fixtureDef->model();
            else
                name = tr("Generic Dimmer");
        }

        /* If we're adding more than one fixture,
           append a number to the end of the name */
        if (af.amount() > 1)
            modname = QString("%1 #%2").arg(name).arg(i+1);
        else
            modname = name;

        /* Create the target fixture */
        Fixture* fxi = new Fixture(m_targetDoc);

        /* Add the first fixture without gap, at the given address */
        fxi->setAddress(address + (i * channels) + (i * gap));
        fxi->setUniverse(universe);
        fxi->setName(modname);

        /* Set a fixture definition & mode if they were selected.
           Otherwise assign channels to a generic dimmer. */
        if (fixtureDef != NULL && mode != NULL)
            fxi->setFixtureDefinition(fixtureDef, mode);
        else
            fxi->setChannels(channels);

        m_targetDoc->addFixture(fxi);
    }

    fillFixturesTree(m_targetDoc, m_targetTree);
}

void FixtureRemap::slotRemoveTargetFixture()
{
}

void FixtureRemap::slotAddRemap()
{
    if (m_sourceTree->selectedItems().count() == 0 ||
        m_targetTree->selectedItems().count() == 0)
    {
        QMessageBox::warning(this,
                             tr("Invalid selection"),
                             tr("Please select a source and a target fixture or channel to perform this operation."));
        return;
    }

    RemapInfo newRemap;
    newRemap.source = m_sourceTree->selectedItems().first();
    newRemap.target = m_targetTree->selectedItems().first();

    bool srcFxiSelected = false;
    bool tgtFxiSelected = false;

    bool ok = false;
    int chIdx = newRemap.source->text(KColumnChIdx).toInt(&ok);
    if (ok == false)
        srcFxiSelected = true;
    ok = false;
    chIdx = newRemap.target->text(KColumnChIdx).toInt(&ok);
    if (ok == false)
        tgtFxiSelected = true;

    qDebug() << "Idx:" << chIdx << ", src:" << srcFxiSelected << ", tgt:" << tgtFxiSelected;

    if ((srcFxiSelected == true && tgtFxiSelected == false) ||
        (srcFxiSelected == false && tgtFxiSelected == true) )
    {
        QMessageBox::warning(this,
                             tr("Invalid selection"),
                             tr("To perform a fixture remap, please select fixtures on both lists."));
        return;
    }
    else if (srcFxiSelected == true && tgtFxiSelected == true)
    {
        // perform a full fixture remap
        quint32 srcFxiID = newRemap.source->text(KColumnID).toUInt();
        Fixture *srcFxi = m_doc->fixture(srcFxiID);
        Q_ASSERT(srcFxi != NULL);
        quint32 tgtFxiID = newRemap.target->text(KColumnID).toUInt();
        Fixture *tgtFxi = m_targetDoc->fixture(tgtFxiID);
        Q_ASSERT(tgtFxi != NULL);

        for (quint32 s = 0; s < srcFxi->channels(); s++)
        {
            const QLCChannel* srcCh = srcFxi->channel(s);

            for (quint32 t = 0; t < tgtFxi->channels(); t++)
            {
                const QLCChannel* tgtCh = tgtFxi->channel(t);
                if ((tgtCh->group() == srcCh->group()) &&
                    (tgtCh->controlByte() == srcCh->controlByte()))
                {
                    if (tgtCh->group() == QLCChannel::Intensity &&
                        tgtCh->colour() != srcCh->colour())
                            continue;
                    RemapInfo matchInfo;
                    matchInfo.source = newRemap.source->child(s);
                    matchInfo.target = newRemap.target->child(t);
                    m_remapList.append(matchInfo);
                    break;
                }
            }
        }
    }
    else
    {
        // perform a single channel remap
        m_remapList.append(newRemap);
    }

    remapWidget->setRemapList(m_remapList);
}

void FixtureRemap::slotUpdateConnections()
{
    remapWidget->update();
}

void FixtureRemap::accept()
{
    // 1 - create a map of SceneValues from the fixtures channel associations
    QMap <SceneValue, SceneValue> m_sceneMap;

    foreach (RemapInfo info, m_remapList)
    {
        quint32 srcFxiID = info.source->text(KColumnID).toUInt();
        quint32 srcChIdx = info.source->text(KColumnChIdx).toUInt();

        quint32 tgtFxiID = info.target->text(KColumnID).toUInt();
        quint32 tgtChIdx = info.target->text(KColumnChIdx).toUInt();

        SceneValue srcVal(srcFxiID, srcChIdx);
        SceneValue tgtVal(tgtFxiID, tgtChIdx);
        m_sceneMap[srcVal] = tgtVal;
    }

    // 2 - replace original project fixtures
    m_doc->replaceFixtures(m_targetDoc->fixtures());

    // 3 - scan project functions and perform remapping
    foreach (Function *func, m_doc->functions())
    {
        switch (func->type())
        {
            case Function::Scene:
            {
                Scene *s = qobject_cast<Scene*>(func);
                QList <SceneValue> newValuesList;
                foreach(SceneValue val, s->values())
                {
                    if (m_sceneMap.contains(val))
                    {
                        SceneValue tgtVal = m_sceneMap[val];
                        qDebug() << "[Scene] Remapping" << val.fxi << val.channel << " to " << tgtVal.fxi << tgtVal.channel;
                        val.fxi = tgtVal.fxi;
                        val.channel = tgtVal.channel;
                        newValuesList.append(val);
                    }
                }
                s->clear();
                for (int i = 0; i < newValuesList.count(); i++)
                    s->setValue(newValuesList.at(i));
            }
            break;
            case Function::Chaser:
            break;
            case Function::Show:
            break;
            case Function::EFX:
            break;
            default:
            break;
        }
    }

    // 4- emit signal to save the remapped project into a new file

    /* Close dialog */
    QDialog::accept();
}
