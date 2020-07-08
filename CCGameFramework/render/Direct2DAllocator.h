#ifndef RENDER_D2D_ALLOCATOR_H
#define RENDER_D2D_ALLOCATOR_H
#include <ui/gdi/Gdi.h>

class Direct2DRenderTarget;

template <class TKey, class TValue, bool GetPtr>
class CachedResourceAllocator
{
public:
    template<bool _GetPtr>
    struct CachedType
    {
    };

    template<>
    struct CachedType<true>
    {
        typedef std::shared_ptr<TValue> ReturnType;
    };

    template<>
    struct CachedType<false>
    {
        typedef TValue ReturnType;
    };
    typedef typename CachedType<GetPtr>::ReturnType ReturnType;

public:
    ReturnType Create(const TKey& key)
    {
        return CreateInternal(key);
    }

protected:
    virtual ReturnType CreateInternal(const TKey&) = 0;
};

template <class TKey, class TValue>
class CachedObjectAllocatorBase : public CachedResourceAllocator<TKey, TValue, true>
{
};

struct D2DTextFormatPackage : std::enable_shared_from_this<D2DTextFormatPackage>
{
    CComPtr<IDWriteTextFormat>		textFormat;
    DWRITE_TRIMMING					trimming;
    CComPtr<IDWriteInlineObject>	ellipseInlineObject;
};

class CachedTextFormatAllocator : public CachedObjectAllocatorBase<Font, D2DTextFormatPackage>
{
public:
    static CComPtr<IDWriteTextFormat> CreateDirect2DFont(const Font& font);
protected:
    std::shared_ptr<D2DTextFormatPackage> CreateInternal(const Font& font)override;
};

template <class TKey, class TValue>
class CachedCOMAllocatorBase : public CachedResourceAllocator<TKey, CComPtr<TValue>, false>
{
public:
    CachedCOMAllocatorBase(){}
    ~CachedCOMAllocatorBase(){}
    void SetRenderTarget(std::shared_ptr<Direct2DRenderTarget> _guiRenderTarget)
    {
        guiRenderTarget = _guiRenderTarget.get();
    }
protected:
    Direct2DRenderTarget* guiRenderTarget;
};

class CachedSolidBrushAllocator : public CachedCOMAllocatorBase<CColor, ID2D1SolidColorBrush>
{
protected:
    CComPtr<ID2D1SolidColorBrush> CreateInternal(const CColor& color);
};

class CachedLinearBrushAllocator : public CachedCOMAllocatorBase<std::pair<CColor, CColor>, ID2D1LinearGradientBrush>
{
protected:
    CComPtr<ID2D1LinearGradientBrush> CreateInternal(const std::pair<CColor, CColor>& colors);
};

#endif