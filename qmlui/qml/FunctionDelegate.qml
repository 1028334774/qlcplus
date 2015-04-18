/*
  Q Light Controller Plus
  FunctionDelegate.qml

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
import com.qlcplus.enums 1.0

Rectangle {
    id: funcDelegate
    width: 100
    height: 35

    color: "transparent"

    property string textLabel
    property int functionID
    property int functionType
    property bool isSelected: false

    signal clicked

    Rectangle {
        anchors.fill: parent
        radius: 3
        color: "#0978FF"
        visible: isSelected
    }

    IconTextEntry {
        width: parent.width
        height: parent.height
        iSrc: {
            switch (functionType)
            {
                case FunctionType.Scene: "qrc:/scene.svg"; break;
                case FunctionType.Chaser: "qrc:/chaser.svg"; break;
                case FunctionType.EFX: "qrc:/efx.svg"; break;
                case FunctionType.Collection: "qrc:/collection.svg"; break;
                case FunctionType.Script: "qrc:/script.svg"; break;
                case FunctionType.RGBMatrix: "qrc:/rgbmatrix.svg"; break;
                case FunctionType.Show: "qrc:/showmanager.svg"; break;
                case FunctionType.Audio: "qrc:/audio.svg"; break;
                case FunctionType.Video: "qrc:/video.svg"; break;
            }
        }

        tLabel: textLabel
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            isSelected = true
            functionManager.selectFunction(functionID, funcDelegate,
                                           (mouse.modifiers & Qt.ControlModifier))
            funcDelegate.clicked()
        }
    }
}

