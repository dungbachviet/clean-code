#ifndef PISCADA_API_SERVER_JSON_H
#define PISCADA_API_SERVER_JSON_H

#include "piscada/optional.h"
#include "piscada/uuid.h"

#include "nlohmann/json.hpp"

#include <QString>
#include <QByteArray>
#include <QHash>
#include <QMap>
#include <QVector>
#include <QList>
#include <QUuid>
#include <QDateTime>
#include <QSet>
#include <QVariant>

#if __cplusplus >= 201703L
#include <optional>
#endif


class JsonError
{
public:
    enum ErrorType
    {
        NoError,
        ParseError,
        TypeError,
        OtherError,
    };

    JsonError() : m_type(JsonError::NoError) {}
    JsonError(ErrorType type, std::string what)
    {
        m_type = type;
        m_reason = QString::fromStdString(what);
    }

    JsonError(const JsonError &other) = default;
    JsonError &operator=(const JsonError &rhs) = default;

    inline ErrorType type() const
    {
        return m_type;
    }

    inline QString reason() const
    {
        return m_reason;
    }

private:
    ErrorType m_type;
    QString m_reason;
};

using Json = nlohmann::json;
using JsonArray = Json::array_t;
using JsonObject = Json::object_t;

namespace JsonUtil
{
    /**
     * Parse a UTF-8 string.
     */
    inline Json parse(const QByteArray &payload, JsonError *error = nullptr)
    {
        try
        {
            return nlohmann::json::parse(payload.data());
        }
        catch (const ::nlohmann::json::parse_error &e)
        {
            if (error)
            {
                *error = JsonError(JsonError::ParseError, e.what());
            }

            return Json();
        }
        catch (const ::nlohmann::json::exception &e)
        {
            if (error)
            {
                *error = JsonError(JsonError::ParseError, e.what());
            }

            return Json();
        }
    }

    /**
     * Returns a UTF-8 encoded string of the JSON type contained by this class.
     */
    inline QByteArray serialize(Json json, JsonError *error = nullptr)
    {
        try
        {
            std::string s = json.dump();
            return QByteArray(s.data());
        }
        catch (const ::nlohmann::json::type_error &e)
        {
            if (error)
            {
                *error = JsonError(JsonError::TypeError, e.what());
            }

            return QByteArray();
        }
    }

    template <typename T, typename = std::enable_if<std::is_default_constructible<T>::value>>
    static inline T get(Json json, JsonError *error = nullptr)
    {
        try
        {
            return json.get<T>();
        }
        catch (const ::nlohmann::json::other_error &e)
        {
            // Might want to just rethrow in this case?
            if (error)
            {
                *error = JsonError(JsonError::OtherError, e.what());
            }

            return T();
        }
        catch (const ::nlohmann::json::type_error &e)
        {
            if (error)
            {
                *error = JsonError(JsonError::TypeError, e.what());
            }

            return T();
        }
        catch (const ::nlohmann::json::out_of_range &e)
        {
            if (error)
            {
                *error = JsonError(JsonError::ParseError, e.what());
            }

            return T();
        }
        catch (const ::nlohmann::json::exception &e)
        {
            if (error)
            {
                *error = JsonError(JsonError::ParseError, e.what());
            }

            return T();
        }
    }

    template<typename T, typename = std::enable_if<std::is_default_constructible<T>::value>>
    static inline JsonError decode(const QByteArray &payload, T *value)
    {
        JsonError error;
        Json json = JsonUtil::parse(payload, &error);
        if (error.type() != JsonError::NoError)
        {
            return error;
        }

        *value = get<T>(json, &error);

        return error;
    }

    template<typename T>
    static inline QByteArray encode(const T &value, JsonError *error = nullptr)
    {
        Json json = value;

        return serialize(json, error);
    }
}

/**
 * Serialization for common Qt types.
 */
namespace nlohmann
{
#if __cplusplus >= 201703L
    template<typename T>
    struct adl_serializer<std::optional<T>>
    {
        static void to_json(json& j, const std::optional<T>& opt)
        {
            if (opt == std::nullopt)
            {
                j = nullptr;
            }
            else
            {
                j = *opt;
            }
        }

        static void from_json(const json& j, std::optional<T>& opt)
        {
            if (j.is_null())
            {
                opt = std::nullopt;
            }
            else
            {
                opt = j.get<T>();
            }
        }
    };
#endif

    template<typename T>
    struct adl_serializer<piscada::optional<T>>
    {
        static void to_json(json& j, const piscada::optional<T>& opt)
        {
            if (!opt.has_value())
            {
                j = nullptr;
            }
            else
            {
                j = *opt;
            }
        }

        static void from_json(const json& j, piscada::optional<T>& opt)
        {
            if (j.is_null())
            {
                opt = piscada::nullopt;
            }
            else
            {
                opt = j.get<T>();
            }
        }
    };

    template <>
    struct adl_serializer<QString>
    {
        static void to_json(json &j, const QString &string)
        {
            j = string.toStdString();
        }

        static void from_json(const json &j, QString &string)
        {
            std::string s = j.get<std::string>();
            string = QString::fromStdString(s);
        }
    };

    template<>
    struct adl_serializer<QByteArray>
    {
        static void to_json(json &j, const QByteArray &bytes)
        {
            j = bytes.toStdString();
        }

        static void from_json(const json &j, QByteArray &bytes)
        {
            std::string b = j.get<std::string>();
            bytes = QByteArray::fromStdString(b);
        }
    };

    template <typename Value>
    struct adl_serializer<QHash<QString, Value>>
    {
        static void to_json(json &j, const QHash<QString, Value> &value)
        {
            j = json::object_t();
            for (const QString &key : value.keys())
            {
                std::string k = key.toStdString();
                j[k] = value[key];
            }
        }

        static void from_json(const json &j, QHash<QString, Value> &value)
        {
            for (auto &el : j.items())
            {
                value.insert(QString::fromStdString(el.key()), el.value().get<Value>());
            }
        }
    };

    template<typename Value>
    struct adl_serializer<QHash<Uuid, Value>>
    {
        static void to_json(json &j, const QHash<Uuid, Value> &value)
        {
            j = json::object_t();
            for (const QUuid &key : value.keys())
            {
                QString stringKey = key.toString();
                std::string k = stringKey.toStdString();
                j[k] = value[key];
            }
        }

        static void from_json(const json &j, QHash<Uuid, Value> &value)
        {
            for (auto &el : j.items())
            {
                QString stringKey = QString::fromStdString(el.key());
                value.insert(Uuid::fromString(stringKey), el.value().get<Value>());
            }
        }
    };

    template <typename Value>
    struct adl_serializer<QMap<QString, Value>>
    {
        static void to_json(json &j, const QMap<QString, Value> &value)
        {
            j = json::object_t();
            for (const QString &key : value.keys())
            {
                std::string k = key.toStdString();
                j[k] = value[key];
            }
        }

        static void from_json(const json &j, QMap<QString, Value> &value)
        {
            for (auto &el : j.items())
            {
                value.insert(QString::fromStdString(el.key()), el.value().get<Value>());
            }
        }
    };

    template <typename T>
    struct adl_serializer<QVector<T>>
    {
        static void to_json(json &j, const QVector<T> &value)
        {
            j = json::array_t();
            for (const T &v : value)
            {
                j.push_back(v);
            }
        }

        static void from_json(const json &j, QVector<T> &value)
        {
            for (const json &e : j)
            {
                value.append(e.get<T>());
            }
        }
    };

    template<>
    struct adl_serializer<QStringList>
    {
        static void to_json(json &j, const QStringList &value)
        {
            j = json::array_t();
            for (const QString &v : value)
            {
                j.push_back(v.toStdString());
            }
        }

        static void from_json(const json &j, QStringList &value)
        {
            for (const json &e : j)
            {
                value.append(QString::fromStdString(e.get<std::string>()));
            }
        }
    };

    template <typename T>
    struct adl_serializer<QList<T>>
    {
        static void to_json(json &j, const QList<T> &value)
        {
            j = json::array_t();
            for (const T &v : value)
            {
                j.push_back(v);
            }
        }

        static void from_json(const json &j, QList<T> &value)
        {
            for (const json &e : j)
            {
                value.append(e.get<T>());
            }
        }
    };

    template<typename T>
    struct adl_serializer<QSet<T>>
    {
        static void to_json(json &j, const QSet<T> &value)
        {
            j = json::array_t();
            for (const T &v : value)
            {
                j.push_back(v);
            }
        }

        static void from_json(const json &j, QSet<T> &value)
        {
            for (const json &e : j)
            {
                value.insert(e.get<T>());
            }
        }
    };

    template<>
    struct adl_serializer<QUuid>
    {
        static void to_json(json &j, const QUuid &value)
        {
            if (value.isNull())
            {
                j = nullptr;
            }
            else
            {
                QString string = value.toString();
                string.remove("{").remove("}");
                j = string;
            }
        }

        static void from_json(const json &j, QUuid &value)
        {
            QString string = j.get<QString>();
            QUuid uuid(string);

            if (uuid.isNull())
            {
                value = nullptr;
            }
            else
            {
                value = uuid;
            }
        }
    };

    template<>
    struct adl_serializer<Uuid>
    {
        static void to_json(json &j, const Uuid &value)
        {
            if (value.isNull())
            {
                j = nullptr;
            }
            else
            {
                QString string = value.toString();
                string.remove("{").remove("}");
                j = string;
            }
        }

        static void from_json(const json &j, Uuid &value)
        {
            QString string = j.get<QString>();
            Uuid uuid(string);

            if (uuid.isNull())
            {
                value = Uuid();
            }
            else
            {
                value = uuid;
            }
        }
    };

    template<>
    struct adl_serializer<QDateTime>
    {
        static void to_json(json &j, const QDateTime &value)
        {
            if (value.isValid())
            {
                j = value.toMSecsSinceEpoch();
            }
            else
            {
                j = nullptr;
            }
        }

        static void from_json(const json &j, QDateTime &value)
        {
            if (j.is_null())
            {
                value = QDateTime();
            }
            else
            {
                qint64 timestamp = j.get<qint64>();
                value = QDateTime::fromMSecsSinceEpoch(timestamp);
            }
        }
    };
    
    template<>
    struct adl_serializer<QVariant>
    {
        static void to_json(json &j, const QVariant &value)
        {
            if (value.isNull())
            {
                j = nullptr;
            }

            switch (value.type())
            {
                case QVariant::Bool: j = value.toBool(); break;
                case QVariant::Int: j = value.toInt(); break;
                case QVariant::UInt: j = value.toUInt(); break;
                case QVariant::LongLong: j = value.toLongLong(); break;
                case QVariant::ULongLong: j = value.toULongLong(); break;
                case QVariant::Double: j = value.toDouble(); break;
                case QVariant::String: j = value.toString(); break;
                default: j = "Invalid QVariant type"; break;
            }
                
        }

        static void from_json(const json &j, QVariant &value)
        {
            switch (j.type())
            {
                case json::value_t::null:
                {
                    value = QVariant();
                } break;
                case json::value_t::object:
                {
                    QVariantHash v;
                    j.get_to(v);
                } break;
                case json::value_t::array:
                {
                    QVariantList v;
                    j.get_to(v);
                } break;
                case json::value_t::string:
                {
                    QString v;
                    j.get_to(v);
                    value = v;
                } break;
                case json::value_t::boolean:
                {
                    bool v;
                    j.get_to(v);
                    value = v;
                } break;
                case json::value_t::number_integer:
                {
                    int v;
                    j.get_to(v);
                    value = v;
                } break;
                case json::value_t::number_unsigned:
                {
                    uint v;
                    j.get_to(v);
                    value = v;
                } break;
                case json::value_t::number_float:
                {
                    double v;
                    j.get_to(v);
                    value = v;
                } break;
                case json::value_t::binary:
                {
                    QByteArray v;
                    j.get_to(v);
                    value = v;
                } break;
                case json::value_t::discarded:
                {
                    value = "Discarded JSON value";
                } break;
            }
        }
    };
}

#endif

