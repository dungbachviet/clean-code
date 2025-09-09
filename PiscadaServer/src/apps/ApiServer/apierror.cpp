#include "apierror.h"

#include "apiserverjson.h"

#include <QSqlError>

int ApiError::statusCodeFromErrorCode(ErrorCode code)
{
    switch (code)
    {
        case NotImplemented:       return 501;
        case NotFound:             return 404;
        case InvalidRequest:       return 400;
        case ConflictError:        return 409;
        case InternalError:        return 500;
        case JsonParseError:       return 400;
        case JsonTypeError:        return 400;
        case JsonOtherError:       return 400;
        case DbConnectionError:    return 500;
        case DbStatementError:     return 500;
        case DbTransactionError:   return 500;
        case UnknownError:         return 500;
        case NoError:              return 200;
        default:                   return 500;
    }
}

ApiError ApiError::notImplemented()
{
    return ApiError(NotImplemented, "Not implemented");
}

ApiError ApiError::conflictError(const QString &message)
{
    return ApiError(ConflictError, message);
}

ApiError ApiError::internalError(const QString &message)
{
    return ApiError(InternalError, message);
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

ApiError::ApiError() 
    : m_errorCode(NoError)
    , m_statusCode(200)
{

}

ApiError::ApiError(ErrorCode code, const QString &message)
    : m_errorCode(code)
    , m_errorMessage(message)
    , m_statusCode(statusCodeFromErrorCode(code))
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

void ApiError::setStatusCode(int statusCode)
{
    m_statusCode = statusCode;
}

int ApiError::statusCode() const
{
    return m_statusCode;
}

void to_json(Json &j, const ApiError &error)
{
    j["errorCode"] = error.m_errorCode;
    j["statusCode"] = error.m_statusCode;
    if (!error.m_errorMessage.isEmpty())
    {
        j["errorMessage"] = error.m_errorMessage;
    }
}

void from_json(const Json &j, ApiError &error)
{
    j["errorCode"].get_to(error.m_errorCode);
    error.m_statusCode = ApiError::statusCodeFromErrorCode(error.m_errorCode);
    if (j.contains("errorMessage"))
    {
        j["errorMessage"].get_to(error.m_errorMessage);
    }
}

