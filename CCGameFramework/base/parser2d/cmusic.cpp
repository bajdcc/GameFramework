//
// Project: cliblisp
// Created by bajdcc
//

#include "stdafx.h"
#include "cmusic.h"
#include <ui\window\Window.h>

#define MAKE_TIME(a,b,c) (((a) << 12) | ((b) << 6) | (c))

namespace clib {

    libZPlay::ZPlay* vfs_node_stream_music::zplay = nullptr;

    vfs_node_stream_music::vfs_node_stream_music(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path) :
        vfs_node_dec(mod), stream(s), call(call) {
        if (zplay) {
            return;
        }
        zplay = libZPlay::CreateZPlay();
        auto p = call->stream_path(path.substr(6));
        auto result = zplay->OpenFile(p.c_str(), libZPlay::sfAutodetect);
        if (result == 0) {
            ATLTRACE("[SYSTEM] PLAY | libzplay error: %s\n", zplay->GetError());
            zplay->Release();
            zplay = nullptr;
            return;
        }
        zplay->GetStreamInfo(&info);
        zplay->Play();
        success = true;
        ZeroMemory(&pos, sizeof(pos));
        all = MAKE_TIME(info.Length.hms.hour, info.Length.hms.minute, info.Length.hms.second);
        all_time.Format("%02d:%02d:%02d", info.Length.hms.hour, info.Length.hms.minute, info.Length.hms.second);
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

    vfs_node_dec* vfs_node_stream_music::create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path)
    {
        vfs_node_stream_music* m = new vfs_node_stream_music(mod, s, call, path);
        if (!m->is_success()) {
            delete m;
            return nullptr;
        }
        return m;
    }

    bool vfs_node_stream_music::is_success() const
    {
        return success;
    }

    bool vfs_node_stream_music::available() const {
        if (success) {
            return now < all;
        }
        return false;
    }

    int vfs_node_stream_music::index() const {
        if (!available())
            return READ_EOF;
        if (idx < text.GetLength())
            return text[idx];
        return DELAY_CHAR;
    }

    void vfs_node_stream_music::advance() {
        if (text.IsEmpty()) {
            zplay->GetPosition(&pos);
            now = MAKE_TIME(pos.hms.hour, pos.hms.minute, pos.hms.second);
            if (now >= last) {
                text.Format("\r%02d:%02d:%02d / %s", pos.hms.hour, pos.hms.minute, pos.hms.second,
                    all_time.GetBuffer(0));
                last = now + 1;
            }
            return;
        }
        if (idx >= text.GetLength()) {
            text = "";
            idx = 0;
        }
        else {
            idx++;
        }
    }

    int vfs_node_stream_music::write(byte c) {
        return -1;
    }

    int vfs_node_stream_music::truncate() {
        return -1;
    }
}

#undef MAKE_TIME