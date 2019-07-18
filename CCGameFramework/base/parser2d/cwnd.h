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

    class cwindow_comctl_label;
    class cwindow_layout;
    class comctl_base {
    public:
        comctl_base(int type);
        virtual void set_rt(std::shared_ptr<Direct2DRenderTarget> rt);
        virtual void paint(const CRect& bounds);
        comctl_base* get_parent() const;
        virtual cwindow_layout* get_layout();
        virtual cwindow_comctl_label* get_label();
        void set_bound(const CRect& bound);
        CRect get_bound() const;
        virtual int set_flag(int flag);
        virtual int hit(int x, int y) const;
        void set_id(int id);
        int get_id() const;
        virtual int handle_msg(int code, uint32 param1, uint32 param2);
    protected:
        int id{ -1 };
        int type{ 0 };
        comctl_base* parent{ nullptr };
        CRect bound;
        std::weak_ptr<Direct2DRenderTarget> rt;
    };

    class cvm;
    class cwindow {
    public:
        explicit cwindow(cvm* vm, int handle, const string_t& caption, const CRect& location);
        ~cwindow();

        void init();
        void paint(const CRect& bounds);
        bool hit(int n, int x = 0, int y = 0);

        enum window_state_t {
            W_NONE,
            W_RUNNING,
            W_CLOSING,
        };

        window_state_t get_state() const;

        int get_msg_data();
        int get_cursor() const;

        struct window_msg {
            int code, comctl;
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

        int handle_msg(const window_msg& msg);
        void post_data(const int& code, int param1 = 0, int param2 = 0, int comctl = -1);

        int create_comctl(window_comctl_type type);
        static string_t cwindow::handle_typename(window_comctl_type t);
        string_t handle_fs(const string_t& path);
        int get_base() const;

        bool connect(int p, int c);
        bool set_bound(int h, const CRect& bound);
        bool set_text(int h, const string_t& text);
        bool set_flag(int h, int flag);

    private:
        void _init();
        bool is_border(const CPoint& pt, int& cx, int& cy);
        static comctl_base* new_comctl(window_comctl_type t);

        void error(const string_t& str) const;

        void destroy();
        void destroy_handle(int handle, bool force = false);

        bool valid_handle(int h) const;

    private:
        string_t caption;
        CRect location;
        std::shared_ptr<IGraphicsElement> root;
        std::shared_ptr<Direct2DRenderTarget> renderTarget;
        CRect bounds1, bounds2, bounds3;
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
        CPoint self_client;
        int cursor{ 1 };
        cvm* vm{ nullptr };
        int base_id{ -1 };
        int comctl_focus{ -1 };
        int comctl_hover{ -1 };

        struct window_handle_t {
            window_comctl_type type{ comctl_none };
            comctl_base* comctl{ nullptr };
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
            comctl_base* comctl{ nullptr };
        } bag;
    };

    class cwindow_layout : public comctl_base {
    public:
        cwindow_layout(int type);
        cwindow_layout* get_layout();
        void add(comctl_base* child);
        void remove(comctl_base* child);
        std::vector<int> get_list() const;
        int hit(int x, int y) const override;
    protected:
        std::vector<comctl_base*> children;
    };

    class cwindow_layout_absolute : public cwindow_layout {
    public:
        cwindow_layout_absolute();
        void paint(const CRect& bounds) override;
    };

    class cwindow_layout_linear : public cwindow_layout {
    public:
        cwindow_layout_linear();
        void paint(const CRect& bounds) override;
        int set_flag(int flag) override;
    private:
        enum align_type {
            vertical,
            horizontal,
        } align{ vertical };
    };

    class cwindow_comctl_label : public comctl_base {
    public:
        cwindow_comctl_label();
        void set_rt(std::shared_ptr<Direct2DRenderTarget> rt) override;
        void paint(const CRect& bounds) override;
        cwindow_comctl_label* get_label() override;
        void set_text(const string_t& text);
        int set_flag(int flag) override;
        int hit(int x, int y) const override;
        int handle_msg(int code, uint32 param1, uint32 param2) override;
    private:
        std::shared_ptr<SolidLabelElement> text;
    };
}

#endif //CPARSER_CWND_H
