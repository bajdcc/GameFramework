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

    bool ready() const override;
    void move_to(int x, int y) override;
    void line_to(int x, int y) override;
    void draw_point(int x, int y) override;
    int get_width() const override;
    int get_height() const override;
    void set_color(uint c) override;
    void clear(uint c) override;
    void fill_rect(int x, int y) override;
    int set_fresh(int fresh) override;
    void reset() override;
    void create_font() override;
    void set_font_size(int size) override;
    void set_font_family(const string_t &name) override;
    void draw_font(const string_t& text) override;

private:
    void RenderDefault(CComPtr<ID2D1RenderTarget> rt, CRect bounds);

    bool check_cord(int x, int y) const;
    void bresenham(int x0, int y0, int x1, int y1);
    bool setpixel(int x, int y);

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

private:
    CComPtr<ID2D1Bitmap> bitmap;
    CComPtr<ID2D1Bitmap> bitmap_effect;
    CComPtr<ID2D1BitmapRenderTarget> rt2;
    std::weak_ptr<Direct2DRenderTarget> d2drt;
    WICRect rect;
    D2D1_RECT_U d2drect;

    CPoint cur_pt;
    CComPtr<ID2D1SolidColorBrush> cur_bursh;
    int auto_fresh{ 1 };

private:
    std::chrono::system_clock::time_point last_clock;
    double dt{ 0 };
    double dt_inv{ 0 };
    int cycles{ 0 };
    double ips{ 0 };
    bool paused{ false };

private:
    std::shared_ptr<D2DTextFormatPackage> font_format;
    Font font, backup_font;
};

#endif