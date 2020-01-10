#ifndef MICE2D_MICE2D_H
#define MICE2D_MICE2D_H
#include <render/Direct2DRenderTarget.h>
#include <memory>
#include "MiceAtom.h"

using string_t = std::string;

class Mice2DEngine
{
public:
    void RenderByType(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

    void Initialize(std::shared_ptr<Direct2DRenderTarget> rt);
    void Finalize(std::shared_ptr<Direct2DRenderTarget> rt);
    void Reset(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget);

    int SetType(cint value);

    void init(std::shared_ptr<Direct2DRenderTarget> rt);
    void destroy(std::shared_ptr<Direct2DRenderTarget> rt);

private:
    void RenderDefault(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void draw(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, mice2d::decimal fps);
    void tick(const CRect& bounds);
    void reset();

private:
    CComPtr<ID2D1SolidColorBrush> bg;
    CComPtr<ID2D1SolidColorBrush> bg_log;
    CColor bgColor;
    CColor bgColorLog;
    Font logoFont;
    Font loggingFont;
    CColor logoColor;
    std::shared_ptr<D2DTextFormatPackage> logoTF;
    std::shared_ptr<D2DTextFormatPackage> loggingTF;
    CComPtr<ID2D1SolidColorBrush> logoBrush;
    std::weak_ptr<Direct2DRenderTarget> rt2;

private:
    std::chrono::system_clock::time_point last_clock;
    mice2d::decimal dt{ 0 };
    mice2d::decimal dt_inv{ 0 };
    int cycles{ 0 };
    mice2d::decimal ips{ 0 };
    bool paused{ false };

    struct global_state_t {
        decltype(std::chrono::system_clock::now()) now;
        int mouse_x{ 0 };
        int mouse_y{ 0 };
        int window_focus{ -1 };
        int window_hover{ -1 };
        std::list<CString> logging;
        bool is_logging{ false };
    } global_state;

    mice2d::DrawBag bag;
    std::vector<mice2d::MiceAtom> mices;
    int global_id{ 1 };

private:
    std::shared_ptr<D2DTextFormatPackage> font_format;
    Font font, backup_font;
};

#endif