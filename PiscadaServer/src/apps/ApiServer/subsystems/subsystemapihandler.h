#ifndef PISCADA_API_SERVER_SUBSYSTEM_API_HANDLER_H
#define PISCADA_API_SERVER_SUBSYSTEM_API_HANDLER_H

#include "apihandler.h"


class SubsystemApiHandler : public TypedApiHandler<SubsystemApiHandler>
{
public:
    SubsystemApiHandler(PiMqttClient *client);
    ~SubsystemApiHandler() override;

private:
    void read(const PiMqttMessage &msg);
    void list(const PiMqttMessage &msg);
    void create(const PiMqttMessage &msg);
    void update(const PiMqttMessage &msg);
    void remove(const PiMqttMessage &msg);
};

#endif // PISCADA_API_SERVER_SUBSYSTEM_API_HANDLER_H
