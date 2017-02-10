#include "stdafx.h"
#include "Direct2DRender.h"

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

void SolidLabelElementRenderer::RenderTargetChangedInternal(PassRefPtr<Direct2DRenderTarget> oldRenderTarget, PassRefPtr<Direct2DRenderTarget> newRenderTarget)
{
    DestroyBrush(oldRenderTarget.get());
    DestroyTextFormat(oldRenderTarget.get());
    CreateBrush(newRenderTarget.get());
    CreateTextFormat(newRenderTarget.get());
    UpdateMinSize();
}

void QRImageElementRenderer::CreateImage(PassRefPtr<Direct2DRenderTarget> renderTarget)
{
    if (renderTarget)
    {
        auto WICBitmap = renderTarget->CreateBitmap(200, 200);
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
        for (auto i = 0; i < count; i++)
        {
            read[0] = 128;//B
            read[1] = 128;//G
            read[2] = 128;//R
            read[3] = 255;//APLHA
            read += 4;
        }
        D2D1_RECT_U d2dRect = D2D1::RectU(0, 0, rect.Width, rect.Height);
        bitmap->CopyFromMemory(&d2dRect, buffer, rect.Width * 4);
        delete[] buffer;
    }
}

void QRImageElementRenderer::Render(CRect bounds)
{
    if (element->flags.self_visible)
    {
        CComPtr<ID2D1RenderTarget> d2dRenderTarget = renderTarget->GetDirect2DRenderTarget();
        d2dRenderTarget->DrawBitmap(
            bitmap,
            D2D1::RectF((FLOAT)bounds.left, (FLOAT)bounds.top, (FLOAT)bounds.right, (FLOAT)bounds.bottom),
            element->GetOpacity()
        );
    }
    GraphicsImageRenderer::Render(bounds);
}

#pragma endregion Image