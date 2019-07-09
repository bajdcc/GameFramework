//
// Project: CParser
// Created by bajdcc
//

#include "stdafx.h"
#include "cvm.h"
#include "cwnd.h"

namespace clib {

    vfs_node_stream_window::vfs_node_stream_window(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, int id) :
        vfs_node_dec(mod), call(call) {
        wnd = call->stream_getwnd(id);
    }

    vfs_node_stream_window::~vfs_node_stream_window()
    {
    }

    vfs_node_dec* vfs_node_stream_window::create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path)
    {
        static string_t pat{ R"(/handle/([0-9]+)/message)" };
        static std::regex re(pat);
        std::smatch res;
        if (std::regex_match(path, res, re)) {
            auto id = atoi(res[1].str().c_str());
            return new vfs_node_stream_window(mod, s, call, id);
        }
        return nullptr;
    }

    bool vfs_node_stream_window::available() const {
        return wnd->get_state() != cwindow::W_CLOSING;
    }

    int vfs_node_stream_window::index() const {
        if (!available())return READ_EOF;
        auto d = wnd->get_msg_data();
        if (d == -1)
            return DELAY_CHAR;
        return (int)d;
    }

    void vfs_node_stream_window::advance() {

    }

    int vfs_node_stream_window::write(byte c) {
        return -1;
    }

    int vfs_node_stream_window::truncate() {
        return -1;
    }

    cwindow::cwindow(const string_t& caption, const CRect& location)
        : caption(caption), location(location)
    {
        auto bg = SolidBackgroundElement::Create();
        bg->SetColor(CColor(Gdiplus::Color::White));
        root = bg;
        root->SetRenderRect(location);
        renderTarget = std::make_shared<Direct2DRenderTarget>(window->shared_from_this());
        renderTarget->Init();
        init();
        root->GetRenderer()->SetRenderTarget(renderTarget);
    }

    cwindow::~cwindow()
    {
    }

    void cwindow::paint(const CRect& bounds)
    {
        if (bounds1 != bounds) {
            CRect rt;
            root->SetRenderRect(location.OfRect(bounds));
            rt = bag.title->GetRenderRect();
            rt.right = root->GetRenderRect().Width();
            bag.title->SetRenderRect(rt);
            rt = bag.close_text->GetRenderRect();
            rt.left = root->GetRenderRect().Width() - 20;
            rt.right = root->GetRenderRect().Width();
            bag.close_text->SetRenderRect(rt);
            bounds1 = bounds;
        }
        root->GetRenderer()->Render(root->GetRenderRect());
    }

    bool cwindow::hit(int n, int x, int y)
    {
        static int mapnc[] = {
            WM_NCLBUTTONDOWN, WM_NCLBUTTONUP, WM_NCLBUTTONDBLCLK,
            WM_NCRBUTTONDOWN, WM_NCRBUTTONUP, WM_NCRBUTTONDBLCLK,
            WM_NCMBUTTONDOWN, WM_NCMBUTTONUP, WM_NCMBUTTONDBLCLK,
        };
        static int mapc[] = {
            WM_LBUTTONDOWN, WM_LBUTTONUP, WM_LBUTTONDBLCLK,
            WM_RBUTTONDOWN, WM_RBUTTONUP, WM_RBUTTONDBLCLK,
            WM_MBUTTONDOWN, WM_MBUTTONUP, WM_MBUTTONDBLCLK,
        };
        auto pt = CPoint(x, y) + -root->GetRenderRect().TopLeft();
        if (pt.x < 0 || pt.y < 0 || pt.x >= location.Width() || pt.y >= location.Height()) return false;
        if (n >= 200 && n <= 208) {
            if (n == 200 && bag.close_text->GetRenderRect().PtInRect(pt)) {
                post_data(WM_CLOSE);
                return true;
            }
            if (bag.title->GetRenderRect().PtInRect(pt)) {
                post_data(mapnc[n - 200], pt.x, pt.y);
                return true;
            }
            else {
                pt.y -= bag.title->GetRenderRect().Height();
                post_data(mapc[n - 200], pt.x, pt.y);
                return true;
            }
        }
        return false;
    }

    cwindow::window_state_t cwindow::get_state() const
    {
        return state;
    }

    int cwindow::get_msg_data()
    {
        if (msg_data.empty())
            return - 1;
        auto d = msg_data.front();
        msg_data.pop();
        return d;
    }

    int cwindow::handle_msg(cvm* vm, const window_msg& msg)
    {
        switch (msg.code)
        {
        case WM_CLOSE:
            state = W_CLOSING;
            break;
        case WM_SETTEXT:
            caption = vm->vmm_getstr(msg.param1);
            bag.title_text->SetText(CString(CStringA(caption.c_str())));
            break;
        case WM_GETTEXT:
            vm->vmm_setstr(msg.param1, caption);
            break;
        default:
            break;
        }
        return 0;
    }

    void cwindow::init()
    {
        auto r = root->GetRenderRect();
        auto& list = root->GetChildren();
        auto title = SolidBackgroundElement::Create();
        title->SetColor(CColor(45, 120, 213));
        title->SetRenderRect(CRect(0, 0, r.Width(), 30));
        bag.title = title;
        list.push_back(title);
        root->GetRenderer()->SetRelativePosition(true);
        auto title_text = SolidLabelElement::Create();
        title_text->SetColor(CColor(Gdiplus::Color::White));
        title_text->SetRenderRect(CRect(5, 2, r.Width(), 30));
        title_text->SetText(CString(CStringA(caption.c_str())));
        bag.title_text = title_text;
        Font f;
        f.size = 20;
        f.fontFamily = "微软雅黑";
        title_text->SetFont(f);
        list.push_back(title_text);
        auto close_text = SolidLabelElement::Create();
        close_text->SetColor(CColor(Gdiplus::Color::White));
        close_text->SetRenderRect(CRect(r.Width() - 20, 2, r.Width(), 30));
        close_text->SetText(_T("×"));
        close_text->SetFont(f);
        bag.close_text = close_text;
        list.push_back(close_text);
    }

    void cwindow::post_data(int code, int param1, int param2)
    {
        window_msg s{ code, (uint32)param1, (uint32)param2 };
        const auto p = (byte*)& s;
        for (auto i = 0; i < sizeof(window_msg); i++) {
            msg_data.push(p[i]);
        }
    }
}