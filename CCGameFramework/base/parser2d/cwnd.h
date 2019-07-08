//
// Project: CParser
// Created by bajdcc
//

#ifndef CPARSER_CWND_H
#define CPARSER_CWND_H

#include "types.h"
#include <ui\window\Window.h>
#include "cvfs.h"

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

    class cvm;
    class cwindow {
    public:
        explicit cwindow(const string_t& caption, const CRect& location);
        ~cwindow();

        void paint(const CRect& bounds);
        bool hit(int n, int x, int y);

        enum window_state_t {
            W_NONE,
            W_RUNNING,
            W_CLOSING,
        };

        window_state_t get_state() const;

        int get_msg_data();

        struct window_msg {
            int code;
            uint32 param1, param2;
        };
        struct window_msg2 {
            int handle;
            window_msg msg;
        };

        void handle_msg(cvm* vm, const window_msg& msg);

    private:
        void init();
        void post_data(int code, int param1 = 0, int param2 = 0);

    private:
        string_t caption;
        CRect location;
        std::shared_ptr<IGraphicsElement> root;
        std::shared_ptr<Direct2DRenderTarget> renderTarget;
        CRect bounds1, bounds2;
        window_state_t state{ W_RUNNING };
        std::queue<byte> msg_data;

        struct SystemBag {
            std::shared_ptr<SolidBackgroundElement> title;
            std::shared_ptr<SolidLabelElement> title_text;
            std::shared_ptr<IGraphicsElement> close_text;
        } bag;
    };
}

#endif //CPARSER_CWND_H
