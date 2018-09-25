/** 
 * @brief zk_cpp
 *
 * a c++ wrapper of zookeeper c apis
 *
 * @author  :   yandaren1220@126.com
 * @date    :   2018-09-25
 */

#ifndef __utility_common_zk_cpp_h__
#define __utility_common_zk_cpp_h__

#ifdef _WIN32
#ifndef WIN32
#define WIN32
#endif
#ifndef _WINDOWS
#define _WINDOWS
#endif
#endif

/** use the lib zookeeper as static lib */
#ifndef USE_STATIC_LIB
#define USE_STATIC_LIB
#endif

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <functional>

namespace utility {

/** error code */
enum zoo_rc {
    z_ok                          = 0,  /*!< Everything is OK */

   /** System and server-side errors.
    * This is never thrown by the server, it shouldn't be used other than
    * to indicate a range. Specifically error codes greater than this
    * value, but lesser than {@link #api_error}, are system errors. */
    z_system_error                = -1,
    z_runtime_inconsistency       = -2, /*!< A runtime inconsistency was found */
    z_data_inconsistency          = -3, /*!< A data inconsistency was found */
    z_connection_loss             = -4, /*!< Connection to the server has been lost */
    z_marshalling_error           = -5, /*!< Error while marshalling or unmarshalling data */
    z_unimplemented               = -6, /*!< Operation is unimplemented */
    z_operation_timeout           = -7, /*!< Operation timeout */
    z_bad_arguments               = -8, /*!< Invalid arguments */
    z_invliad_state               = -9, /*!< Invliad zhandle state */

   /** API errors.
    * This is never thrown by the server, it shouldn't be used other than
    * to indicate a range. Specifically error codes greater than this
    * value are API errors (while values less than this indicate a
    * {@link #system_error}).
    */
    z_api_error                   = -100,
    z_no_node                     = -101, /*!< Node does not exist */
    z_no_auth                     = -102, /*!< Not authenticated */
    z_bad_version                 = -103, /*!< Version conflict */
    z_no_children_for_ephemeral   = -108, /*!< Ephemeral nodes may not have children */
    z_node_exists                 = -110, /*!< The node already exists */
    z_not_empty                   = -111, /*!< The node has children */
    z_session_expired             = -112, /*!< The session has been expired by the server */
    z_invalid_callback            = -113, /*!< Invalid callback specified */
    z_invalid_acl                 = -114, /*!< Invalid ACL specified */
    z_auth_failed                 = -115, /*!< Client authentication failed */
    z_closing                     = -116, /*!< ZooKeeper is closing */
    z_nothing                     = -117, /*!< (not error) no server responses to process */
    z_session_moved               = -118  /*!<session moved to another server, so operation is ignored */
};

/** permissions(ACL Consts)*/
enum zoo_perm {
    zoo_perm_read    = 1 << 0,
    zoo_perm_write   = 1 << 1,
    zoo_perm_create  = 1 << 2,
    zoo_perm_delete  = 1 << 3,
    zoo_perm_admin   = 1 << 4,
    zoo_perm_all     = 0x1f,
};

enum zoo_scheme {
    zoo_scheme_world = 0,
    zoo_scheme_auth = 1,
    zoo_scheme_digest = 2,
    zoo_scheme_ip = 3,
};

struct zoo_acl_t {
    std::string scheme; // one of { 'world', 'auth', 'digest', 'ip'}
    std::string id;     // the value type is different case the scheme
    int32_t     perm;   // see {@link #zoo_perm}

    zoo_acl_t() : perm(0){}
    zoo_acl_t(const char* _scheme, const char* _id, int32_t _perm)
        : scheme(_scheme), id(_id), perm(_perm) {
    }
};

/** zoo_state 
 * These constants represent the states of a zookeeper connection. They are
 * possible parameters of the watcher callback.
 */
enum z_state {
    zoo_state_expired_session   = -112,
    zoo_state_auth_failed       = -113,
    zoo_state_closed            = 0,
    zoo_state_connecting        = 1,
    zoo_state_associating       = 2,
    zoo_state_connected         = 3,
};

/** internal log lvl */
enum zoo_log_lvl {
    zoo_log_lvl_error = 1,
    zoo_log_lvl_warn = 2,
    zoo_log_lvl_info = 3,
    zoo_log_lvl_debug = 4,
};

/** zoo node info */
struct zoo_state_t {
    int64_t ctime;              // node create time
    int64_t mtime;              // node last modify time
    int32_t version;            // node version
    int32_t children_count;     // the number of children of the node
};

namespace utils {

class noncopyable
{
protected:
    noncopyable() {}
    ~noncopyable() {}

private:
    noncopyable(const noncopyable&);
    noncopyable& operator=(const noncopyable&);
};
}

class zk_cpp : public utils::noncopyable
{
public:
    /** callbacks */
    /** watch events */

    /** the  path data change event call back */
    typedef std::function<void(const std::string& path, const std::string& new_value)> data_change_event_handler_t;
    typedef std::shared_ptr<data_change_event_handler_t> data_event_handler_ptr;

    typedef std::function<void(const std::string& path, const std::vector<std::string>& new_children)> child_event_handler_t;
    typedef std::shared_ptr<child_event_handler_t>       child_event_handler_ptr;

protected:
    typedef std::unordered_map<std::string, data_event_handler_ptr>     data_event_map_type;
    typedef std::unordered_map<std::string, child_event_handler_ptr>    child_event_map_type;

protected:
    void*                   m_zh;       // zhandle_t
    std::string             m_url;      // zookeeper server urls

    std::mutex              m_mtx;
    data_event_map_type     m_data_event_map;
    child_event_map_type    m_child_event_map;

public:
    zk_cpp();
    ~zk_cpp();

public:
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

    /** 
     * @brief create world acl
     * @param perms     - see {@link #zoo_perm}
     */
    static zoo_acl_t    create_world_acl(int32_t perms);

    /** 
     * @brief create auth acl
     * @param perms     - see {@link #zoo_perm}
     * @notice:         auth_acl need the session {@link #addauth} first, 
                        and then use the authentic user:passwd as id, scheme is 'auth';
                        if not addauth yet, will return {@link #z_invalid_acl }
     */
    static zoo_acl_t    create_auth_acl(int32_t perms);

    /** 
     * @brief create digest acl
     * @param perms     - see {@link #zoo_perm}
     * @param user      - username
     * @param passwd    - the user passwd(the passwd should be BASE64(SHA1(realpasswd)))
     */
    static zoo_acl_t    create_digest_acl(int32_t perms, const std::string& user, const std::string& passwd);

    /** 
     * @brief create ip acl
     * @param perms     - see {@link #zoo_perm}
     * @param ip_info   - the form is addr/bits where the most significant bits of addr 
                        - are matched against the most significant bits of the client host IP.
                        - eg:  ip:192.168.1.0/16, the 16 bits of the '192.168.1.0' is 192.168,
                        - the expression matches all the ip start with '192.168'
     */
    static zoo_acl_t    create_ip_acl(int32_t perms, const std::string& ip_info);

public:
    /** 
     * @brief try connect zookeeper servers
     * 
     * @param url : format is "127.0.0.1:3000,127.0.0.1:3001,127.0.0.1:3002"
     */
    zoo_rc      connect(const std::string& url);

    /**
     * @brief return the timeout for this session, only valid if the connections
     * is currently connected, This value may change after a server re-connect.
     */
    int32_t     get_recv_time_out();

    /** 
     * @brief get current state
     */
    z_state     get_state();

    /**
     * @brief checks if the current zookeeper connection state can't be recovered.
     *
     *  The application must close the zhandle and try to reconnect.
     *
     */
    bool        unrecoverable();

public:
    /** events */

    /** event sesion connected */
    void        on_session_connected();

    /** event session expired */
    void        on_session_expired();

    /** event node created */
    void        on_path_created(const char* path);

    /** event node delete */
    void        on_path_delete(const char* path);

    /** event path data change */
    void        on_path_data_change(const char* path);

    /** event path child change */
    void        on_path_child_change(const char* path);

protected:
    /** 
     * @brief   try create a node synchronously
     *
     * @param   path            - the node name
     * @param   value           - the node value
     * @param   acl             - the access control list
     * @param   create_flag     - {@link #zoo_create_flag}.
     * @param   path_buffer     - buffer which will be filled with the path of the
     *                          - new node (this might be different than the supplied path
     *                          - because of the ZOO_SEQUENCE flag), The path string will always be
     *                          - null-terminated. This parameter may be NULL if path_buffer_len = 0.
     * @param   path_buffer_len - Size of path buffer; if the path of the new
     *                          - node (including space for the null terminator) exceeds the buffer size,
     *                          - the path string will be truncated to fit.  The actual path of the
     *                          - new node in the server will not be affected by the truncation.
     *                          - The path string will always be null-terminated.
     * @return  
     *          - z_ok              : operation completed successfully
     *          - z_nonode          : the parent node does not exist.
     *          - z_node_exists     : the node already exists
     *          - z_noauth          : the client does not have permission.
     *          - z_no_children_for_ephemeral : cannot create children of ephemeral nodes.
     *          - z_bad_arguments   : invalid input parameters
     *          - z_invalid_state   : zhandle state is either {@link #zoo_state_expired_session} or {@link #zoo_state_auth_failed}
     *          - z_marshallerror   : failed to marshall a request; possibly, out of memory
     */
    zoo_rc      create_node(const char* path, const std::string& value, const std::vector<zoo_acl_t>& acl, int32_t create_flags, char *path_buffer, int32_t path_buffer_len);

public:
    /** synchronous apis */

    /** 
     * @brief create persistent node
     */
    zoo_rc      create_persistent_node(const char* path, const std::string& value, const std::vector<zoo_acl_t>& acl);

    /** 
     * @brief create sequence_node
     *
     * sequance node's name is 'path-xx' rather than 'path', the xx is auto-increment number
     *
     * @param returned_path_name    - return the real path name of the node
     *
     */
    zoo_rc      create_sequence_node(const char* path, const std::string& value, const std::vector<zoo_acl_t>& acl, std::string& returned_path_name);

    /** 
     * @brief create ephemeral node
     *
     * the ephemeral node will auto delete if the session is disconnect
     */
    zoo_rc      create_ephemeral_node(const char* path, const std::string& value, const std::vector<zoo_acl_t>& acl);

    /** 
     * @brief create sequance and ephemeral node
     *
     * @param returned_path_name    - return the real path name of the node
     */
    zoo_rc      create_sequance_ephemeral_node(const char* path, const std::string& value, const std::vector<zoo_acl_t>& acl, std::string& returned_path_name);


    /** 
     * @brief   try delete a node  synchronously
     *
     * @param   path            - the node name
     * @param   version         - the expected version of the node. The function will fail if the
     *                          - actual version of the node does not match the expected version.
     *                          - If -1 is used the version check will not take place.
     * @return
     *          - z_ok              : operation completed successfully
     *          - z_nonode          : the node does not exist.
     *          - z_noauth          : the client does not have permission.
     *          - z_bad_version     : expected version does not match actual version.
     *          - z_not_empty       : children are present; node cannot be deleted.
     *          - z_bad_arguments   : invalid input parameters
     *          - z_invalid_state   : zhandle state is either {@link #zoo_state_expired_session} or {@link #zoo_state_auth_failed}
     *          - z_marshallerror   : failed to marshall a request; possibly, out of memory
     */
    zoo_rc      delete_node(const char* path, int32_t version);

    /** 
     * @brief   check the node exist synchronously
     *
     * @param   path            - the node name
     * @param   info            - return the state info of the node
     * @param   watch           - watch if true, a watch will be set at the server to notify the
     *                          - client if the node changes. The watch will be set even if the node does not
     *                          - exist. This allows clients to watch for nodes to appear.
     *                          - but the watch take affect only once
     * @return 
     *          - z_ok              : operation completed successfully
     *          - z_nonode          : the node does not exist.
     *          - z_noauth          : the client does not have permission.
     *          - z_bad_arguments   : invalid input parameters
     *          - z_invalid_state   : zhandle state is either {@link #zoo_state_expired_session} or {@link #zoo_state_auth_failed}
     *          - z_marshallerror   : failed to marshall a request; possibly, out of memory
     */
    zoo_rc      exists_node(const char* path, zoo_state_t* info, bool watch);

    /** 
     * @brief gets the data associated with a node synchronously.
     *
     * @param   path            - the node name
     * @param   out_value       - return the data associated with the node
     * @param   info            - return the state info of the node
     * @param   watch           - watch if true, a watch will be set at the server to notify
     *                          - the client if the node changes.
     *                          - exist. This allows clients to watch for nodes to appear.
     *                          - but the watch take affect only once
     * @return
     *          - z_ok              : operation completed successfully
     *          - z_nonode          : the node does not exist.
     *          - z_noauth          : the client does not have permission.
     *          - z_bad_arguments   : invalid input parameters
     *          - z_invalid_state   : zhandle state is either in  {@link #zoo_state_expired_session} or {@link #zoo_state_auth_failed}
     *          - z_marshallerror   : failed to marshall a request; possibly, out of memory
     */
    zoo_rc      get_node(const char* path, std::string& out_value, zoo_state_t* info, bool watch);


    /** 
     * @brief  sets the data associated with a node.
     *
     * @param   path            - the node name
     * @param   value           - the new data to written to the node
     * @param   version         - the expected version of the node. The function will fail if
     *                          - the actual version of the node does not match the expected version. 
     *                          - If -1 is used the version check will not take place.
     * @return 
     *          - z_ok              : operation completed successfully
     *          - z_nonode          : the node does not exist.
     *          - z_noauth          : the client does not have permission.
     *          - z_bad_version     : expected version does not match actual version.
     *          - z_bad_arguments   : invalid input parameters
     *          - z_invalid_state   : zhandle state is either {@link #zoo_state_expired_session} or {@link #zoo_state_auth_failed}
     *          - z_marshallerror   : failed to marshall a request; possibly, out of memory
     */
    zoo_rc      set_node(const char* path, const std::string& value, int32_t version);


    /** 
     * @brief lists the children of a node synchronously.
     *
     * @param   path            - the node name
     * @param   children        - return value of children paths
     * @param   watch           - watch if true, a watch will be set at the server to notify
     *                          - the client if the node changes.
     *                          - but the watch take affect only once
     * @return 
     *          - z_ok              : operation completed successfully
     *          - z_nonode          : the node does not exist.
     *          - z_noauth          : the client does not have permission.
     *          - z_bad_arguments   : invalid input parameters
     *          - z_invalid_state   : zhandle state is either {@link #zoo_state_expired_session} or {@link #zoo_state_auth_failed}
     *          - z_marshallerror   : failed to marshall a request; possibly, out of memory
     */
    zoo_rc      get_children(const char* path, std::vector<std::string>& children, bool watch);

    /** 
     * @brief   sets the acl associated with a node synchronously.
     *
     * @param   path            - the node name
     * @param   acl             - the acl
     * @param   version         - the expected version of the node. The function will fail if
     *                          - the actual version of the node does not match the expected version.
     *                          - If -1 is used the version check will not take place.
     *
     * @retrun  
     *          - z_ok              : operation completed successfully
     *          - z_nonode          : the node does not exist.
     *          - z_noauth          : the client does not have permission.
     *          - z_invalid_acl     : invalid ACL specified
     *          - z_bad_version     : expected version does not match actual version.
     *          - z_bad_arguments   : invalid input parameters
     *          - z_invalid_state   : zhandle state is either {@link #zoo_state_expired_session} or {@link #zoo_state_auth_failed}
     *          - z_marshallerror   : failed to marshall a request; possibly, out of memory
     */
    zoo_rc      set_acl(const char* path, const std::vector<zoo_acl_t>& acl, int32_t version);

    /** 
     * @brief   gets the acl associated with a node synchronously.
     *
     * @param   path            - the node name
     * @param   acl             - return the the acl info
     *
     * @return
     *          - z_ok              : operation completed successfully
     *          - z_nonode          : the node does not exist.
     *          - z_noauth          : the client does not have permission.
     *          - z_bad_arguments   : invalid input parameters
     *          - z_invalid_state   : zhandle state is either {@link #zoo_state_expired_session} or {@link #zoo_state_auth_failed}
     *          - z_marshallerror   : failed to marshall a request; possibly, out of memory
     */
    zoo_rc      get_acl(const char* path, std::vector<zoo_acl_t>& acl);


    /** 
     * @brief   specify application credentials.
     *
     * @param   - user_name
     * @param   - user_passwd (Plaintext no encryption)
     * 
     * @return
     *          - z_ok              : operation completed successfully
     *          - z_bad_arguments   : invalid input parameters
     *          - z_invalid_state   : zhandle state is either {@link #zoo_state_expired_session} or {@link #zoo_state_auth_failed}
     *          - z_marshallerror   : failed to marshall a request; possibly, out of memory
     *          - z_systemerror     : a system error occurred
     */
    zoo_rc      add_auth(const std::string& user_name, const std::string& user_passwd);

    /** 
     * @brief request watch the path's data change event
     * 
     * @param   path            - the node name
     * @param   handler         - the path's data change callback fuction
     * @param   value           - return the node's name if not NULL
     *
     * @return  see {@link #get_node}
     */
    zoo_rc      watch_data_change(const char* path, const data_change_event_handler_t& handler, std::string* value);

    /** 
     * @brief request watch the path's child change event, child create/delete
     *
     * @param   path            - the node name
     * @param   handler         - the path's child change callback fuction
     * @param   out_children    - return value of children paths if not NULL
     *
     * @return  see {@link #get_children}
     */
    zoo_rc      watch_children_event(const char* path, const child_event_handler_t& handler, std::vector<std::string>* out_children );

public:
    /*  asynchronous apis */

protected:
    void                    close();
    void                    reconnect();
    void                    add_data_event_handler(const std::string& path, data_event_handler_ptr handler);
    void                    add_child_event_handler(const std::string& path, child_event_handler_ptr handler);
    data_event_handler_ptr  get_data_event_handler(const std::string& path);
    child_event_handler_ptr get_child_event_handler(const std::string& path);
};

}

#endif