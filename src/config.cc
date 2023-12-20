#include "config.h"
#include "yaml-cpp/yaml.h"
#include <algorithm>
#include <list>

namespace cpp_high_perf {

Config::ConfigVarMap Config::s_datas;

ConfigVarBase::ptr Config::lookupBase(const std::string& name) {
    auto it = s_datas.find(name);
    return it == s_datas.end() ? nullptr : it->second;
}

//这个地方是获取配置信息的地方
//“A, B”, 10  #等价于下面这个
//A:
//   B: 10
//   C: str
static void listAllMember(const std::string& prefix, const YAML::Node& node, 
    std::list<std::pair<std::string, const YAML::Node>>& output) {
        //std::list<std::pair<std::string, const YAML::Node>>& output 就是输出的结果
        //我的理解yaml文件，读取其实就应该是一行行的读取的
        
        CHPE_LOG_ERROR(CHPE_LOG_ROOT()) << "listAllMember prefix=" << prefix << " node_type=" << node.Type();
        //检索一下非法字符
        if (prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
            CHPE_LOG_ERROR(CHPE_LOG_ROOT()) << "Config invalid name: " << prefix << " : " << node;
            return;
        }
        
        output.push_back(std::make_pair(prefix, node));//把这个节点放进去
        if (node.IsMap()) {
            for (auto it = node.begin(); it != node.end(); ++it) {
                listAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(), it->second, output);
            }
        }
}

void Config::loadFromYaml(const YAML::Node& root) {
    std::list<std::pair<std::string, const YAML::Node>> all_nodes;
    listAllMember("", root, all_nodes);

    for(auto& i : all_nodes) {
        std::string key = i.first;
        if (key.empty()) {
            continue;
        }

        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        ConfigVarBase::ptr var = Config::lookupBase(key);

        if(var) {
            if(i.second.IsScalar()) {
                var->fromString(i.second.Scalar());
                CHPE_LOG_INFO(CHPE_LOG_ROOT()) << "is scalar: " << var->getName();
            } else {
                std::stringstream ss;
                ss << i.second;
                var->fromString(ss.str());
                CHPE_LOG_INFO(CHPE_LOG_ROOT()) << "not scalar: "  << ss.str();
            }
        }
    }
}

}