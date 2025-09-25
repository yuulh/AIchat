// #include "RocketConsumer.h"

// MessageListener::MessageListener()
// {
// }

// MessageListener::MessageListener(const message_callback_t& cb)
//     : callback(cb)
// {
// }

// MessageListener::MessageListener(const Resource &resource, const message_callback_t& cb)
//     : callback(cb), resource(resource)
// {
// }

// ConsumeStatus MessageListener::handleMessageCB(const std::vector<MQMessageExt> &msgs)
// {
//     if(this->callback){
//         callback(msgs, this->resource);
//     }
// }

// void MessageListener::setMessageCB(const message_callback_t& cb)
// {
//     this->callback = cb;
// }

// void MessageListener::setResouce(const Resource &p)
// {
//     this->resource = p;
// }

// ConsumeStatus MessageListener::consumeMessage(const std::vector<MQMessageExt>& msgs)
// {
//     handleMessageCB(msgs);
// }


// /*------------------------------------------------------*/


// MqConsumer::MqConsumer()
//     :consumer("DefaultGroup")
// {

// }

// MqConsumer::MqConsumer(const std::string& group)
//     :consumer(group)
// {

// }

// MqConsumer::MqConsumer(const std::string& group, const std::string& ip, const std::string& port)
//     :consumer(group)
// {
//     this->setNameServer(ip, port);
// }

// void MqConsumer::setListenrt(MessageListener& listener)
// {
//     this->consumer.registerMessageListener(&listener);
// }

// void MqConsumer::setSubscribe(const std::string& topic, const std::string& tag)
// {
//     this->consumer.subscribe(topic, tag);
// }

// void MqConsumer::setNameServer(const std::string &nameserver)
// {
//     this->consumer.setNamesrvAddr(nameserver);
// }

// void MqConsumer::setNameServer(const std::string &ip, const std::string &port)
// {
//     this->setNameServer(ip + ":" + port);
// }

// void MqConsumer::shutdown()
// {
//     this->consumer.shutdown();
// }

// void MqConsumer::start()
// {
//     this->consumer.start();
// }

