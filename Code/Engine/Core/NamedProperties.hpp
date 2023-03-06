#pragma once

#include <string>
#include <map>
#include <typeinfo>

typedef unsigned char byte;

class NamedProperties;

char tolower(char in);

std::string tolower(std::string str);

class NamedPropertyBase
{
    friend class NamedProperties;

protected:
    explicit NamedPropertyBase(const type_info& type_info, std::string key) : m_type(type_info), m_key(std::move(key)) {}

public:
    virtual ~NamedPropertyBase() = default;

    NamedPropertyBase(const NamedPropertyBase&) = delete;
    NamedPropertyBase(NamedPropertyBase&&) = delete;

    void operator=(const NamedPropertyBase&) = delete;
    void operator=(NamedPropertyBase&&) = delete;

public:
    template<typename T>
    bool IsOfType() const
    {
        return m_type == typeid(T);
    }

private:
    const type_info& m_type;
    std::string m_key;
};

template<typename T>
class NamedProperty : public NamedPropertyBase
{
public:
    NamedProperty(std::string key) : NamedPropertyBase(typeid(T), key) {}
    explicit NamedProperty(const T& value, std::string key) : NamedPropertyBase(typeid(T), key), m_value(value) {}

    void Set(const T& value)
    {
        m_value = value;
    }

    const T& Get()
    {
        return m_value;
    }

private:
    T m_value;
};

class NamedProperties
{
public:
    template<typename T>
    void SetValue(const std::string& key, const T& value)
    {
        auto& propBase = m_properties[tolower(key)];
        if (propBase)
        {
            if (propBase->IsOfType<T>())
            {
                propBase->m_key = key;
                auto* prop = (NamedProperty<T>*) propBase;
                prop->Set(value);
            }
            else
            {
                delete propBase;
                propBase = new NamedProperty<T>(value, key);
            }
        }
        else
        {
            propBase = new NamedProperty<T>(value, key);
        }
    }

    template<typename T>
    const T& GetValue(const std::string& key, const T& defaultValue)
    {
        auto ite = m_properties.find(tolower(key));
        if (ite != m_properties.end())
        {
            auto& propBase = ite->second;
            if (propBase->IsOfType<T>())
            {
                auto* prop = (NamedProperty<T>*) propBase;
                return prop->Get();
            }
        }
        return defaultValue;
    }

    inline void SetValue(const std::string& key, const char* value)
    {
        SetValue<std::string>(key, value);
    }

    inline std::string GetValue(const std::string& key, const char* defValue)
    {
        auto ite = m_properties.find(tolower(key));
        if (ite != m_properties.end())
        {
            auto& propBase = ite->second;
            if (propBase->IsOfType<std::string>())
            {
                auto* prop = (NamedProperty<std::string>*) propBase;
                return prop->Get();
            }
        }
        return defValue;
    }

private:
    std::map<std::string, NamedPropertyBase*> m_properties;
};
