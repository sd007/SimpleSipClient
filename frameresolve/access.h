#ifndef ACCESS_H
#define ACCESS_H

#include <QObject>
#include <QDebug>
#include <QtWebSockets/QWebSocket>
#include <QTimer>
#include <QNetworkReply>
#include "frameresolve/sms.h"

class Access : public QObject
{
    Q_OBJECT
public:
    explicit Access(QString proxyurl, QString proxyid, QString faceEnterUrl, QObject *parent = 0);
    ~Access();
    void writeFrame(QByteArray barray);
private:
    QWebSocket m_ws;
    QNetworkAccessManager m_faceEnterManager;
    QString m_faceUrl;
    QString m_proxyurl;
    QString m_proxyid;
    QTimer m_heartbeatTimer;
    SMS *m_sms;
    bool m_smsStarted;
signals:
    void textRecv(QString);
public slots:
    void stopSMS();
    void onConnected();
    void onHeartbeatTimeout();
    void onTextMessageReceived(QString);
    void onError(QAbstractSocket::SocketError);
    void onFaceEnterFinished(QNetworkReply *r);
};

#endif // ACCESS_H
