#include "apierror.h"

#include "apiserverjson.h"

#include <QSqlError>

ApiError ApiError::notImplemented()
{
    return ApiError(NotImplemented, "Not implemented");
}

ApiError ApiError::notFound(const QString &resource, const QString &id)
{
    QString errorMessage = QString("Resource '%1' with id '%2' not found").arg(resource, id);

    return ApiError(NotFound, errorMessage);
}

ApiError ApiError::fromJsonError(const JsonError &error)
{
    ErrorCode code = NoError;
    switch (error.type())
    {
        case JsonError::NoError: code = NoError; break;
        case JsonError::ParseError: code = JsonParseError; break;
        case JsonError::TypeError: code = JsonTypeError; break;
        case JsonError::OtherError: code = JsonOtherError; break;
    }

    return ApiError(code, error.reason());
}

ApiError ApiError::fromSqlError(const QSqlError &error)
{
    ApiError::ErrorCode errorCode = ApiError::NoError;
    switch (error.type())
    {
        case QSqlError::NoError: errorCode = NoError; break;
        case QSqlError::ConnectionError: errorCode = DbConnectionError; break;
        case QSqlError::StatementError: errorCode = DbStatementError; break;
        case QSqlError::TransactionError: errorCode = DbTransactionError; break;
        case QSqlError::UnknownError: errorCode = UnknownError; break;
    }

    return ApiError(errorCode, error.text());
}

ApiError::ApiError() : m_errorCode(NoError)
{

}

ApiError::ApiError(ErrorCode code, const QString &message)
    : m_errorCode(code)
    , m_errorMessage(message)
{

}

ApiError::~ApiError() = default;

bool ApiError::isError() const
{
    return m_errorCode != NoError;
}

void ApiError::setError(ErrorCode code, const QString &message)
{
    m_errorCode = code;
    m_errorMessage = message;
}

void ApiError::setErrorCode(ErrorCode code)
{
    m_errorCode = code;
}

ApiError::ErrorCode ApiError::errorCode() const
{
    return m_errorCode;
}

void ApiError::setErrorMessage(const QString &message)
{
    m_errorMessage = message;
}

QString ApiError::errorMessage() const
{
    return m_errorMessage;
}

void to_json(Json &j, const ApiError &error)
{
    j["errorCode"] = error.m_errorCode;
    if (!error.m_errorMessage.isEmpty())
    {
        j["errorMessage"] = error.m_errorMessage;
    }
}

void from_json(const Json &j, ApiError &error)
{
    j["errorCode"].get_to(error.m_errorCode);
    if (j.contains("errorMessage"))
    {
        j["errorMessage"].get_to(error.m_errorMessage);
    }
}

