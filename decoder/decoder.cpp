#include "decoder.h"
#include <string>
#include <stdlib.h>

#define H264BUFF_SIZE 500*1024

Decoder::Decoder(QObject *parent) : QObject(parent)
{
    m_h264Buff = (unsigned char*)malloc(H264BUFF_SIZE);
    memset(m_h264Buff, 0, H264BUFF_SIZE);
    m_swsContext = Q_NULLPTR;
}

Decoder::~Decoder()
{
    if(m_h264Buff)
    {
        free(m_h264Buff);
        m_h264Buff = NULL;
    }
    if(m_swsContext)
    {
        sws_freeContext(m_swsContext);
    }
    avformat_network_deinit();
}

int Decoder::init()
{
    /*注册CODEC*/
    avcodec_register_all();
    avformat_network_init();
    /*查找 H264 CODEC*/
    m_avCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if(!m_avCodec) return -1;

    m_codecContext = avcodec_alloc_context3(m_avCodec);
    if(!m_codecContext) return -2;

    int width = 1920;
    int height = 1080;
    //初始化参数，下面的参数应该由具体的业务决定
    m_codecContext->time_base.num = 1;
    m_codecContext->frame_number =1;
    m_codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    m_codecContext->bit_rate = 0;
    m_codecContext->time_base.den = 30;//帧率
    m_codecContext->width = width;//视频宽
    m_codecContext->height = height;//视频高

    if(avcodec_open2(m_codecContext, m_avCodec, NULL) < 0)
    {
        return -3;
    }
    m_avframe = av_frame_alloc();
    if(!m_avframe) return -4;
    m_frameRGB = av_frame_alloc();
    int numBytes = avpicture_get_size(AV_PIX_FMT_RGB32, width, height);
    m_rgbBuffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) m_frameRGB, m_rgbBuffer, AV_PIX_FMT_RGB32, width, height);
    //特别注意 img_convert_ctx 该在做H264流媒体解码时候，发现sws_getContext /sws_scale内存泄露问题，
    //注意sws_getContext只能调用一次，在初始化时候调用即可，另外调用完后，在析构函数中使用sws_free_Context，将它的内存释放。
    //设置图像转换上下文
    m_swsContext = sws_getContext(width, height, AV_PIX_FMT_YUV420P, width, height, AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);

}

int Decoder::decode(char *buff, int bufflen)
{
    AVPacket packet;
    int gotframe = 0;
    av_init_packet(&packet);
    packet.data = (unsigned char*)buff;//这里填入一个指向完整H264数据帧的指针
    packet.size = bufflen;//这个填入H264数据帧的大小
    //下面开始真正的解码
    int ret = avcodec_decode_video2(m_codecContext, m_avframe, &gotframe, &packet);
    if(gotframe && ret > 0)//成功解码
    {
        int picSize = m_codecContext->height * m_codecContext->width;
        int newSize = picSize * 1.5;

        //申请内存
        unsigned char *buf = new unsigned char[newSize];
        int height = m_avframe->height;
        int width = m_avframe->width;
        //写入数据
        int a=0,i;
        for (i=0; i<height; i++)
        {
            memcpy(buf+a,m_avframe->data[0] + i * m_avframe->linesize[0], width);
            a+=width;
        }
        for (i=0; i<height/2; i++)
        {
            memcpy(buf+a,m_avframe->data[1] + i * m_avframe->linesize[1], width/2);
            a+=width/2;
        }
        for (i=0; i<height/2; i++)
        {
            memcpy(buf+a,m_avframe->data[2] + i * m_avframe->linesize[2], width/2);
            a+=width/2;
        }
        //到这里，buf里面已经是yuv420p的数据了，可以对它做任何的处理拉！
        avpicture_fill((AVPicture *) m_avframe, (uint8_t *)buf, AV_PIX_FMT_YUV420P, width, height);//这里的长度和高度跟之前保持一致
           //转换图像格式，将解压出来的YUV420P的图像转换为RGB的图像
        sws_scale(m_swsContext,
               (uint8_t const * const *) m_avframe->data,
               m_avframe->linesize, 0, height, m_frameRGB->data,
               m_frameRGB->linesize);
        delete [] buf;
        //把这个RGB数据 用QImage加载
         QImage tmpImg((uchar *)m_rgbBuffer, width, height,QImage::Format_RGB32);
         QImage image = tmpImg.copy(); //把图像复制一份 传递给界面显示
         emit sigGetOneFrame(image);  //发送信号，将次imamge发送到要显示的控件，用paintEvent绘制出来
    }
}
