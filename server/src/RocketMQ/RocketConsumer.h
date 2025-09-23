#ifndef ROCKET_CONSUMER_H
#define ROCKET_CONSUMER_H

#include <rocketmq/DefaultMQPushConsumer.h>
#include <rocketmq/MQMessageListener.h>
#include <functional>
#include <memory>

using namespace rocketmq;

struct Resource{  // 消费时需要用到的资源
    const TCPConnect *con;  // 处理与客户端的连接
    const ThreadPool *pool;  // 主要是锁资源
};

using message_callback_t = std::function<ConsumeStatus (const std::vector<MQMessageExt> &, Resource &)>;

class MessageListener : public MessageListenerConcurrently{
    message_callback_t callback;  // 收到消息的回调，主要是消费的逻辑

    Resource *resource;  // 消费时用到的一些资源，构造或者手动设置

    ConsumeStatus handleMessageCB(const std::vector<MQMessageExt> &msgs);
public:
    MessageListener();
    MessageListener(const message_callback_t &cb);
    MessageListener(const Resource *resource, const message_callback_t& cb);

    void setMessageCB(const message_callback_t &cb);
    void setResouce(const Resource *p);

    ConsumeStatus consumeMessage(const std::vector<MQMessageExt>& msgs) override;
        /* for (const auto& msg : msgs) {
            std::cout << "\n======= Received Message ======="
                      << "\nTopic: " << msg.getTopic()
                      << "\nTags: " << msg.getTags()
                      << "\nBody: " << msg.getBody()
                      << "\nMsgID: " << msg.getMsgId()
                      << "\nOffset: " << msg.getQueueOffset()
                      << "\n===============================" << std::endl;
        } */
        // return CONSUME_SUCCESS; // 消费成功
        // return RECONSUME_LATER; // 消费失败，稍后重试
};

class MqConsumer{
    DefaultMQPushConsumer consumer;

public:
    MqConsumer();
    MqConsumer(const std::string &group);
    MqConsumer(const std::string& group, const std::string &ip, const std::string &port);

    void setListenrt(MessageListener &listener);
    void setNameServer(const std::string &nameserver);
    void setNameServer(const std::string &ip, const std::string &port);
    void setSubscribe(const std::string &topic, const std::string &tag);
    
    void shutdown();
    void start();
};



#endif