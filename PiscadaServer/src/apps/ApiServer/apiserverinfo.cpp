#include "apiserverinfo.h"

#include "serverinfo.h"


ApiServerInfo info;


ApiServerInfo::ApiServerInfo() : m_initialized(false)
{

}

ApiServerInfo::~ApiServerInfo() = default;

bool ApiServerInfo::startUp()
{
    m_controllerId = ServerInfo::instance()->getControllerId(); 
    m_initialized = true;

    return true;
}

bool ApiServerInfo::shutDown()
{
    m_initialized = false;

    return true;
}

QString ApiServerInfo::controllerId() const
{
    Q_ASSERT_X(m_initialized, Q_FUNC_INFO, "ApiServerInfo not initialized");

    return m_controllerId;
}

