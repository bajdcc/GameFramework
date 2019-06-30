//
// Project: CParser
// Created by bajdcc
//

#include "stdafx.h"
#include "cwnd.h"

namespace clib {

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
            rt.right = root->GetRenderRect().right;
            bag.title->SetRenderRect(rt);
            bounds1 = bounds;
        }
        root->GetRenderer()->Render(root->GetRenderRect());
    }

    void cwindow::init()
    {
        auto r = root->GetRenderRect();
        auto& list = root->GetChildren();
        auto title = SolidBackgroundElement::Create();
        title->SetColor(CColor(45, 120, 213));
        title->SetRenderRect(CRect(0, 0, r.right, 30));
        bag.title = title;
        list.push_back(title);
        root->GetRenderer()->SetRelativePosition(true);
        auto title_text = SolidLabelElement::Create();
        title_text->SetColor(CColor(Gdiplus::Color::White));
        title_text->SetRenderRect(CRect(5, 2, r.right, 30));
        title_text->SetText(CString(CStringA(caption.c_str())));
        bag.title_text = title_text;
        Font f;
        f.size = 20;
        f.fontFamily = "微软雅黑";
        title_text->SetFont(f);
        list.push_back(title_text);
    }
}