#include "appmqtt.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "RC.h"

#ifdef CONFIG_NETUTILS_MQTTC_WITH_MBEDTLS
//MQTT-C库已启用SSL，则CONFIG_APP_MAIN_MQTTCLIENT_SSL必须被启用
#ifndef CONFIG_APP_MAIN_MQTTCLIENT_SSL
#error  当MQTT-C库已启用SSL时必须启用APP_MAIN_MQTTCLIENT_SSL
#endif // CONFIG_APP_MAIN_MQTTCLIENT_SSL
#endif // ONFIG_NETUTILS_MQTTC_WITH_MBEDTLS

#include <mqtt.h>
#ifdef CONFIG_APP_MAIN_MQTTCLIENT_SSL
#include "mbedtls_sockets.h"
#else
#include "posix_sockets.h"
#endif
#include "mutex"
#include "queue"


static MQTT_Callback_t callback;
static MQTT_Cfg_t cfg;
static struct mqtt_client client = {0};
static const char *TAG = "MQTT";

static void publish_callback(void **unused, struct mqtt_response_publish *published)
{
    std::shared_ptr<MQTT_Message_t> msg = std::make_shared<MQTT_Message_t>();
    msg->payload = std::string((char *)published->application_message, published->application_message_size);
    msg->topic = std::string((char *)published->topic_name, published->topic_name_size);
    msg->qos = published->qos_level;
    msg->retain = published->retain_flag;
    printf("%s:message topic=%s\r\n", TAG, msg->topic.c_str());
    if (callback.onmessage != NULL)
    {
        callback.onmessage(cfg, msg);
    }
}

static std::mutex publish_lock;
static std::queue<MQTT_Message_Ptr_t> msgqueue;
static void __MQTT_Publish_Message(MQTT_Message_Ptr_t msg)
{
    if (msg == NULL)
    {
        return ;
    }
    uint8_t flag = 0;
    if (msg->retain)
    {
        flag |= MQTT_PUBLISH_RETAIN;
    }
    switch (msg->qos)
    {
    case 1:
    {
        flag |= MQTT_PUBLISH_QOS_1;
    }
    break;
    case 2:
    {
        flag |= MQTT_PUBLISH_QOS_2;
    }
    break;
    default:
    {
        flag |= MQTT_PUBLISH_QOS_0;
    }
    break;
    }
    mqtt_publish(&client, msg->topic.c_str(), msg->payload.c_str(), msg->payload.length(), flag);
}


#ifdef CONFIG_APP_MAIN_MQTTCLIENT_SSL
static struct mbedtls_context ctx;
#endif // CONFIG_APP_MAIN_MQTTCLIENT_SSL
static uint8_t sendbuf[CONFIG_APP_MAIN_MQTTCLIENT_SENDBUFF_SIZE];
static uint8_t recvbuf[CONFIG_APP_MAIN_MQTTCLIENT_RECVBUFF_SIZE];
static void reconnect_client(struct mqtt_client *ptr, void **reconnect_state_vptr)
{
    if (client.error != MQTT_ERROR_INITIAL_RECONNECT)
    {
#ifdef CONFIG_APP_MAIN_MQTTCLIENT_SSL
        mbedtls_ssl_free(&ctx.ssl_ctx);
        mbedtls_net_free(&ctx.net_ctx);
        mbedtls_ssl_config_free(&ctx.ssl_conf);
        mbedtls_x509_crt_free(&ctx.ca_crt);
        mbedtls_ctr_drbg_free(&ctx.ctr_drbg);
        mbedtls_entropy_free(&ctx.entropy);
#else
        close(client.socketfd);
#endif // CONFIG_APP_MAIN_MQTTCLIENT_SSL
        if (callback.disconnect != NULL)
        {
            callback.disconnect(cfg);
        }
    }
#ifdef CONFIG_APP_MAIN_MQTTCLIENT_SSL
    while (!open_nb_socket(&ctx, cfg.host.c_str(), std::to_string(cfg.port).c_str(), cfg.ssl.cacert.c_str()))
    {
        printf("%s:connect to %s:%d failed!\r\n", TAG, cfg.host.c_str(), (int)cfg.port);
        sleep(5);
    }

    mqtt_reinit(&client, (int)&ctx.ssl_ctx, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf));

#else
    int sockfd = -1;
    while (-1 == (sockfd = open_nb_socket(cfg.host.c_str(), std::to_string(cfg.port).c_str())))
    {
        printf("%s:connect to %s:%d failed!\r\n", TAG, cfg.host.c_str(), (int)cfg.port);
        sleep(5);
    }

    mqtt_reinit(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf));

#endif // CONFIG_APP_MAIN_MQTTCLIENT_SSL

    uint8_t connect_flags = 0;

    if (cfg.cleansession)
    {
        connect_flags |= MQTT_CONNECT_CLEAN_SESSION;
    }

    if (cfg.will.will_retain)
    {
        connect_flags |= MQTT_CONNECT_WILL_RETAIN;
    }

    switch (cfg.will.will_qos)
    {
    case 1:
    {
        connect_flags |=  MQTT_CONNECT_WILL_QOS_1;
    }
    break;
    case 2:
    {
        connect_flags |=  MQTT_CONNECT_WILL_QOS_2;
    }
    break;
    default:
    {
        connect_flags |=  MQTT_CONNECT_WILL_QOS_0;
    }
    break;
    }

    //连接MQTT
    if (MQTT_OK == mqtt_connect(ptr, cfg.clientid.c_str(), cfg.will.will_topic.c_str(), cfg.will.will_payload.c_str(), cfg.will.will_payload.length(), cfg.auth.username.c_str(), cfg.auth.password.c_str(), connect_flags, cfg.keepalive))
    {
        printf("%s:connect to %s:%d success!\r\n", TAG, cfg.host.c_str(), (int)cfg.port);
        if (callback.connect != NULL)
        {
            callback.connect(cfg);
        }
    }

    mqtt_subscribe(ptr, cfg.subscribe.subtopic.c_str(), cfg.subscribe.qos);
}


static bool check_cfg(MQTT_Cfg_t &Cfg)
{
    if (Cfg.host.empty())
    {
        return false;
    }

    if (Cfg.port == 0)
    {
        return false;
    }

    if (Cfg.keepalive == 0)
    {
        Cfg.keepalive = 120;
    }

    if (Cfg.clientid.empty())
    {
        return false;
    }

    return true;
}

void *client_refresher(void *ptr)
{
#ifdef CONFIG_APP_MAIN_MQTTCLIENT_SSL
    {
        const char *rc = (char *)RCGetHandle("emqx/cacert.pem");
        if (rc)
        {
            cfg.ssl.cacert = std::string(rc);
        }
    }
#endif // CONFIG_APP_MAIN_MQTTCLIENT_SSL

#ifdef CONFIG_APP_MAIN_MQTTCLIENT_PROTOCOL_HELLO
    extern int MQTT_HELLO_Reg();
    MQTT_HELLO_Reg();
#endif // CONFIG_APP_MAIN_MQTTCLIENT_PROTOCOL_HELLO


    mqtt_init_reconnect(&client, reconnect_client, &cfg, publish_callback);
    printf("%s:mqtt client daemon is starting.\n", TAG);
    while (!check_cfg(cfg))
    {
        if (callback.init != NULL)
        {
            printf("%s:wait for config!!\r\n", TAG);
            callback.init(cfg);
            sleep(5);
        }
        else
        {
            printf("%s:mqtt not init!!\r\n", TAG);
            sleep(60);
        }
    }

    while (1)
    {
        mqtt_sync((struct mqtt_client *) &client);
        {
            //检查发布队列
            while (msgqueue.size() > 0)
            {
                std::lock_guard<std::mutex> lock(publish_lock);
                MQTT_Message_Ptr_t msg = msgqueue.front();
                msgqueue.pop();
                __MQTT_Publish_Message(msg);
            }
        }
        usleep(100000U);
    }
    return NULL;
}


/*
初始化MQTT
*/
static pthread_t mqtt_daemon = NULL;
void MQTT_Init()
{
    if (mqtt_daemon != NULL)
    {
        pthread_cancel(mqtt_daemon);
    }

    pthread_attr_t attr;
    pthread_attr_setstacksize(&attr, 8192); //由于是网络任务，栈必须够大。
    if (pthread_create(&mqtt_daemon, &attr, client_refresher, &client))
    {
        fprintf(stderr, "Failed to start mqtt client daemon.\n");
    }
}

/*
设置回调函数
*/
void MQTT_Set_Callback(MQTT_Callback_t cb)
{
    callback = cb;
}

/*
发布消息
*/
bool MQTT_Publish_Message(MQTT_Message_Ptr_t msg)
{
    std::lock_guard<std::mutex> lock(publish_lock);
    msgqueue.push(msg);
    return true;
}
