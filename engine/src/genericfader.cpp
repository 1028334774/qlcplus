/*
  Q Light Controller
  genericfader.cpp

  Copyright (c) Heikki Junnila

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

#include <cmath>
#include <QDebug>

#include "genericfader.h"
#include "fadechannel.h"
#include "doc.h"

GenericFader::GenericFader()
    : m_intensity(1.0)
    , m_paused(false)
    , m_enabled(true)
    , m_fadeOut(false)
    , m_blendMode(Universe::NormalBlend)
{
}

GenericFader::~GenericFader()
{
}

void GenericFader::add(const FadeChannel& ch)
{
    quint32 hash = (ch.fixture() << 16) | ch.channel();

    QHash<quint32,FadeChannel>::iterator channelIterator = m_channels.find(hash);
    if (channelIterator != m_channels.end())
    {
        // perform a HTP check
        if (channelIterator.value().current() <= ch.current())
            channelIterator.value() = ch;
    }
    else
    {
        m_channels.insert(hash, ch);
    }
}

void GenericFader::replace(const FadeChannel &ch)
{
    quint32 hash = (ch.fixture() << 16) | ch.channel();
    m_channels.insert(hash, ch);
}

void GenericFader::remove(const FadeChannel& ch)
{
    quint32 hash = (ch.fixture() << 16) | ch.channel();
    m_channels.remove(hash);
}

void GenericFader::removeAll()
{
    m_channels.clear();
}

FadeChannel *GenericFader::getChannelFader(const Doc *doc, Universe *universe, quint32 fixtureID, quint32 channel)
{
    quint32 hash = (fixtureID << 16) | channel;
    QHash<quint32,FadeChannel>::iterator channelIterator = m_channels.find(hash);
    if (channelIterator != m_channels.end())
        return &channelIterator.value();

    FadeChannel fc(doc, fixtureID, channel);
    if (fc.type() & QLCChannel::Intensity)
        fc.setStart(0); // Intensity channels must start at zero
    else
        fc.setStart(universe->preGMValue(fc.address()));

    m_channels[hash] = fc;
    return &m_channels[hash];
}

const QHash<quint32, FadeChannel> &GenericFader::channels() const
{
    return m_channels;
}

void GenericFader::write(Universe *universe)
{
    QMutableHashIterator <quint32,FadeChannel> it(m_channels);
    while (it.hasNext() == true)
    {
        FadeChannel& fc(it.next().value());
        int channelType = fc.type();
        quint32 address = fc.addressInUniverse();
        uchar value;

        // Calculate the next step
        if (m_paused)
            value = fc.current();
        else
            value = fc.nextStep(MasterTimer::tick());

        // Apply intensity to HTP channels
        if ((channelType & FadeChannel::Intensity) && fc.canFade())
            value = fc.current(intensity());

        //qDebug() << "[GenericFader] >>> uni:" << universe->id() << ", address:" << address << ", value:" << value;
        if (channelType & FadeChannel::Relative)
            universe->writeRelative(address, value);
        else
            universe->writeBlended(address, value, m_blendMode);

        if ((channelType & FadeChannel::Intensity) && m_blendMode == Universe::NormalBlend)
        {
            // Remove all HTP channels that reach their target _zero_ value.
            // They have no effect either way so removing them saves CPU a bit.
            if (fc.current() == 0 && fc.target() == 0)
            {
                it.remove();
                continue;
            }
        }
/*
        else
        {
            // Remove all LTP channels after their time is up
            if (fc.elapsed() >= fc.fadeTime())
                it.remove();
        }
*/
        if (channelType & FadeChannel::Flashing)
            it.remove();
    }
}

qreal GenericFader::intensity() const
{
    return m_intensity;
}

void GenericFader::adjustIntensity(qreal fraction)
{
    m_intensity = fraction;
}

bool GenericFader::isPaused() const
{
    return m_paused;
}

void GenericFader::setPaused(bool paused)
{
    m_paused = paused;
}

bool GenericFader::isEnabled() const
{
    return m_enabled;
}

void GenericFader::setEnabled(bool enable)
{
    m_enabled = enable;
}

bool GenericFader::isFadingOut() const
{
    return m_fadeOut;
}

void GenericFader::setFadeOut(bool enable, uint fadeTime)
{
    m_fadeOut = enable;

    if (fadeTime)
    {
        QMutableHashIterator <quint32,FadeChannel> it(m_channels);
        while (it.hasNext() == true)
        {
            FadeChannel& fc(it.next().value());
            int channelType = fc.type();

            if ((channelType & FadeChannel::Intensity) == 0)
                continue;

            fc.setStart(fc.current());
            fc.setElapsed(0);
            fc.setReady(false);

            if (fc.canFade() == false)
            {
                fc.setFadeTime(0);
            }
            else
            {
                fc.setFadeTime(fadeTime);
                fc.setTarget(0);
            }
        }
    }
}

void GenericFader::setBlendMode(Universe::BlendMode mode)
{
    m_blendMode = mode;
}
