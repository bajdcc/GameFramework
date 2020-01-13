#ifdef CCOSEXTFS_EXPORTS
#define CCOSEXTFS_API extern "C" __declspec(dllexport)
#else
#define CCOSEXTFS_API extern "C" __declspec(dllimport)
#endif

#include "../CCGameFramework/base/parser2d/cext.h"
#include "fs.h"

namespace clib {

    class ext_web : public vfs_func_t, public vfs_stream_call, public vfs_oper {
    public:
        ext_web();
        vfs_stream_t stream_type(const string_t& path) const override;
        string_t stream_callback(const string_t& path) override;
        vfs_node_dec* stream_create(const vfs_mod_query* mod, vfs_stream_t type, const string_t& path, int* ret = nullptr) override;
        vfs_oper* stream_oper() override;
        int stream_index(vfs_stream_t type) override;
        string_t stream_net(vfs_stream_t type, const string_t& path, bool& post, string_t& postfield, bool& bin) override;
        int stream_write(vfs_stream_t type, byte c) override;
        bool stream_path(const string_t& path, std::vector<byte>& data) override;
        cwindow* stream_getwnd(int id) override;
        int mkdir(const string_t& path) override;
        int touch(const string_t& path) override;
        int rm(const string_t& path) override;
        int rm_safe(const string_t& path) override;
        void set_fs_path(const string_t& path);
    private:
        string_t root_path;
        cextfs fs;
    };
}

CCOSEXTFS_API int ccos_ext_load(clib::cext*);
CCOSEXTFS_API int ccos_ext_unload(clib::cext*);
