#include "pch.h"
#include "ext.h"
#include "fs.h"

using namespace clib;

#define EXT_NAME "web"
#define EXT_NORMAL_TEXT "网页扩展 - by bajdcc"
#define EXT_NORMAL_VERSION "0.1.0"

std::string func_path;
std::unique_ptr<ext_web> g_ext;
cext* global_ext;

CCOSEXTWEB_API int ccos_ext_load(clib::cext* ptr)
{
    if (g_ext) return -1;
    g_ext = std::make_unique<ext_web>();
    auto ext = (cext*)ptr;
    global_ext = ext;
    func_path = ext->ext_get_path(EXT_NAME);
    g_ext->set_fs_path(func_path);
    ext->ext_load(EXT_NAME, g_ext.get());
    return 0;
}

CCOSEXTWEB_API int ccos_ext_unload(clib::cext* ptr)
{
    if (!g_ext) return -1;
    auto ext = (cext*)ptr;
    ext->ext_unload(EXT_NAME);
    g_ext.reset(nullptr);
    return 0;
}

namespace clib {

    enum ext_vfs_t {
        fss_normal = 100,
        fss_version,
        fss_file,
    };

    ext_web::ext_web()
    {
        fs.as_root(false);
    }

    vfs_stream_t ext_web::stream_type(const string_t& path) const
    {
        return fss_none;
    }

    string_t ext_web::stream_callback(const string_t& path)
    {
        return "[Not implemented]";
    }

    vfs_node_dec* ext_web::stream_create(const vfs_mod_query* mod, vfs_stream_t type, const string_t& path, int* ret)
    {
        switch (type) {
        case fss_normal:
            return new vfs_node_text(mod, EXT_NORMAL_TEXT);
        case fss_version:
            return new vfs_node_text(mod, EXT_NORMAL_VERSION);
        case fss_file: {
            vfs_node_dec* dec = nullptr;
            int r;
            if ((r = fs.get(path.substr(5), &dec)) == 0) {
                return dec;
            }
            else {
                *ret = r;
                return nullptr;
            }
        }
        default:
            break;
        }
        if (path.size() > func_path.size()) {
            auto p = path.substr(func_path.size());
            if (p == "/version")
                return stream_create(mod, fss_version, path, ret);
            if (p.substr(0, 6) == "/file/")
                return stream_create(mod, fss_file, p, ret);
        }
        return stream_create(mod, fss_normal, path, ret);
    }

    vfs_oper* ext_web::stream_oper()
    {
        return this;
    }

    int ext_web::stream_index(vfs_stream_t type)
    {
        return 0;
    }

    string_t ext_web::stream_net(vfs_stream_t type, const string_t& path, bool& post, string_t& postfield, bool& bin)
    {
        return "[Not implemented]";
    }

    int ext_web::stream_write(vfs_stream_t type, byte c)
    {
        return 0;
    }

    bool ext_web::stream_path(const string_t& path, std::vector<byte>& data)
    {
        return false;
    }

    cwindow* ext_web::stream_getwnd(int id)
    {
        return nullptr;
    }

    int ext_web::mkdir(const string_t& path)
    {
        return fs.mkdir(path.substr(root_path.size()));
    }

    int ext_web::touch(const string_t& path)
    {
        return fs.touch(path.substr(root_path.size()));
    }

    int ext_web::rm(const string_t& path)
    {
        return fs.rm(path.substr(root_path.size()));
    }

    int ext_web::rm_safe(const string_t& path)
    {
        return fs.rm_safe(path.substr(root_path.size()));
    }

    void ext_web::set_fs_path(const string_t& path)
    {
        root_path = path + "/file";
    }

    vfs_node_stream_ext::vfs_node_stream_ext(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path) :
        vfs_node_dec(mod), stream(s), call(call) {
        success = true;
    }

    vfs_node_stream_ext::~vfs_node_stream_ext()
    {
    }

    vfs_node_dec* vfs_node_stream_ext::create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path)
    {
        vfs_node_stream_ext* m = new vfs_node_stream_ext(mod, s, call, path);
        if (!m->is_success()) {
            delete m;
            return nullptr;
        }
        return m;
    }

    bool vfs_node_stream_ext::is_success() const
    {
        return success;
    }

    bool vfs_node_stream_ext::available() const {
        if (success) {
            return true;
        }
        return false;
    }

    int vfs_node_stream_ext::index() const {
        if (!available())
            return READ_EOF;
        return DELAY_CHAR;
    }

    void vfs_node_stream_ext::advance() {
    }

    int vfs_node_stream_ext::write(byte c) {
        return -1;
    }

    int vfs_node_stream_ext::truncate() {
        return -1;
    }
}