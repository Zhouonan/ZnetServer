#ifndef __ZNS_CONFIG_H__
#define __ZNS_CONFIG_H__

#include <memory>
#include <string>
#include <map>
#include <boost/lexical_cast.hpp>
#include "log.h"

namespace ZnetServer {

class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string &name, const std::string& desciption = "")
        :m_name(name)
        ,m_description(desciption) {}
    virtual ~ConfigVarBase() {}

    const std::string& getName() const { return m_name; }
    const std::string& getDescription() const { return m_description; }

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string& val) = 0;
protected:
    std::string m_name;
    std::string m_description;
};

template<class T>
class ConfigVar : public ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "")
        :ConfigVarBase(name, description), m_value(default_value) {}

    T getValue() const { return m_value;}
    void setValue(const T& value) { m_value = value;}

    std::string toString() override { 
        try {
            return boost::lexical_cast<std::string>(m_value);
        } catch (const std::exception& e) {
            ZNS_LOG_ERROR(ZNS_LOG_ROOT()) << "ConfigVar::toString() error, name=" << m_name
                << " exception: " << e.what();
        }
        return "";
    }
    bool fromString(const std::string& val) override { 
        try {
            m_value = boost::lexical_cast<T>(val);
            return true;
        } catch (const std::exception& e) {
            ZNS_LOG_ERROR(ZNS_LOG_ROOT()) << "ConfigVar::fromString() error, name=" << m_name
                << " exception: " << e.what();
        }
        return false;
    }
private:
    T m_value;
};

class Config {
public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        auto it = s_datas.find(name);
        if (it == s_datas.end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, const T& value, const std::string& description) {
        auto tmp = Lookup<T>(name);
        if (tmp) {
            ZNS_LOG_INFO(ZNS_LOG_ROOT()) << "Config::Lookup name=" << name << " already exists";
            return tmp;
        }
        // 检查name是否合法
        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._") != std::string::npos) {
            ZNS_LOG_ERROR(ZNS_LOG_ROOT()) << "Config::Lookup name is invalid, name=" << name;
            throw std::invalid_argument(name);
        }
        typename ConfigVar<T>::ptr var(new ConfigVar<T>(name, value, description));
        s_datas[name] = var;
        return var;
    }
    private:
        static ConfigVarMap s_datas;
    };
}

#endif