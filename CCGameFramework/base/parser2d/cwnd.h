//
// Project: CParser
// Created by bajdcc
//

#ifndef CPARSER_CWND_H
#define CPARSER_CWND_H

#include "types.h"
#include <ui\window\Window.h>
#include "cvfs.h"

#define WINDOW_HANDLE_NUM 1024

namespace clib {
    class vfs_node_stream_window : public vfs_node_dec {
        friend class cvfs;
    public:
        bool available() const override;
        int index() const override;
        void advance() override;
        int write(byte c) override;
        int truncate() override;
        explicit vfs_node_stream_window(const vfs_mod_query*, vfs_stream_t, vfs_stream_call*, int id);
        ~vfs_node_stream_window();

        static vfs_node_dec* create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path);

    private:
        vfs_stream_call* call{ nullptr };
        cwindow* wnd{ nullptr };
    };

    class comctl_base {

    };

    class cvm;
    class cwindow {
    public:
        explicit cwindow(int handle, const string_t& caption, const CRect& location);
        ~cwindow();

        void init(cvm *vm);
        void paint(const CRect& bounds);
        bool hit(cvm* vm, int n, int x = 0, int y = 0);

        enum window_state_t {
            W_NONE,
            W_RUNNING,
            W_CLOSING,
        };

        window_state_t get_state() const;

        int get_msg_data();
        int get_cursor() const;

        struct window_msg {
            int code;
            uint32 param1, param2;
        };
        struct window_msg2 {
            int handle;
            window_msg msg;
        };

        enum window_comctl_type {
            comctl_none = 0,
            layout_absolute = 1,
            layout_linear,
            layout_grid,
            comctl_label = 100,
            comctl_end = 1000,
        };

        int handle_msg(cvm* vm, const window_msg& msg);
        void post_data(const int& code, int param1 = 0, int param2 = 0);

        int create_comctl(cvm* vm, window_comctl_type type);
        static string_t cwindow::handle_typename(window_comctl_type t);
        string_t handle_fs(const string_t& path);

    private:
        void init();
        bool is_border(const CPoint& pt, int& cx, int& cy);

    private:
        string_t caption;
        CRect location;
        std::shared_ptr<IGraphicsElement> root;
        std::shared_ptr<Direct2DRenderTarget> renderTarget;
        CRect bounds1, bounds2;
        window_state_t state{ W_RUNNING };
        std::queue<byte> msg_data;
        int handle{ -1 };
        bool self_focused{ false };
        bool self_hovered{ false };
        bool self_drag{ false };
        bool need_repaint{ false };
        bool self_size{ false };
        CPoint self_size_pt;
        CPoint self_drag_pt;
        CSize self_min_size;
        CRect self_drag_rt;
        int cursor{ 1 };

        struct window_handle_t {
            window_comctl_type type{ comctl_none };
            comctl_base* comctl;
        };
        std::unordered_set<int> handles_set;
        int handle_ids{ 0 };
        int available_handles{ 0 };
        std::array<window_handle_t, WINDOW_HANDLE_NUM> handles;

        struct SystemBag {
            std::shared_ptr<SolidBackgroundElement> title;
            std::shared_ptr<SolidLabelElement> title_text;
            std::shared_ptr<SolidLabelElement> close_text;
            std::shared_ptr<RoundBorderElement> border;
        } bag;
    };
}

#endif //CPARSER_CWND_H
