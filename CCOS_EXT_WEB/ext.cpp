#include "pch.h"
#include "ext.h"

using namespace clib;

#define EXT_NAME "web"
#define EXT_NORMAL_TEXT "网页扩展 - by bajdcc"
#define EXT_NORMAL_VERSION "0.1.0"

std::unique_ptr<ext_web> g_ext;
std::string func_path;

CCOSEXTWEB_API int ccos_ext_load(clib::cext* ptr)
{
    if (g_ext) return -1;
    g_ext = std::make_unique<ext_web>();
    auto ext = (cext*)ptr;
    func_path = ext->ext_get_path(EXT_NAME);
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
    };

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

    vfs_node_text::vfs_node_text(const vfs_mod_query* mod, const string_t& str) :
        vfs_node_dec(mod), cache(str) {}

    bool vfs_node_text::available() const {
        return idx < cache.length();
    }

    int vfs_node_text::index() const {
        return idx < cache.length() ? cache[idx] : READ_EOF;
    }

    int vfs_node_text::get_length() const {
        return (int)cache.length();
    }

    bool vfs_node_text::get_data(std::vector<byte>& data) const
    {
        data.resize(cache.size());
        std::copy(cache.begin(), cache.end(), data.begin());
        return true;
    }

    vfs_stream_t ext_web::stream_type(const string_t& path) const
    {
        return fss_none;
    }

    string_t ext_web::stream_callback(const string_t& path)
    {
        return "[Not implemented]";
    }

    vfs_node_dec* ext_web::stream_create(const vfs_mod_query* mod, vfs_stream_t type, const string_t& path)
    {
        switch (type) {
        case fss_normal:
            return new vfs_node_text(mod, EXT_NORMAL_TEXT);
        case fss_version:
            return new vfs_node_text(mod, EXT_NORMAL_VERSION);
        default:
            break;
        }
        if (path.size() > func_path.size()) {
            auto p = path.substr(func_path.size());
            if (p == "/version")
                return stream_create(mod, fss_version, path);
        }
        return stream_create(mod, fss_normal, path);
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