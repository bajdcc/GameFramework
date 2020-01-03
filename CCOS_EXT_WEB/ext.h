#ifdef CCOSEXTWEB_EXPORTS
#define CCOSEXTWEB_API extern "C" __declspec(dllexport)
#else
#define CCOSEXTWEB_API extern "C" __declspec(dllimport)
#endif

#include "../CCGameFramework/base/parser2d/cext.h"

namespace clib {

    class vfs_node_text : public vfs_node_dec {
        friend class ext_web;
    public:
        bool available() const override;
        int index() const override;
        int get_length() const override;
        bool get_data(std::vector<byte>& data) const override;
    private:
        explicit vfs_node_text(const vfs_mod_query*, const string_t& str);
        std::string cache;
    };

    class vfs_node_stream_ext : public vfs_node_dec {
        friend class ext_web;
    public:
        bool available() const override;
        int index() const override;
        void advance() override;
        int write(byte c) override;
        int truncate() override;
        explicit vfs_node_stream_ext(const vfs_mod_query*, vfs_stream_t, vfs_stream_call*, const string_t& path);
        ~vfs_node_stream_ext();

        static vfs_node_dec* create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path);

        bool is_success() const;

    private:
        bool success{ false };
        vfs_stream_t stream{ fss_none };
        vfs_stream_call* call{ nullptr };
    };

    class ext_web : public vfs_func_t, public vfs_stream_call {
    public:
        vfs_stream_t stream_type(const string_t& path) const override;
        string_t stream_callback(const string_t& path) override;
        vfs_node_dec* stream_create(const vfs_mod_query* mod, vfs_stream_t type, const string_t& path) override;
        int stream_index(vfs_stream_t type) override;
        string_t stream_net(vfs_stream_t type, const string_t& path, bool& post, string_t& postfield, bool& bin) override;
        int stream_write(vfs_stream_t type, byte c) override;
        bool stream_path(const string_t& path, std::vector<byte>& data) override;
        cwindow* stream_getwnd(int id) override;
    };
}

CCOSEXTWEB_API int ccos_ext_load(clib::cext*);
CCOSEXTWEB_API int ccos_ext_unload(clib::cext*);
