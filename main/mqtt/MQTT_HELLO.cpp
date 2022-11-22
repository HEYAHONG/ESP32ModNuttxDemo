#include "appmqtt.h"
#include "RC.h"
#include "netutils/netlib.h"

/*
Hello MQTT协议栈,用于MQTT测试。
主题为2层,分别为目的地址与源地址。接收到消息直接调转目的地址与源地址原样回复。
*/

static std::string SerialID;
//默认目标地址
#define DEFAULT_DEST_ADDR "DESTADDR"

static void MQTT_HELLO_Init(MQTT_Cfg_t &cfg)
{
    if (!SerialID.empty())
    {
        //此函数可能不止一次被调用，故而已初始化后不再初始化
        return;
    }
    {
        //获取序列号
        uint8_t mac[6] = {0};
        //获取wlan0 mac地址
        netlib_getmacaddr("wlan0", mac);
        for (size_t i = 0; i < 6; i++)
        {
            char buff[4] = {0};
            sprintf(buff, "%02X", (int)mac[i]);
            SerialID += buff;
        }
    }

    cfg.host = CONFIG_APP_MAIN_MQTTCLIENT_HOST;
    cfg.port = CONFIG_APP_MAIN_MQTTCLIENT_PORT;
    cfg.clientid = SerialID;
    cfg.auth.username = SerialID;
    cfg.auth.password = SerialID;
    cfg.cleansession = true;
    cfg.keepalive = 120;
    cfg.subscribe.qos = 0;
    cfg.subscribe.subtopic = (SerialID + "/#");
    cfg.will.will_qos = 0;
    cfg.will.will_retain = false;
    cfg.will.will_payload = "Offline";
    cfg.will.will_topic = (std::string(DEFAULT_DEST_ADDR) + "/" + SerialID);

}

static void MQTT_HELLO_Connect(MQTT_Cfg_t &cfg)
{
    //发送上线消息
    std::shared_ptr<MQTT_Message_t> msg = std::make_shared<MQTT_Message_t>();
    msg->qos = 0;
    msg->retain = false;
    msg->topic = (std::string(DEFAULT_DEST_ADDR) + "/" + SerialID);
    msg->payload = "Online";
    MQTT_Publish_Message(msg);
}

static void MQTT_HELLO_DisConnect(MQTT_Cfg_t &cfg)
{

}

static void MQTT_HELLO_OnMessage(MQTT_Cfg_t &cfg, MQTT_Message_Ptr_t msg)
{
    //修改主题后原样回复
    size_t pos = 0;
    if ((pos = msg->topic.find("/")) != std::string::npos)
    {
        std::string srcaddr =    msg->topic.substr(pos + 1);

        if (srcaddr.empty())
        {
            return;
        }
        msg->topic = (srcaddr + "/" + SerialID);
        MQTT_Publish_Message(msg);
    }

}


int MQTT_HELLO_Reg()
{
    //注册回调
    MQTT_Callback_t cb = {0};
    cb.init = MQTT_HELLO_Init;
    cb.connect = MQTT_HELLO_Connect ;
    cb.disconnect = MQTT_HELLO_DisConnect ;
    cb.onmessage = MQTT_HELLO_OnMessage;
    MQTT_Set_Callback(cb);

    return 0;
};
