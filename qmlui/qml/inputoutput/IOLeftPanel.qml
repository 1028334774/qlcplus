/*
  Q Light Controller Plus
  IOLeftPanel.qml

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
import QtQuick.Controls 1.0

Rectangle
{
    id: leftSidePanel
    anchors.left: parent.left;
    anchors.leftMargin: 0
    width: collapseWidth
    height: parent.height
    color: "#232323"

    property bool isOpen: false
    property int collapseWidth: 50
    property int expandedWidth: 400
    property string editorSource: ""
    property int universeIndex
    property bool showAudioButton: false
    property bool showPluginsButton: false
    property int iconSize: collapseWidth - 4

    function animatePanel(checked)
    {
        if (checked === isOpen)
            return

        if (isOpen == false)
        {
            editorLoader.source = editorSource
            animateOpen.start()
            isOpen = true
        }
        else
        {
            animateClose.start()
            isOpen = false
            editorLoader.source = ""
        }
    }

    onUniverseIndexChanged:
    {
        if (isOpen == true)
        {
            editorLoader.source = ""
            editorLoader.source = editorSource
        }
    }

    Rectangle
    {
        id: editorArea
        z: 5
        width: leftSidePanel.width - collapseWidth;
        height: parent.height
        color: "transparent"

        Loader
        {
            id: editorLoader
            anchors.fill: parent
            source: editorSource
            onLoaded:
            {
                item.universeIndex = universeIndex
                item.loadSources(true)
            }
        }
    }

    Rectangle
    {
        id: sideBar
        x: parent.width - collapseWidth
        width: collapseWidth
        height: parent.height
        color: "#00000000"
        z: 2

        Column
        {
            anchors.fill: parent
            anchors.leftMargin: 1
            spacing: 3

            ExclusiveGroup { id: ioInputGroup }

            IconButton
            {
                id: audioInputButton
                z: 2
                visible: showAudioButton
                width: iconSize
                height: iconSize
                imgSource: "qrc:/audiocard.svg"
                checkable: true
                exclusiveGroup: ioInputGroup
                tooltip: qsTr("Show the audio input sources")
                onToggled:
                {
                    if (checked == true)
                        editorSource = "qrc:/AudioCardsList.qml"
                    animatePanel(checked);
                }
            }

            IconButton
            {
                id: uniInputButton
                z: 2
                visible: showPluginsButton
                width: iconSize
                height: iconSize
                imgSource: "qrc:/inputoutput.svg"
                checkable: true
                exclusiveGroup: ioInputGroup
                tooltip: qsTr("Show the universe input sources")
                onToggled:
                {
                    if (checked == true)
                        editorSource = "qrc:/PluginsList.qml"
                    animatePanel(checked);
                }
            }

            IconButton
            {
                id: uniProfilesButton
                z: 2
                visible: showPluginsButton
                width: iconSize
                height: iconSize
                imgSource: ""
                checkable: true
                exclusiveGroup: ioInputGroup
                tooltip: qsTr("Show the universe input profiles")
                onToggled:
                {
                    if (checked == true)
                        editorSource = "qrc:/ProfilesList.qml"
                    animatePanel(checked);
                }

                RobotoText
                {
                    anchors.centerIn: parent
                    label: "P"
                    fontSize: 18
                    fontBold: true
                }
            }
        }
    }

    PropertyAnimation
    {
        id: animateOpen
        target: leftSidePanel
        properties: "width"
        to: expandedWidth
        duration: 200
    }

    PropertyAnimation
    {
        id: animateClose;
        target: leftSidePanel
        properties: "width"
        to: collapseWidth
        duration: 200
    }

    Rectangle
    {
        id: gradientBorder
        y: width
        x: parent.width - height
        height: collapseWidth
        color: "#141414"
        width: parent.height
        transformOrigin: Item.TopLeft
        rotation: 270
        gradient: Gradient
        {
            GradientStop { position: 0; color: "#141414" }
            GradientStop { position: 0.213; color: "#232323" }
            GradientStop { position: 0.79; color: "#232323" }
            GradientStop { position: 1; color: "#141414" }
        }

        MouseArea
        {
            id: lpClickArea
            anchors.fill: parent
            z: 1
            x: parent.width - width
            hoverEnabled: true
            cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
            drag.target: leftSidePanel
            drag.axis: Drag.XAxis
            drag.minimumX: collapseWidth

            onPositionChanged:
            {
                if (drag.active == true)
                {
                    var obj = mapToItem(null, mouseX, mouseY);
                    leftSidePanel.width = obj.x + (collapseWidth / 2);
                    //console.log("mouseX:", mouseX, "mapToItem().x:", obj.x);
                }
            }

            //onClicked: animatePanel("")
        }
    }
}
