#ifndef MPM2D_MPM2D_H
#define MPM2D_MPM2D_H
#include <render/Direct2DRenderTarget.h>
#include <memory>
#include <random>
#include "math/v2.h"
#include "math/m2.h"

using string_t = std::string;
using clib::mpm::decimal;
using vec = clib::mpm::v2<>;
using mat = clib::mpm::m2<>;

class MPM2DEngine
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
    void draw(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, decimal fps);
    void tick(const CRect& bounds);
    void reset();
    void substep();
    void input(int value);

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

    struct DrawBag {
        CComPtr<ID2D1SolidColorBrush> brush;
        std::default_random_engine random;
    } bag;

private:
    std::chrono::system_clock::time_point last_clock;
    decimal dt{ 0 };
    decimal dt_inv{ 0 };
    int cycles{ 0 };
    decimal ips{ 0 };
    bool paused{ false };
    CString err;

    struct global_state_t {
        decltype(std::chrono::system_clock::now()) now;
        int mouse_x{ 0 };
        int mouse_y{ 0 };
        int window_focus{ -1 };
        int window_hover{ -1 };
        std::list<CString> logging;
        bool is_logging{ false };
    } global_state;

    struct mpm_state_t {
        int n_particles;
        decimal n_particles_1;
        int n_grid;
        int n_grid_2;
        int n_grid2;
        decimal dx;
        decimal inv_dx;
        decimal inv_dx2;
        decimal dt;
        decimal p_vol;
        decimal p_rho;
        decimal p_mass;
        decimal E;
        std::vector<vec> x;
        std::vector<vec> v;
        std::vector<mat> C;
        std::vector<decimal> J;
        std::vector<vec> grid_v;
        std::vector<decimal> grid_m;
        vec mass_center;
        vec gravity;
        int mode;
        int grid;
        decimal vortex;
        int frame;
    } s;

private:
    std::shared_ptr<D2DTextFormatPackage> font_format;
    Font font, backup_font;
};

#endif