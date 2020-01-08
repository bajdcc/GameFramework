#include "stdafx.h"
#include "MPM2D.h"
#include "render/Direct2DRenderTarget.h"
#include <ui\window\Window.h>
#include <random>

#define FRAME (1.0f / 30.0f)
#define N_SUBSTEP 50
#define N_FRAME 10000

#define sqr(x) ((x)*(x))

void MPM2DEngine::init(std::shared_ptr<Direct2DRenderTarget> rt)
{
    rt2 = rt;
    bag.brush = rt->CreateDirect2DBrush(D2D1::ColorF::White);
    bag.random = std::default_random_engine((uint32_t)time(nullptr));
    s.n_particles = 8192;                   // 粒子数
    s.n_grid = 128;                         // 网格数
    s.n_grid2 = s.n_grid * s.n_grid;
    s.dx = 1.0f / s.n_grid;
    s.inv_dx = (decimal)s.n_grid;
    s.inv_dx2 = s.inv_dx * s.inv_dx;
    s.dt = 2.0e-4f;
    s.p_vol = pow(s.dx * 0.5f, 2.0f);
    s.p_rho = 1.0f;
    s.p_mass = s.p_vol * s.p_rho;
    s.E = 400.0f;                           // 杨氏模量
    s.x.resize(s.n_particles);              // 粒子位置
    std::fill(s.x.begin(), s.x.end(), vec{ 0,0 });
    s.v.resize(s.n_particles);              // 粒子速度
    std::fill(s.v.begin(), s.v.end(), vec{ 0,0 });
    s.C.resize(s.n_particles);              // 仿射速度场
    std::fill(s.C.begin(), s.C.end(), mat{ 0,0,0,0 });
    s.J.resize(s.n_particles);              // 塑性形变
    std::fill(s.J.begin(), s.J.end(), 0.0f);
    s.grid_v.resize(s.n_grid2);             // 网络结点速度
    std::fill(s.grid_v.begin(), s.grid_v.end(), vec{ 0,0 });
    s.grid_m.resize(s.n_grid2);             // 网络结点质量
    std::fill(s.grid_m.begin(), s.grid_m.end(), 0.0f);
    s.frame = 0;
    auto& e = bag.random;
    std::uniform_real_distribution<decimal> dr{ 0.1f, 0.7f };
    for (auto i = 0; i < s.n_particles; i++) {
        s.x[i] = { dr(e), dr(e) };          // 初始位置
        s.v[i] = { 0.0f, -1.0f };           // 初始速度
        s.J[i] = 1.0f;                      // 初始塑性形变
    }
}

void MPM2DEngine::destroy(std::shared_ptr<Direct2DRenderTarget> rt)
{
    bag.brush.Release();
}

void MPM2DEngine::RenderByType(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderDefault(rt, bounds);
}

void MPM2DEngine::Render(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    RenderByType(rt, bounds);
}

void MPM2DEngine::Initialize(std::shared_ptr<Direct2DRenderTarget> rt)
{
    logoFont.size = 20;
    logoFont.fontFamily = "宋体";
    logoFont.bold = false;
    logoFont.italic = false;
    logoFont.underline = false;
    loggingFont.size = 12;
    loggingFont.fontFamily = "Courier New";
    loggingFont.bold = false;
    loggingFont.italic = false;
    loggingFont.underline = false;
    bgColor = CColor(15, 15, 15);
    bgColorLog = CColor(128, 128, 128, 200);
    logoColor = CColor(250, 250, 250, 240);
    last_clock = std::chrono::system_clock::now();
    dt = 30.0f;
    dt_inv = 1.0f / dt;
    font.size = 20;
    font.fontFamily = "宋体";
    font.bold = false;
    font.italic = false;
    font.underline = false;
    backup_font.size = 20;
    backup_font.fontFamily = "宋体";
    backup_font.bold = false;
    backup_font.italic = false;
    backup_font.underline = false;
}

void MPM2DEngine::Finalize(std::shared_ptr<Direct2DRenderTarget> rt)
{
    reset();
}

void MPM2DEngine::Reset(std::shared_ptr<Direct2DRenderTarget> oldRenderTarget, std::shared_ptr<Direct2DRenderTarget> newRenderTarget)
{
    if (oldRenderTarget.get() == newRenderTarget.get())
        return;
    if (oldRenderTarget)
    {
        oldRenderTarget->DestroyDirect2DBrush(bgColor); bg = nullptr;
        oldRenderTarget->DestroyDirect2DBrush(bgColorLog); bg_log = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(logoFont); logoTF = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(loggingFont); loggingTF = nullptr;
        oldRenderTarget->DestroyDirect2DBrush(logoColor); logoBrush = nullptr;
        oldRenderTarget->DestroyDirect2DTextFormat(backup_font); font_format = nullptr;
        destroy(oldRenderTarget);
    }
    if (newRenderTarget)
    {
        bg = newRenderTarget->CreateDirect2DBrush(bgColor);
        bg_log = newRenderTarget->CreateDirect2DBrush(bgColorLog);
        logoTF = newRenderTarget->CreateDirect2DTextFormat(logoFont);
        loggingTF = newRenderTarget->CreateDirect2DTextFormat(loggingFont);
        logoBrush = newRenderTarget->CreateDirect2DBrush(logoColor);
        font_format = newRenderTarget->CreateDirect2DTextFormat(font);
        init(newRenderTarget);
    }
}

int MPM2DEngine::SetType(cint value)
{
    if (value == -100) {
        paused = !paused;
        return 1;
    }
    if (value == -101) {
        reset();
        return 1;
    }
    if (value == -103) {
        global_state.is_logging = !global_state.is_logging;
        return 1;
    }
    if (value == -102) {
        return 1;
    }
    if (value & 0x40000) {
        global_state.mouse_x = value & 0xffff;
        return 0;
    }
    if (value & 0x80000) {
        global_state.mouse_y = value & 0xffff;
        return 0;
    }
    if (value & 0x100000) {
        // hit(value & 0xffff)
        return 0;
    }
    // input(value)
    return 0;
}

static char* ipsf(decimal ips) {
    static char _ipsf[32];
    if (ips < 1e3) {
        snprintf(_ipsf, sizeof(_ipsf), "%.1f", ips);
    }
    else if (ips < 1e6) {
        snprintf(_ipsf, sizeof(_ipsf), "%.1fK", ips * 1e-3);
    }
    else if (ips < 1e9) {
        snprintf(_ipsf, sizeof(_ipsf), "%.1fM", ips * 1e-6);
    }
    return _ipsf;
}

void MPM2DEngine::RenderDefault(CComPtr<ID2D1RenderTarget> rt, CRect bounds)
{
    auto now = std::chrono::system_clock::now();
    // 计算每帧时间间隔
    dt = std::chrono::duration_cast<std::chrono::duration<decimal>>(now - last_clock).count();

    auto inv = 1.0f / dt;
    if (dt > FRAME) {
        ips = cycles * dt_inv;
        cycles = 0;
        // dt = min(dt, FRAME);
        dt_inv = 1.0f / dt;
        last_clock = now;
        if (!paused)
            tick(bounds);
    }

    rt->FillRectangle(
        D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
        bg
    );
    
    draw(rt, bounds, dt_inv * FRAME);

    CString logo(_T("粒子特效 @bajdcc"));

    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.left + 10, (float)bounds.top + 5, (float)bounds.left + 200, (float)bounds.top + 50), logoBrush);

    logo.Format(_T("FPS: %2.1f Turn: %S"), inv, ipsf(ips));
    rt->DrawText(logo.GetBuffer(0), logo.GetLength(), logoTF->textFormat,
        D2D1::RectF((float)bounds.right - 210, (float)bounds.top + 5, (float)bounds.right, (float)bounds.top + 50), logoBrush);

    if (global_state.is_logging) {
        const int span = 12;
        auto R = D2D1::RectF((float)bounds.left + 10, (float)bounds.top + 10, (float)bounds.right - 10, (float)bounds.top + 60);
        rt->FillRectangle(
            D2D1::RectF((float)bounds.left, (float)bounds.top, (float)bounds.right, (float)bounds.bottom),
            bg_log
        );
        for (auto& l : global_state.logging) {
            rt->DrawText(l.GetBuffer(0), l.GetLength(), loggingTF->textFormat, R, logoBrush);
            R.top += span;
            R.bottom += span;
            if (R.top + span >= bounds.bottom) {
                break;
            }
        }
        R.top += span;
        R.bottom = (float)bounds.bottom;
        std::wstringstream wss;
        {
            wss << L"-- MPM Information --" << std::endl;
        }
        auto disp = CString(wss.str().c_str());
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
        R.top += span;
        auto lines = 1;
        {
            for (auto i = 0; i < disp.GetLength(); i++) {
                if (disp[i] == L'\n') lines++;
            }
        }
        R.top += lines * span;
        {
            wss.str(L"");
            TCHAR buf[255];
            wss << L"-- Environment --" << std::endl;
            auto size = bounds.Size();
            _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), L"Screen= (%4d, %4d)", (int)size.cx, (int)size.cy);
            wss << buf << std::endl;
        }
        disp = CString(wss.str().c_str());
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
        R = D2D1::RectF((float)bounds.right - 400, (float)bounds.top + 10, (float)bounds.right - 10, (float)bounds.bottom);
        disp = CString(_T("显示3"));
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
        lines = 3;
        {
            for (auto i = 0; i < disp.GetLength(); i++) {
                if (disp[i] == L'\n') lines++;
            }
        }
        R.top += lines * span;
        disp = CString(_T("显示4"));
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
        lines = 3;
        {
            for (auto i = 0; i < disp.GetLength(); i++) {
                if (disp[i] == L'\n') lines++;
            }
        }
        R.top += lines * span;
        disp = CString(_T("显示5"));
        rt->DrawText(disp, disp.GetLength(), loggingTF->textFormat, R, logoBrush);
    }
}

void MPM2DEngine::draw(CComPtr<ID2D1RenderTarget>& rt, const CRect& bounds, decimal fps) {
    auto center = bounds.TopLeft();
    auto size = bounds.Size();
    auto w = (decimal)size.cx;
    auto h = (decimal)size.cy;
    auto grid_w = w * s.dx;
    auto grid_h = h * s.dx;
    rt->SetTransform(
        D2D1::Matrix3x2F::Translation({ (decimal)center.x, (decimal)center.y })
    );
    auto clr = bag.brush->GetColor();
    for (auto i = 0; i < s.n_grid; i++) {
        for (auto j = 0; j < s.n_grid; j++) {
            auto idx = i * s.n_grid + j;
            if (s.grid_m[idx] > 0.0f) {
                auto r = 0.5f + s.grid_v[idx].x * 0.5f;
                r = min(1.0f, max(r, 0.0f));
                auto g = 0.5f + s.grid_v[idx].y * 0.5f;
                g = min(1.0f, max(g, 0.0f));
                auto b = (log10(1.0f + s.grid_m[idx]) - 1.0f);
                b = 1.0f - min(1.0f, b);
                bag.brush->SetColor(D2D1::ColorF(r, g, b, 0.6f));
                rt->FillRectangle({ floor((decimal)i * grid_w), floor((decimal)(s.n_grid - j) * grid_h), ceil((decimal)(i + 1) * grid_w), ceil((decimal)(s.n_grid - j + 1) * grid_h) }, bag.brush);
            }
        }
    }
    bag.brush->SetColor(clr);
    for (const auto& p : s.x) {
        rt->FillRectangle({ p.x * w, (1.0f - p.y) * h , p.x * w + 1.2f, (1.0f - p.y) * h + 1.2f }, bag.brush);
    }
    rt->SetTransform(D2D1::Matrix3x2F::Identity());
}

void MPM2DEngine::tick(const CRect& bounds)
{
    if (!paused) {
        if (s.frame < N_FRAME) {
            for (auto i = 0; i < N_SUBSTEP; i++) {
                cycles++;
                substep();
            }
            s.frame++;
        }
    }
}

void MPM2DEngine::reset()
{
    auto rt = rt2.lock();
    if (!rt) return;
    destroy(rt);
    init(rt);
}

void MPM2DEngine::substep()
{
    std::fill(s.grid_v.begin(), s.grid_v.end(), vec{ 0,0 });
    std::fill(s.grid_m.begin(), s.grid_m.end(), 0.0f);
    std::vector<vec> w(3);
    // 粒子状态更新，粒子到网格（P2G）
    for (auto p = 0; p < s.n_particles; p++) {
        auto base = (s.x[p] * s.inv_dx - 0.5f).to_int();
        auto fx = s.x[p] * s.inv_dx - base;
        // 链接：http://mpm.graphics
        // 二次核函数（Eqn. 123, with x=fx,fx-1,fx-2]）
        w[0].x = 0.5f * sqr(1.5f - fx.x);
        w[0].y = 0.5f * sqr(1.5f - fx.y);
        w[1].x = 0.75f - sqr(fx.x - 1.0f);
        w[1].y = 0.75f - sqr(fx.y - 1.0f);
        w[2].x = 0.5f * sqr(fx.x - 0.5f);
        w[2].y = 0.5f * sqr(fx.y - 0.5f);
        // 计算应力
        auto stress = -s.dt * s.p_vol * (s.J[p] - 1.0f) * 4.0f * s.inv_dx2 * s.E;
        // 计算形变
        auto affine = mat{ stress, 0, 0, stress } + s.p_mass * s.C[p];
        // 影响3x3网格
        for (auto i = 0; i < 3; i++) {
            for (auto j = 0; j < 3; j++) {
                auto offset = vec{ (decimal)i, (decimal)j };
                auto dpos = (offset - fx) * s.dx;
                auto weight = w[i].x * w[j].y;
                auto idx = ((int)base.x + (int)offset.x) * s.n_grid + (int)base.y + (int)offset.y;
                if (idx >= 0 && idx < s.n_grid2) {
                    s.grid_v[idx] += weight * (s.p_mass * s.v[p] + affine * dpos);
                    s.grid_m[idx] += weight * s.p_mass;
                }
            }
        }
    }
    for (auto i = 0; i < s.n_grid; i++) {
        for (auto j = 0; j < s.n_grid; j++) {
            auto idx = i * s.n_grid + j;
            if (s.grid_m[idx] > 0) {
                auto bound = 3;
                auto inv_m = 1.0f / s.grid_m[idx];
                // 冲量转换成速度
                s.grid_v[idx] *= inv_m;
                // 重力
                s.grid_v[idx].y -= s.dt * 9.8f;
                // 范围约束
                if (i < bound && s.grid_v[idx].x < 0.0f)
                    s.grid_v[idx].x = 0.01f * -s.grid_v[idx].x;
                if (i > s.n_grid - bound && s.grid_v[idx].x > 0.0f)
                    s.grid_v[idx].x = 0.01f * -s.grid_v[idx].x;
                if (j < bound && s.grid_v[idx].y < 0.0f)
                    s.grid_v[idx].y = 0.01f * -s.grid_v[idx].y;
                if (j > s.n_grid - bound && s.grid_v[idx].y > 0.0f)
                    s.grid_v[idx].y = 0.01f * -s.grid_v[idx].y;
            }
        }
    }
    // 网格到粒子（G2P）
    for (auto p = 0; p < s.n_particles; p++) {
        auto base = (s.x[p] * s.inv_dx - 0.5f).to_int();
        auto fx = s.x[p] * s.inv_dx - base;
        w[0].x = 0.5f * sqr(1.5f - fx.x);
        w[0].y = 0.5f * sqr(1.5f - fx.y);
        w[1].x = 0.75f - sqr(fx.x - 1.0f);
        w[1].y = 0.75f - sqr(fx.y - 1.0f);
        w[2].x = 0.5f * sqr(fx.x - 0.5f);
        w[2].y = 0.5f * sqr(fx.y - 0.5f);
        vec new_v;
        mat new_C{ 0,0,0,0 };
        for (auto i = 0; i < 3; i++) {
            for (auto j = 0; j < 3; j++) {
                auto offset = vec{ (decimal)i, (decimal)j };
                auto dpos = offset - fx;
                auto weight = w[i].x * w[j].y;
                auto idx = ((int)base.x + i) * s.n_grid + (int)base.y + j;
                if (idx >= 0 && idx < s.n_grid2) {
                    auto g_v = s.grid_v[idx];
                    new_v += weight * g_v;
                    new_C += mat(g_v, dpos) * (4.0f * weight * s.inv_dx);
                }
            }
        }
        s.v[p] = new_v;
        s.x[p] += s.dt * s.v[p]; // 流动
        s.J[p] *= 1.0f + s.dt * (new_C.x1 + new_C.y2);
        s.C[p] = new_C;
    }
}
