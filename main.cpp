#include <QApplication>
#include "mediaclient.h"
#include <QThread>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MediaClient mlient;
    mlient.setUpUI();
    mlient.setClientParamContext();
    mlient.initDecoder();
//    QString proxyurl("ws://192.168.5.136:6666");
//    QString proxyid("0000971531");
//    QString faceenterurl("http://192.168.5.112:9317/distribute");
//    mlient.initRecver(proxyurl, proxyid, faceenterurl);
    mlient.initPJlib();
    mlient.initSipMoudle("sipenpt", 5060);
    mlient.startEventLoop();
    mlient.show();
    qDebug()<<"aaa";
    return app.exec();
}






