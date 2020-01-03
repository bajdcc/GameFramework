//
// Project: clibparser
// Created by bajdcc
//

#include "stdafx.h"
#include <ctime>
#include <iterator>
#include <algorithm>
#include <fstream>
#include "cvfs.h"
#include "cmusic.h"
#include "cexception.h"

namespace clib {

    void vfs_node_dec::advance() {
        if (available())
            idx++;
    }

    int vfs_node_dec::write(byte c) {
        return -1;
    }

    int vfs_node_dec::truncate() {
        return -1;
    }

    void vfs_node_dec::add_handle(int handle, vfs_op_t type)
    {

    }

    vfs_op_t vfs_node_dec::get_handle(int handle, vfs_op_t type)
    {
        return v_none;
    }

    void vfs_node_dec::remove_handle(int handle)
    {

    }

    bool vfs_node_dec::set_data(const std::vector<byte>& data)
    {
        return false;
    }

    bool vfs_node_dec::get_data(std::vector<byte>& data) const
    {
        return false;
    }

    bool vfs_node_dec::set_link(const string_t& data)
    {
        return false;
    }

    bool vfs_node_dec::get_link(string_t& data) const
    {
        return false;
    }

    int vfs_node_dec::get_length() const
    {
        return -1;
    }

    vfs_node_dec::vfs_node_dec(const vfs_mod_query* mod) : mod(mod) {}

    vfs_node_solid::vfs_node_solid(const vfs_mod_query* mod, const vfs_node::ref& ref) :
        vfs_node_dec(mod), node(ref) {
        node.lock()->refs++;
    }

    vfs_node_solid::~vfs_node_solid() {
        node.lock()->refs--;
    }

    bool vfs_node_solid::available() const {
        auto n = node.lock();
        if (!n)
            return false;
        return idx < n->data.size();
    }

    int vfs_node_solid::index() const {
        auto n = node.lock();
        if (!n)
            return READ_ERROR;
        if (idx < n->data.size())
            return n->data[idx];
        return READ_EOF;
    }

    int vfs_node_solid::write(byte c) {
        auto n = node.lock();
        if (!n)
            return -1;
        if (!mod->can_mod(n, 1))
            return -2;
        n->data.push_back(c);
        idx = n->data.size() - 1;
        return 0;
    }

    int vfs_node_solid::truncate() {
        auto n = node.lock();
        if (!n)
            return -1;
        if (!mod->can_mod(n, 1))
            return -2;
        n->data.clear();
        idx = 0;
        return 0;
    }

    void vfs_node_solid::add_handle(int handle, vfs_op_t type)
    {
        assert(this_handle == -1);
        this_handle = handle;
        auto n = node.lock();
        assert(n->handles.find(handle) == n->handles.end());
        n->handles.insert(std::make_pair(handle, type));
        if (type == v_write)
            n->handles_write.push_back(handle);
        else if (type == v_read)
            n->handles_read.push_back(handle);
    }

    vfs_op_t vfs_node_solid::get_handle(int handle, vfs_op_t type)
    {
        auto n = node.lock();
        auto f = n->handles.find(handle);
        if (f == n->handles.end())
            return v_none;
        return f->second;
    }

    void vfs_node_solid::remove_handle(int handle)
    {
        auto n = node.lock();
        auto f = n->handles.find(handle);
        if (f == n->handles.end())
            return;
        if (f->second == v_write)
            n->handles_write.remove(handle);
        else if (f->second == v_read)
            n->handles_read.remove(handle);
        n->handles.erase(handle);
    }

    bool vfs_node_solid::set_data(const std::vector<byte>& data)
    {
        auto n = node.lock();
        if (!n)
            return false;
        if (!mod->can_mod(n, 1))
            return false;
        n->data = data;
        return true;
    }

    bool vfs_node_solid::get_data(std::vector<byte>& data) const
    {
        auto n = node.lock();
        if (!n)
            return false;
        if (!mod->can_mod(n, 0))
            return false;
        data = n->data;
        return true;
    }

    bool vfs_node_solid::set_link(const string_t& data)
    {
        auto n = node.lock();
        if (!n)
            return false;
        if (!mod->can_mod(n, 0))
            return false;
        if (n->type != fs_file)
            return false;
        n->type = fs_link;
        n->data.resize(data.size());
        std::copy(data.begin(), data.end(), n->data.begin());
        return true;
    }

    bool vfs_node_solid::get_link(string_t& data) const
    {
        auto n = node.lock();
        if (!n)
            return false;
        if (!mod->can_mod(n, 0))
            return false;
        if (n->type != fs_link)
            return false;
        data.resize(n->data.size());
        std::copy(n->data.begin(), n->data.end(), data.begin());
        return true;
    }

    int vfs_node_solid::get_length() const
    {
        auto n = node.lock();
        if (!n)
            return -1;
        if (!mod->can_mod(n, 0))
            return -1;
        return (int)n->data.size();
    }

    // -----------------------------------------

    vfs_node_pipe::vfs_node_pipe(const vfs_mod_query* mod, const vfs_node::ref& ref) :
        vfs_node_solid(mod, ref) {
        auto n = node.lock();
        if (!n->pipe)
            n->pipe = std::make_unique<std::queue<byte>>();
    }

    int vfs_node_pipe::count(vfs_op_t t) const
    {
        auto n = node.lock();
        int cnt = 0;
        for (auto& h : n->handles) {
            if (h.second == t)
                cnt++;
        }
        return cnt;
    }

    vfs_node_pipe::~vfs_node_pipe() {
    }

    bool vfs_node_pipe::available() const {
        auto n = node.lock();
        if (!n)
            return false;
        return true;
    }

    int vfs_node_pipe::index() const {
        auto n = node.lock();
        if (!n)
            return READ_ERROR;
        if (n->pipe->empty()) {
            if (count(v_write) == 0)
                return READ_EOF;
            return DELAY_CHAR;
        }
        auto c = n->pipe->front();
        n->pipe->pop();
        return c;
    }

    int vfs_node_pipe::write(byte c) {
        auto n = node.lock();
        if (!n)
            return -1;
        if (!mod->can_mod(n, 1))
            return -2;
        n->pipe->push(c);
        return 0;
    }

    int vfs_node_pipe::truncate() {
        auto n = node.lock();
        if (!n)
            return -1;
        if (!mod->can_mod(n, 1))
            return -2;
        while (!n->pipe->empty()) {
            n->pipe->pop();
        }
        return 0;
    }

    // -----------------------------------------

    vfs_node_semaphore::vfs_node_semaphore(const vfs_mod_query* mod, const vfs_node::ref& ref, int count) :
        vfs_node_pipe(mod, ref), count(count) {
    }

    vfs_node_semaphore::~vfs_node_semaphore() {
    }

    bool vfs_node_semaphore::available() const {
        return !entered;
    }

    int vfs_node_semaphore::index() const {
        if (entered) { // LEAVE
            return READ_EOF;
        }
        else {
            auto n = node.lock();
            auto i = n->handles_read.begin();
            for (int j = 0; i != n->handles_read.end() && j < count; i++, j++) {
                if (this_handle == *i) { // ENTER
                    *const_cast<bool*>(&entered) = true;
                    return 'A';
                }
            }
            return WAIT_CHAR;
        }
    }

    int vfs_node_semaphore::write(byte c) {
        return -1;
    }

    int vfs_node_semaphore::truncate() {
        return -1;
    }

    int vfs_node_semaphore::get_length() const
    {
        auto n = node.lock();
        if (!n)
            return -1;
        return (int)n->handles_read.size();
    }

    // -----------------------------------------

    vfs_node_fifo::vfs_node_fifo(const vfs_mod_query* mod, const vfs_node::ref& ref) :
        vfs_node_solid(mod, ref) {
        auto n = node.lock();
        if (!n->pipe)
            n->pipe = std::make_unique<std::queue<byte>>();
        if (n->data.empty()) {
            n->data.resize(sizeof(int) * 2);
            *(int*)n->data.data() = -1;
            *((int*)(n->data.data()) + 1) = -1;
        }
        reads = (int*)n->data.data();
        writes = ((int*)(n->data.data()) + 1);
    }

    int vfs_node_fifo::count(vfs_op_t t) const
    {
        auto n = node.lock();
        int cnt = 0;
        for (auto& h : n->handles) {
            if (h.second == t)
                cnt++;
        }
        return cnt;
    }

    vfs_node_fifo::~vfs_node_fifo() {
    }

    bool vfs_node_fifo::available() const {
        auto n = node.lock();
        if (!n)
            return false;
        return true;
    }

    int vfs_node_fifo::index() const {
        auto n = node.lock();
        if (!n)
            return READ_ERROR;
        if (n->pipe->empty()) {
            if (this_handle == *reads) {
                return READ_EOF;
            }
            return DELAY_CHAR;
        }
        auto c = n->pipe->front();
        return c;
    }

    void vfs_node_fifo::advance()
    {
        auto n = node.lock();
        if (!n)
            return;
        if (n->pipe->empty()) {
            return;
        }
        vfs_node_solid::advance();
        n->pipe->pop();
    }

    int vfs_node_fifo::write(byte c) {
        auto n = node.lock();
        if (!n)
            return -1;
        if (!mod->can_mod(n, 1))
            return -2;
        n->pipe->push(c);
        return 0;
    }

    int vfs_node_fifo::truncate() {
        auto n = node.lock();
        if (!n)
            return -1;
        if (!mod->can_mod(n, 1))
            return -2;
        while (!n->pipe->empty()) {
            n->pipe->pop();
        }
        return 0;
    }

    void vfs_node_fifo::add_handle(int handle, vfs_op_t type)
    {
        auto n = node.lock();
        assert(n->handles.find(handle) == n->handles.end());
        if (type == v_write) {
            n->handles_write.push_back(handle);
            n->handles.insert(std::make_pair(handle, type));
        }
        else if (type == v_read) {
            n->handles_read.push_back(handle);
            n->handles.insert(std::make_pair(handle, type));
        }
    }

    vfs_op_t vfs_node_fifo::get_handle(int handle, vfs_op_t type)
    {
        auto n = node.lock();
        auto f = n->handles.find(handle);
        this_handle = handle;
        if (f != n->handles.end()) {
            auto t = f->second;
            if (t == v_read) {
                if (*reads == handle)
                    return t;
                if (n->handles_write.empty()) {
                    return v_wait;
                }
                if (!n->handles_read.empty() && handle != n->handles_read.front()) {
                    return v_wait;
                }
            }
            else if (t == v_write) {
                if (*writes == handle)
                    return v_error;
                if (n->handles_read.empty()) {
                    return v_wait;
                }
                if (!n->handles_write.empty() && handle != n->handles_write.front()) {
                    return v_wait;
                }
            }
            return t;
        }
        add_handle(handle, type);
        return v_wait;
    }

    void vfs_node_fifo::remove_handle(int handle)
    {
        auto n = node.lock();
        auto f = n->handles.find(handle);
        if (f == n->handles.end())
            return;
        if (f->second == v_write) {
            if (*writes != -1)
                *writes = -1;
            assert(*reads == -1);
            if (!n->handles_read.empty())
                *reads = n->handles_read.front();
            n->handles_write.remove(handle);
        }
        else if (f->second == v_read) {
            if (*reads != -1)
                *reads = -1;
            assert(*writes == -1);
            if (!n->pipe->empty() && !n->handles_write.empty())
                *writes = n->handles_write.front();
            n->handles_read.remove(handle);
        }
        n->handles.erase(handle);
    }

    int vfs_node_fifo::get_length() const
    {
        return -1;
    }

    // -----------------------------------------

    vfs_node_cached::vfs_node_cached(const vfs_mod_query* mod, const string_t& str) :
        vfs_node_dec(mod), cache(str) {}

    bool vfs_node_cached::available() const {
        return idx < cache.length();
    }

    int vfs_node_cached::index() const {
        return idx < cache.length() ? cache[idx] : READ_EOF;
    }

    int vfs_node_cached::get_length() const {
        return (int)cache.length();
    }

    bool vfs_node_cached::get_data(std::vector<byte>& data) const
    {
        data.resize(cache.size());
        std::copy(cache.begin(), cache.end(), data.begin());
        return true;
    }

    vfs_node_stream::vfs_node_stream(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call) :
        vfs_node_dec(mod), stream(s), call(call) {}

    vfs_node_dec* vfs_node_stream::create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call)
    {
        return new vfs_node_stream(mod, s, call);
    }

    bool vfs_node_stream::available() const {
        return true;
    }

    int vfs_node_stream::index() const {
        return call->stream_index(stream);
    }

    void vfs_node_stream::advance() {
    }

    int vfs_node_stream::write(byte c) {
        return call->stream_write(stream, c);
    }

    int vfs_node_stream::truncate() {
        return -1;
    }

    vfs_node_stream_write::vfs_node_stream_write(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call) :
        vfs_node_dec(mod), stream(s), call(call) {}

    vfs_node_dec* vfs_node_stream_write::create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call)
    {
        return new vfs_node_stream_write(mod, s, call);
    }

    bool vfs_node_stream_write::available() const {
        return false;
    }

    int vfs_node_stream_write::index() const {
        return READ_ERROR;
    }

    void vfs_node_stream_write::advance() {
    }

    int vfs_node_stream_write::write(byte c) {
        return call->stream_write(stream, c);
    }

    int vfs_node_stream_write::truncate() {
        return -1;
    }

    // -------------------------------------------

    cvfs::cvfs() {
        reset();
    }

    static void mod_copy(vfs_mod * mod, const char* s) {
        for (int i = 0; i < 9; ++i) {
            ((char*)mod)[i] = *s++;
        }
    }

    void cvfs::reset() {
        account.clear();
        account.insert({ 0, vfs_user{ 0, "root", "root" } });
        account.insert({cc, vfs_user{ cc, "cc", "cc" } });
        account.insert({ ext, vfs_user{ ext, "ext", "ext" } });
        current_user = 0;
        last_user = 1;
        root = new_node(fs_dir);
        mod_copy(root->mod, "rw-r--rw-"); // make '/' writable
        pwd = "/";
        auto n = now();
        year = localtime(&n)->tm_year;
        current_user = 1;
        last_user = 0;
    }

    void cvfs::error(const string_t & str) {
        throw cexception(ex_vm, str);
    }

    vfs_node::ref cvfs::new_node(vfs_file_t type) {
        auto node = std::make_shared<vfs_node>();
        node->type = type;
        if (type == fs_file) {
            mod_copy(node->mod, "rw-r--r--");
        }
        else if (type == fs_dir) {
            mod_copy(node->mod, "rw-r--r--");
        }
        else {
            error("invalid mod");
        }
        time_t ctime;
        time(&ctime);
        node->time.create = ctime;
        node->time.access = ctime;
        node->time.modify = ctime;
        node->owner = current_user;
        node->refs = 0;
        node->locked = false;
        node->callback = nullptr;
        node->magic = fss_none;
        return node;
    }

    string_t cvfs::get_user() const {
        return account.at(current_user).name;
    }

    string_t cvfs::get_pwd() const {
        return pwd;
    }

    char* cvfs::file_time(const time_t & t) const {
        auto timeptr = localtime(&t);
        /*static const char wday_name[][4] = {
                "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
        };*/
        static const char mon_name[][4] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };
        static char result[32];
        if (year == timeptr->tm_year) {
            snprintf(result, sizeof(result), "%.3s%3d %.2d:%.2d",
                mon_name[timeptr->tm_mon],
                timeptr->tm_mday, timeptr->tm_hour,
                timeptr->tm_min);
        }
        else {
            snprintf(result, sizeof(result), "%.3s%3d %5d",
                //wday_name[timeptr->tm_wday],
                mon_name[timeptr->tm_mon],
                timeptr->tm_mday,
                1900 + timeptr->tm_year);
        }
        return result;
    }

    extern string_t limit_string(const string_t& s, uint len);

    void cvfs::ll(const string_t & name, const vfs_node::ref & node, std::ostream & os) const {
        if (!node)
            return;
        static const char* types[] = {
            "35EA3F", // file
            "44FC7D", // dir
            "76FC44", // func
            "BCDD29", // magic
            "9AD9FB", // link
        };
        static char fmt[256];
        snprintf(fmt, sizeof(fmt), "\033FFFA0A0A0\033%c%9s \033FFFB3B920\033%4s \033S4\033%9d \033FFF51C2A8\033%s \033FFF%s\033%s\033S4\033",
            node->type == fs_dir ? 'd' : '-',
            (char*)node->mod,
            account.at(node->owner).name.data(),
            node->data.size(),
            file_time(node->time.create),
            types[(int)node->type],
            limit_string(name, 40).data());
        os << fmt << std::endl;
    }

    string_t full_path(const vfs_node::ref & node) {
        std::stringstream ss;
        std::vector<vfs_node::ref> paths;
        auto p = node;
        while (p) {
            paths.push_back(p);
            p = p->parent.lock();
        }
        std::transform(paths.rbegin(), paths.rend(),
            std::ostream_iterator<string_t>(ss, "/"),
            [](const auto & p) { return p->name; });
        auto str = ss.str();
        str.erase(str.begin() + str.size() - 1);
        return std::move(str);
    }

    int cvfs::macro(const std::vector<string_t> & m, const vfs_node::ref & node, vfs_node_dec * *dec) const {
        if (m[1] == "ls") {
            std::stringstream ss;
            std::transform(node->children.begin(), node->children.end(),
                std::ostream_iterator<string_t>(ss, "\n"),
                [](const auto & p) { return p.first; });
            auto str = ss.str();
            if (!str.empty())
                str.pop_back();
            *dec = new vfs_node_cached(this, str);
            return 0;
        }
        if (m[1] == "ll") {
            std::stringstream ss;
            ll("..", node->parent.lock(), ss); // parent
            ll(".", node, ss); // self
            for (auto& c : node->children) {
                ll(c.first, c.second, ss); // children
            }
            auto str = ss.str();
            if (!str.empty())
                str.pop_back();
            *dec = new vfs_node_cached(this, str);
            return 0;
        }
        if (m[1] == "tree") {
            std::stringstream ss;
            std::vector<vfs_node::ref> stacks;
            stacks.push_back(node);
            while (!stacks.empty()) {
                auto n = stacks.back();
                stacks.pop_back();
                if (!can_mod(node, 0))
                    continue;
                ll(full_path(n), n, ss); // children
                if (n->type == fs_dir) {
                    for (auto c = n->children.rbegin(); c != n->children.rend(); c++) {
                        stacks.push_back(c->second);
                    }
                }
            }
            auto str = ss.str();
            if (!str.empty())
                str.pop_back();
            *dec = new vfs_node_cached(this, str);
            return 0;
        }
        return -2;
    }

    int cvfs::get(const string_t & path, vfs_node_dec * *dec, vfs_func_t * f) const {
        if (path.empty())
            return -1;
        std::vector<string_t> m;
        split_path(path, m, ':');
        auto p = combine(pwd, m[0]);
        auto node = get_node(p);
        if (!node)
            return -1;
        if (node->type == fs_file) {
            if (node->locked)
                return -3;
            node->time.access = now();
            if (dec) {
                if (path.substr(0, 6) == "/pipe/") {
                    node->magic = fss_pipe;
                    *dec = new vfs_node_pipe(this, node);
                }
                else if (path.substr(0, 11) == "/semaphore/") {
                    if (m.size() > 1) {
                        static string_t pat{ R"(^\d+$)" };
                        static std::regex re(pat);
                        std::smatch res;
                        if (std::regex_match(m[1], res, re)) {
                            node->magic = fss_semaphore;
                            *dec = new vfs_node_semaphore(this, node, std::atoi(m[1].c_str()));
                        }
                        else {
                            return -1;
                        }
                    }
                    else {
                        node->magic = fss_semaphore;
                        *dec = new vfs_node_semaphore(this, node, 1);
                    }
                }
                else if (path.substr(0, 7) == "/mutex/") {
                    node->magic = fss_mutex;
                    *dec = new vfs_node_semaphore(this, node, 1);
                }
                else if (path.substr(0, 6) == "/fifo/") {
                    node->magic = fss_fifo;
                    *dec = new vfs_node_fifo(this, node);
                }
                else {
                    *dec = new vfs_node_solid(this, node);
                }
            }
            return 0;
        }
        else if (node->type == fs_func) {
            node->time.access = now();
            if (dec) {
                if (f) {
                    auto t = f->stream_type(p);
                    if (t == fss_none) {
                        *dec = new vfs_node_cached(this, f->stream_callback(p));
                    }
                    else {
                        *dec = f->stream_create(this, t, p);
                    }
                }
                else {
                    return -2;
                }
            }
            return 0;
        }
        else if (node->type == fs_dir) {
            if (m.size() > 1) {
                return macro(m, node, dec);
            }
        }
        else if (node->type == fs_magic) {
            node->time.access = now();
            *dec = node->callback->stream_create(this, node->magic, p);
            if (*dec == nullptr) {
                return -1;
            }
            return 0;
        }
        else if (node->type == fs_link) {
            if (node->locked)
                return -3;
            node->time.access = now();
            *dec = new vfs_node_solid(this, node);
            return 0;
        }
        return -2;
    }

    bool cvfs::exist_vfs(const string_t& path) const
    {
        auto node = get_node(path);
        if (!node)
            return false;
        if (node->type != fs_file)
            return false;
        return true;
    }

    bool cvfs::read_vfs(const string_t & path, std::vector<byte> & data) const {
        auto node = get_node(path);
        if (!node)
            return false;
        if (node->type != fs_file)
            return false;
        data.resize(node->data.size());
        std::copy(node->data.begin(), node->data.end(), data.begin());
        return true;
    }

    void cvfs::as_root(bool flag) {
        if (flag) {
            if (current_user != 0) {
                last_user = current_user;
                current_user = 0;
            }
        }
        else {
            if (current_user == 0) {
                current_user = last_user;
                last_user = 0;
            }
        }
    }

    void cvfs::as_user(int uid, bool flag)
    {
        if (uid == 0)
            return;
        if (flag) {
            last_user = current_user;
            current_user = uid;
        }
        else {
            current_user = last_user;
            last_user = uid;
        }
    }

    bool cvfs::write_vfs(const string_t & path, const std::vector<byte> & data) {
        auto node = get_node(path);
        if (!node) {
            touch(path);
            node = get_node(path);
            if (!node)
                return false;
        }
        if (node->type != fs_file)
            return false;
        if (!node->data.empty())
            return false;
        node->data.resize(data.size());
        std::copy(data.begin(), data.end(), node->data.begin());
        return true;
    }

    string_t get_parent(const string_t & path) {
        assert(path[0] == '/');
        if (path == "/")
            return path;
        auto f = path.find_last_of('/');
        assert(f != string_t::npos);
        if (f == 0)
            return "/";
        return path.substr(0, f);
    }

    time_t cvfs::now() {
        time_t ctime;
        time(&ctime);
        return ctime;
    }

    void cvfs::split_path(const string_t & path, std::vector<string_t> & args, char c) {
        std::stringstream ss(path);
        string_t temp;
        while (std::getline(ss, temp, c)) {
            args.push_back(temp);
        }
    }

    vfs_node::ref cvfs::get_node(const string_t & path) const {
        std::vector<string_t> paths;
        split_path(path, paths, '/');
        auto cur = root;
        for (size_t i = 0; i < paths.size(); ++i) {
            if (!can_mod(cur, 0))
                return nullptr;
            auto& p = paths[i];
            if (!p.empty()) {
                auto f = cur->children.find(p);
                if (f != cur->children.end()) {
                    if (f->second->type == fs_magic) {
                        return f->second;
                    }
                    if (i < paths.size() - 1 && f->second->type != fs_dir)
                        return nullptr;
                    cur = f->second;
                }
                else {
                    return nullptr;
                }
            }
        }
        return cur;
    }

    int cvfs::cd(const string_t & path) {
        auto p = combine(pwd, path);
        auto node = get_node(p);
        if (!node)
            return -1;
        switch (node->type) {
        case fs_file:
            return -2;
        case fs_dir:
            pwd = p;
            break;
        case fs_func:
            break;
        }
        return 0;
    }

    int cvfs::_mkdir(const string_t & path, vfs_node::ref & cur) {
        std::vector<string_t> paths;
        split_path(path, paths, '/');
        cur = root;
        bool update = false;
        for (auto& p : paths) {
            if (!p.empty()) {
                auto f = cur->children.find(p);
                if (f != cur->children.end()) {
                    cur = f->second;
                    if (f->second->type != fs_dir)
                        return -2;
                }
                else {
                    if (!update)
                        update = true;
                    if (!can_mod(cur, 1))
                        return -3;
                    auto node = new_node(fs_dir);
                    node->parent = cur;
                    node->name = p;
                    cur->children.insert(std::make_pair(p, node));
                    cur = node;
                }
            }
        }
        if (update)
            return 0;
        return -1;
    }

    int cvfs::mkdir(const string_t & path) {
        auto p = combine(pwd, path);
        vfs_node::ref cur;
        return _mkdir(p, cur);
    }

    string_t cvfs::combine(const string_t & pwd, const string_t & path) const {
        if (path.empty())
            return pwd;
        auto res = (path[0] == '/') ? "/" : pwd;
        std::vector<string_t> paths;
        split_path(path, paths, '/');
        for (auto& p : paths) {
            if (!p.empty()) {
                if (p == ".")
                    continue;
                else if (p == "..")
                    res = get_parent(res);
                else if (res.back() == '/')
                    res += p;
                else
                    res += "/" + p;
            }
        }
        return res;
    }

    int cvfs::touch(const string_t & path) {
        auto p = combine(pwd, path);
        auto node = get_node(p);
        if (!node) {
            vfs_node::ref cur;
            auto s = _mkdir(p, cur);
            if (s == 0) { // new dir
                cur->type = fs_file;
                return -1;
            }
            else { // exists
                _touch(cur);
                return 0;
            }
        }
        if (!can_mod(node, 1))
            return -3;
        switch (node->type) {
        case fs_file:
        case fs_dir:
            _touch(node);
            return 0;
        default:
            return -2;
        }
    }

    void cvfs::_touch(vfs_node::ref & node) {
        auto ctime = now();
        node->time.create = ctime;
        node->time.access = ctime;
        node->time.modify = ctime;
    }

    int cvfs::func(const string_t & path, vfs_func_t * f) {
        auto node = get_node(path);
        if (!node) {
            vfs_node::ref cur;
            auto s = _mkdir(path, cur);
            if (s == 0) { // new dir
                cur->type = fs_func;
                cur->callback = f;
                return 0;
            }
            else { // exists
                return 1;
            }
        }
        return -2;
    }

    int cvfs::magic(const string_t & path, vfs_func_t * f, vfs_stream_t magic) {
        auto node = get_node(path);
        if (!node) {
            vfs_node::ref cur;
            auto s = _mkdir(path, cur);
            if (s == 0) { // new dir
                cur->type = fs_magic;
                cur->magic = magic;
                cur->callback = f;
                return 0;
            }
            else { // exists
                return 1;
            }
        }
        return -2;
    }

    string_t cvfs::get_filename(const string_t & path) {
        if (path.empty())
            return "";
        if (path == "/")
            return "";
        auto f = path.find_last_of('/');
        if (f == string_t::npos)
            return "";
        return path.substr(f + 1);
    }

    string_t cvfs::get_realpath(const string_t& path)
    {
        return FILE_ROOT + combine(pwd, path);
    }

    void cvfs::convert_utf8_to_gbk(string_t& str)
    {
        // UTF-8 to GBK
        int len = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)str.c_str(), -1, NULL, 0);
        wchar_t* wszGBK = new wchar_t[len];
        memset(wszGBK, 0, len);
        MultiByteToWideChar(CP_UTF8, 0, (LPCCH)str.c_str(), -1, wszGBK, len);

        len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
        char* szGBK = new char[len + 1];
        memset(szGBK, 0, len + 1);
        WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);

        str = szGBK;
        delete[] szGBK;
        delete[] wszGBK;
    }

    static uint64 sum_fs(const std::shared_ptr<vfs_node>& node)
    {
        if (node->type == fs_dir) {
            uint64 s = 0ULL;
            for (const auto& c : node->children) {
                s += sum_fs(c.second);
            }
            return s;
        }
        else if (node->type == fs_file) {
            return node->data.size();
        }
        return 0ULL;
    }

    uint64 cvfs::size() const
    {
        return sum_fs(root);
    }

    libZPlay::ZPlay* cvfs::get_zplay() const
    {
        return vfs_node_stream_music::zplay;
    }

    int cvfs::rm(const string_t & path) {
        auto p = combine(pwd, path);
        auto node = get_node(p);
        if (!node)
            return -1;
        return node->parent.lock()->children.erase(get_filename(path)) == 0 ?
            -2 : (node->type != fs_dir ? 0 : 1);
    }

    int cvfs::rm_safe(const string_t & path) {
        auto p = combine(pwd, path);
        auto node = get_node(p);
        if (!node)
            return -1;
        if (!can_rm(node))
            return -2;
        return node->parent.lock()->children.erase(get_filename(path)) == 0 ?
            -3 : (node->type != fs_dir ? 0 : 1);
    }

    bool cvfs::can_rm(const vfs_node::ref & node) const {
        if (!can_mod(node, 1))
            return false;
        if (node->refs > 0)
            return false;
        if (node->locked)
            return false;
        if (node->type == fs_dir) {
            for (auto& c : node->children) {
                if (!can_rm(c.second))
                    return false;
            }
        }
        return true;
    }

    bool cvfs::can_mod(const vfs_node::ref & node, int mod) const {
        if (mod != -1) {
            if (node->owner == current_user)
                return node->mod[0].rwx[mod] != '-';
            else {
                if (node->mod[1].rwx[mod] != '-')
                    return true;
                if (node->mod[2].rwx[mod] != '-')
                    return true;
            }
            return false;
        }
        return true;
    }

    void cvfs::load(const string_t & path) {
        std::ifstream t(FILE_ROOT + path);
        if (t) {
            std::stringstream buffer;
            buffer << t.rdbuf();
            auto str = buffer.str();
            cvfs::convert_utf8_to_gbk(str);
            std::vector<byte> data(str.begin(), str.end());
            write_vfs(path, data);
        }
    }

    void cvfs::load_bin(const string_t& path)
    {
        std::ifstream t(FILE_ROOT + path, std::ifstream::binary);
        if (t) {
            auto p = t.rdbuf();
            auto size = p->pubseekoff(0, std::ios::end, std::ios::in);
            p->pubseekpos(0, std::ios::in);
            std::vector<byte> data((size_t)size);
            p->sgetn((char*)data.data(), size);
            write_vfs(path, data);
        }
    }

    void cvfs::load_dir_rec(const CString& path)
    {
        WIN32_FIND_DATA FindFileData;
        auto pa = string_t(CStringA(path).GetBuffer(0));
        auto p = WFILE_ROOT + path + _T("/*");
        auto hListFile = FindFirstFile(p.GetBuffer(0), &FindFileData);
        if (hListFile == INVALID_HANDLE_VALUE)
            return;
        else
        {
            do
            {
                auto name = CString(FindFileData.cFileName);
                if(name == _T(".") || name == _T(".."))
                    continue;
                if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)
                    continue;
                if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
                    continue;
                if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    load_dir_rec(path + _T("/") + name);
                else
                {
                    string_t nm(CStringA(name).GetBuffer(0));
                    load_bin(pa + "/" + nm);
                }
            } while (FindNextFile(hListFile, &FindFileData));
            FindClose(hListFile);
        }
    }

    void cvfs::load_dir(const string_t& path)
    {
        load_dir_rec(CString(CStringA(path.c_str())));
    }
}
