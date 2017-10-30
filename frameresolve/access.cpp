#include "access.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

Access::Access(QString proxyurl, QString proxyid, QString faceEnterUrl, QObject *parent) :
    QObject(parent),
    m_ws(),
    m_faceEnterManager(),
    m_faceUrl(faceEnterUrl),
    m_proxyurl(proxyurl),
    m_proxyid(proxyid),
    m_heartbeatTimer()
{
    m_faceEnterManager.setCookieJar(Q_NULLPTR);
    connect(&m_ws, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&m_ws, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));
    connect(&m_ws, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(&m_heartbeatTimer, SIGNAL(timeout()), this, SLOT(onHeartbeatTimeout()));
    connect(&m_faceEnterManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onFaceEnterFinished(QNetworkReply*)));
    m_ws.open(QUrl(proxyurl));
    m_heartbeatTimer.start(1000);
    m_smsStarted = false;
}

Access::~Access()
{
    m_ws.disconnect();
    m_ws.close();
}

void Access::writeFrame(QByteArray barray)
{
    if(m_smsStarted)
    {
        m_sms->writeFrame(barray);
    }
}

void Access::onConnected()
{
    QJsonObject jsonLogin;
    jsonLogin.insert("cmd_version", "1.0");
    jsonLogin.insert("data_type", 0);
    jsonLogin.insert("client_type", 0);
    jsonLogin.insert("client_version", "0.1");
    jsonLogin.insert("client_id", m_proxyid);
    jsonLogin.insert("length", QJsonDocument(jsonLogin).toJson(QJsonDocument::Compact).size() - 2);
    jsonLogin.insert("type", 1);
    m_ws.sendTextMessage(QJsonDocument(jsonLogin).toJson(QJsonDocument::Compact));
}

void Access::onHeartbeatTimeout()
{
    if (m_ws.state() == QAbstractSocket::UnconnectedState)
    {
        m_ws.open(QUrl(m_proxyurl));
        return ;
    }
    else if (m_ws.state() != QAbstractSocket::ConnectedState)
    {
        return ;
    }

    QJsonObject jsonHeartbeat;
    jsonHeartbeat.insert("type", 2);
    jsonHeartbeat.insert("length", 0);
    m_ws.sendTextMessage(QJsonDocument(jsonHeartbeat).toJson(QJsonDocument::Compact));

    QJsonArray ipcList;
    QJsonObject ipcId;
    ipcId.insert("ipc_id", "0001");
    ipcList.append(ipcId);

    QJsonObject jsonStatus;
    jsonStatus.insert("cmd", 0x02000002);
    jsonStatus.insert("proxy_id", m_proxyid);
    jsonStatus.insert("ipc_num", 1);
    jsonStatus.insert("ipc_list", ipcList);
    jsonStatus.insert("length", QJsonDocument(jsonStatus).toJson(QJsonDocument::Compact).size() - 2);
    jsonStatus.insert("type", 5);
    m_ws.sendTextMessage(QJsonDocument(jsonStatus).toJson(QJsonDocument::Compact));
}

void Access::onTextMessageReceived(QString recv)
{

    QJsonParseError jsonErr;
    QJsonObject jsonObj;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(recv.toLatin1(), &jsonErr);
    if (jsonErr.error != QJsonParseError::NoError && !jsonDoc.isObject())
    {
        qDebug() << "recv unkown msg" << recv;
        return ;
    }
    jsonObj = jsonDoc.object();
    switch (jsonObj["type"].toInt())
    {
    case 1:
        if (jsonObj["code"].toInt() == 200)
        {
            onHeartbeatTimeout();
            m_heartbeatTimer.start(jsonObj["heartbeat"].toInt() * 1000);
        }
        break;
    case 2:
        break;
    case 3:
        switch (jsonObj["cmd"].toInt())
        {
        case 0x3000001:
        {
            QString code= jsonObj["stream_id"].toString();
            QString ip = jsonObj["sms_ip"].toString();
            int port = jsonObj["sms_port"].toInt();
            m_sms = new SMS(ip, port, code, this);
            connect(&m_sms->m_fd, SIGNAL(disconnected()), this, SLOT(stopSMS()));
            m_smsStarted = true;
        }
            break;
        case 0x3000002:
            //处理完全一样，不break
        case 0x3000003:
        {
            m_faceEnterManager.clearAccessCache();
            if (m_faceEnterManager.children().size() > 0)
            {
                m_faceEnterManager.children().first()->deleteLater();
            }
            QNetworkRequest req;
            req.setUrl(QUrl(m_faceUrl));
            req.setHeader(QNetworkRequest::ContentTypeHeader, " ");
            m_faceEnterManager.post(req, recv.toLatin1());
        }
            break;
        default:
            break;
        }
        break;
    case 5:
        if (jsonObj["result_code"].toInt() != 0)
        {
            qDebug()<< jsonObj["result_reason"].toString();
        }
        break;
    default:
        break;
    }
}

void Access::stopSMS()
{
    m_sms->deleteLater();
    m_sms = Q_NULLPTR;
}

void Access::onError(QAbstractSocket::SocketError err)
{
    qDebug() << err;
    m_heartbeatTimer.start(1000);
}

void Access::onFaceEnterFinished(QNetworkReply *r)
{
    QByteArray data = r->readAll();
    qDebug() << data;
    Q_UNUSED(data);
    r->deleteLater();
}
