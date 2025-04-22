#ifndef PISCADA_API_HANDLER_H
#define PISCADA_API_HANDLER_H

#include "apierror.h"
#include "apiserverjson.h"

#include "pimqtt/pimqttmessage.h"

#include <QString>
#include <QHash>

class PiMqttClient;
class PiMqttMessage;

ApiError getUuidFromTopic(const QString &topic, Uuid &id);
ApiError getStringIdFromTopic(const QString &topic, QString &id);
ApiError getControllerIdStringFromTopic(const QString &topic, QString &controllerId);

class ApiHandler
{
public:
    ApiHandler(PiMqttClient *client);
    virtual ~ApiHandler();

    virtual void handle(const PiMqttMessage &msg) = 0;

protected:
    template<typename T>
    void sendResponse(const PiMqttMessage &req, const T &response)
    {
        Json json = response;

        sendResponse(req, json);
    }

    void sendResponse(const PiMqttMessage &req, const Json &response);
    void sendErrorResponse(const PiMqttMessage &msg, const ApiError &error);

    PiMqttClient *m_client;
};


template<typename T>
class TypedApiHandler : public ApiHandler
{
public:
    TypedApiHandler(PiMqttClient *client) : ApiHandler(client) {}
    ~TypedApiHandler() override = default;

    void handle(const PiMqttMessage &msg) override
    {
        QString topic = msg.topic();
        QStringList path = topic.split('/');
        if (path.size() < 6)
        {
            return sendErrorResponse(msg, ApiError(ApiError::InvalidRequest, "Topic too short"));
        }

        QString action = path.at(5);
        auto it = m_handlers.find(action);
        if (it == m_handlers.end())
        {
            return sendErrorResponse(msg, ApiError(ApiError::NoError, QString("Unknown action: '%1'").arg(action)));
        }

        (static_cast<T*>(this)->*it.value())(msg);
    }

protected:
    using HandlerFn = void (T::*)(const PiMqttMessage &msg);

    QHash<QString, HandlerFn> m_handlers;
};

#endif // PISCADA_API_HANDLER_H
