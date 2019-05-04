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
            return READ_EOF + 1;
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

    vfs_node_cached::vfs_node_cached(const vfs_mod_query* mod, const string_t& str) :
        vfs_node_dec(mod), cache(str) {}

    bool vfs_node_cached::available() const {
        return idx < cache.length();
    }

    int vfs_node_cached::index() const {
        return idx < cache.length() ? cache[idx] : READ_EOF;
    }

    vfs_node_stream::vfs_node_stream(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call) :
        vfs_node_dec(mod), stream(s), call(call) {}

    bool vfs_node_stream::available() const {
        return true;
    }

    int vfs_node_stream::index() const {
        return call->stream_index(stream);
    }

    void vfs_node_stream::advance() {
    }

    int vfs_node_stream::write(byte c) {
        return 0;
    }

    int vfs_node_stream::truncate() {
        return 0;
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
        account.push_back(vfs_user{ 0, "root", "root" });
        account.push_back(vfs_user{ 1, "cc", "cc" });
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
        return node;
    }

    string_t cvfs::get_user() const {
        return account[current_user].name;
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
            sprintf(result, "%.3s%3d %.2d:%.2d",
                mon_name[timeptr->tm_mon],
                timeptr->tm_mday, timeptr->tm_hour,
                timeptr->tm_min);
        }
        else {
            sprintf(result, "%.3s%3d %5d",
                //wday_name[timeptr->tm_wday],
                mon_name[timeptr->tm_mon],
                timeptr->tm_mday,
                1900 + timeptr->tm_year);
        }
        return result;
    }

    void cvfs::ll(const string_t & name, const vfs_node::ref & node, std::ostream & os) const {
        if (!node)
            return;
        static const char* types[] = {
            "35EA3F", // file
            "44FC7D", // dir
            "76FC44", // func
            "BCDD29", // magic
        };
        static char fmt[256];
        sprintf(fmt, "\033FFFA0A0A0\033%c%9s \033FFFB3B920\033%4s \033S4\033%9d \033FFF51C2A8\033%s \033FFF%s\033%s\033S4\033",
            node->type == fs_dir ? 'd' : '-',
            (char*)node->mod,
            account[node->owner].name.data(),
            node->data.size(),
            file_time(node->time.create),
            types[(int)node->type],
            name.data());
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
            if (dec)
                * dec = new vfs_node_solid(this, node);
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
            *dec = f->stream_create(this, fss_net, p);
            if (*dec == nullptr) {
                return -1;
            }
            return 0;
        }
        return -2;
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
        if (path[0] == '/')
            return path;
        auto res = pwd;
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

    int cvfs::magic(const string_t & path, vfs_func_t * f) {
        auto node = get_node(path);
        if (!node) {
            vfs_node::ref cur;
            auto s = _mkdir(path, cur);
            if (s == 0) { // new dir
                cur->type = fs_magic;
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
            if (node->mod[0].rwx[mod] != '-')
                return true;
            if (node->owner != current_user) {
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
            std::vector<byte> data(str.begin(), str.end());
            write_vfs(path, data);
        }
    }
}
