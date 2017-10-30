#include "sms.h"
#include <QFile>
#include <QDebug>

SMS::SMS(QString ip, unsigned short port, QString code, QObject *parent) :
    QObject(parent),
    m_fd(),
    m_ip(ip),
    m_port(port),
    m_code(code),
    m_seq(0),
    m_heartTimer()
{
    m_fd.connectToHost(m_ip, m_port);
    connect(&m_fd, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&m_fd, SIGNAL(readyRead()), this, SLOT(onRead()));
    connect(&m_fd, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

    connect(&m_heartTimer, SIGNAL(timeout()), this, SLOT(onHeartbeatTimeout()));
    m_heartTimer.start(10000);
}

SMS::~SMS()
{
    m_fd.close();
}

void SMS::writeFrame(QByteArray frame)
{
    if (m_fd.isWritable())
    {
        VideoFrameHeader vHeader;
        QByteArray realFrame("");
        QByteArray h264Flag("\x00\x00\x00\x01", 4);
        int type = frame.at(frame.indexOf(h264Flag) + h264Flag.size()) & 0x1f;

        switch (type)
        {
        case 2:
        case 5:
        case 7:
        case 8:
            vHeader.m_nVHeaderFlag = SMS_VIDEO_I_FRAME;
            break;
        case 1:
            vHeader.m_nVHeaderFlag = SMS_VIDEO_P_FRAME;
            m_bfirstIFrame = true;
            break;
        default:
            QFile file("test.h264");
            if (file.open(QIODevice::WriteOnly))
            {
                file.write(frame);
            }
            return ;
            break;
        }

        if(!m_bfirstIFrame)
        {
            return;
        }
        if (vHeader.m_nVHeaderFlag != SMS_VIDEO_I_FRAME && m_fd.bytesToWrite() > 2*1024*1024)
        {
            qDebug()<< "drop frame" << type << m_fd.bytesToWrite();
            return ;
        }

        vHeader.m_nVFrameLen = frame.size();
        vHeader.m_u8Hour = QTime::currentTime().hour();
        vHeader.m_u8Minute = QTime::currentTime().minute();
        vHeader.m_u8Sec = QTime::currentTime().second();
        vHeader.m_u8Flag = 0;
        vHeader.m_nReserved = 0;
        vHeader.m_lVPts = QDateTime::currentDateTime().toMSecsSinceEpoch() * 1000;

        realFrame.append((char*)&vHeader, sizeof(vHeader));
        realFrame.append(frame);

        int slic_cnt = (realFrame.size() + 999) / 1000;
        int j = 0;
        for (int i = 0, len = 1000; i < realFrame.size(); i += len)
        {
            SMS_HEADER mHeader;
            QByteArray msg;

            if (realFrame.size() - i < 1000)
            {
                len = realFrame.size() - i;
            }
            mHeader.magic = qToBigEndian((unsigned int)0xFFFF559F);
            mHeader.channel_id = qToBigEndian((unsigned char)0);
            mHeader.cmd = qToBigEndian((unsigned int)31000);
            mHeader.cmd_type = qToBigEndian((short)0);
            mHeader.length = qToBigEndian((unsigned int)len);
            mHeader.seqnum = slic_cnt;
            mHeader.seqnum |= j++ << 16;
            mHeader.seqnum = qToBigEndian(mHeader.seqnum);
            msg.append((char*)&mHeader, sizeof(mHeader));
            msg.append(realFrame.mid(i, len));

            m_fd.write(msg);
        }
    }
}

void SMS::onConnected()
{
    SMS_HEADER header;
    QByteArray msg;

    header.magic = qToBigEndian((unsigned int)0xFFFF559F);
    header.channel_id = qToBigEndian((unsigned char)0);
    header.cmd_type = qToBigEndian((short)1);
    header.cmd = qToBigEndian((unsigned int)21000);
    header.seqnum = qToBigEndian(m_seq++);
    header.length = qToBigEndian((unsigned int)32);

    msg.append((char*)&header, sizeof(header));
    msg.append(m_code.toLatin1().data(), 32);

    m_fd.write(msg);
}

void SMS::onRead()
{
    QByteArray msg;
    SMS_HEADER *header;

    msg = m_fd.readAll();
    header = (SMS_HEADER*)msg.data();

    switch (qFromBigEndian(header->cmd))
    {
    case 21100:
        onHeartbeatTimeout();
        break;
    case 21101:
        break;
    default:
        break;
    }
}

void SMS::onHeartbeatTimeout()
{
    SMS_HEADER header;
    QByteArray msg;

    header.magic = qToBigEndian((unsigned int)0xFFFF559F);
    header.cmd = qToBigEndian((unsigned int)21001);
    header.cmd_type = qToBigEndian((short)1);
    header.length = qToBigEndian((unsigned int)0);
    header.seqnum = qToBigEndian(m_seq++);
    msg.append((char*)&header, sizeof(header));

    if (m_fd.isWritable())
    {
        m_fd.write(msg);
    }
}

void SMS::onError(QAbstractSocket::SocketError err)
{
    m_fd.close();
}
