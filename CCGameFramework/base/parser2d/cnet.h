//
// Project: clibparser
// Created by bajdcc
//

#ifndef CLIBPARSER_CNET_H
#define CLIBPARSER_CNET_H

#include "types.h"
#include "cvfs.h"
#include <event2/http.h>
#include <event2/buffer.h>
#include <pthread/pthread.h>
#include <pthread/semaphore.h>
#include <libmicrohttpd/microhttpd.h>

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
        bool get_data(std::vector<byte>& data) const override;

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
        cint id{ 0 };
        string_t url;
        bool b64{ false };
        bool post{ false };
        string_t postfield;
        bool bin{ false };
        vfs_node_stream_net* net{ nullptr };
        int* received{ nullptr };
    };

    struct net_http_response
    {
        cint id{ 0 };
        UINT code{ 0 };
        std::vector<char> data;
        bool b64{ false };
        bool post{ false };
        bool bin{ false };
        vfs_node_stream_net* net{ nullptr };
        int* received{ nullptr };
    };

    class vfs_node_stream_server : public vfs_node_dec {
        friend class cvfs;
    public:
        bool available() const override;
        int index() const override;
        void advance() override;
        int write(byte c) override;
        int truncate() override;
        bool set_data(const std::vector<byte>& data) override;
        explicit vfs_node_stream_server(const vfs_mod_query*, vfs_stream_t, vfs_stream_call*, const string_t& path, int port);
        ~vfs_node_stream_server();

        static vfs_node_dec* create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path);

        void gen_req(const char* url);
        void reset();

    public:
        static int net_server_thread(
            void* cls,
            MHD_Connection* connection,
            const char* url,
            const char* method,
            const char* version,
            const char* upload_data,
            size_t* upload_data_size,
            void** ptr);

    private:
        vfs_stream_t stream{ fss_none };
        vfs_stream_call* call{ nullptr };
        int state{ 0 };
        int port{ -1 };
        string_t req_string;
        std::vector<byte> resp;
        MHD_Daemon* daemon{ nullptr };
        sem_t sem;
        pthread_mutex_t enter;
        int ref{ 0 };
        int error{ 0 };
    };
}

#endif //CLIBPARSER_CNET_H
