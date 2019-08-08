//
// Project: clibparser
// Created by bajdcc
//

#ifndef CLIBPARSER_CNET_H
#define CLIBPARSER_CNET_H

#include "types.h"
#include "cvfs.h"

namespace clib {

    class cnet {
    public:
        cnet() = default;

        cnet(const cnet&) = delete;
        cnet& operator=(const cnet&) = delete;

        string_t http_get(const string_t& url, bool& post, string_t& postfield, bool& bin);

        static CString Utf8ToStringT(LPCSTR str);
        static CString GBKToStringT(LPCSTR str);
        static CStringA StringTToUtf8(CString str);

        static int get_id();

    private:
        static int req_id;
    };

    class vfs_node_stream_net : public vfs_node_dec {
        friend class cvfs;
    public:
        bool available() const override;
        int index() const override;
        void advance() override;
        int write(byte c) override;
        int truncate() override;
        explicit vfs_node_stream_net(const vfs_mod_query*, vfs_stream_t, vfs_stream_call*, const string_t& path);
        ~vfs_node_stream_net();

        static vfs_node_dec* create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path);

    public:
        string_t get_url() const;
        int* get_received() const;
        void set_response(const std::vector<char>& resp);

    private:
        vfs_stream_t stream{ fss_none };
        vfs_stream_call* call{ nullptr };
        string_t url;
        std::vector<char> content;
        int* received{ nullptr };
        int id{ -1 };
        bool post{ false };
        string_t postfield;
        bool bin{ false };
    };

    struct net_http_request
    {
        cint id;
        string_t url;
        bool b64;
        bool post;
        string_t postfield;
        bool bin;
        vfs_node_stream_net* net;
        int* received;
    };

    struct net_http_response
    {
        cint id;
        UINT code;
        std::vector<char> data;
        bool b64;
        bool post;
        bool bin;
        vfs_node_stream_net* net;
        int* received;
    };
}

#endif //CLIBPARSER_CNET_H
