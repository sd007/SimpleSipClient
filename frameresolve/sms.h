#ifndef SMS_H
#define SMS_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QtEndian>
#include <QDateTime>
//#include "oeasycommon.h"

typedef struct sms_msg_hearder
{
    unsigned int    magic;              /* 特殊头 ，固定定义为0xFFFF559F*/
    unsigned char   channel_id;         /* channelId通道号0,1,2,3…*/
    unsigned char   flag;               /* 保留字段*/
    short           cmd_type;           /* 包类型： 0: 音视频数据包，1:指令包*/
    unsigned int    cmd;                /* 指令字*/
    unsigned int    seqnum;             /* 流水号 ，从0开始递增+1*/
    unsigned int    length;             /* 包体长度 */
}SMS_HEADER;

#define SMS_VIDEO_I_FRAME   0x63643035  //I帧标识
#define SMS_VIDEO_P_FRAME   0x63643135  //P帧标识
#define SMS_AUDIO_FRAME     0x62773135  //音频帧标识

typedef struct
{
    unsigned int    m_nVHeaderFlag; // 帧标识，VIDEO_I_FRAME (I帧), VIDEO_P_FRAME（p帧）
    unsigned int    m_nVFrameLen;   // 帧的长度
    unsigned char   m_u8Hour;
    unsigned char   m_u8Minute;
    unsigned char   m_u8Sec;
    unsigned char   m_u8Flag;       //保留
    unsigned int    m_nReserved;
    long long       m_lVPts;		// 时间戳单位微秒
}VideoFrameHeader;

class SMS : public QObject
{
    Q_OBJECT
public:
    explicit SMS(QString ip, unsigned short  port, QString code, QObject *parent = 0);
    ~SMS();
    QTcpSocket m_fd;

private:
    QString m_ip;
    int m_port;
    QString m_code;
    unsigned int m_seq;
    QTimer m_heartTimer;
    bool m_bfirstIFrame = false;

public slots:
    void writeFrame(QByteArray frame);
    void onConnected();
    void onRead();
    void onHeartbeatTimeout();
    void onError(QAbstractSocket::SocketError);
};

#endif // SMS_H
