/*
  Q Light Controller Plus
  vcbutton.h

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

#ifndef VCBUTTON_H
#define VCBUTTON_H

#include "vcwidget.h"

#define KXMLQLCVCButton "Button"

#define KXMLQLCVCButtonFunction "Function"
#define KXMLQLCVCButtonFunctionID "ID"

#define KXMLQLCVCButtonAction "Action"
#define KXMLQLCVCButtonActionFlash "Flash"
#define KXMLQLCVCButtonActionToggle "Toggle"
#define KXMLQLCVCButtonActionBlackout "Blackout"
#define KXMLQLCVCButtonActionStopAll "StopAll"

class VCButton : public VCWidget
{
    Q_OBJECT

    Q_PROPERTY(Action actionType READ actionType WRITE setActionType NOTIFY actionTypeChanged)
    Q_PROPERTY(bool isOn READ isOn WRITE setOn NOTIFY isOnChanged)

    /*********************************************************************
     * Initialization
     *********************************************************************/

public:
    VCButton(Doc* doc = NULL, QObject *parent = 0);
    virtual ~VCButton();

    void setID(quint32 id);

    void render(QQuickView *view, QQuickItem *parent);

    /*********************************************************************
     * Function attachment
     *********************************************************************/
public:
    /**
     * Attach a function to a VCButton. This function is started when the
     * button is pressed down.
     *
     * @param function An ID of a function to attach
     */
    Q_INVOKABLE void setFunction(quint32 fid);

    /**
     * Get the ID of the function attached to a VCButton
     *
     * @return The ID of the attached function or Function::invalidId()
     *         if there isn't one
     */
    quint32 function() const;

protected:
    /** The function that this button is controlling */
    quint32 m_function;

    /*********************************************************************
     * Button state
     *********************************************************************/
public:
    /** Get the current on/off state of the button */
    bool isOn() const;

    /** Set the button on/off state */
    void setOn(bool isOn);

signals:
    void isOnChanged(bool isOn);

protected:
    bool m_isOn;

    /*********************************************************************
     * Button action
     *********************************************************************/
public:
    /**
     * Toggle: Start/stop the assigned function.
     * Flash: Keep the function running as long as the button is kept down.
     * Blackout: Toggle blackout on/off.
     * StopAll: Stop all functions (panic button).
     */
    enum Action { Toggle, Flash, Blackout, StopAll };
    Q_ENUMS(Action)

    Action actionType() const;

    void setActionType(Action actionType);

    static QString actionToString(Action action);
    static Action stringToAction(const QString& str);

signals:
    void actionTypeChanged(Action actionType);

protected:
    Action m_actionType;

    /*********************************************************************
     * Load & Save
     *********************************************************************/

public:
    bool loadXML(const QDomElement* root);
    //bool saveXML(QDomDocument* doc, QDomElement* vc_root);
};

#endif
