***
>zookeeper 官方只提供了 c client api, 没有c++的api，为了使用更加的方便，接口更加好用，估简单对c client进行了下封装，使c++用户用起来更方便

### 1. 一些静态的函数
这些函数主要是全局作用范围的
```
    /**
     * @brief get the errocode string
     */
    static const char*  error_string(int32_t rc);

    /** 
     * @brief state to string
     */
    static const char*  state_to_string(int32_t state);

    /** 
     * @brief set zookeeper client internal log level
     */
    static void         set_log_lvl(zoo_log_lvl lvl);

    /** 
     * @brief set the log stream
     */
    static void         set_log_stream(FILE* file);
```

### 2. 然后就是对节点的一些常规访问操作接口
- 创建节点
```
	zoo_rc      create_persistent_node(const char* path, const std::string& value, const std::vector<zoo_acl_t>& acl);
	zoo_rc      create_sequence_node(const char* path, const std::string& value, const std::vector<zoo_acl_t>& acl, std::string& returned_path_name);
	zoo_rc      create_ephemeral_node(const char* path, const std::string& value, const std::vector<zoo_acl_t>& acl);
	zoo_rc      create_sequance_ephemeral_node(const char* path, const std::string& value, const std::vector<zoo_acl_t>& acl, std::string& returned_path_name);
```
- 设置节点的值
```
 	zoo_rc      set_node(const char* path, const std::string& value, int32_t version);
```
- 获取节点的值
```
 	zoo_rc      get_node(const char* path, std::string& out_value, zoo_state_t* info, bool watch);
```
- 获取节点的所有子节点
```
	zoo_rc      get_children(const char* path, std::vector<std::string>& children, bool watch);
```
- 删除节点
```
	zoo_rc      delete_node(const char* path, int32_t version);
```
- 节点是否存在
```
	zoo_rc      exists_node(const char* path, zoo_state_t* info, bool watch);
```
- 设置节点的acl
```
	zoo_rc      set_acl(const char* path, const std::vector<zoo_acl_t>& acl, int32_t version);
```
- 获取节点的acl
```
	zoo_rc      get_acl(const char* path, std::vector<zoo_acl_t>& acl);
```
- 添加权限认证
```
	zoo_rc      add_auth(const std::string& user_name, const std::string& user_passwd);
```

### 3. 设置一些事件回调
- 设置节点的值变化的通知回调函数
```
	zoo_rc      watch_data_change(const char* path, const data_change_event_handler_t& handler, std::string* value);
```
- 设置节点的子节点变化(增/减)的通知回调函数
```
	zoo_rc      watch_children_event(const char* path, const child_event_handler_t& handler, std::vector<std::string>* out_children );
```

### 4. 具体的使用见zk_cpp_test.cpp
