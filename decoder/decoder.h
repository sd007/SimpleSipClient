#ifndef DECODER_H
#define DECODER_H

#include <QObject>
#include <QImage>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
}

class Decoder : public QObject
{
    Q_OBJECT
public:
    explicit Decoder(QObject *parent = 0);
    ~Decoder();
    int init();
    int decode(char *buff, int bufflen);
private:

    AVCodec *m_avCodec;
    AVCodecContext *m_codecContext;
    //befor decode
    AVPacket *m_avpacket;
    //after decode
    AVFrame *m_avframe;
    AVFrame *m_frameRGB;
    unsigned char* m_h264Buff;
    SwsContext *m_swsContext;
    unsigned char* m_rgbBuffer;
signals:
    void sigGetOneFrame(QImage img);
public slots:
};

#endif // DECODER_H
