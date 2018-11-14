/*
  Q Light Controller Plus
  LightEntity.qml

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

import QtQuick 2.7

import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Extras 2.0

import "Math3DView.js" as Math3D
import "."

Entity
{
    id: beamEntity

    property int headIndex
    property real dimmerValue: 0
    property real shutterValue: 0
    property real lightIntensity: dimmerValue * shutterValue

    property color lightColor: Qt.rgba(0, 0, 0, 1)
    property vector3d lightPos: Qt.vector3d(0, 0, 0)
    property vector3d lightDir: Qt.vector3d(0, 0, 0)

    property real raymarchSteps
    property real cutoffAngle
    property real distCutoff
    property real headLength
    property real coneBottomRadius
    property real coneTopRadius
    property real tiltRotation
    property real panRotation: 0
    property Texture2D goboTexture
    property real goboRotation: 0

    readonly property Layer spotlightShadingLayer: Layer { }
    readonly property Layer outputDepthLayer: Layer { }
    readonly property Layer spotlightScatteringLayer: Layer { }

    /* ********************** Light matrices ********************** */
    property matrix4x4 lightMatrix
    property matrix4x4 lightViewMatrix:
        Math3D.getLightViewMatrix(lightMatrix, 0, tiltRotation, lightPos)
    property matrix4x4 lightProjectionMatrix:
        Math3D.getLightProjectionMatrix(distCutoff, coneBottomRadius, coneTopRadius, headLength, cutoffAngle)
    property matrix4x4 lightViewProjectionMatrix: lightProjectionMatrix.times(lightViewMatrix)
    property matrix4x4 lightViewProjectionScaleAndOffsetMatrix:
        Math3D.getLightViewProjectionScaleOffsetMatrix(lightViewProjectionMatrix)

    property Transform headTransform: Transform { translation: Qt.vector3d(0.1 * headIndex, 0, 0) }

    function setupScattering(shadingEffect, scatteringEffect, depthEffect, sceneEntity)
    {
        shadingCone.coneEffect = shadingEffect
        shadingCone.parent = sceneEntity
        shadingCone.spotlightConeMesh = sceneEntity.coneMesh

        //scatteringCone.coneEffect = scatteringEffect
        scatteringCone.parent = sceneEntity
        scatteringCone.spotlightConeMesh = sceneEntity.coneMesh

        outDepthCone.coneEffect = depthEffect
        outDepthCone.parent = sceneEntity
        outDepthCone.spotlightConeMesh = sceneEntity.coneMesh
    }

    property RenderTarget shadowMap:
        RenderTarget
        {
            property alias depth: depthAttachment

            attachments: [
                RenderTargetOutput
                {
                    attachmentPoint: RenderTargetOutput.Depth
                    texture:
                        Texture2D
                        {
                            id: depthAttachment
                            width: 1024
                            height: 1024
                            format: Texture.D32F
                            generateMipMaps: false
                            magnificationFilter: Texture.Linear
                            minificationFilter: Texture.Linear
                            wrapMode
                            {
                                x: WrapMode.ClampToEdge
                                y: WrapMode.ClampToEdge
                            }
                        }
                }
            ] // attachments
        }

    /* Cone meshes used for scattering. These get re-parented to
       the main Scene entity via setupScattering */
    SpotlightConeEntity
    {
        id: shadingCone
        coneLayer: spotlightShadingLayer
        fxEntity: beamEntity
    }
    SpotlightConeEntity
    {
        id: scatteringCone
        coneLayer: spotlightScatteringLayer
        fxEntity: beamEntity
    }
    SpotlightConeEntity
    {
        id: outDepthCone
        coneLayer: outputDepthLayer
        fxEntity: beamEntity
    }

    components: [
        headTransform
    ]
} // Entity
