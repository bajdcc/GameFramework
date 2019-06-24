#include "stdafx.h"
#include "Direct2DRender.h"
#include <base/libqrencode/qrencode.h>
#include "base64/b64.h"
#include "../ui/window/Window.h"

#pragma region Image

QRImageElement::QRImageElement()
{

}

QRImageElement::~QRImageElement()
{
    renderer->Finalize();
}

CString QRImageElement::GetElementTypeName()
{
    return _T("QRImage");
}

CColor QRImageElement::GetColor() const
{
    return color;
}

void QRImageElement::SetColor(CColor value)
{
    if (color != value)
    {
        color = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

CStringA QRImageElement::GetText() const
{
    return text;
}

void QRImageElement::SetText(CStringA value)
{
    if (text != value)
    {
        text = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

FLOAT QRImageElement::GetOpacity() const
{
    return opacity;
}

void QRImageElement::SetOpacity(FLOAT value)
{
    opacity = value;
}

cint QRImageElement::GetTypeId()
{
    return QRImage;
}

void QRImageElementRenderer::CreateImage(std::shared_ptr<Direct2DRenderTarget> renderTarget)
{
    if (renderTarget)
    {
        auto e = element.lock();
        auto qrcode = QRcode_encodeString(
            e->GetText(),
            0,
            QR_ECLEVEL_H,
            QR_MODE_8,
            1
        );
        auto WICBitmap = renderTarget->CreateBitmap(qrcode->width, qrcode->width);
        bitmap = renderTarget->GetBitmapFromWIC(WICBitmap);
        WICRect rect;
        rect.X = 0;
        rect.Y = 0;
        rect.Width = bitmap->GetPixelSize().width;
        rect.Height = bitmap->GetPixelSize().height;
        BYTE* buffer = new BYTE[rect.Width * rect.Height * 4];
        HRESULT hr = WICBitmap->CopyPixels(&rect, rect.Width * 4, rect.Width * rect.Height * 4, buffer);
        if (FAILED(hr))
            ATLASSERT(!"CopyPixels failed");
        auto count = rect.Width * rect.Height;
        BYTE* read = buffer;
        auto color = e->GetColor();
        for (auto i = 0; i < count; i++)
        {
            if ((qrcode->data[i] & 1) == 0)
            {
                *reinterpret_cast<int*>(read) = -1;
            }
            else
            {
                read[0] = color.b;//B
                read[1] = color.g;//G
                read[2] = color.r;//R
                read[3] = color.a;//A
            }
            read += 4;
        }
        QRcode_free(qrcode);
        D2D1_RECT_U d2dRect = D2D1::RectU(0, 0, rect.Width, rect.Height);
        bitmap->CopyFromMemory(&d2dRect, buffer, rect.Width * 4);
        delete[] buffer;
    }
}

void QRImageElementRenderer::Render(CRect bounds)
{
    auto e = element.lock();
    if (e->flags.self_visible)
    {
        CComPtr<ID2D1RenderTarget> d2dRenderTarget = renderTarget.lock()->GetDirect2DRenderTarget();
        d2dRenderTarget->DrawBitmap(
            bitmap,
            D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
            e->GetOpacity(),
            D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
        );
    }
    GraphicsImageRenderer::Render(bounds);
}

Base64ImageElement::Base64ImageElement()
{

}

Base64ImageElement::~Base64ImageElement()
{
    renderer->Finalize();
}

CString Base64ImageElement::GetElementTypeName()
{
    return _T("Base64Image");
}

CStringA Base64ImageElement::GetText() const
{
    return text;
}

void Base64ImageElement::SetText(CStringA value)
{
    if (text != value)
    {
        text = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

CStringA Base64ImageElement::GetUrl() const
{
    return url;
}

void Base64ImageElement::SetUrl(CStringA value)
{
    if (url != value)
    {
        url = value;
    }
}

FLOAT Base64ImageElement::GetOpacity() const
{
    return opacity;
}

void Base64ImageElement::SetOpacity(FLOAT value)
{
    opacity = value;
}

cint Base64ImageElement::GetTypeId()
{
    return Base64Image;
}

void Base64ImageElementRenderer::CreateImage(std::shared_ptr<Direct2DRenderTarget> renderTarget)
{
    if (renderTarget)
    {
        auto e = element.lock();
        auto txt = e->GetText();
        if (e->GetUrl().CompareNoCase(url) != 0 && text.CompareNoCase(txt) != 0)
        {
            if (txt.IsEmpty())
                return;
            auto bin = base64_decode(txt.GetBuffer(0));
            DWORD dw = MAKELONG(MAKEWORD(bin[0], bin[1]), MAKEWORD(bin[2], bin[3]));
            auto b = (std::vector<byte>*)dw;
            if (b->empty())
            {
                //TODO: ´Ë´¦ÓÐbug
                window->remove_lua_ptr(b);
                return;
            }
            wic = renderTarget->CreateImageFromMemory(b->data(), b->size());
            url = e->GetUrl();
            text = txt;
            window->remove_lua_ptr(b);
        }
        if (wic)
            bitmap = renderTarget->GetBitmapFromWIC(wic);
    }
}

void Base64ImageElementRenderer::Render(CRect bounds)
{
    auto e = element.lock();
    if (e->flags.self_visible)
    {
        CComPtr<ID2D1RenderTarget> d2dRenderTarget = renderTarget.lock()->GetDirect2DRenderTarget();
        if (bitmap)
        {
            d2dRenderTarget->DrawBitmap(
                bitmap,
                D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                e->GetOpacity(),
                D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
            );
        }
    }
    GraphicsImageRenderer::Render(bounds);
}

WireworldAutomatonImageElement::WireworldAutomatonImageElement()
{
}

WireworldAutomatonImageElement::~WireworldAutomatonImageElement()
{
}

CString WireworldAutomatonImageElement::GetElementTypeName()
{
    return _T("WireworldATM");
}

cint WireworldAutomatonImageElement::GetTypeId()
{
    return WireworldAutomaton;
}

CStringA WireworldAutomatonImageElement::GetText() const
{
    return text;
}

void WireworldAutomatonImageElement::SetText(CStringA value)
{
    if (text != value)
    {
        text = value;
        if (renderer)
        {
            renderer->OnElementStateChanged();
        }
    }
}

FLOAT WireworldAutomatonImageElement::GetOpacity() const
{
    return opacity;
}

void WireworldAutomatonImageElement::SetOpacity(FLOAT value)
{
    opacity = value;
}

int WireworldAutomatonImageElement::Refresh(int arg)
{
    return std::dynamic_pointer_cast<WireworldAutomatonImageElementRenderer, IGraphicsRenderer>(renderer)->Refresh(arg);
}

DWORD watm_clr[] =
{
    CColor::Parse("#111111").value,
    CColor::Parse("#EFB949").value,
    CColor::Parse("#49D0EF").value,
    CColor::Parse("#EF4949").value
};

void WireworldAutomatonImageElementRenderer::CreateImage(std::shared_ptr<Direct2DRenderTarget> renderTarget)
{
    if (renderTarget)
    {
        auto e = element.lock();
        auto txt = e->GetText();
        if (!buffer)
        {
            if (txt.IsEmpty())
                return;
            auto str = std::string(txt.GetBuffer(0));
            {
                std::ifstream in(str, std::ios::in);
                if (!in) return;
                int _w, _h;
                in >> _w >> _h;//h=958 w=631
                auto line = new char[_w + 1];
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
                BYTE* read = buffer;
                in.getline(line, _w + 1);
                for (auto i = 0, k = 0; i < _h; i++)
                {
                    in.getline(line, _w + 1);
                    for (auto j = 0; j < _w; j++)
                    {
                        auto c = line[j];
                        if (c == ' ')
                            k = 0;//empty
                        else if (c == '#')
                            k = 1;//wire
                        else if (c == '@')
                            k = 2;//head
                        else if (c == '~')
                            k = 3;//tail
                        data[i * _w + j] = k;
                        ((DWORD*)read)[0] = watm_clr[k];//B
                        read += 4;
                    }
                }
                d2dRect = D2D1::RectU(0, 0, rect.Width, rect.Height);
                delete[] line;
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

void WireworldAutomatonImageElementRenderer::Render(CRect bounds)
{
    auto e = element.lock();
    if (e->flags.self_visible && bitmap)
    {
        CComPtr<ID2D1RenderTarget> d2dRenderTarget = renderTarget.lock()->GetDirect2DRenderTarget();
        if ((rect.Width > 1000 || rect.Height > 600) && bounds.Width() > rect.Width * 2 && bounds.Height() > rect.Width / 2)
        {
            d2dRenderTarget->DrawBitmap(
                bitmap,
                D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.left + rect.Width, (FLOAT)bounds.top + (rect.Height) / 2),
                e->GetOpacity(),
                D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                D2D1::RectF((FLOAT)0, (FLOAT)rect.Height / 2, (FLOAT)rect.Width, (FLOAT)rect.Height)
            );
            d2dRenderTarget->DrawBitmap(
                bitmap,
                D2D1::RectF((FLOAT)bounds.left + rect.Width + 1, (FLOAT)bounds.top, (FLOAT)bounds.left + (rect.Width) * 2 + 1, (FLOAT)bounds.top + (rect.Height) / 2),
                e->GetOpacity(),
                D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                D2D1::RectF((FLOAT)0, (FLOAT)0, (FLOAT)rect.Width, (FLOAT)rect.Height / 2)
            );
        }
        else
        {
            d2dRenderTarget->DrawBitmap(
                bitmap,
                D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
                e->GetOpacity(),
                D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
            );
        }
    }
    GraphicsImageRenderer::Render(bounds);
}

WireworldAutomatonImageElementRenderer::~WireworldAutomatonImageElementRenderer()
{
    if (buffer) delete buffer;
}

bool IsValidPoint(const INT& w, const INT& h, const INT& x, const INT& y)
{
    return x >= 0 && y >= 0 && x < w && y < h;
}

int IsVal(const BYTE* data, const INT& w, const INT& h, const INT& x, const INT& y, const BYTE& value)
{
    if (IsValidPoint(w, h, x, y) && data[y * w + x] == value)
    {
        return 1;
    }
    return 0;
}

int IsHead(const BYTE* data, const INT& w, const INT& h, const INT& x, const INT& y)
{
    return IsVal(data, w, h, x, y, 2);
}

bool CanHead(const BYTE* d, const INT& w, const INT& h, const INT& j, const INT& i)
{
    if (!IsVal(d, w, h, j, i, 1)) return false;
    auto n = IsHead(d, w, h, j + 1, i) + IsHead(d, w, h, j - 1, i) + IsHead(d, w, h, j, i + 1) + IsHead(d, w, h, j, i - 1)
        + IsHead(d, w, h, j + 1, i + 1) + IsHead(d, w, h, j - 1, i - 1) + IsHead(d, w, h, j - 1, i + 1) + IsHead(d, w, h, j + 1, i - 1);
    return n == 1 || n == 2;
}

int WireworldAutomatonImageElementRenderer::Refresh(int arg)
{
    if (arg == 1) buffer = nullptr;
    if (arg != 0) return -1;
    wires.clear();
    heads.clear();
    tails.clear();
    auto w = rect.Width, h = rect.Height;
    int k;
    auto d = data.data();
    DWORD* read = (DWORD*)buffer;
    for (auto i = 0; i < h; i++)
    {
        k = i * w;
        for (auto j = 0; j < w; j++)
        {
            auto v = d[k + j];
            if (v == 3)
            {
                wires.push_back(k + j);
            }
            else if (v == 2)
            {
#define CANHEAD(jj, ii) if (CanHead(d, w, h, (jj), (ii))) {heads.push_back((ii) * w + (jj));}
                CANHEAD(j + 1, i + 1)
                CANHEAD(j + 1, i + 0)
                CANHEAD(j + 1, i - 1)
                CANHEAD(j + 0, i + 1)
                CANHEAD(j + 0, i - 1)
                CANHEAD(j - 1, i + 1)
                CANHEAD(j - 1, i + 0)
                CANHEAD(j - 1, i - 1)
#undef CANHEAD
                tails.push_back(k + j);
            }
        }
    }
    for (auto & i : heads)
    {
        data[i] = 2;
        read[i] = watm_clr[2];
    }
    for (auto & i : tails)
    {
        data[i] = 3;
        read[i] = watm_clr[3];
    }
    for (auto & i : wires)
    {
        data[i] = 1;
        read[i] = watm_clr[1];
    }
    if (bitmap)
    {
        bitmap->CopyFromMemory(&d2dRect, buffer, rect.Width * 4);
    }
    return 0;
}
#pragma endregion Image
