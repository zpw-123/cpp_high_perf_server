#include "../src/log.h"
#include "../src/config.h"
#include "yaml-cpp/yaml.h"
#include <cstddef>
#include <string>

//上面实现的是简单类型的配置信息，比如int float等等；但是还有一些自定义类型
cpp_high_perf::ConfigVar<int>::ptr g_int_value_config = 
    cpp_high_perf::Config::lookup("system.port", (int)8080, "system port");//只有这个才往map里面放数据

cpp_high_perf::ConfigVar<std::vector<int> >::ptr g_int_vec_value_config = 
    cpp_high_perf::Config::lookup("system.int_vec", std::vector<int>{1, 2}, "system int vec");

cpp_high_perf::ConfigVar<std::list<int> >::ptr g_int_list_value_config = 
    cpp_high_perf::Config::lookup("system.int_list", std::list<int>{1, 2}, "system int list");

cpp_high_perf::ConfigVar<std::set<int> >::ptr g_int_set_value_config = 
    cpp_high_perf::Config::lookup("system.int_set", std::set<int>{1, 2}, "system int set");

cpp_high_perf::ConfigVar<std::unordered_set<int> >::ptr g_int_uset_value_config = 
    cpp_high_perf::Config::lookup("system.int_uset", std::unordered_set<int>{1, 2}, "system int uset");

cpp_high_perf::ConfigVar<std::map<std::string, int> >::ptr g_str_int_map_value_config = 
    cpp_high_perf::Config::lookup("system.str_int_map", std::map<std::string, int>{{"k", 2}}, "system str int map");

cpp_high_perf::ConfigVar<std::unordered_map<std::string, int> >::ptr g_str_int_umap_value_config = 
    cpp_high_perf::Config::lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"k", 2}}, "system str int umap");

//然后就应该解析yaml文件, 这个level是递归的层数, type就是类型是 map/sequence/scalar
void print_yaml(const YAML::Node& node, int level) {
    if (node.IsScalar()) {
        CHPE_LOG_INFO(CHPE_LOG_ROOT()) << std::string(level * 4, ' ') << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if (node.IsNull()) {
        CHPE_LOG_INFO(CHPE_LOG_ROOT()) << std::string(level * 4, ' ') << "NULL - " << node.Type() << " - " << level;
    } else if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            CHPE_LOG_INFO(CHPE_LOG_ROOT()) << std::string(level * 4, ' ') << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if (node.IsSequence()) {
        for (size_t i = 0; i < node.size(); ++i) {
            CHPE_LOG_INFO(CHPE_LOG_ROOT()) << std::string(level * 4, ' ') << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

//测试一下yaml安装功能是否可以正常使用
void test_yaml() {
    //是加载进来了配置文件
    YAML::Node root = YAML::LoadFile("/home/zpw/cpp_high_perf_server/conf/log.yaml");
    print_yaml(root, 0);
    //CHPE_LOG_INFO(CHPE_LOG_ROOT()) << root;
}

void test_config() {
    CHPE_LOG_INFO(CHPE_LOG_ROOT()) << "before: " << g_int_value_config->getValue();

#define XX(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for (auto& i : v) { \
            CHPE_LOG_INFO(CHPE_LOG_ROOT()) << #prefix " " #name ": " << i; \
        } \
        CHPE_LOG_INFO(CHPE_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

#define XX_M(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for (auto& i : v) { \
            CHPE_LOG_INFO(CHPE_LOG_ROOT()) << #prefix " " #name ": {" \
                << i.first << " - " << i.second << "}"; \
        } \
        CHPE_LOG_INFO(CHPE_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }
    
    XX(g_int_vec_value_config, int_vec, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_uset_value_config, int_uset, before);
    XX_M(g_str_int_map_value_config, str_int_map, before);
    XX_M(g_str_int_umap_value_config, str_int_umap, before);

    YAML::Node root = YAML::LoadFile("/home/zpw/cpp_high_perf_server/conf/log.yaml");
    cpp_high_perf::Config::loadFromYaml(root);

    CHPE_LOG_INFO(CHPE_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
    XX(g_int_vec_value_config, int_vec, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_uset_value_config, int_uset, after);
    XX_M(g_str_int_map_value_config, str_int_map, after);
    XX_M(g_str_int_umap_value_config, str_int_umap, after);
}

int main(int argc, char** argv) {
    //test_yaml();
    test_config();

    //test_yaml();
    return 0;
}