#ifndef MICE2D_MICE2D_H
#define MICE2D_MICE2D_H
#include "render/Direct2DAllocator.h"
#include <memory>

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

    struct BrushBag {
        Font cmdFont; std::shared_ptr<D2DTextFormatPackage> cmdTF;
        Font gbkFont; std::shared_ptr<D2DTextFormatPackage> gbkTF;
    } brushes;

    bool ready() const;
    void move_to(int x, int y);
    void line_to(int x, int y);
    void draw_point(int x, int y);
    int get_width() const;
    int get_height() const;
    void set_color(uint c);
    void clear(uint c);
    void fill_rect(int x, int y);
    int set_fresh(int fresh);
    void reset();
    void create_font();
    void set_font_size(int size);
    void set_font_family(const string_t &name);
    void draw_font(const string_t& text);

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

    struct global_state_t {
        decltype(std::chrono::system_clock::now()) now;
        int mouse_x{ 0 };
        int mouse_y{ 0 };
        int window_focus{ -1 };
        int window_hover{ -1 };
        std::list<CString> logging;
        bool is_logging{ false };
    } global_state;

private:
    std::shared_ptr<D2DTextFormatPackage> font_format;
    Font font, backup_font;
};

#endif