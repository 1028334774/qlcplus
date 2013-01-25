/*
  Q Light Controller
  artnetnode.cpp

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "artnetnode.h"

#include <QDebug>

ArtNetNode::ArtNetNode(QString ipaddr, int port, QObject *parent)
    : QObject(parent)
{
    // calculate the broadcast address
    /*
    QNetworkAddressEntry *tmpBcast = new QNetworkAddressEntry();
    tmpBcast->setIp(QHostAddress(m_ipAddr));
    tmpBcast->setNetmask(QHostAddress("255.255.255.0"));
    m_broadcastAddr = tmpBcast->broadcast();
    */
    m_ipAddr = QHostAddress(ipaddr);
    quint32 ip = m_ipAddr.toIPv4Address();
    quint32 mask = QHostAddress("255.255.255.0").toIPv4Address(); // will it work in all cases ?
    quint32 broadcast = (ip & mask) | (0xFFFFFFFFU & ~mask);
    m_broadcastAddr = QHostAddress(broadcast);

    qDebug() << Q_FUNC_INFO << "Broadcast address: " << m_broadcastAddr.toString();
    m_packetizer = new ArtNetPacketizer();

    m_UdpSocket = new QUdpSocket(this);
    if (m_UdpSocket->bind(m_broadcastAddr, ARTNET_DEFAULT_PORT, QUdpSocket::ShareAddress) == false)
        return;

    connect(m_UdpSocket, SIGNAL(readyRead()),
            this, SLOT(processPendingPackets()));

    QByteArray pollPacket;
    m_packetizer->setupArtNetPoll(pollPacket);
    m_UdpSocket->writeDatagram(pollPacket.data(), pollPacket.size(),
                               m_broadcastAddr, ARTNET_DEFAULT_PORT);

    addPort(port);
}

ArtNetNode::~ArtNetNode()
{
    m_UdpSocket->close();
}

void ArtNetNode::sendDmx(const int &universe, const QByteArray &data)
{
    QByteArray dmxPacket;
    m_packetizer->setupArtNetDmx(dmxPacket, universe, data);
    m_UdpSocket->writeDatagram(dmxPacket.data(), dmxPacket.size(),
                               m_broadcastAddr, ARTNET_DEFAULT_PORT);
}

void ArtNetNode::addPort(int port)
{
    if (m_ports.contains(port) == false)
        m_ports.append(port);
}

QString ArtNetNode::getNetworkIP()
{
    return m_ipAddr.toString();
}

void ArtNetNode::processPendingPackets()
{
    while (m_UdpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        QHostAddress senderAddress;
        datagram.resize(m_UdpSocket->pendingDatagramSize());
        m_UdpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress);
        if (senderAddress != m_ipAddr)
        {
            qDebug() << "Received packet with size: " << datagram.size() << ", host: " << senderAddress.toString();
            int opCode = -1;
            if (m_packetizer->checkPacketAndCode(datagram, opCode) == true)
            {
                switch (opCode)
                {
                    case ARTNET_POLLREPLY:
                    {
                        qDebug() << "Received and ArtPollReply";
                        ArtNetNodeInfo newNode;
                        if (m_packetizer->fillArtPollReplyInfo(datagram, newNode) == true)
                        {
                            if (m_nodesList.contains(senderAddress) == false)
                                m_nodesList[senderAddress] = newNode;
                        }
                    }
                    break;
                    default:
                        qDebug() << "opCode not supported yet (" << opCode << ")";
                    break;
                }
            }
        }
     }
}
