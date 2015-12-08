/*
  Q Light Controller Plus
  artnetcontroller.cpp

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

#include "artnetcontroller.h"

#include <QMutexLocker>
#include <QDebug>
#include <QMessageBox>

#define TRANSMIT_FULL    "Full"
#define TRANSMIT_PARTIAL "Partial"

ArtNetController::ArtNetController(QNetworkInterface interface, QNetworkAddressEntry address,
                                   quint32 line, QObject *parent)
    : QObject(parent)
    , m_interface(interface)
    , m_address(address)
    , m_ipAddr(address.ip())
    , m_packetSent(0)
    , m_packetReceived(0)
    , m_line(line)
    , m_udpSocket(new QUdpSocket(this))
    , m_udpSocketBroadcastRecv(NULL)
    , m_packetizer(new ArtNetPacketizer())
    , m_pollTimer(NULL)
{
    if (m_ipAddr == QHostAddress::LocalHost)
    {
        m_broadcastAddr = QHostAddress::LocalHost;
        m_MACAddress = "11:22:33:44:55:66";
    }
    else
    {
        m_broadcastAddr = address.broadcast();
        m_MACAddress = interface.hardwareAddress();
    }

    qDebug() << "[ArtNetController] IP Address:" << m_ipAddr.toString() << " Broadcast address:" << m_broadcastAddr.toString() << "(MAC:" << m_MACAddress << ")";

    bool bindError = false;

    if (m_udpSocket->bind(m_ipAddr, ARTNET_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
    {
        connect(m_udpSocket, SIGNAL(readyRead()),
                this, SLOT(slotReadyRead()));
    }
    else
    {
        qWarning() << "ArtNet: could not bind socket to address" << QString("%1:%2").arg(m_ipAddr.toString()).arg(ARTNET_PORT);
        bindError = true;
    }

// Binding to the interface adress is sufficient to receive broadcasts on windows
#if !(defined(WIN32) || defined(Q_OS_WIN))
    if (m_ipAddr != QHostAddress::LocalHost)
    {
        m_udpSocketBroadcastRecv = new QUdpSocket(this);
        if (m_udpSocketBroadcastRecv->bind(m_broadcastAddr, ARTNET_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
        {
            connect(m_udpSocketBroadcastRecv, SIGNAL(readyRead()),
                    this, SLOT(slotReadyRead()));
        }
        else
        {
            qWarning() << "ArtNet: could not bind socket to address" << QString("%1:%2").arg(m_broadcastAddr.toString()).arg(ARTNET_PORT);
            bindError = true;
        }
    }
#endif

    if (bindError)
        QMessageBox::warning(NULL, tr("Socket error"),
                tr("ArtNet: Could not bind to ArtNet port (%1).\n"
                   "It may be already in use by another program.").arg(ARTNET_PORT));
}

ArtNetController::~ArtNetController()
{
    qDebug() << Q_FUNC_INFO;
    qDeleteAll(m_dmxValuesMap);
}

ArtNetController::Type ArtNetController::type()
{
    int type = Unknown;
    foreach(UniverseInfo info, m_universeMap.values())
    {
        type |= info.type;
    }

    return Type(type);
}

quint32 ArtNetController::line()
{
    return m_line;
}

quint64 ArtNetController::getPacketSentNumber()
{
    return m_packetSent;
}

quint64 ArtNetController::getPacketReceivedNumber()
{
    return m_packetReceived;
}

QString ArtNetController::getNetworkIP()
{
    return m_ipAddr.toString();
}

QHash<QHostAddress, ArtNetNodeInfo> ArtNetController::getNodesList()
{
    return m_nodesList;
}

void ArtNetController::addUniverse(quint32 universe, ArtNetController::Type type)
{
    qDebug() << "[ArtNet] addUniverse - universe" << universe << ", type" << type;
    if (m_universeMap.contains(universe))
    {
        m_universeMap[universe].type |= (int)type;
    }
    else
    {
        UniverseInfo info;
        info.inputUniverse = universe;
        info.outputAddress = m_broadcastAddr;
        info.outputUniverse = universe;
        info.outputTransmissionMode = Full;
        info.type = type;
        m_universeMap[universe] = info;
    }

    // send Polls if we open an Output
    if (type == Output && m_pollTimer == NULL)
    {
        sendPoll();

        m_pollTimer = new QTimer(this);
        m_pollTimer->setInterval(5000);
        m_pollTimer->setSingleShot(false);
        connect(m_pollTimer, SIGNAL(timeout()),
                this, SLOT(slotPollTimeout()));
        m_pollTimer->start();
    }
}

void ArtNetController::removeUniverse(quint32 universe, ArtNetController::Type type)
{
    if (m_universeMap.contains(universe))
    {
        if (m_universeMap[universe].type == type)
            m_universeMap.take(universe);
        else
            m_universeMap[universe].type &= ~type;

        if (type == Output && ((this->type() | Output) == 0))
        {
            delete m_pollTimer;
            m_pollTimer = NULL;
        }
    }
}

void ArtNetController::setInputUniverse(quint32 universe, quint32 artnetUni)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].inputUniverse = artnetUni;
}

void ArtNetController::setOutputIPAddress(quint32 universe, QString address)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    QStringList iFaceIP = m_ipAddr.toString().split(".");
    QStringList addList = address.split(".");

    for (int i = 0; i < addList.count(); i++)
        iFaceIP.replace(4 - addList.count() + i , addList.at(i));

    QString newIP = iFaceIP.join(".");
    qDebug() << "[setOutputIPAddress] transmit to IP: " << newIP;

    m_universeMap[universe].outputAddress = QHostAddress(newIP);
}

void ArtNetController::setOutputUniverse(quint32 universe, quint32 artnetUni)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].outputUniverse = artnetUni;
}

void ArtNetController::setTransmissionMode(quint32 universe, ArtNetController::TransmissionMode mode)
{
    if (m_universeMap.contains(universe) == false)
        return;

    QMutexLocker locker(&m_dataMutex);
    m_universeMap[universe].outputTransmissionMode = int(mode);
}

QString ArtNetController::transmissionModeToString(ArtNetController::TransmissionMode mode)
{
    switch (mode)
    {
        default:
        case Full:
            return QString(TRANSMIT_FULL);
        break;
        case Partial:
            return QString(TRANSMIT_PARTIAL);
        break;
    }
}

ArtNetController::TransmissionMode ArtNetController::stringToTransmissionMode(const QString &mode)
{
    if (mode == QString(TRANSMIT_PARTIAL))
        return Partial;
    else
        return Full;
}

QList<quint32> ArtNetController::universesList()
{
    return m_universeMap.keys();
}

UniverseInfo *ArtNetController::getUniverseInfo(quint32 universe)
{
    if (m_universeMap.contains(universe))
        return &m_universeMap[universe];

    return NULL;
}

void ArtNetController::sendDmx(const quint32 universe, const QByteArray &data)
{
    QMutexLocker locker(&m_dataMutex);
    QByteArray dmxPacket;
    QHostAddress outAddress = m_broadcastAddr;
    quint32 outUniverse = universe;
    TransmissionMode transmitMode = Full;

    if (m_universeMap.contains(universe))
    {
        UniverseInfo info = m_universeMap[universe];
        outAddress = info.outputAddress;
        outUniverse = info.outputUniverse;
        transmitMode = TransmissionMode(info.outputTransmissionMode);
    }

    if (transmitMode == Full)
    {
        QByteArray wholeuniverse(512, 0);
        wholeuniverse.replace(0, data.length(), data);
        m_packetizer->setupArtNetDmx(dmxPacket, outUniverse, wholeuniverse);
    }
    else
        m_packetizer->setupArtNetDmx(dmxPacket, outUniverse, data);

    qint64 sent = m_udpSocket->writeDatagram(dmxPacket.data(), dmxPacket.size(),
                                             outAddress, ARTNET_PORT);
    if (sent < 0)
    {
        qDebug() << "sendDmx failed";
        qDebug() << "Errno: " << m_udpSocket->error();
        qDebug() << "Errmgs: " << m_udpSocket->errorString();
    }
    else
        m_packetSent++;
}

bool ArtNetController::handleArtNetPollReply(QByteArray const& datagram, QHostAddress const& senderAddress)
{
    ArtNetNodeInfo newNode;
    if (!m_packetizer->fillArtPollReplyInfo(datagram, newNode))
    {
        qDebug() << "[ArtNet] Bad ArtPollReply received";
        return false;
    }

    qDebug() << "[ArtNet] ArtPollReply received";

    if (m_nodesList.contains(senderAddress) == false)
        m_nodesList[senderAddress] = newNode;
    ++m_packetReceived;
    return true;
}

bool ArtNetController::handleArtNetPoll(QByteArray const& datagram, QHostAddress const& senderAddress)
{
    Q_UNUSED(datagram);

    qDebug() << "[ArtNet] ArtPoll received";
    QByteArray pollReplyPacket;
    m_packetizer->setupArtNetPollReply(pollReplyPacket, m_ipAddr, m_MACAddress);
    m_udpSocket->writeDatagram(pollReplyPacket.data(), pollReplyPacket.size(),
            senderAddress, ARTNET_PORT);
    ++m_packetSent;
    ++m_packetReceived;
    return true;
}

bool ArtNetController::handleArtNetDmx(QByteArray const& datagram, QHostAddress const& senderAddress)
{
    Q_UNUSED(senderAddress);

    QByteArray dmxData;
    quint32 artnetUniverse;
    if (!m_packetizer->fillDMXdata(datagram, dmxData, artnetUniverse))
    {
        qDebug() << "[ArtNet] Bad DMX packet received";
        return false;
    }

    qDebug() << "[ArtNet] DMX data received. Universe:" << artnetUniverse << ", Data size:" << dmxData.size()
        << ", data[0]=" << (int)dmxData[0]
        << ", from=" << senderAddress.toString();

    for (QMap<quint32, UniverseInfo>::iterator it = m_universeMap.begin(); it != m_universeMap.end(); ++it)
    {
        quint32 universe = it.key();
        UniverseInfo const& info = it.value();

        if ((info.type & Input) && info.inputUniverse == artnetUniverse)
        {
            QByteArray *dmxValues;
            if (m_dmxValuesMap.contains(universe) == false)
                m_dmxValuesMap[universe] = new QByteArray(512, 0);
            dmxValues = m_dmxValuesMap[universe];

            qDebug() << "[ArtNet] -> universe" << (universe + 1);

            for (int i = 0; i < dmxData.length(); i++)
            {
                if (dmxValues->at(i) != dmxData.at(i))
                {
                    qDebug() << "[ArtNet] a value differs";
                    dmxValues->replace(i, 1, (const char *)(dmxData.data() + i), 1);
                    emit valueChanged(universe, m_line, i, (uchar)dmxData.at(i));
                }
            }
            ++m_packetReceived;
            return true;
        }
    }
    return false;
}

bool ArtNetController::handlePacket(QByteArray const& datagram, QHostAddress const& senderAddress)
{
    //qDebug() << "Received packet with size: " << datagram.size() << ", host: " << senderAddress.toString();
    int opCode = -1;
    if (m_packetizer->checkPacketAndCode(datagram, opCode) == true)
    {
        switch (opCode)
        {
            case ARTNET_POLLREPLY:
                return handleArtNetPollReply(datagram, senderAddress);
            case ARTNET_POLL:
                return handleArtNetPoll(datagram, senderAddress);
            case ARTNET_DMX:
                return handleArtNetDmx(datagram, senderAddress);
            default:
                qDebug() << "[ArtNet] opCode not supported yet (" << opCode << ")";
                break;
        }
    }
    else
        qDebug() << "[ArtNet] Malformed packet received";

    return false;
}

void ArtNetController::slotReadyRead()
{
    QUdpSocket* udpSocket = qobject_cast<QUdpSocket*>(sender());
    Q_ASSERT(udpSocket != NULL);

    QByteArray datagram;
    QHostAddress senderAddress;
    while (udpSocket->hasPendingDatagrams())
    {
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress);
        handlePacket(datagram, senderAddress);
    }
}

void ArtNetController::slotPollTimeout()
{
    sendPoll();
}

void ArtNetController::sendPoll()
{
    QByteArray pollPacket;
    m_packetizer->setupArtNetPoll(pollPacket);
    qint64 sent = m_udpSocket->writeDatagram(pollPacket.data(), pollPacket.size(),
            m_broadcastAddr, ARTNET_PORT);
    if (sent < 0)
        qWarning() << "Unable to send Poll packet: errno=" << m_udpSocket->error() << "(" << m_udpSocket->errorString() << ")";
    else
        m_packetSent++;
}
