/*
  Q Light Controller Plus
  VCWidgetItem.qml

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

import com.qlcplus.classes 1.0

Rectangle
{
    id: wRoot
    width: 100
    height: 100
    color: "darkgray"
    border.width: 1
    border.color: "#111"

    property VCWidget wObj: null

    function setCommonProperties(obj)
    {
        if (obj === null)
            return;

        wObj = obj

        x = wObj.geometry.x
        y = wObj.geometry.y
        width = wObj.geometry.width
        height = wObj.geometry.height
        color = wObj.backgroundColor
    }

    // resize area
    Rectangle
    {
        id: resizeLayer
        anchors.fill: parent
        color: "transparent"
        // this must be above the widget root but
        // underneath the widget children (if any)
        z: 1
        visible: virtualConsole.resizeMode && wObj.allowResize

        // mouse area to move the widget
        MouseArea
        {
            anchors.fill: parent

            onPressed:
            {
                drag.target = wRoot
                drag.minimumX = 0
                drag.minimumY = 0
                drag.maximumX = wRoot.parent.width - wRoot.width
                drag.maximumY = wRoot.parent.height - wRoot.height
            }

            onReleased:
            {
                if (drag.target !== null)
                {
                    drag.target = null
                    wObj.geometry = Qt.rect(wRoot.x, wRoot.y, wRoot.width, wRoot.height)
                }
            }
        }

        // top-left corner
        Image
        {
            id: tlHandle
            rotation: 180
            source: "qrc:/arrow-corner.svg"
            sourceSize: Qt.size(32, 32)

            MouseArea
            {
                anchors.fill: parent
                cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
            }
        }
        // top-right corner
        Image
        {
            id: trHandle
            x: parent.width - 32
            rotation: 270
            source: "qrc:/arrow-corner.svg"
            sourceSize: Qt.size(32, 32)

            MouseArea
            {
                anchors.fill: parent
                cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
            }
        }
        // bottom-right corner
        Image
        {
            id: brHandle
            x: parent.width - 32
            y: parent.height - 32
            source: "qrc:/arrow-corner.svg"
            sourceSize: Qt.size(32, 32)

            MouseArea
            {
                anchors.fill: parent
                cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
                onPressed:
                {
                    drag.target = brHandle
                    drag.minimumX = 0
                }
                onPositionChanged:
                {
                    if (drag.target === null)
                        return;
                    wRoot.width = brHandle.x + brHandle.width
                    wRoot.height = brHandle.y + brHandle.height
                }
                onReleased:
                {
                    drag.target = null
                    wObj.geometry = Qt.rect(wRoot.x, wRoot.y, wRoot.width, wRoot.height)
                }
            }
        }
        // bottom-left corner
        Image
        {
            id: blHandle
            rotation: 90
            y: parent.height - 32
            source: "qrc:/arrow-corner.svg"
            sourceSize: Qt.size(32, 32)

            MouseArea
            {
                anchors.fill: parent
                cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
            }
        }
    }
}
