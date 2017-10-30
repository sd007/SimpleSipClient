#include "pstoh264.h"
#include <iostream>

bool PsToH264::m_firstMainFrame = false;

PsToH264::PsToH264(QObject *parent) : QObject(parent)
{

}

int PsToH264::GetH246FromPs(char *buffer, int length, char **h264Buffer, int *h264length)
{
    int leftlength = 0;
    char *NextPack = 0;

    *h264Buffer = buffer;
    *h264length = 0;
    // 00 00 01 ba
    if(ProgramStreamPackHeader(buffer, length, &NextPack, &leftlength)==0)
        return 0;

    char *PayloadData=NULL;
    int PayloadDataLen=0;

    while(leftlength >= sizeof(pack_start_code))
    {
        PayloadData=NULL;
        PayloadDataLen=0;

        if(/*m_firstMainFrame && */NextPack
        && NextPack[0]=='\x00'
        && NextPack[1]=='\x00'
        && NextPack[2]=='\x01'
        && NextPack[3]=='\xE0')
        {
            if(Pes(NextPack, leftlength, &NextPack, &leftlength, &PayloadData, &PayloadDataLen))
            {
                if(PayloadDataLen)
                {
                    memcpy(buffer, PayloadData, PayloadDataLen);
                    buffer += PayloadDataLen;
                    *h264length += PayloadDataLen;
                }

            }
            else
            {
                if(PayloadDataLen)
                {
                    memcpy(buffer, PayloadData, PayloadDataLen);
                    buffer += PayloadDataLen;
                    *h264length += PayloadDataLen;
                }
                break;
            }
        }
        else if(NextPack
                && NextPack[0]=='\x00'
                && NextPack[1]=='\x00'
                && NextPack[2]=='\x01'
                && NextPack[3]=='\xBB')
        {
            if(length < sizeof(pack_title_header)) break;
            pack_title_header *title = (pack_title_header *)NextPack;

            littel_endian_size titile_length;
            titile_length.byte[0] = title->headerLength.byte[1];
            titile_length.byte[1] = title->headerLength.byte[0];

            NextPack += titile_length.length + sizeof(pack_title_header);
            leftlength = leftlength - titile_length.length - sizeof(pack_title_header);
        }else if(NextPack
            && NextPack[0]=='\x00'
            && NextPack[1]=='\x00'
            && NextPack[2]=='\x01'
            && NextPack[3]=='\xBC')
        {
            if(leftlength < sizeof(program_stream_map)) break;
            program_stream_map* PSMPack = (program_stream_map*)NextPack;

            littel_endian_size psm_length;
            psm_length.byte[0] = PSMPack->PackLength.byte[1];
            psm_length.byte[1] = PSMPack->PackLength.byte[0];

            NextPack = NextPack + psm_length.length + sizeof(program_stream_map);
            leftlength = leftlength - psm_length.length - sizeof(program_stream_map);
//            m_firstMainFrame = true;
        }
        else
        {
      //      printf("[%s]no konw %x %x %x %x\n", __FUNCTION__, NextPack[0], NextPack[1], NextPack[2], NextPack[3]);
            break;
        }
    }
    return *h264length;
}

int PsToH264::Pes(char *Pack, int length, char **NextPack, int *leftlength, char **PayloadData, int *PayloadDataLen)
{
    *PayloadData = 0;
    *PayloadDataLen = 0;
    if(length < sizeof(program_stream_e)) return 0;

    program_stream_e* PSEPack = (program_stream_e*)Pack;
    littel_endian_size pse_length;
    pse_length.byte[0] = PSEPack->PackLength.byte[1];
    pse_length.byte[1] = PSEPack->PackLength.byte[0];
    //2 字节表示长度
    //1 字节固定值
    *PayloadDataLen = pse_length.length - 2 - 1 - PSEPack->stuffing_length;
    if(*PayloadDataLen > 0)
    {
        *PayloadData = Pack + sizeof(program_stream_e) + PSEPack->stuffing_length;
    }
    *leftlength = length - pse_length.length - sizeof(pack_start_code) - sizeof(littel_endian_size);

    if(*leftlength <= 0) return 0;

    *NextPack = Pack + sizeof(pack_start_code) + sizeof(littel_endian_size) + pse_length.length;

    return *leftlength;
}


int PsToH264::ProgramStreamPackHeader(char *Pack, int length, char **NextPack, int *leftlength)
{
    //printf("[%s]%x %x %x %x\n", __FUNCTION__, Pack[0], Pack[1], Pack[2], Pack[3]);
    //通过 00 00 01 ba头的第14个字节的最后3位来确定头部填充了多少字节
    program_stream_pack_header *PsHead = (program_stream_pack_header *)Pack;
    unsigned char pack_stuffing_length = PsHead->stuffinglen & '\x07';

    *leftlength = length - sizeof(program_stream_pack_header) - pack_stuffing_length;//减去头和填充的字节
    *NextPack = Pack+sizeof(program_stream_pack_header) + pack_stuffing_length;

    if(*leftlength<4) return 0;

    //printf("[%s]2 %x %x %x %x\n", __FUNCTION__, (*NextPack)[0], (*NextPack)[1], (*NextPack)[2], (*NextPack)[3]);

    return *leftlength;
}
