#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <algorithm>
#include <memory>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <yaml-cpp/node/parse.h>
#include "yaml-cpp/yaml.h"
#include "log.h"

namespace cpp_high_perf {

//共有的属性放大这个基类里面
class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& description = "")
        :m_name(name)
        ,m_description(description) {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }

    virtual ~ConfigVarBase() {}

    const std::string& getName() const { return m_name; }
    const std::string& getDescription() const { return m_description; }

    virtual std::string toString() = 0;//方便调试 / 输出文件
    virtual bool fromString(const std::string& val) = 0;//解析配置文件

private:
    std::string m_name;
    std::string m_description;//描述

};

//定义两个基础类型的转化，担心是基础类型转化出问题，还是用库来转化基础类型吧
template <class F, class T>
class LexicalCast {
public:
    T operator()(const F& str) {
        return boost::lexical_cast<T>(str);
    }
};

//偏特化//string to/from vector<T>
template <class T>
class LexicalCast<std::string, std::vector<T>> {
public:
    std::vector<T> operator()(const std::string& str) {
        YAML::Node node  = YAML::Load(str);
        typename std::vector<T> Vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            Vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return Vec;
    }
};

//偏特化
template <class T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T>& v) {
        YAML::Node node; 
        for (auto& it : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//偏特化//string to/from list<T>
template <class T>
class LexicalCast<std::string, std::list<T>> {
public:
    std::list<T> operator()(const std::string& str) {
        YAML::Node node  = YAML::Load(str);
        typename std::list<T> Vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            Vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return Vec;
    }
};

template <class T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T>& v) {
        YAML::Node node;
        for (auto& it : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//偏特化set
template <class T>
class LexicalCast<std::string, std::set<T>> {
public:
    std::set<T> operator()(const std::string& str) {
        YAML::Node node  = YAML::Load(str);
        typename std::set<T> Vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            Vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return Vec;
    }
};

template <class T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T>& v) {
        YAML::Node node;
        for (auto& it : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
}; 

//偏特化unordered_set
template <class T>
class LexicalCast<std::string, std::unordered_set<T>> {
public:
    std::unordered_set<T> operator()(const std::string& str) {
        YAML::Node node  = YAML::Load(str);
        typename std::unordered_set<T> Vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            Vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return Vec;
    }
};

template <class T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T>& v) {
        YAML::Node node;
        for (auto& it : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//偏特化map
template <class T>
class LexicalCast<std::string, std::map<std::string, T>> {
public:
    std::map<std::string, T> operator()(const std::string& str) {
        YAML::Node node  = YAML::Load(str);
        typename std::map<std::string, T> Vec;
        std::stringstream ss;
        for(auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            Vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return Vec;
    }
};

template <class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T>& v) {
        YAML::Node node;
        for (auto& it : v) {
            node[it.first] = YAML::Load(LexicalCast<T, std::string>()(it.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//偏特化unordered_map
template <class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
public:
    std::unordered_map<std::string, T> operator()(const std::string& str) {
        YAML::Node node  = YAML::Load(str);
        typename std::unordered_map<std::string, T> Vec;
        std::stringstream ss;
        for(auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            Vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return Vec;
    }
};

template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T>& v) {
        YAML::Node node;
        for (auto& it : v) {
            node[it.first] = YAML::Load(LexicalCast<T, std::string>()(it.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


//对于具体的类型，需要继承这个类，类型肯定很多，所以需要定义模板类
//定义仿函数 FromStr T operator()(const std::string& str)
//定义仿函数 ToStr std::string operator()(const T& v)
template<class T, class FromStr = cpp_high_perf::LexicalCast<std::string, T>, 
        class ToStr = cpp_high_perf::LexicalCast<T, std::string>>
class ConfigVar : public cpp_high_perf::ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVar> ptr;
    ConfigVar (const std::string& name, const T& default_valse, const std::string& description = ""):
        ConfigVarBase(name, description)
        ,m_val(default_valse) {

    }

    std::string toString() override {
        try {
            //return boost::lexical_cast<std::string>(m_val);
            return ToStr()(m_val);
        } catch (std::exception& e) {
            CHPE_LOG_ERROR(CHPE_LOG_ROOT()) << "ConfigVar::toString exception"
                << e.what() << " convert: " << typeid(m_val).name() << " to string";
        }
        return "";
    }

    bool fromString(const std::string& val) override {
        try {
            //m_val = boost::lexical_cast<T>(val);
            setValue(FromStr()(val));
        } catch (std::exception& e) {
            CHPE_LOG_ERROR(CHPE_LOG_ROOT()) << "ConfigVar::fromString exception"
                << e.what() << " convert: string to " << typeid(m_val).name();
        }
        return false;
    }

    const T getValue() const { return m_val; }
    void setValue(const T& v) { m_val = v; }

private:
    T m_val;//配置文件里面要写入的值，类型很多(int, string等等)
};

//ConfigVar的管理类
class Config {
public:
    typedef std::map<std::string, cpp_high_perf::ConfigVarBase::ptr> ConfigVarMap;
    
    //一个是创建
    template<class T>
    static typename ConfigVar<T>::ptr lookup(const std::string& name, 
        const T& default_valse, const std::string& description = "") {
            //先看能不能找得到
            auto it = lookup<T>(name);
            if (it != nullptr) {
                CHPE_LOG_INFO(CHPE_LOG_ROOT()) << "Lookup name=" << name << " exists";
                return it;
            }

            //创建之前先判断字符串是否合理，是否有下面这些字符之外的，有就是不合理
            if (name.find_first_not_of("abcdefghigklmnopqrstuvwxyz._012345678")
                != std::string::npos) {
                    CHPE_LOG_ERROR(CHPE_LOG_ROOT()) << "Lookup name invalid " << name;
                    throw std::invalid_argument(name);
            }

            //下面就可以创建了
            typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_valse, description));
            s_datas[name] = v;

            return v;
    }

    //一个是查找
    template<class T>
    static typename ConfigVar<T>::ptr lookup(const std::string& name) {
        auto it = s_datas.find(name);
        if (it == s_datas.end()) {
            return nullptr;
        }

        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);//智能指针的强制类型转化，调用拷贝构造函数吧
    }

    //和yaml来进行交互
    static void loadFromYaml(const YAML::Node& root);

    //查找配置参数,返回配置参数的基类
    static cpp_high_perf::ConfigVarBase::ptr lookupBase(const std::string& name);

private:
    static ConfigVarMap s_datas;

};


}

#endif