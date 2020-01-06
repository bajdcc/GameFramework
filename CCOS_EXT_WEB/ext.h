#ifdef CCOSEXTWEB_EXPORTS
#define CCOSEXTWEB_API extern "C" __declspec(dllexport)
#else
#define CCOSEXTWEB_API extern "C" __declspec(dllimport)
#endif

#include "../CCGameFramework/base/parser2d/cext.h"
#include "fs.h"

namespace clib {

    class ext_web : public vfs_func_t, public vfs_stream_call {
    public:
        ext_web();
        vfs_stream_t stream_type(const string_t& path) const override;
        string_t stream_callback(const string_t& path) override;
        vfs_node_dec* stream_create(const vfs_mod_query* mod, vfs_stream_t type, const string_t& path, int* ret = nullptr) override;
        int stream_index(vfs_stream_t type) override;
        string_t stream_net(vfs_stream_t type, const string_t& path, bool& post, string_t& postfield, bool& bin) override;
        int stream_write(vfs_stream_t type, byte c) override;
        bool stream_path(const string_t& path, std::vector<byte>& data) override;
        cwindow* stream_getwnd(int id) override;
    private:
        cextfs fs;
    };
}

CCOSEXTWEB_API int ccos_ext_load(clib::cext*);
CCOSEXTWEB_API int ccos_ext_unload(clib::cext*);
