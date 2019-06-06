//
// Project: cliblisp
// Created by bajdcc
//

#include "stdafx.h"
#include "cmusic.h"
#include <ui\window\Window.h>

namespace clib {

    libZPlay::ZPlay* vfs_node_stream_music::zplay = nullptr;

    vfs_node_stream_music::vfs_node_stream_music(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path) :
        vfs_node_dec(mod), stream(s), call(call) {
        if (zplay) {
            success = false;
            return;
        }
        zplay = libZPlay::CreateZPlay();
        auto p = call->stream_path(path.substr(6));
        auto result = zplay->OpenFile(p.c_str(), libZPlay::sfAutodetect);
        if (result == 0) {
            ATLTRACE("[SYSTEM] PLAY | libzplay error: %s\n", zplay->GetError());
            zplay->Release();
            zplay = nullptr;
            success = false;
            return;
        }
        zplay->GetStreamInfo(&info);
        zplay->Play();
    }

    vfs_node_stream_music::~vfs_node_stream_music()
    {
        if (success) {
            zplay->Stop();
            zplay->Close();
            zplay->Release();
            zplay = nullptr;
        }
    }

#define MAKE_TIME(a,b,c) (((a) << 12) | ((b) << 6) | (c))
    bool vfs_node_stream_music::available() const {
        if (success) {
            static libZPlay::TStreamTime pos;
            zplay->GetPosition(&pos);
            return MAKE_TIME(pos.hms.hour, pos.hms.minute, pos.hms.second) <
                   MAKE_TIME(info.Length.hms.hour, info.Length.hms.minute, info.Length.hms.second);
        }
        return false;
    }
#undef MAKE_TIME

    int vfs_node_stream_music::index() const {
        return available() ? WAIT_CHAR : READ_ERROR;
    }

    void vfs_node_stream_music::advance() {
    }

    int vfs_node_stream_music::write(byte c) {
        return -1;
    }

    int vfs_node_stream_music::truncate() {
        return -1;
    }
}
