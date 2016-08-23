/*
  Q Light Controller Plus
  virtualconsole.h

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

#ifndef VIRTUALCONSOLE_H
#define VIRTUALCONSOLE_H

#include <QQuickView>
#include <QObject>
#include <QHash>

#include "previewcontext.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class VCWidget;
class VCFrame;
class Doc;

#define KXMLQLCVirtualConsole "VirtualConsole"

class VirtualConsole : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(int pagesCount READ pagesCount NOTIFY pagesCountChanged)
    Q_PROPERTY(int selectedPage READ selectedPage WRITE setSelectedPage NOTIFY selectedPageChanged)
    Q_PROPERTY(bool editMode READ editMode WRITE setEditMode NOTIFY editModeChanged)
    Q_PROPERTY(VCWidget *selectedWidget READ selectedWidget NOTIFY selectedWidgetChanged)

public:
    VirtualConsole(QQuickView *view, Doc *doc, QObject *parent = 0);

    /** Return the number of pixels in 1mm */
    qreal pixelDensity() const;

    Q_INVOKABLE void renderPage(QQuickItem *parent, QQuickItem *contentItem, int page);

    Q_INVOKABLE void setWidgetSelection(quint32 wID, QQuickItem *item, bool enable);

    Q_INVOKABLE void moveWidget(VCWidget *widget, VCFrame *targetFrame, QPoint pos);

    /** Return the reference of the currently selected VC page */
    Q_INVOKABLE QQuickItem *currentPageItem() const;

    /** Return a list of strings with the currently selected VC widget names */
    Q_INVOKABLE QStringList selectedWidgetNames();

    /** Return a list of the currently selected VC widget IDs */
    Q_INVOKABLE QVariantList selectedWidgetIDs();

    /** Resets the currently selected widgets selection list */
    Q_INVOKABLE void resetWidgetSelection();

    /** Delete the VC widgets with the IDs specified in $IDList */
    void deleteVCWidgets(QVariantList IDList);

    /*********************************************************************
     * Contents
     *********************************************************************/

public:
    /** Get the Virtual Console's frame representing the given $page,
     *  where all the widgets are placed */
    Q_INVOKABLE VCFrame* page(int page) const;

    /** Reset the Virtual Console contents to an initial state */
    void resetContents();

    /** Add $widget to the global VC widgets map */
    void addWidgetToMap(VCWidget* widget);

    /** Remove $widget from the global VC widgets map */
    void removeWidgetFromMap(VCWidget* widget);

    /** Return a reference to the VC widget with the specified $id.
     *  On invalid $id, NULL is returned */
    VCWidget *widget(quint32 id);

    /** Return a list with the VC page names */
    int pagesCount() const;

    /** Add a new VC page at $index */
    Q_INVOKABLE void addPage(int index);

    /** Delete a VC page at $index */
    void deletePage(int index);

    /** Set a protection PIN for the page at $index */
    Q_INVOKABLE bool setPagePIN(int index, QString currentPIN, QString newPIN);

    /** Validate a PIN for a VC Page. Returns true if the user entered the
     *  correct PIN, otherwise false is returned.
     *  The $remember flag is used to avoid requesting the PIN again
     *  for the entire session (on PIN check success) */
    Q_INVOKABLE bool validatePagePIN(int index, QString PIN, bool remember);

    /** Set/Get the currently selected VC page index */
    int selectedPage() const;
    void setSelectedPage(int selectedPage);

    /** Set/Get the VC edit mode flag */
    bool editMode() const;
    void setEditMode(bool editMode);

    /** Enable/disable the current page scroll interaction */
    Q_INVOKABLE void setPageInteraction(bool enable);

    /** Return a reference to the currently selected VC widget */
    VCWidget *selectedWidget() const;

signals:
    void editModeChanged(bool editMode);

    /** Notify the listeners that the currenly selected VC widget has changed */
    void selectedWidgetChanged(VCWidget* selectedWidget);

    /** Notify the listeners that the currenly selected VC page has changed */
    void selectedPageChanged(int selectedPage);

    /** Notify the listener that some page names have changed */
    void pagesCountChanged();

protected:
    /** Create a new widget ID */
    quint32 newWidgetId();

protected:
    /** A list of VCFrames representing the main VC pages */
    QVector<VCFrame*> m_pages;

    /** A map of all the VC widgets references with their IDs */
    QHash <quint32, VCWidget *> m_widgetsMap;

    /** Latest assigned widget ID */
    quint32 m_latestWidgetId;

    bool m_editMode;

    int m_selectedPage;

    VCWidget *m_selectedWidget;

    /*********************************************************************
     * Drag & Drop
     *********************************************************************/
public:
    /** Add or remove a target to the dropTargets list.
     *  This is used to handle the stacking order of highlight areas
     *  of frames when dragging/dropping a new widget on the VC */
    Q_INVOKABLE void setDropTarget(QQuickItem *target, bool enable);

    /** Reset the drop targets list.
     *  deleteTargets is true when a new widget is dropped, so
     *  drop areas highlight is no more needed */
    void resetDropTargets(bool deleteTargets);

protected:
    /** A list of the QML targets used for drag & drop of new widgets.
     *  Items are stacked in a precise order to handle the enter/exit events
     *  of a drag item and highlight only the last item entered */
    QList<QQuickItem *>m_dropTargets;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** Load properties and contents from an XML tree */
    bool loadXML(QXmlStreamReader &root);

    /** Load the Virtual Console global properties XML tree */
    bool loadPropertiesXML(QXmlStreamReader &root);

    /** Save properties and contents to an XML document */
    bool saveXML(QXmlStreamWriter *doc);

    /** Do post-load cleanup & checks */
    void postLoad();
};

#endif // VIRTUALCONSOLE_H
