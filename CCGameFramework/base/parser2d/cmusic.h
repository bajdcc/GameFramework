//
// Project: CParser
// Created by bajdcc
//

#ifndef CPARSER_CMUSIC_H
#define CPARSER_CMUSIC_H

#include "cvfs.h"
#include "libzplay/libzplay.h"

namespace clib {

    class vfs_node_stream_music : public vfs_node_dec {
        friend class cvfs;
    public:
        bool available() const override;
        int index() const override;
        void advance() override;
        int write(byte c) override;
        int truncate() override;
        explicit vfs_node_stream_music(const vfs_mod_query*, vfs_stream_t, vfs_stream_call*, const string_t& path);
        ~vfs_node_stream_music();

        static vfs_node_dec* create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path);

        bool is_success() const;

    private:
        void parse_lyric();

    private:
        bool success{false};
        vfs_stream_t stream{ fss_none };
        vfs_stream_call* call{ nullptr };
        static libZPlay::ZPlay* zplay;
        libZPlay::TStreamInfo info;
        libZPlay::TStreamTime pos;
        CStringA text;
        CStringA all_time;
        int all{ 0 };
        int idx{ 0 };
        int now{ 0 };
        int last{ -1 };
        std::vector<byte> data;
        string_t lyric_str;
        int lyric_id{ -1 };
        std::vector<std::tuple<int, string_t>> lyrics;
    };
}

#endif //CPARSER_CMUSIC_H
