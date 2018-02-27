/*
  Q Light Controller Plus
  qlccapability.h

  Copyright (C) Heikki Junnila
                Massimo Callegari

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

#ifndef QLCCAPABILITY_H
#define QLCCAPABILITY_H

#include <QObject>
#include <climits>
#include <QColor>
#include <QList>

class QXmlStreamReader;
class QXmlStreamWriter;
class QLCCapability;
class QString;
class QFile;

/** @addtogroup engine Engine
 * @{
 */

#define KXMLQLCCapability    "Capability"
#define KXMLQLCCapabilityMin "Min"
#define KXMLQLCCapabilityMax "Max"
#define KXMLQLCCapabilityPreset "Preset"
#define KXMLQLCCapabilityRes1 "Res1"
#define KXMLQLCCapabilityRes2 "Res2"

/** ****************** LEGACY ***************** */
#define KXMLQLCCapabilityResource "Res"
#define KXMLQLCCapabilityColor1 "Color"
#define KXMLQLCCapabilityColor2 "Color2"



/**
 * QLCCapability represents one value range with a special meaning in a
 * QLCChannel. For example, a sunburst gobo might be set on a "gobo" channel
 * with any DMX value between 15 and 25. This is represented as a
 * QLCCapability, whose min == 15, max == 25 and name == "Sunburst". Single
 * values can be represented by setting the same value to both, for example:
 * min == 15 and max == 15.
 */
class QLCCapability: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(int min READ min CONSTANT)
    Q_PROPERTY(int max READ max CONSTANT)
    Q_PROPERTY(QVariantList resources READ resources CONSTANT)

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /** Default constructor */
    QLCCapability(uchar min = 0, uchar max = UCHAR_MAX,
                  const QString& name = QString(), QObject *parent = 0);

    QLCCapability *createCopy();

    /** Destructor */
    ~QLCCapability();

    /** Assignment operator */
    QLCCapability& operator=(const QLCCapability& capability);

    /** Comparing operator for qSort */
    bool operator<(const QLCCapability& capability) const;

    /********************************************************************
     * Preset
     ********************************************************************/
public:
    enum Preset
    {
        Custom = 0,
        SlowToFast,
        FastToSlow,
        NearToFar,
        FarToNear,
        BigToSmall,
        SmallToBig,
        ShutterOpen,
        ShutterClose,
        StrobeSlowToFast,
        StrobeFastToSlow,
        StrobeRandomSlowToFast,
        StrobeRandomFastToSlow,
        PulseSlowToFast,
        PulseFastToSlow,
        StrobeFreq, // precise frequency value in hertz specified in m_resource
        StrobeFreqRange, // specified in m_resource as "min-max" hertz
        PulseFreq,
        PulseFreqRange,
        RotationClockwise,
        RotationCounterClockwise,
        TiltContinuous,
        ColorMacro,
        ColorDoubleMacro,
        ColorWheelIndex,
        GoboMacro,
        GenericPicture,
        Alias,
        LastPreset // dummy for cycles
    };
#if QT_VERSION >= 0x050500
    Q_ENUM(Preset)
#else
    Q_ENUMS(Preset)
#endif

    enum PresetType
    {
        None,
        SingleColor,
        DoubleColor,
        SingleValue,
        DoubleValue,
        Picture
    };

    static QString presetToString(Preset preset);
    static Preset stringToPreset(const QString &preset);

    Preset preset() const;
    void setPreset(Preset preset);

    /** Return the type of the current preset.
     *  This is useful for the UI to understand the type
     *  of resources this capability is exposing */
    PresetType presetType() const;

protected:
    Preset m_preset;

    /********************************************************************
     * Properties
     ********************************************************************/
public:
    uchar min() const;
    void setMin(uchar value);

    uchar max() const;
    void setMax(uchar value);

    uchar middle() const;

    QString name() const;
    void setName(const QString& name);

    QVariant resource(int index);
    void setResource(int index, QVariant value);

    QVariantList resources();

    /** Check, whether the given capability overlaps with this */
    bool overlaps(const QLCCapability* cap);

protected:
    uchar m_min;
    uchar m_max;
    QString m_name;
    QVariantList m_resources;

    /********************************************************************
     * Load & Save
     ********************************************************************/
public:
    /** Save the capability into a QXmlStreamWriter */
    bool saveXML(QXmlStreamWriter *doc);

    /** Load capability contents from an XML element */
    bool loadXML(QXmlStreamReader &doc);
};

/** @} */

#endif
