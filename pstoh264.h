#ifndef PSTOH264_H
#define PSTOH264_H

#include <QObject>
#include <string.h>
#include <QDebug>

#pragma pack(1)
union littel_endian_size
{
    unsigned short int  length;
    unsigned char       byte[2];
};

struct pack_title_header
{
    unsigned char header[4];
    littel_endian_size headerLength;
};

struct pack_start_code
{
    unsigned char start_code[3];
    unsigned char stream_id[1];
};

struct program_stream_pack_header
{
    pack_start_code PackStart;// 4
    unsigned char Buf[9];
    unsigned char stuffinglen;
};

struct program_stream_map
{
    pack_start_code PackStart;
    littel_endian_size PackLength;//we mast do exchange
    //program_stream_info_length
    //info
    //elementary_stream_map_length
    //elem
};

struct program_stream_e
{
    pack_start_code     PackStart;
    littel_endian_size  PackLength;//we mast do exchange
    char                PackInfo1[2];
    unsigned char       stuffing_length;
};

#pragma pack()


class PsToH264 : public QObject
{
    Q_OBJECT
public:
    explicit PsToH264(QObject *parent = 0);
    static int GetH246FromPs(char* buffer, int length, char **h264Buffer, int *h264length);
private:
    static int Pes(char* Pack, int length, char **NextPack, int *leftlength, char **PayloadData, int *PayloadDataLen);
    static int ProgramStreamPackHeader(char* Pack, int length, char **NextPack, int *leftlength);
private:
    static bool m_firstMainFrame;
signals:

public slots:
};

#endif // PSTOH264_H
