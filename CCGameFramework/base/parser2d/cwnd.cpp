﻿//
// Project: CParser
// Created by bajdcc
//

#include "stdafx.h"
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
        return available() ? DELAY_CHAR : READ_EOF;
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
        if (n == 200) {
            if (bag.close_text->GetRenderRect().PtInRect(CPoint(x, y) + -root->GetRenderRect().TopLeft())) {
                state = W_CLOSING;
                return true;
            }
        }
        return false;
    }

    cwindow::window_state_t cwindow::get_state() const
    {
        return state;
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
        close_text->SetText(_T("X"));
        close_text->SetFont(f);
        bag.close_text = close_text;
        list.push_back(close_text);
    }
}