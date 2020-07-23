//
// Project: clibjs
// Created by bajdcc
//

#include "stdafx.h"
#include "../cjsruntime.h"
#include "../cjsgui.h"

#define LOG_PUZZLE 0
#define N 16

namespace clib {

    using BOARD_DST_TYPE = std::string;
    using BOARD_SRC_TYPE = std::vector<int>;
    using BOARD_DIR_TYPE = std::vector<bool>;
    using BOARD_TEST_TYPE = std::bitset<N* N>;
    using BOARD_MAP_TYPE = std::array<char, N* N>;

    static void trans_map(const std::vector<CRect>& blocks, const BOARD_SRC_TYPE& src, BOARD_TEST_TYPE& bs) {
        bs.reset();
        for (size_t i = 0; i < blocks.size(); i++) {
            const auto& b = blocks[i];
            if (b.Width() == 0) {
                auto start = src[i];
                auto end = src[i] + b.Height();
                for (auto j = start; j <= end; j++) {
                    bs.set(j * N + b.left);
                }
            }
            else {
                auto start = src[i];
                auto end = src[i] + b.Width();
                for (auto j = start; j <= end; j++) {
                    bs.set(j + b.top * N);
                }
            }
        }
    }

    static void build_map(const std::vector<CRect>& blocks, const BOARD_SRC_TYPE& src, BOARD_MAP_TYPE& m) {
        memset(&m, ' ', sizeof(m));
        for (size_t i = 0; i < blocks.size(); i++) {
            const auto& b = blocks[i];
            if (b.Width() == 0) {
                auto start = src[i];
                auto end = start + b.Height();
                for (auto j = start; j <= end; j++) {
                    m[j * N + b.left] = '|';
                }
            }
            else {
                auto start = src[i];
                auto end = start + b.Width();
                for (auto j = start; j <= end; j++) {
                    m[j + b.top * N] = '-';
                }
            }
        }
    }

    static void build_map(const std::vector<CRect>& blocks, const BOARD_DST_TYPE& dst, BOARD_MAP_TYPE& m) {
        memset(&m, ' ', sizeof(m));
        for (size_t i = 0; i < blocks.size(); i++) {
            const auto& b = blocks[i];
            if (b.Width() == 0) {
                auto start = dst[i] - '0';
                auto end = start + b.Height();
                for (auto j = start; j <= end; j++) {
                    m[j * N + b.left] = '|';
                }
            }
            else {
                auto start = dst[i] - '0';
                auto end = start + b.Width();
                for (auto j = start; j <= end; j++) {
                    m[j + b.top * N] = '-';
                }
            }
        }
    }

    static void trans(const BOARD_SRC_TYPE& blocks, BOARD_DST_TYPE& out) {
        out.resize(blocks.size());
        for (size_t i = 0; i < blocks.size(); i++) {
            out[i] = blocks[i] + '0';
        }
    }

    static int dist(int size, BOARD_TEST_TYPE& bs, int x, int y, int dx, int dy) {
        if (dx == 0) {
            if (dy < 0) {
                for (auto i = y - 1; i > 0; i--) {
                    if (bs[i * N + x]) {
                        if (i == y - 1)
                            return -1;
                        return i + 1;
                    }
                }
                return 1;
            }
            else {
                for (auto i = y + 1; i <= size; i++) {
                    if (bs[i * N + x]) {
                        if (i == y + 1)
                            return -1;
                        return i - 1;
                    }
                }
                return size;
            }
        }
        else {
            if (dx < 0) {
                for (auto i = x - 1; i > 0; i--) {
                    if (bs[y * N + i]) {
                        if (i == x - 1)
                            return -1;
                        return i + 1;
                    }
                }
                return 1;
            }
            else {
                for (auto i = x + 1; i <= size; i++) {
                    if (bs[y * N + i]) {
                        if (i == x + 1)
                            return -1;
                        return i - 1;
                    }
                }
                return size;
            }
        }
        return -1;
    }

    int helper_Klotski_1(js_value_new* js, int size, const std::vector<CRect>& blocks, int red, js_value::ref& out) {
        if (size >= 10)
            return 0;
        std::unordered_map<BOARD_DST_TYPE, std::tuple<int, BOARD_DST_TYPE>> visited;
        std::deque<BOARD_SRC_TYPE> queue;
        BOARD_SRC_TYPE tmp;
        BOARD_DST_TYPE id, parent;
        BOARD_TEST_TYPE test;
#if LOG_PUZZLE
        BOARD_MAP_TYPE map;
#endif
        auto horizon = blocks[red].Width() != 0; // horizontal = true
        auto red_idx = horizon ? blocks[red].top : blocks[red].left;
        tmp.reserve(blocks.size());
        for (const auto& b : blocks) {
            tmp.push_back(b.Width() == 0 ? b.top : b.left);
        }
        queue.push_back(tmp);
        trans(tmp, id);
        visited.insert({ id, {0,""} });
        auto dst = 0;
#if LOG_PUZZLE
        fprintf(stdout, "[MAP] %s\n", id.c_str());
#endif
        decltype(visited)::mapped_type result = { INT_MAX,"" };
        while (!queue.empty()) {
            auto q = queue.front();
            queue.pop_front();
            trans(q, parent);
            auto pp = visited.at(parent);
            trans_map(blocks, q, test);
#if LOG_PUZZLE
            build_map(blocks, q, map);
            fprintf(stdout, "\n%s\n", parent.c_str());
            for (auto i = 1; i <= size; i++) {
                map[i * N + size + 1] = '\0';
                fprintf(stdout, "%s\n", &map[i * N + 1]);
            }
#endif
            for (size_t i = 0; i < blocks.size(); i++) {
                const auto& b = blocks[i];
                if (b.Width() == 0) {
                    if ((dst = dist(size, test, b.left, q[i], 0, -1)) != -1) {
                        std::copy(q.begin(), q.end(), tmp.begin());
                        tmp[i] = dst;
                        trans(tmp, id);
                        auto f = visited.find(id);
                        if (f == visited.end()) {
                            queue.push_back(tmp);
                            visited.insert({ id, {std::get<0>(pp) + 1,parent} });
#if LOG_PUZZLE
                            fprintf(stdout, "[MAP] %s, parent= %s\n", id.c_str(), parent.c_str());
#endif
                        }
                        else if (std::get<0>(f->second) > std::get<0>(pp) + 1) {
                            visited[id] = { std::get<0>(pp) + 1, std::get<1>(pp) };
                        }
                    }
                    if ((dst = dist(size, test, b.right, q[i] + b.Height(), 0, 1)) != -1) {
                        if (!horizon && b.left == red_idx && dst == size) {
                            if (std::get<0>(pp) + 1 < std::get<0>(result)) {
                                result = { std::get<0>(pp) + 1, parent };
#if LOG_PUZZLE
                                fprintf(stdout, "[MAP] ok\n");
#endif
                            }
                            continue;
                        }
                        std::copy(q.begin(), q.end(), tmp.begin());
                        tmp[i] = dst - b.Height();
                        trans(tmp, id);
                        auto f = visited.find(id);
                        if (f == visited.end()) {
                            queue.push_back(tmp);
                            visited.insert({ id, {std::get<0>(pp) + 1,parent} });
#if LOG_PUZZLE
                            fprintf(stdout, "[MAP] %s, parent= %s\n", id.c_str(), parent.c_str());
#endif
                        }
                        else if (std::get<0>(f->second) > std::get<0>(pp) + 1) {
                            visited[id] = { std::get<0>(pp) + 1, std::get<1>(pp) };
                        }
                    }
                }
                else {
                    if ((dst = dist(size, test, q[i], b.top, -1, 0)) != -1) {
                        std::copy(q.begin(), q.end(), tmp.begin());
                        tmp[i] = dst;
                        trans(tmp, id);
                        auto f = visited.find(id);
                        if (f == visited.end()) {
                            queue.push_back(tmp);
                            visited.insert({ id, {std::get<0>(pp) + 1,parent} });
#if LOG_PUZZLE
                            fprintf(stdout, "[MAP] %s, parent= %s\n", id.c_str(), parent.c_str());
#endif
                        }
                        else if (std::get<0>(f->second) > std::get<0>(pp) + 1) {
                            visited[id] = { std::get<0>(pp) + 1, std::get<1>(pp) };
                        }
                    }
                    if ((dst = dist(size, test, q[i] + b.Width(), b.bottom, 1, 0)) != -1) {
                        if (horizon && b.top == red_idx && dst == size) {
                            if (std::get<0>(pp) + 1 < std::get<0>(result)) {
                                result = { std::get<0>(pp) + 1, parent };
                                visited.insert({ id, {std::get<0>(pp) + 1,parent} });
#if LOG_PUZZLE
                                fprintf(stdout, "[MAP] ok\n");
#endif
                            }
                            continue;
                        }
                        std::copy(q.begin(), q.end(), tmp.begin());
                        tmp[i] = dst - b.Width();
                        trans(tmp, id);
                        auto f = visited.find(id);
                        if (f == visited.end()) {
                            queue.push_back(tmp);
                            visited.insert({ id, {std::get<0>(pp) + 1,parent} });
#if LOG_PUZZLE
                            fprintf(stdout, "[MAP] %s, parent= %s\n", id.c_str(), parent.c_str());
#endif
                        }
                        else if (std::get<0>(f->second) > std::get<0>(pp) + 1) {
                            visited[id] = { std::get<0>(pp) + 1, std::get<1>(pp) };
                        }
                    }
                }
            }
        }
        if (!std::get<1>(result).empty()) {
            std::vector<BOARD_DST_TYPE> path;
            auto current = std::get<1>(result);
            while (!current.empty()) {
                path.push_back(current);
                current = std::get<1>(visited.at(current));
            }
            std::reverse(path.begin(), path.end());
            auto origin = path.front();
            size_t j;
            std::vector<js_value::ref> steps;
            for (const auto& p : path) {
                for (j = 0; j < p.size(); j++) {
                    if (p[j] != origin[j]) {
                        break;
                    }
                }
                js->new_object();
                if (j < p.size()) {
                    auto step = js->new_object();
                    step->add("id", js->new_number(j));
                    step->add("dx", js->new_number(blocks[j].Height() == 0 ? ((int)(p[j]) - (int)(origin[j])) : 0));
                    step->add("dy", js->new_number(blocks[j].Width() == 0 ? ((int)(p[j]) - (int)(origin[j])) : 0));
                    steps.push_back(step);
                    origin = p;
                }
            }
            out = js->new_array(steps);
#if LOG_PUZZLE
            fprintf(stdout, "[ANSWER]\n");
            for (const auto& p : path) {
                build_map(blocks, p, map);
                fprintf(stdout, "\n%s\n", p.c_str());
                for (auto i = 1; i <= size; i++) {
                    map[i * N + size + 1] = '\0';
                    fprintf(stdout, "%s\n", &map[i * N + 1]);
                }
            }
#endif
        }
        return 0;
    }
}