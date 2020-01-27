//
// Project: cliblisp
// Created by bajdcc
//

#include "stdafx.h"
#include "cmusic.h"
#include <ui\window\Window.h>
#include <base\utils.h>

#define MAKE_TIME(a,b,c) (((a) << 12) | ((b) << 6) | (c))

namespace clib {

    libZPlay::ZPlay* vfs_node_stream_music::zplay = nullptr;

    vfs_node_stream_music::vfs_node_stream_music(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path) :
        vfs_node_dec(mod), stream(s), call(call) {
        if (zplay) {
            return;
        }
        zplay = libZPlay::CreateZPlay();
        auto paths = std::split(path.substr(6), '!');
        auto music_path = paths[0];
        if (!call->stream_path(music_path, data)) {
            ATLTRACE("[SYSTEM] PLAY | file not exists: %s\n", path);
            zplay->Release();
            zplay = nullptr;
            return;
        }
        std::vector<byte> lyric_data;
        if (paths.size() > 1 && !call->stream_path(paths[1], lyric_data)) {
            ATLTRACE("[SYSTEM] PLAY | lyric not exists: %s\n", path);
        }
        lyric_data.push_back(0);
        lyric_str = string_t((char*)lyric_data.data());
        parse_lyric();
        auto type = libZPlay::sfAutodetect;
        if (music_path.length() > 4 && music_path.substr(music_path.length() - 4) == ".mp3") {
            type = libZPlay::sfMp3;
        }
        auto result = zplay->OpenStream(0, 0, data.data(), data.size(), type);
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
            libZPlay::TStreamStatus status;
            zplay->GetStatus(&status);
            return status.fPlay == 1 || status.fPause == 1;
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
                if (lyric_id >= 0) {
                    auto width = max_lyric_width;
                    if (now >= std::get<0>(lyrics[lyric_id])) {
                        width -= (int)std::get<1>(lyrics[lyric_id]).length();
                        text.AppendFormat(" | %s ", std::get<1>(lyrics[lyric_id]).c_str());
                        for (auto i = 0; i < width; i++) {
                            text.AppendChar(' ');
                        }
                        max_lyric_width = max(max_lyric_width, (int)std::get<1>(lyrics[lyric_id]).length());
                        if (lyric_id < (int)lyrics.size() - 1)
                            lyric_id++;
                    }
                }
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

    void vfs_node_stream_music::parse_lyric()
    {
        const auto& str = lyric_str;
        static std::regex re1(R"((\[\d*:\d*[.:]\d*\])(.*))");
        static std::regex re2(R"(\[(\d*):(\d*)([.:])(\d*)\].*)");
        std::map<int, std::string> ly;
        std::unordered_set<int> emptys;
        auto _lyrics = std::split(str);
        for (const auto& lyr : _lyrics)
        {
            std::vector<std::string> times;
            std::smatch sm;
            auto lycn = lyr;
            if (!lycn.empty() && lycn.back() > 0 && std::isspace(lycn.back()))
                lycn.erase(lycn.end() - 1);
            for (;;) {
                if (std::regex_match(lycn, sm, re1)) {
                    times.push_back(sm[1].str());
                    lycn = sm[2].str();
                }
                else {
                    break;
                }
            }
            for (const auto& ti : times) {
                std::smatch tm;
                if (std::regex_match(ti, tm, re2)) {
                    auto hour = atoi(tm[1].str().c_str());
                    auto minute = atoi(tm[2].str().c_str());
                    auto second = atoi(tm[4].str().c_str());
                    auto delim = tm[3].str();
                    if (delim == ".")
                        second = MAKE_TIME(0, hour, minute);
                    else
                        second = MAKE_TIME(hour, minute, second);
                    if (lycn.empty())
                    {
                        emptys.insert(second);
                    }
                    else
                    {
                        ly.insert(std::make_pair(second, lycn));
                    }
                }
            }
        }
        for (const auto& em : emptys)
        {
            if (ly.find(em) == ly.end())
                ly.insert(std::make_pair(em, ""));
        }
        for (auto& y : ly)
        {
            lyrics.push_back(std::make_tuple(y.first, y.second));
        }
        if (!lyrics.empty()) {
            lyric_id = 0;
        }
    }
}

#undef MAKE_TIME