/*
  Q Light Controller Plus
  VCButtonItem.qml

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

import QtQuick 2.0
import QtQuick.Layouts 1.1

import com.qlcplus.classes 1.0

VCWidgetItem
{
    id: buttonRoot
    property VCButton buttonObj: null
    property bool isOn: buttonObj ? buttonObj.isOn : false
    radius: 4

    gradient: Gradient
    {
        GradientStop { id: lightCol; position: 0 ; color: buttonRoot.color }
        GradientStop { position: 1 ; color: buttonRoot.color }
    }

    onButtonObjChanged:
    {
        setCommonProperties(buttonObj)
        lightCol.color = Qt.lighter(buttonRoot.color, 1.2)
    }

    Rectangle
    {
        id: activeBorder
        x: 1
        y: 1
        width: parent.width - 2
        height: parent.height - 2
        color: "transparent"
        border.width: (buttonRoot.width > 80) ? 3 : 2
        border.color: isOn ? "green" : "#A0A0A0"
        radius: 3

        Rectangle
        {
            x: 3
            y: 3
            width: parent.width - 6
            height: parent.height - 6
            radius: 2
            border.width: 1
            color: "transparent"
            clip: true

            Text
            {
                x: 2
                width: parent.width - 4
                height: parent.height
                font: buttonObj ? buttonObj.font : null
                text: buttonObj ? buttonObj.caption : ""
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                lineHeight: 0.8
                color: buttonObj ? buttonObj.foregroundColor : "#111"
            }

            Image
            {
                visible: buttonObj ? (buttonObj.actionType === VCButton.Flash) : false
                x: parent.width - 22
                y: 4
                width: 18
                height: 18
                source: "qrc:/flash.svg"
                sourceSize: Qt.size(width, height)
            }
        }
    }

    MouseArea
    {
        anchors.fill: parent
        onClicked:
        {
            if (buttonObj.actionType === VCButton.Toggle)
                buttonObj.isOn = !buttonObj.isOn
        }
        onPressed:
        {
            if (buttonObj.actionType === VCButton.Flash)
                buttonObj.isOn = true
        }
        onReleased:
        {
            if (buttonObj.actionType === VCButton.Flash)
                buttonObj.isOn = false
        }
    }

    DropArea
    {
        id: dropArea
        anchors.fill: parent
        z: 2 // this area must be above the VCWidget resize controls
        keys: [ "function" ]

        onDropped:
        {
            // attach function here
            buttonObj.setFunction(drag.source.funcID)
        }

        states: [
            State
            {
                when: dropArea.containsDrag
                PropertyChanges
                {
                    target: buttonRoot
                    color: "#9DFF52"
                }
            }
        ]
    }
}
