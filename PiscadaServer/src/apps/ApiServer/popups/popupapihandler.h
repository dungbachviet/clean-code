#ifndef PISCADA_API_SERVER_POPUPS_API_HANDLER_H
#define PISCADA_API_SERVER_POPUPS_API_HANDLER_H

#include "apihandler.h"


class PopupApiHandler : public TypedApiHandler<PopupApiHandler>
{
public:
    PopupApiHandler(PiMqttClient *client);
    ~PopupApiHandler() override;

private:
    void read(const PiMqttMessage &msg);
    void list(const PiMqttMessage &msg);
    void create(const PiMqttMessage &msg);
    void clone(const PiMqttMessage &msg);
    void update(const PiMqttMessage &msg);
    void remove(const PiMqttMessage &msg);
};

#endif // PISCADA_API_SERVER_POPUPS_API_HANDLER_H
