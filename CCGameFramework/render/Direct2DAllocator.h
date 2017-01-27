#ifndef RENDER_D2D_ALLOCATOR_H
#define RENDER_D2D_ALLOCATOR_H
#include <WTF/RefPtr.h>
#include <WTF/refcounted.h>
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
        typedef RefPtr<TValue> ValueType;
        typedef PassRefPtr<TValue> ReturnType;
    };

    template<>
    struct CachedType<false>
    {
        typedef TValue ValueType;
        typedef TValue ReturnType;
    };

    typedef typename CachedType<GetPtr>::ValueType ValueType;
    typedef typename CachedType<GetPtr>::ReturnType ReturnType;

    struct Package
    {
        ValueType resource;
        cint reference;
        bool operator==(const Package& package) const { return false; }
        bool operator!=(const Package& package) const { return true; }
    };

    std::map<TKey, Package> aliveResources;

public:
    ReturnType Create(const TKey& key)
    {
        auto alive = aliveResources.find(key);
        if (alive != aliveResources.end())
        {
            ++(alive->second.reference);
        }
        ValueType resource = CreateInternal(key);
        Package package;
        package.resource = resource;
        package.reference = 1;
        aliveResources.insert(std::make_pair(key, package));
        return package.resource;
    }

    void Destroy(const TKey& key)
    {
        auto alive = aliveResources.find(key);
        if (alive != aliveResources.end())
        {
            auto package = alive->second;
            --(alive->second.reference);
            if (alive->second.reference == 0)
            {
                aliveResources.erase(alive);
            }
            else
            {
                aliveResources.insert(std::make_pair(key, package));
            }
        }
    }

protected:
    virtual ReturnType CreateInternal(const TKey&) = 0;
};

template <class TKey, class TValue>
class CachedObjectAllocatorBase : public CachedResourceAllocator<TKey, TValue, true>
{
};

struct D2DTextFormatPackage : RefCounted<D2DTextFormatPackage>
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
    PassRefPtr<D2DTextFormatPackage> CreateInternal(const Font& font)override;
};

template <class TKey, class TValue>
class CachedCOMAllocatorBase : public CachedResourceAllocator<TKey, CComPtr<TValue>, false>
{
public:
    CachedCOMAllocatorBase(){}
    ~CachedCOMAllocatorBase(){}
    void SetRenderTarget(PassRefPtr<Direct2DRenderTarget> _guiRenderTarget)
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