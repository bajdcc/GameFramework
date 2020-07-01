#include "stdafx.h"
#include "Direct2DRender.h"
#include "bochs/sim.h"
#include "bochs/bochs.h"
#include "bochs/gui/d2d.h"
#include "lua_ext/ext.h"

#pragma region X86

BYTE* X86WindowElementRenderer::g_buffer;
CSize X86WindowElementRenderer::g_size;
std::auto_ptr<std::semaphore> X86WindowElementRenderer::g_signal;
BOOL X86WindowElementRenderer::g_error = FALSE;

X86WindowElement::X86WindowElement()
{
}

X86WindowElement::~X86WindowElement()
{
}

CString X86WindowElement::GetElementTypeName()
{
    return _T("X86 Window");
}

cint X86WindowElement::GetTypeId()
{
    return X86Window;
}

CStringA X86WindowElement::GetText() const
{
    return text;
}

void X86WindowElement::SetText(CStringA value)
{
    text = value;
}

FLOAT X86WindowElement::GetOpacity() const
{
    return opacity;
}

void X86WindowElement::SetOpacity(FLOAT value)
{
    opacity = value;
}

int X86WindowElement::Refresh(int arg)
{
    return std::dynamic_pointer_cast<X86WindowElementRenderer, IGraphicsRenderer>(renderer)->Refresh(arg);
}

void X86WindowElementRenderer::CreateImage(std::shared_ptr<Direct2DRenderTarget> renderTarget)
{
    if (renderTarget)
    {
        auto e = element.lock();
        auto txt = e->GetText();
        if (!buffer)
        {
            {
                int _w, _h;
                _w = 480, _h = 320;
                wic = renderTarget->CreateBitmap(_w, _h);
                data.resize(_w * _h);
                rect.X = 0;
                rect.Y = 0;
                rect.Width = _w;
                rect.Height = _h;
                buffer = new BYTE[rect.Width * rect.Height * 4];
                HRESULT hr = wic->CopyPixels(&rect, rect.Width * 4, rect.Width * rect.Height * 4, buffer);
                if (FAILED(hr))
                    ATLASSERT(!"CopyPixels failed");
                DWORD* read = (DWORD*)buffer;
                for (DWORD *read = (DWORD*)buffer, *read_end = read + (_w * _h); read != read_end; read++)
                    *read = 0xFF000000;
                d2dRect = D2D1::RectU(0, 0, rect.Width, rect.Height);

                g_signal.reset(new std::semaphore);
                g_buffer = buffer;
                g_size.cx = _w, g_size.cy = _h;
                bochs_thread.reset(new std::thread(Sim));
            }
        }
        if (wic)
        {
            if (!bitmap)
                bitmap = renderTarget->GetBitmapFromWIC(wic);
            bitmap->CopyFromMemory(&d2dRect, buffer, rect.Width * 4);
        }
    }
}

void X86WindowElementRenderer::Render(CRect bounds, CComPtr<ID2D1RenderTarget> r)
{
    auto e = element.lock();
    if (e->flags.self_visible && bitmap)
    {
        CComPtr<ID2D1RenderTarget> d2dRenderTarget = r ? r : renderTarget.lock()->GetDirect2DRenderTarget();
        CRect rt(bounds);
        if (!scaling && bounds.Width() > rect.Width && bounds.Height() > rect.Height)
        {
            rt.DeflateRect((bounds.Width() - rect.Width) / 2, (bounds.Height() - rect.Height) / 2);
            rt.right = rt.left + rect.Width;
            rt.bottom = rt.top + rect.Height;
        }
        d2dRenderTarget->DrawBitmap(
            bitmap,
            D2D1::RectF((FLOAT)rt.left, (FLOAT)rt.top, (FLOAT)rt.right, (FLOAT)rt.bottom),
            e->GetOpacity(),
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );
    }
    GraphicsImageRenderer::Render(bounds, r);
}

X86WindowElementRenderer::~X86WindowElementRenderer()
{
    bx_pc_system.kill_bochs_request = 1;
    g_error = TRUE;
    g_signal->signal();
    bochs_thread->join();
    bochs_thread.reset();
    if (buffer) delete buffer;
    g_buffer = nullptr;
    g_size.cx = g_size.cy = 0;
    g_error = FALSE;
    scaling = FALSE;
    g_signal.reset();
}

int X86WindowElementRenderer::Refresh(int arg)
{
    if (arg == 1) buffer = nullptr;
    else if (arg == 5)
    {
        // TODO: Some work
        if (SIM->get_init_done())
            bx_pc_system.Reset(BX_RESET_HARDWARE);
    }
	else if (arg == 10)
	{
		if (SIM->get_init_done())
			g_ui_map["X86-TICK"] = (lua_Integer)bx_pc_system.time_ticks();
	}
    else if (arg == 12)
    {
        if (SIM->get_init_done())
            bx_d2d_gui_c::AddKeyboardEvent((Bit32u)g_ui_map["X86-KBD"]);
    }
    else if (arg == 14)
    {
        if (SIM->get_init_done())
            scaling = !scaling;
    }
    if (arg != 0) return -1;
    if (g_size.cx != rect.Width || g_size.cy != rect.Height)
    {
        if (buffer) delete buffer;
        int _w, _h;
        _w = g_size.cx, _h = g_size.cy;
        wic = renderTarget.lock()->CreateBitmap(_w, _h);
        data.resize(_w * _h);
        rect.X = 0;
        rect.Y = 0;
        rect.Width = _w;
        rect.Height = _h;
        buffer = new BYTE[rect.Width * rect.Height * 4];
        HRESULT hr = wic->CopyPixels(&rect, rect.Width * 4, rect.Width * rect.Height * 4, buffer);
        if (FAILED(hr))
            ATLASSERT(!"CopyPixels failed");
        for (DWORD *read = (DWORD*)buffer, *read_end = read + (_w * _h); read != read_end; read++)
            *read = 0xFF000000;
        d2dRect = D2D1::RectU(0, 0, rect.Width, rect.Height);

        g_buffer = buffer;
        g_signal->signal();
    }
    if (bitmap)
    {
        bitmap->CopyFromMemory(&d2dRect, buffer, rect.Width * 4);
    }
    return 0;
}

BYTE* X86WindowElementRenderer::GetBuffer()
{
    return g_buffer;
}

CSize X86WindowElementRenderer::GetSize()
{
    return g_size;
}

BOOL X86WindowElementRenderer::SetSize(CSize size)
{
    g_size = size;
    g_signal->wait();
    return g_error;
}

#pragma endregion X86
