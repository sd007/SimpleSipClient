#include "mediaclient.h"
#include <QDebug>
#include <QXmlStreamReader>
#include "pjsua.h"
#include <QThread>


bool MediaClient::quit_flag = false;
pjsip_endpoint* MediaClient::m_sipEndpt = NULL;

static pjsip_module clientSipMoudle =
{
    NULL, NULL,			    /* prev, next.		*/
    { "oeasy-client", 12 },	    /* Name.			*/
    -1,				    /* Id			*/
    PJSIP_MOD_PRIORITY_APPLICATION, /* Priority			*/
    NULL,			    /* load()			*/
    NULL,			    /* start()			*/
    NULL,			    /* stop()			*/
    NULL,			    /* unload()			*/
    &MediaClient::on_rx_request,		    /* on_rx_request()		*/
    NULL,			    /* on_rx_response()		*/
    NULL,			    /* on_tx_request.		*/
    NULL,			    /* on_tx_response()		*/
    NULL,			    /* on_tsx_state()		*/
};

static void call_on_state_changed(pjsip_inv_session *inv, pjsip_event *e)
{
    qDebug()<<"call_on_state_changed ......"<<inv->state;
}

static void call_on_forked(pjsip_inv_session *inv, pjsip_event *e)
{
    qDebug()<<"call_on_forked ......";
}

static  void call_on_media_update(pjsip_inv_session *inv_ses,
                                pj_status_t status)
{
    qDebug()<<"call_on_media_update ......";
}

static void call_on_send_ack(pjsip_inv_session *inv, pjsip_rx_data *rdata)
{

    pj_status_t status;
    pjsip_tx_data *tdata;
    status = pjsip_inv_create_ack(inv, rdata->msg_info.cseq->cseq, &tdata);
    pj_assert(status == PJ_SUCCESS);
    status = pjsip_inv_send_msg( inv,tdata);
    pj_assert(status == PJ_SUCCESS);
    qDebug()<<"call_on_send_ack ......";
}

/* regc callback */
static void register_cb(struct pjsip_regc_cbparam *param)
{
    pjsip_regc_info info;
    pj_status_t status; 
    status = pjsip_regc_get_info(param->regc, &info);
    pj_assert(status == PJ_SUCCESS);
    MediaClient *client = (MediaClient *)param->token;
}

QString MediaClient::createPlaySDP(QString fromDeviceid, QString mediaRecvIp, unsigned short mediaRecvPort)
{
    std::string deviceId = fromDeviceid.toStdString();
    std::string recvip = mediaRecvIp.toStdString();
    char str[512] = { 0 };
    pj_ansi_snprintf(str, 512,
    "v=0\n"
    "o=%s 0 0 IN IP4 %s\n"
    "s=Play\n"
    "c=IN IP4 %s\n"
    "t=0 0\n"
    "m=video %d RTP/AVP 96 98 97\n"
    "a=recvonly\n"
    "a=rtpmap:96 PS/90000\n"
    "a=rtpmap:98 H264/90000\n"
    "a=rtpmap:97 MPEG4/90000\n"
    "y=0100000001\n",deviceId.data(), recvip.data(), recvip.data(), mediaRecvPort);
    return str;
}

int MediaClient::initInvParam(TransportContext &tsxContext)
{
    if(m_invInit) return PJ_SUCCESS;
    pj_status_t status;
    pjsip_inv_callback inv_cb;
    /* Init the callback for INVITE session: */
    pj_bzero(&inv_cb, sizeof(inv_cb));
    inv_cb.on_state_changed = &call_on_state_changed;
    inv_cb.on_new_session = &call_on_forked;
    inv_cb.on_media_update = &call_on_media_update;
    inv_cb.on_send_ack = &call_on_send_ack; //must
    /* Initialize invite session module:  */
    status = pjsip_inv_usage_init(m_sipEndpt, &inv_cb);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    char from[64] = {0};
    char to[64] = {0};
    char target[64] = {0};
    char contact[64] = {0};
    //transfer issue, must do this
    std::string tempid = tsxContext.fromID.toStdString();
    std::string tempip = tsxContext.fromIP.toStdString();
    pj_ansi_snprintf(from,64,"sip:%s@%s:%d", tempid.data(), tempip.data(), tsxContext.fromPort);
    // contact means recver
    pj_ansi_snprintf(contact,64,"sip:%s@%s:%d", tempid.data(), tempip.data(), tsxContext.contactPort);
    tempid = tsxContext.toID.toStdString();
    tempip = tsxContext.toIP.toStdString();
    pj_ansi_snprintf(target, 64, "sip:%s@%s:%d", tempid.data(), tempip.data(), tsxContext.toPort);
//    pj_ansi_snprintf(to, 64, "sip:%s@%s:%d", "34020000001320000002", tempip.data(), tsxContext.toPort);
    pj_ansi_snprintf(to, 64, "sip:%s@%s:%d",  tempid.data(), tempip.data(), tsxContext.toPort);


    pj_str_t fromstr = pj_str(from);
    pj_str_t targetstr = pj_str(target);
    pj_str_t tostr = pj_str(to);
    /* Create UAC dialog */
    status = pjsip_dlg_create_uac( pjsip_ua_instance(),
                       &fromstr,  /* local URI */
                       NULL,  /* local Contact */
                       &tostr,    /* remote URI */
                       &tostr,    /* remote target */
                       &m_invitedlg);	    /* dialog */
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    m_invInit = true;
    return PJ_SUCCESS;
}

pj_bool_t MediaClient::on_rx_request( pjsip_rx_data *rdata ){
    char * rdata_info;
    pj_status_t status;
    pjsip_transaction *tsx;
    pjsip_tx_data *tdata;
    rdata_info = pjsip_rx_data_get_info(rdata);
    printf("The received transmission data info is %s \n",rdata_info);
    printf("On_rx_repuest is called");
    status = pjsip_tsx_create_uas(&clientSipMoudle, rdata, &tsx);
    pjsip_tsx_recv_msg(tsx, rdata);
    status = pjsip_endpt_create_response(m_sipEndpt, rdata, 200, NULL, &tdata);
    pjsip_tsx_send_msg(tsx, tdata);

    return PJ_TRUE;
}

pj_status_t MediaClient::initPJlib()
{
    pj_status_t status;
    /* Must init PJLIB first: */
    status = pj_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    /* Then init PJLIB-UTIL: */
    status = pjlib_util_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    /* Must create a pool factory before we can allocate any memory. */
    pj_caching_pool_init(&m_caching_pool, &pj_pool_factory_default_policy, 0);
    /*set log level*/
    pj_log_set_level(5);
    return status;
}

pj_status_t MediaClient::initSipMoudle(QString endptName, unsigned short tsxPort)
{
    pj_status_t status;
    /* Create the endpoint: */
    status = pjsip_endpt_create(&m_caching_pool.factory, endptName.toStdString().c_str(), &m_sipEndpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    m_sipPool = pjsip_endpt_create_pool(m_sipEndpt, "clientpool", 4000, 4000);
    /*Add UDP transport, with hard-coded port*/
    pj_sockaddr_in addr;
    addr.sin_family = pj_AF_INET();
    addr.sin_addr.s_addr = 0;
    addr.sin_port = pj_htons(tsxPort);
    status = pjsip_udp_transport_start( m_sipEndpt, &addr, NULL, 1, NULL);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    status = pjsip_tsx_layer_init_module(m_sipEndpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    status = pjsip_ua_init_module(m_sipEndpt, NULL);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /* Initialize 100rel support */
    status = pjsip_100rel_init_module(m_sipEndpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    status = pjsip_endpt_register_module(m_sipEndpt, &clientSipMoudle);
    return status;
}

pj_status_t MediaClient::registerClient(SIPClient &cltparam)
{
    pj_status_t status;
    pjsip_regc *regc;

    char local[64] = {0};
    char dst[64] = {0};
    status = pjsip_regc_create(m_sipEndpt, this, &register_cb, &regc);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    pj_ansi_sprintf(local, "<sip:%s@%s:%d>", cltparam.localDeviceID.toStdString().c_str(),  cltparam.localAddress.toStdString().c_str(), cltparam.localSipPort);

    pj_ansi_sprintf(dst, "<sip:%s@%s:%d>", cltparam.sipserverID.toStdString().c_str(),  cltparam.sipserverAddress.toStdString().c_str(), cltparam.sipserverPort);
    pj_str_t contacts[] = {
    { local, strlen(local) }
    };
    pj_str_t localstr = pj_str(local);
    pj_str_t dststr = pj_str(dst);
    status = pjsip_regc_init(regc, &dststr, &localstr, &localstr, 1, contacts, cltparam.regValidSeconds);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    pjsip_cred_info cred;
    pj_bzero(&cred, sizeof(cred));
    cred.realm = pj_str("*");
    cred.scheme = pj_str("digest");
    char username[64] = {0};
    pj_ansi_snprintf(username, 64,"%s", cltparam.localDeviceID.toStdString().c_str());
    cred.username = pj_str(username);
    cred.data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
    char passwd[64] = {0};
    pj_ansi_snprintf(passwd, 64,"%s", cltparam.localPasswd.toStdString().c_str());
    cred.data = pj_str(passwd);
    status = pjsip_regc_set_credentials(regc, 1, &cred);

    /* Register */
    pjsip_tx_data *tdata;
    status = pjsip_regc_register(regc, PJ_TRUE, &tdata);
    status = pjsip_regc_send(regc, tdata);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
}

pj_status_t MediaClient::unregisterClient(SIPClient &cltparam)
{
    pj_status_t status;
    pjsip_regc *regc;

    char local[64] = {0};
    char dst[64] = {0};
    status = pjsip_regc_create(m_sipEndpt, this, &register_cb, &regc);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    pj_ansi_sprintf(local, "<sip:%s@%s:%d>", cltparam.localDeviceID.toStdString().c_str(),  cltparam.localAddress.toStdString().c_str(), cltparam.localSipPort);

    pj_ansi_sprintf(dst, "<sip:%s@%s:%d>", cltparam.sipserverID.toStdString().c_str(),  cltparam.sipserverAddress.toStdString().c_str(), cltparam.sipserverPort);
    pj_str_t contacts[] = {
    { local, strlen(local) }
    };
    pj_str_t localstr = pj_str(local);
    pj_str_t dststr = pj_str(dst);
    status = pjsip_regc_init(regc, &dststr, &localstr, &localstr, 1, contacts, cltparam.regValidSeconds);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
    pjsip_cred_info cred;
    pj_bzero(&cred, sizeof(cred));
    cred.realm = pj_str("*");
    cred.scheme = pj_str("digest");
    char username[64] = {0};
    pj_ansi_snprintf(username, 64,"%s", cltparam.localDeviceID.toStdString().c_str());
    cred.username = pj_str(username);
    cred.data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
    char passwd[64] = {0};
    pj_ansi_snprintf(passwd, 64,"%s", cltparam.localPasswd.toStdString().c_str());
    cred.data = pj_str(passwd);
    status = pjsip_regc_set_credentials(regc, 1, &cred);

    /* Register */
    pjsip_tx_data *tdata;
    status = pjsip_regc_unregister(regc, &tdata);
    status = pjsip_regc_send(regc, tdata);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);
}

void MediaClient::startEventLoop()
{
    pj_status_t status;
    if(m_localSipType == TYPE_SIPClient)
    {
        status = pj_thread_create(m_sipPool, "keepalive", &keepAlive_thread, this, 0, 0, &m_keepalivethread);
    }
    status = pj_thread_create(m_sipPool, "eventloop", &eventloop_thread, this, 0, 0, &m_eventloopthread);
}

bool MediaClient::sendInvite(QString deviceid, QString mediaRecvIp, short mediaRecvPort)
{
     int ret = initInvParam(m_tsxContext);
     if(ret != PJ_SUCCESS) return false;
     pjsip_tx_data *tdata;
     if (PJ_SUCCESS != pjsip_inv_create_uac(m_invitedlg, nullptr, 0, &m_invsession))
     {
         return false;
     }
     if (PJ_SUCCESS != pjsip_inv_invite(m_invsession, &tdata)) return false;
     pjsip_media_type type;
     type.type = pj_str("application");
     type.subtype = pj_str("sdp");
     QString sdp = createPlaySDP(deviceid, mediaRecvIp, mediaRecvPort);
     std::string tempstring = sdp.toStdString();
     pj_str_t sdptext= pj_str(const_cast<char *>(tempstring.data()));
     try
     {
         tdata->msg->body = pjsip_msg_body_create(m_sipPool, &type.type, &type.subtype, &sdptext);
         auto hName = pj_str("Subject");
         char subjectUrl[128] = {0};
         pj_ansi_snprintf(subjectUrl, 128,"%s:0, %s:0", m_sipClientparam.localDeviceID.toStdString().c_str(), deviceid.toStdString().c_str());
         auto hValue = pj_str(const_cast<char*>(subjectUrl));
         auto hdr = pjsip_generic_string_hdr_create(m_sipPool, &hName, &hValue);
         pjsip_msg_add_hdr(tdata->msg, reinterpret_cast<pjsip_hdr*>(hdr));
         pjsip_inv_send_msg(m_invsession, tdata);
     }
     catch (...)
     {
     }
     return true;

}

int MediaClient::sendBye()
{
    pj_status_t status;
    pjsip_tx_data *tdata;
    status = pjsip_inv_end_session(m_invsession, 603, NULL, &tdata);
    pjsip_inv_send_msg(m_invsession, tdata);
    pjsip_inv_terminate(m_invsession, 603, true);
    pjsip_endpt_unregister_module(m_sipEndpt, pjsip_inv_usage_instance());
    m_invInit = false;
}

void MediaClient::queryDeviveInfo(QString deviceid, QString cmdType)
{
    char querInfo[256] = {0};
    char from[64] = {0};
    char to[64] = {0};
    char target[64] = {0};
    char contact[64] = {0};
    //transfer issue, must do this
    std::string tempid = m_tsxContext.fromID.toStdString();
    std::string tempip = m_tsxContext.fromIP.toStdString();
    pj_ansi_snprintf(from,64,"sip:%s@%s:%d", tempid.data(), tempip.data(), m_tsxContext.fromPort);
    // contact means recver
    pj_ansi_snprintf(contact,64,"sip:%s@%s:%d", tempid.data(), tempip.data(), m_tsxContext.contactPort);
    tempid = m_tsxContext.toID.toStdString();
    tempip = m_tsxContext.toIP.toStdString();
    pj_ansi_snprintf(target, 64, "sip:%s@%s:%d", tempid.data(), tempip.data(), m_tsxContext.toPort);
    pj_ansi_snprintf(to, 64, "sip:%s@%s:%d", tempid.data(), tempip.data(), m_tsxContext.toPort);

    pj_ansi_snprintf(querInfo, 256,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                     "<Query>\n"
                                     "<CmdType>%s</CmdType>\n"
                                     "<SN>17430</SN>\n"
                                     "<DeviceID>%s</DeviceID>\n"
                                     "</Query>\n",
                                     cmdType.toStdString().c_str(),
                                     deviceid.toStdString().c_str());
    pjsip_tx_data *tdata;
    pjsip_method method = { PJSIP_OTHER_METHOD, { "MESSAGE", 7 }};
    auto querStr = pj_str(querInfo);
    auto contactStr = pj_str(contact);
    auto fromStr = pj_str(from);
    auto toStr = pj_str(to);
    auto targetStr = pj_str(target);
    pjsip_endpt_create_request(m_sipEndpt, &method, &targetStr, &fromStr, &toStr, &contactStr, nullptr, -1, &querStr, &tdata);
    tdata->msg->body->content_type.type = pj_str("Application");
    tdata->msg->body->content_type.subtype = pj_str("MANSCDP+xml");
    pjsip_endpt_send_request(m_sipEndpt, tdata, -1, nullptr, nullptr);
}

void MediaClient::sendKeepAlive(QString deviceid)
{
    char querInfo[256] = {0};

    char from[64] = {0};
    char to[64] = {0};
    char target[64] = {0};
    char contact[64] = {0};
    //transfer issue, must do this
    std::string tempid = m_tsxContext.fromID.toStdString();
    std::string tempip = m_tsxContext.fromIP.toStdString();
    pj_ansi_snprintf(from,64,"sip:%s@%s:%d", tempid.data(), tempip.data(), m_tsxContext.fromPort);
    // contact means recver
    pj_ansi_snprintf(contact,64,"sip:%s@%s:%d", tempid.data(), tempip.data(), m_tsxContext.contactPort);
    tempid = m_tsxContext.toID.toStdString();
    tempip = m_tsxContext.toIP.toStdString();
    pj_ansi_snprintf(target, 64, "sip:%s@%s:%d", tempid.data(), tempip.data(), m_tsxContext.toPort);
    pj_ansi_snprintf(to, 64, "sip:%s@%s:%d", tempid.data(), tempip.data(), m_tsxContext.toPort);

    pj_ansi_snprintf(querInfo, 256,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                     "<Notify>\n"
                                     "<CmdType>Keepalive</CmdType>\n"
                                     "<SN>43</SN>\n"
                                     "<DeviceID>%s</DeviceID>\n"
                                     "</Notify>\n", deviceid.toStdString().c_str());
    pjsip_tx_data *tdata;
    pjsip_method method = { PJSIP_OTHER_METHOD, { "MESSAGE", 7 }};

    auto infoStr = pj_str(querInfo);
    auto contactStr = pj_str(contact);
    auto fromStr = pj_str(from);
    auto toStr = pj_str(to);
    auto targetStr = pj_str(target);
    pjsip_endpt_create_request(m_sipEndpt, &method, &targetStr, &fromStr, &toStr, &contactStr, nullptr, -1, &infoStr, &tdata);

    tdata->msg->body->content_type.type = pj_str("Application");
    tdata->msg->body->content_type.subtype = pj_str("MANSCDP+xml");
    pjsip_endpt_send_request(m_sipEndpt, tdata, -1, nullptr, nullptr);
}

void MediaClient::messageResolve(char *txt)
{
    QString pkgData = QString(txt);
    QString pkgMsg;
    QStringList list = pkgData.split("<?xml version=\"1.0\" ");
    qDebug()<<"zzzzz---------:"<<list[0];
    if(list.size() > 1)
    {
        pkgMsg = QString("<?xml version=\"1.0\" ")+list[1];
        qDebug()<<"------------------------------------------------";
        qDebug()<<pkgMsg;
        QXmlStreamReader xmlReader(pkgMsg.toStdString().c_str());
        while(!xmlReader.atEnd() && !xmlReader.hasError())
        {
            xmlReader.readNext();
            if(xmlReader.isStartElement())
            {
                if(xmlReader.name()=="Response")
                {
                }else if (xmlReader.name()=="CmdType") {
                    qDebug()<<"date:"<<xmlReader.readElementText();
                }else if (xmlReader.name()=="DeviceID") {
                    qDebug()<<"message:"<<xmlReader.readElementText();
                }
            }

        }

    }
}

void MediaClient::onRegister()
{
    this->setClientParamContext();
    this->registerClient(m_sipClientparam);
}


MediaClient::MediaClient(QWidget *parent) : QWidget(parent)
{

}


MediaClient::~MediaClient()
{
    quit_flag = true;
    this->unregisterClient(m_sipClientparam);
    destroy();
}

void MediaClient::setUpUI()
{
    this->setWindowTitle("SIPCLIENT from sdguet@163.com");
    this->move(300,200);
    this->setFixedSize(740,600);
    m_widget = new QWidget(this);
    m_widget->resize(this->size());

    m_lbserverip = new QLabel("ServerIP:", m_widget);
    m_lbserverip->setFixedSize(QSize(100,25));
    m_lbserverip->move(20,20);
    m_edserverip = new QLineEdit("192.168.5.138",m_widget);
    m_edserverip->setFixedSize(QSize(200,25));
    m_edserverip->move(150,20);
    //server
    m_lbServid = new QLabel("ServerID:", m_widget);
    m_lbServid->setFixedSize(QSize(100,25));
    m_lbServid->move(20,50);
    m_edServid = new QLineEdit("34020000000020000002",m_widget);
    m_edServid->setFixedSize(QSize(200,25));
    m_edServid->move(150,50);
    m_lbServSipport = new QLabel("ServerPort:", m_widget);
    m_lbServSipport->setFixedSize(QSize(100,25));
    m_lbServSipport->move(20,80);
    m_edServSipport = new QLineEdit("5060",m_widget);
    m_edServSipport->setFixedSize(QSize(200,25));
    m_edServSipport->move(150,80);

    //client
    m_lbclientip = new QLabel("ClientIP:", m_widget);
    m_lbclientip->setFixedSize(QSize(100,25));
    m_lbclientip->move(380,20);
    m_edclientip = new QLineEdit("192.168.21.76",m_widget);
    m_edclientip->setFixedSize(QSize(200,25));
    m_edclientip->move(500,20);
    m_lbClientID = new QLabel("ClientID:", m_widget);
    m_lbClientID->setFixedSize(QSize(100,25));
    m_lbClientID->move(380,50);
    m_edClientID = new QLineEdit("34020000001320000001",m_widget);
    m_edClientID->setFixedSize(QSize(200,25));
    m_edClientID->move(500,50);
    m_lbclientSipport = new QLabel("ClientSipPort:", m_widget);
    m_lbclientSipport->setFixedSize(QSize(100,25));
    m_lbclientSipport->move(380,80);
    m_edclientSipport = new QLineEdit("5060",m_widget);
    m_edclientSipport->setFixedSize(QSize(200,25));
    m_edclientSipport->move(500,80);
    //video pic
    m_imgLabel = new ImageLabel(m_widget);
    m_imgLabel->setStyleSheet("border-width: 1px; border-style: solid; border-color: rgb(0, 0, 0)");
    m_imgLabel->move(QPoint(70,110));
    m_imgLabel->resize(600,400);
    //media recv
    m_lbmediarecvip = new QLabel("MeidaRecvIP:", m_widget);
    m_lbmediarecvip->setFixedSize(QSize(100,25));
    m_lbmediarecvip->move(20,520);
    m_edmediarecvip = new QLineEdit("192.168.21.76",m_widget);
    m_edmediarecvip->setFixedSize(QSize(200,25));
    m_edmediarecvip->move(150,520);
    m_lbmediarecvport = new QLabel("MeidaRecvPort:", m_widget);
    m_lbmediarecvport->setFixedSize(QSize(100,25));
    m_lbmediarecvport->move(381,520);
    m_edmediarecvport = new QLineEdit("10520",m_widget);
    m_edmediarecvport->setFixedSize(QSize(200,25));
    m_edmediarecvport->move(500,520);

    m_lbmediaDeviceId = new QLabel("MeidaDeviceID:", m_widget);
    m_lbmediaDeviceId->setFixedSize(QSize(100,25));
    m_lbmediaDeviceId->move(20,550);
    m_edmediaDeviceId = new QLineEdit("34020000001320000002",m_widget);
    m_edmediaDeviceId->setFixedSize(QSize(200,25));
    m_edmediaDeviceId->move(150,550);

    m_btreg = new QPushButton("register", m_widget);
    connect(m_btreg, SIGNAL(clicked(bool)), this, SLOT(onRegister()));
    m_btreg->resize(QSize(80, 30));
    m_btreg->move(QPoint(380,550));
    m_btplay = new QPushButton("play", m_widget);
    connect(m_btplay, SIGNAL(clicked(bool)), this, SLOT(onPlay()));
    m_btplay->resize(QSize(80, 30));
    m_btplay->move(QPoint(500,550));
    m_btstop = new QPushButton("stop", m_widget);
    connect(m_btstop, SIGNAL(clicked(bool)), this, SLOT(onStop()));
    m_btstop->resize(QSize(80,30));
    m_btstop->move(QPoint(620,550));
}

void MediaClient::initDecoder()
{
    m_avDecoder = new Decoder(this);
    connect(m_avDecoder, SIGNAL(sigGetOneFrame(QImage)), m_imgLabel, SLOT(onGetImage(QImage)));
    m_avDecoder->init();
}

void MediaClient::initRecver(QString proxyurl, QString proxyid, QString faceurl)
{
    m_access = new Access(proxyurl, proxyid, faceurl);
}


/* Worker thread */
int MediaClient::keepAlive_thread(void *arg)
{
    MediaClient *client = (MediaClient*)arg;
    while (!quit_flag && client) {
        QThread::sleep(30);
        QString localid = QString("34020000001320000001");
        client->sendKeepAlive(localid);
    }

    return 0;
}

/* Worker thread */
int MediaClient::eventloop_thread(void *arg)
{
    MediaClient *client = (MediaClient*)arg;
    while (!quit_flag && client) {
        pj_time_val timeout = {0, 10};
        pjsip_endpt_handle_events(m_sipEndpt, &timeout);
    }
    return 0;
}


void MediaClient::setClientParamContext()
{
    QString serverid;
    QString serverip;
    QString clientid;
    QString clientip;

    if(!m_edServid->text().isEmpty())
    {
       serverid = m_edServid->text();
    }else {
       serverid = QString("34020000000020000001");
    }
    if(!m_edserverip->text().isEmpty())
    {
       serverip = m_edserverip->text();
    }else
    {
       serverip = QString("192.168.1.110");
    }

    if(!m_edClientID->text().isEmpty())
    {
       clientid = m_edClientID->text();
    }else {
       clientid = QString("34020000001320000001");
    }
    if(!m_edclientip->text().isEmpty())
    {
       clientip = m_edclientip->text();
    }else
    {
       clientip = QString("192.168.1.64");
    }
    m_tsxContext.fromID = clientid;
    m_tsxContext.fromIP = clientip;
    m_tsxContext.fromPort = m_edclientSipport->text().toShort();

    m_tsxContext.toID = serverid;
    m_tsxContext.toIP = serverip;
    m_tsxContext.toPort = m_edServSipport->text().toShort();

    m_tsxContext.contactID = clientid;
    m_tsxContext.contactIP = clientip;
    m_tsxContext.contactPort = m_edclientSipport->text().toShort();


    m_sipClientparam.localAddress = clientip;
    m_sipClientparam.localDeviceID = clientid;
    m_sipClientparam.localSipPort = m_edclientSipport->text().toShort();
    m_sipClientparam.localPasswd = QString("12345678");
    m_sipClientparam.sipserverAddress = serverip;
    m_sipClientparam.sipserverID = serverid;
    m_sipClientparam.sipserverPort = m_edServSipport->text().toShort();
    m_sipClientparam.heartbeatInterval = 30;
    m_sipClientparam.maxTimeoutCount = 3;
    m_sipClientparam.regValidSeconds = 600;

    m_localSipType = TYPE_SIPClient;
}

void MediaClient::destroy()
{
    pj_pool_release(m_sipPool);
    pj_thread_join(m_keepalivethread);
    pjsip_endpt_destroy(m_sipEndpt);
    pj_caching_pool_destroy(&m_caching_pool);
    pj_shutdown();
}

void MediaClient::onStop()
{

    if(m_rtpRecver)
    {
        m_rtpRecver->destroy();
        m_rtpRecver->quit();
    }
    this->sendBye();
}

void MediaClient::onPlay()
{
    this->setClientParamContext();
    m_rtpRecver = new RtpRecver(this);
    QString deviceid = m_edmediaDeviceId->text();
    QString recvip = m_edmediarecvip->text();
    int recvport = m_edmediarecvport->text().toInt();

    connect(m_rtpRecver, SIGNAL(sigRecvedFrame(QByteArray)), this, SLOT(onRecvedFrame(QByteArray)), Qt::QueuedConnection);
    m_rtpRecver->init(recvip, recvport);
    m_rtpRecver->start();
    this->sendInvite(deviceid, recvip, recvport);
}

void MediaClient::onRecvedFrame(QByteArray buff)
{
    if(m_avDecoder)
    {
        m_avDecoder->decode(buff.data(), buff.size());
    }
}
