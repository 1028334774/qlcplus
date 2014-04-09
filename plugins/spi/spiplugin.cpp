/*
  Q Light Controller Plus
  spiplugin.cpp

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

#include <QStringList>
#include <QSettings>
#include <QString>
#include <QDebug>
#include <QFile>

#include <unistd.h>
#include <fcntl.h>

#include "spiplugin.h"
#include "spioutthread.h"
#include "spiconfiguration.h"

#define SPI_DEFAULT_DEVICE  "/dev/spidev0.0"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

SPIPlugin::~SPIPlugin()
{
    if (m_outThread != NULL)
        m_outThread->stopThread();

    if (m_spifd != -1)
        close(m_spifd);
}

void SPIPlugin::init()
{
    m_spifd = -1;
    m_outThread = NULL;
}

QString SPIPlugin::name()
{
    return QString("SPI");
}

int SPIPlugin::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Infinite;
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

void SPIPlugin::openOutput(quint32 output)
{
    if (output != 0 || m_spifd != -1)
        return;

    m_spifd = open(SPI_DEFAULT_DEVICE, O_RDWR);
    if(m_spifd < 0)
    {
        qWarning() << "Cannot open SPI device !";
        return;
    }

    m_outThread = new SPIOutThread();
    m_outThread->runThread(m_spifd);
}

void SPIPlugin::closeOutput(quint32 output)
{
    if (output != 0)
        return;

    if (m_spifd != -1)
        close(m_spifd);
    m_spifd = -1;
}

QStringList SPIPlugin::outputs()
{
    QStringList list;
    QFile file(QString(SPI_DEFAULT_DEVICE));
    if (file.exists() == true)
        list << QString("1: SPI0 CS0");
    return list;
}

QString SPIPlugin::pluginInfo()
{
    QString str;

    str += QString("<HTML>");
    str += QString("<HEAD>");
    str += QString("<TITLE>%1</TITLE>").arg(name());
    str += QString("</HEAD>");
    str += QString("<BODY>");

    str += QString("<P>");
    str += QString("<H3>%1</H3>").arg(name());
    str += tr("This plugin provides DMX output for SPI devices.");
    str += QString("</P>");

    return str;
}

void SPIPlugin::setParameter(QString name, QVariant &value)
{
    QString prop(name);

    // If property name is UniverseChannels-ID, map the channels count
    if (prop.startsWith("UniverseChannels"))
    {
        QString uniStrId = prop.split("-").at(1);
        quint32 uniID = uniStrId.toUInt();
        int chans = value.toInt();
        SPIUniverse uniStruct;
        uniStruct.m_channels = chans;

        quint32 totalChannels = 0;
        quint32 absOffset = 0;

        QHashIterator <quint32, SPIUniverse> it(m_uniChannelsMap);
        while (it.hasNext() == true)
        {
            it.next();
            quint32 mapUniID = it.key();
            if (mapUniID < uniID)
                absOffset += it.value().m_channels;

            totalChannels += it.value().m_channels;
        }
        uniStruct.m_absoluteAddress = absOffset;
        m_uniChannelsMap[uniID] = uniStruct;
        totalChannels += chans;
        qDebug() << "[SPI] universe" << uniID << "has" << uniStruct.m_channels << "and starts at" << uniStruct.m_absoluteAddress;
        m_serializedData.resize(totalChannels);
        qDebug() << "[SPI] total bytes to transmit:" << m_serializedData.size();
    }
}

QString SPIPlugin::outputInfo(quint32 output)
{
    QString str;

    if (output != QLCIOPlugin::invalidLine() && output == 0)
    {
        str += QString("<H3>%1</H3>").arg(outputs()[output]);
    }

    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

void SPIPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
    if (output != 0 || m_spifd == -1)
        return;

    if (m_serializedData.size() == 0)
    {
        m_serializedData = data;
    }
    else
    {
        SPIUniverse uniInfo = m_uniChannelsMap[universe];
        m_serializedData.replace(uniInfo.m_absoluteAddress, data.size(), data);
    }

    m_outThread->writeData(m_serializedData);
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void SPIPlugin::configure()
{
    SPIConfiguration conf(this);
    if (conf.exec() == QDialog::Accepted)
    {
        QSettings settings;
        settings.setValue("SPIPlugin/frequency", QVariant(conf.frequency()));
    }
}

bool SPIPlugin::canConfigure()
{
    return true;
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(spiplugin, SPIPlugin)
#endif
