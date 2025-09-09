#ifndef PISCADA_API_ERROR_H
#define PISCADA_API_ERROR_H

#include "apiserverjson.h"

#include <QString>
class QSqlError;

class JsonError;


class ApiError
{
public:
    enum ErrorCode
    {
        NoError,
        NotImplemented,
        NotFound,
        InvalidRequest,
        UnknownError,
        ConflictError,
        InternalError,

        JsonParseError,
        JsonTypeError,
        JsonOtherError,

        DbConnectionError,
        DbStatementError,
        DbTransactionError,
    };

    static int statusCodeFromErrorCode(ErrorCode code);

    static ApiError notImplemented();
    static ApiError conflictError(const QString &message);
    static ApiError internalError(const QString &message);

    static ApiError notFound(const QString &resource, const QString &id);
    static ApiError fromJsonError(const JsonError &error);
    static ApiError fromSqlError(const QSqlError &error);
    
    ApiError();
    ApiError(ErrorCode code, const QString &message = QString());
    ~ApiError();

    bool isError() const;

    void setError(ErrorCode code, const QString &message);

    void setErrorCode(ErrorCode code);
    ErrorCode errorCode() const;

    void setErrorMessage(const QString &message);
    QString errorMessage() const;

    void setStatusCode(int statusCode);
    int statusCode() const;

private:
    friend void to_json(Json &j, const ApiError &error);
    friend void from_json(const Json &j, ApiError &error);

    ErrorCode m_errorCode;
    QString m_errorMessage;
    int m_statusCode;

};

NLOHMANN_JSON_SERIALIZE_ENUM(ApiError::ErrorCode, {
    { ApiError::ErrorCode(-1), nullptr },
    { ApiError::NoError, "no-error" },
    { ApiError::NotImplemented, "not-implemented" },
    { ApiError::NotFound, "not-found" },
    { ApiError::InvalidRequest, "invalid-request" },
    { ApiError::UnknownError, "unknown-error" },
    { ApiError::ConflictError, "conflict-error" },
    { ApiError::InternalError, "internal-error" },
    { ApiError::JsonParseError, "json-parse-error" },
    { ApiError::JsonTypeError, "json-type-error" },
    { ApiError::JsonOtherError, "json-other-error" },
    { ApiError::DbConnectionError, "db-connection-error" },
    { ApiError::DbStatementError, "db-statement-error" },
    { ApiError::DbTransactionError, "db-transaction-error" },
});

#endif // PISCADA_API_ERROR_H
