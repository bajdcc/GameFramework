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

    class cwindow_style : public std::enable_shared_from_this<cwindow_style> {
    public:
        enum color_t {
            c__none,
            c_window_nonclient,
            c_window_nonclient_lost,
            c_window_background,
            c_window_title_text,
            c_window_close_btn,
            c_window_close_bg,
            c_window_close_bg_lost,
            c_window_border_def,
            c_window_border_lost,
            c_button_bg_def,
            c_button_bg_focus,
            c_button_fg_def,
            c_button_fg_hover,
            c__end,
        };
        enum str_t {
            s__none,
            s_title_hang,
            s__end,
        };
        enum px_t {
            p__none,
            p_title_y,
            p_close_btn_x,
            p_border_x,
            p_hang_blur,
            p_min_size_x,
            p_min_size_y,
            p__end,
        };
        enum float_t {
            f__none,
            f_window_border_radius,
            f_button_radius,
            f__end,
        };
        enum style_t {
            S__NONE,
            S_WIN10_DEFAULT,
            S_WIN10_WHITE,
            S__END
        };

        using ref = std::shared_ptr<cwindow_style>;

        virtual CColor get_color(color_t t) const = 0;
        virtual string_t get_str(str_t t) const = 0;
        virtual int get_int(px_t t) const = 0;
        virtual float get_float(float_t t) const = 0;
        virtual string_t get_name() const = 0;

        static ref create_style(style_t t);
    };

    class cwindow_comctl_label;
    class cwindow_layout;
    class comctl_base {
    public:
        comctl_base(int type);
        virtual void set_rt(std::shared_ptr<Direct2DRenderTarget> rt, cwindow_style::ref);
        virtual void paint(const CRect& bounds);
        comctl_base* get_parent() const;
        void set_parent(comctl_base* parent);
        virtual cwindow_layout* get_layout();
        virtual cwindow_comctl_label* get_label();
        void set_bound(const CRect& bound);
        CRect get_bound() const;
        virtual int set_flag(int flag);
        virtual int hit(int x, int y) const;
        void set_id(int id);
        int get_id() const;
        virtual int handle_msg(int code, uint32 param1, uint32 param2);
        virtual CSize min_size() const;
    protected:
        int id{ -1 };
        int type{ 0 };
        comctl_base* parent{ nullptr };
        CRect bound;
        std::weak_ptr<Direct2DRenderTarget> rt;
    };

    class cwindow_style_win : public cwindow_style {
    public:
        CColor get_color(color_t t) const override;
        string_t get_str(str_t t) const override;
        int get_int(px_t t) const override;
        float get_float(float_t t) const override;
        string_t get_name() const override;
    };

    class cwindow_style_win_white : public cwindow_style_win {
    public:
        CColor get_color(color_t t) const override;
        int get_int(px_t t) const override;
        float get_float(float_t t) const override;
        string_t get_name() const override;
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
            W_BUSY,
            W_IDLE,
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
            comctl_none,
            layout_absolute,
            layout_linear,
            layout_grid,
            comctl_label,
            comctl_button,
            comctl_end,
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
        bool set_style(int style);

        std::wstring to_string() const;

    private:
        void _init();
        void _init_style();
        bool is_border(const CPoint& pt, int& cx, int& cy);
        static comctl_base* new_comctl(window_comctl_type t);

        void error(const string_t& str) const;

        void destroy();
        void destroy_handle(int handle, bool force = false);

        bool valid_handle(int h) const;
        bool is_nonclient(const CPoint& pt) const;

        void set_caption(const string_t&);

    private:
        string_t caption;
        CRect location;
        std::shared_ptr<SolidBackgroundElement> root;
        std::shared_ptr<Direct2DRenderTarget> renderTarget;
        CRect bounds1, bounds2, bounds3;
        window_state_t state{ W_RUNNING };
        std::queue<byte> msg_data;
        int handle{ -1 };
        bool self_focused{ false };
        bool self_hovered{ false };
        bool self_drag{ false };
        bool self_drag_start{ false };
        bool need_repaint{ false };
        bool self_size{ false };
        CPoint self_size_pt;
        CPoint self_drag_pt;
        CSize self_min_size;
        CRect self_drag_rt;
        CPoint self_client;
        CRect self_title;
        int cursor{ 1 };
        cvm* vm{ nullptr };
        int base_id{ -1 };
        int comctl_focus{ -1 };
        int comctl_hover{ -1 };
        std::chrono::time_point<std::chrono::system_clock> time_handler;

        struct window_handle_t {
            window_comctl_type type{ comctl_none };
            comctl_base* comctl{ nullptr };
        };
        std::unordered_set<int> handles_set;
        int handle_ids{ 0 };
        int available_handles{ 0 };
        std::array<window_handle_t, WINDOW_HANDLE_NUM> handles;

        cwindow_style::ref style;

        struct SystemBag {
            std::shared_ptr<SolidBackgroundElement> title;
            std::shared_ptr<SolidBackgroundElement> client;
            std::shared_ptr<SolidLabelElement> title_text;
            std::shared_ptr<SolidLabelElement> close_text;
            std::shared_ptr<SolidBackgroundElement> close_bg;
            std::shared_ptr<RoundBorderElement> border;
            comctl_base* comctl{ nullptr };
        } bag;
    };

    class cwindow_layout : public comctl_base {
    public:
        cwindow_layout(int type);
        void set_rt(std::shared_ptr<Direct2DRenderTarget> rt, cwindow_style::ref) override;
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
        CSize min_size() const override;
    private:
        enum align_type {
            vertical,
            horizontal,
        } align{ vertical };
    };

    class cwindow_comctl_label : public comctl_base {
    public:
        cwindow_comctl_label();
        void set_rt(std::shared_ptr<Direct2DRenderTarget> rt, cwindow_style::ref) override;
        void paint(const CRect& bounds) override;
        cwindow_comctl_label* get_label() override;
        void set_text(const string_t& text);
        int set_flag(int flag) override;
        int hit(int x, int y) const override;
        int handle_msg(int code, uint32 param1, uint32 param2) override;
        CSize min_size() const override;
    protected:
        std::shared_ptr<SolidLabelElement> text;
    };

    class cwindow_comctl_button : public cwindow_comctl_label {
    public:
        cwindow_comctl_button();
        void set_rt(std::shared_ptr<Direct2DRenderTarget> rt, cwindow_style::ref) override;
        void paint(const CRect& bounds) override;
        int hit(int x, int y) const override;
        int handle_msg(int code, uint32 param1, uint32 param2) override;
        CSize min_size() const override;
    private:
        std::shared_ptr<RoundBorderElement> background;
        std::weak_ptr<cwindow_style> _style;
    };
}

#endif //CPARSER_CWND_H
