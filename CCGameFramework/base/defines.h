#ifndef DEFINES_H
#define DEFINES_H

#include <WTF/PassRefPtr.h>

using cint8 = signed __int8;
using uint8 = unsigned __int8;
using cint16 = signed __int16;
using uint16 = unsigned __int16;
using cint32 = signed __int32;
using uint32 = unsigned __int32;
using cint64 = signed __int64;
using uint64 = unsigned __int64;

#ifdef WIN32
using cint = cint32;
using uint = uint32;
#else
using cint = cint64;
using uint = uint64;
#endif

using byte = uint8;
using size_t = uint;

template <class T>
class RawPtr
{
public:
    RawPtr() : m_ptr(nullptr) {}
    RawPtr(T* ptr) : m_ptr(ptr) {}
    RawPtr(const PassRefPtr<T>& ptr) { m_ptr = ptr.get(); }
    ~RawPtr() { m_ptr = nullptr; }

    RawPtr& operator=(T* ptr)
    {
        m_ptr = ptr;
        return *this;
    }
    RawPtr& operator=(const PassRefPtr<T>& ptr)
    {
        m_ptr = ptr.get();
        return *this;
    }
    T* operator->() const
    {
        ASSERT(m_ptr);
        return m_ptr;
    }
    T* get() { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    T* operator~() const { return m_ptr; }
    bool operator!() const { return !m_ptr; }
    bool operator!=(const PassRefPtr<T>& ptr) const { return m_ptr != ptr.get(); }
    bool operator==(const PassRefPtr<T>& ptr) const { return m_ptr == ptr.get(); }
    bool operator!=(const RawPtr& ptr) const { return m_ptr != ptr.m_ptr; }
    bool operator==(const RawPtr& ptr) const { return m_ptr == ptr.m_ptr; }
    bool operator<(const RawPtr& ptr) const { return m_ptr < ptr.m_ptr; }
    bool operator>(const RawPtr& ptr) const { return m_ptr > ptr.m_ptr; }
    operator T*() { return m_ptr; }
    template<class X> operator PassRefPtr<X>() { return dynamic_cast<X*>(m_ptr); }
    template<class X> operator X*() { return dynamic_cast<X*>(m_ptr); }

private:
    T* m_ptr;
};

#endif