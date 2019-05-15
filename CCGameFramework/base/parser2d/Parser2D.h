#ifndef PARSER2D_PARSER2D_H
#define PARSER2D_PARSER2D_H
#include "render/Direct2DAllocator.h"
#include "cui.h"
#include <memory>

class Parser2DEngine : public cgui_op
{
public:
    void RenderByType(CComPtr<ID2D1RenderTarget> rt, CRect bounds);
    void Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

    void Initialize(std::shared_ptr<Direct2DRenderTarget> rt);
    void Finalize(std::shared_ptr<Direct2DRenderTarget> rt);
    void Reset(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget);

    int SetType(cint value);

    struct BrushBag {
        Font cmdFont; std::shared_ptr<D2DTextFormatPackage> cmdTF;
        Font gbkFont; std::shared_ptr<D2DTextFormatPackage> gbkTF;
    } brushes;

    void move_to(int x, int y) override;
    void line_to(int x, int y) override;
    void draw_point(int x, int y) override;
    int get_width() const override;
    int get_height() const override;

private:
    void RenderDefault(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

    bool check_cord(int x, int y) const;
    void bresenham(int x0, int y0, int x1, int y1);
    bool setpixel(int x, int y);

private:
    CComPtr<ID2D1SolidColorBrush> bg;
    CColor bgColor;
    Font logoFont;
    CColor logoColor;
    std::shared_ptr<D2DTextFormatPackage> logoTF;
    CComPtr<ID2D1SolidColorBrush> logoBrush;

private:
    std::vector<BYTE> buffer_mem;
    BYTE* buffer{ nullptr };
    CComPtr<ID2D1Bitmap> bitmap;
    WICRect rect;
    D2D1_RECT_U d2drect;

    CPoint cur_pt;
    CColor cur_bursh;

private:
    std::chrono::system_clock::time_point last_clock;
    double dt{ 0 };
    double dt_inv{ 0 };
    int cycles{ 0 };
    double ips{ 0 };
    bool paused{ false };
};

#endif