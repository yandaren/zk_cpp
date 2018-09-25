#include "zk_cpp/zk_cpp.h"
#include <stdio.h>
#include <iostream>
#include <string>

namespace utils {
    static std::string perms_to_string(int32_t perms) {
        if (perms == utility::zoo_perm_all) {
            return "all";
        }

        std::string ret;
        if (perms & utility::zoo_perm_create) {
            ret.append("c");
        }
        if (perms & utility::zoo_perm_read) {
            ret.append("r");
        }
        if (perms & utility::zoo_perm_delete) {
            ret.append("d");
        }
        if (perms & utility::zoo_perm_write) {
            ret.append("w");
        }
        if (perms & utility::zoo_perm_admin) {
            ret.append("a");
        }
        return ret;
    }

    static int32_t perms_string_to_int(const std::string& perm_str) {
        if (perm_str == "all") {
            return utility::zoo_perm_all;
        }

        int32_t perms = 0;
        for (auto c : perm_str) {
            if (c == 'c') {
                perms |= utility::zoo_perm_create;
            }
            else if (c == 'r') {
                perms |= utility::zoo_perm_read;
            }
            else if (c == 'd') {
                perms |= utility::zoo_perm_delete;
            }
            else if (c == 'w') {
                perms |= utility::zoo_perm_write;
            }
            else if (c == 'a') {
                perms |= utility::zoo_perm_admin;
            }
        }
        return perms;
    }

    static void        string_splits(const char* in_str, const char* sep_str, std::vector<std::string>& out_splits){
        std::string in(in_str);
        std::string sep(sep_str);
        std::size_t start_pos = 0;
        while (start_pos < in.size()){
            std::size_t pos = in.find(sep, start_pos);
            if (pos != std::string::npos){
                out_splits.push_back(std::move(in.substr(start_pos, pos - start_pos)));
                start_pos = pos + 1;
            }
            else{
                out_splits.push_back(std::move(in.substr(start_pos, in.size() - start_pos)));
                break;
            }
        }
    }
}

void print_zk_cpp_usage() {
    fprintf(stderr, "usage\n");
    fprintf(stderr, "    create <path> <value> <flag>\n"
                    "                          0 - persistence\n"
                    "                          1 - ephemeral \n"
                    "                          2 - sequence \n"
                    "                          3 - sequence and ephemeral\n");
    fprintf(stderr, "    delete <path>\n");
    fprintf(stderr, "    set <path> <data>\n");
    fprintf(stderr, "    get <path>\n");
    fprintf(stderr, "    ls <path>\n");
    fprintf(stderr, "    exists <path>\n");
    fprintf(stderr, "    setacl <path> scheme:id:perm\n");
    fprintf(stderr, "    getacl <path>\n");
    fprintf(stderr, "    addauth username passwd\n");
    fprintf(stderr, "    watch_data <path> \n");
    fprintf(stderr, "    watch_child <path> \n");
}

void data_change_event(const std::string& path, const std::string& new_value) {
    printf("data_change_event, path[%s] new_data[%s]\n", path.c_str(), new_value.c_str());
}

void child_change_events(const std::string& path, const std::vector<std::string>& children) {
    printf("child_change_events, path[%s] new_child_count[%d]\n", path.c_str(), (int32_t)children.size());

    for (int32_t i = 0; i < (int32_t)children.size(); ++i) {
        printf("%d, %s\n", i, children[i].c_str());
    }
}

int main() {
    printf("zk_cpp test\n");

    /** format is "127.0.0.1:3000,127.0.0.1:3001,127.0.0.1:3002" */
    std::string urls;

    printf("url formt is '127.0.0.1:3000,127.0.0.1:3001,127.0.0.1:3002'\n");
    printf("input zk server urls:\n");
    std::cin >> urls;

    utility::zk_cpp zk;
    
    do {
        utility::zoo_rc ret = zk.connect(urls);
        if (ret != utility::z_ok) {
            printf("try connect zk server failed, code[%d][%s]\n", 
                ret, utility::zk_cpp::error_string(ret));
            break;
        }

        print_zk_cpp_usage();

        std::string cmd;
        while (std::cin >> cmd) {
            if (cmd == "create") {
                std::string path, value;
                int32_t flag;

                std::cin >> path >> value >> flag;

                std::string rpath = path;
                utility::zoo_rc ret = utility::z_ok;
                if (flag == 0) {
                    std::vector<utility::zoo_acl_t> acl;
                    acl.push_back(utility::zk_cpp::create_world_acl(utility::zoo_perm_all));
                    ret = zk.create_persistent_node(path.c_str(), value, acl);
                }
                else if (flag == 1) {
                    std::vector<utility::zoo_acl_t> acl;
                    acl.push_back(utility::zk_cpp::create_world_acl(utility::zoo_perm_all));
                    ret = zk.create_ephemeral_node(path.c_str(), value, acl);
                }
                else if (flag == 2) {
                    std::vector<utility::zoo_acl_t> acl;
                    acl.push_back(utility::zk_cpp::create_world_acl(utility::zoo_perm_all));
                    ret = zk.create_sequence_node(path.c_str(), value, acl, rpath);
                }
                else if (flag == 3) {
                    std::vector<utility::zoo_acl_t> acl;
                    acl.push_back(utility::zk_cpp::create_world_acl(utility::zoo_perm_all));
                    ret = zk.create_sequance_ephemeral_node(path.c_str(), value, acl, rpath);
                }
                else {
                    printf("invalid create path flag[%d]\n", flag);
                    continue;
                }

                printf("create path[%s] flag[%d] ret[%d][%s], rpath[%s]\n",
                    path.c_str(), flag, ret, utility::zk_cpp::error_string(ret), rpath.c_str());
            }
            else if (cmd == "get") {
                std::string path;
                std::string value;

                std::cin >> path;

                utility::zoo_rc ret = zk.get_node(path.c_str(), value, nullptr, true);
                printf("try get path[%s]'s value, value[%s] ret[%d][%s]\n",
                    path.c_str(), value.c_str(), ret, utility::zk_cpp::error_string(ret));
            }
            else if (cmd == "set") {
                std::string path;
                std::string value;

                std::cin >> path >> value;

                utility::zoo_rc ret = zk.set_node(path.c_str(), value, -1);
                printf("try set path[%s]'s value to [%s] ret[%d][%s]\n",
                    path.c_str(), value.c_str(), ret, utility::zk_cpp::error_string(ret));
            }
            else if (cmd == "exist") {
                std::string path;

                std::cin >> path;

                utility::zoo_rc ret = zk.exists_node(path.c_str(), nullptr, true);
                printf("try_check path[%s] exist[%d], ret[%d][%s]\n",
                    path.c_str(), ret == utility::z_ok, ret, utility::zk_cpp::error_string(ret));
            }
            else if (cmd == "ls") {
                std::string path;
                std::vector<std::string> children;

                std::cin >> path;

                utility::zoo_rc ret = zk.get_children(path.c_str(), children, true);
                printf("try get path[%s]'s children's, children count[%d], ret[%d][%s]\n",
                    path.c_str(), (int32_t)children.size(), ret, utility::zk_cpp::error_string(ret));

                std::string list;
                list.append("[");

                for (int32_t i = 0; i < (int32_t)children.size(); ++i) {
                    //printf("%s\n", children[i].c_str());
                    list.append(children[i]).append(", ");
                }

                list.append("]");
                printf("%s\n", list.c_str());
            }
            else if (cmd == "delete") {
                std::string path;

                std::cin >> path;

                utility::zoo_rc ret = zk.delete_node(path.c_str(), -1);
                printf("try delete path[%s], ret[%d][%s]\n",
                    path.c_str(), ret, utility::zk_cpp::error_string(ret));
            }
            else if (cmd == "setacl") {
                /* setacl <path> scheme:id:perm */
                std::string path;
                std::string acl_string;

                std::cin >> path >> acl_string;

                std::vector<std::string> splits;

                utils::string_splits(acl_string.c_str(), ":", splits);

                if (splits.size() < 3) {
                    printf("acl formt[%s] error\n", acl_string.c_str());
                    continue;
                }
                std::vector<utility::zoo_acl_t> acl;

                const std::string& scheme = splits[0];
                int32_t perms = 0; 
                if (scheme == "world") {
                    const std::string& id = splits[1];
                    if (id != "anyone") {
                        printf("acl world formt error, id[%s]\n", id.c_str());
                        continue;
                    }
                    const std::string& perm_str = splits[2];
                    perms = utils::perms_string_to_int(perm_str);

                    auto ac = utility::zk_cpp::create_world_acl(perms);
                    acl.push_back(ac);
                }
                else if (scheme == "auth") {
                    // "id" is empty
                    const std::string& perm_str = splits[2];
                    perms = utils::perms_string_to_int(perm_str);

                    auto ac = utility::zk_cpp::create_auth_acl(perms);
                    acl.push_back(ac);
                }
                else if (scheme == "digest") {
                    if (splits.size() < 4) {
                        printf("acl digest formt error, acl_str[%s]\n", acl_string.c_str());
                        continue;
                    }

                    const std::string& user_name = splits[1];
                    const std::string& passwd = splits[2];
                    const std::string& perm_str = splits[3];
                    perms = utils::perms_string_to_int(perm_str);

                    auto ac = utility::zk_cpp::create_digest_acl(perms, user_name, passwd);
                    acl.push_back(ac);
                }
                else if (scheme == "ip") {
                    const std::string& id = splits[1];
                    const std::string& perm_str = splits[2];
                    perms = utils::perms_string_to_int(perm_str);
                    auto ac = utility::zk_cpp::create_ip_acl(perms, id);
                    acl.push_back(ac);
                }
                else {
                    printf("unsupported scheme[%s]\n", scheme.c_str());
                    continue;
                }

                auto ret = zk.set_acl(path.c_str(), acl, -1);
                printf("set acl for path[%s], ret[%d][%s]\n", path.c_str(), ret, utility::zk_cpp::error_string(ret));
            }
            else if (cmd == "getacl") {
                std::string path;
                std::cin >> path;

                std::vector<utility::zoo_acl_t> acl;
                utility::zoo_rc rt = zk.get_acl(path.c_str(), acl);
                printf("get acl of path[%s], ret[%d][%s], acl_count[%d]\n",
                    path.c_str(), ret, utility::zk_cpp::error_string(ret), (int32_t)acl.size());

                for (auto& ac : acl) {
                    printf("%s:%s:%s\n", ac.scheme.c_str(), ac.id.c_str(), utils::perms_to_string(ac.perm).c_str());
                }
            }
            else if (cmd == "addauth") {
                std::string user_name;
                std::string user_passwd;
                std::cin >> user_name >> user_passwd;

                utility::zoo_rc rt = zk.add_auth(user_name, user_passwd);
                printf("add_auth, ret[%d][%s].\n", ret, utility::zk_cpp::error_string(ret));
            }
            else if (cmd == "watch_data") {
                std::string path;

                std::cin >> path;
                std::string value;

                utility::zoo_rc ret = zk.watch_data_change(path.c_str(), data_change_event, &value);

                printf("try watch_data change of path[%s], value[%s] ret[%d][%s]\n",
                    path.c_str(), value.c_str(), ret, utility::zk_cpp::error_string(ret));
            }
            else if (cmd == "watch_child") {
                std::string path;
                std::cin >> path;

                std::vector<std::string> children;

                utility::zoo_rc ret = zk.watch_children_event(path.c_str(), child_change_events, &children);

                printf("try watch_child change of path[%s], child_count[%d] ret[%d][%s]\n",
                    path.c_str(), (int32_t)children.size(), ret, utility::zk_cpp::error_string(ret));

                for (int32_t i = 0; i < (int32_t)children.size(); ++i) {
                    printf("%d, %s\n", i, children[i].c_str());
                }
            }
            else {
                printf("unsupported msg[%s]\n", cmd.c_str());
            }
        }

    } while (0);

#ifdef _WIN32
    system("pause");
#endif

    return 0;
}