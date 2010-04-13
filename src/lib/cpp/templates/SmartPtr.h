#pragma once

namespace TEMPL {

namespace HELPER {

    template <typename T>
    class TDelete
    {
    public:
        static void Destroy(T* pT)
        {
            if (pT) delete pT;
        }
    };

    template <typename T>
    class TRelease
    {
    public:
        static void Destroy(T* pT)
        {
            if (pT) pT->Release();
        }
    };

} // namespace HELPER

/**
 * AutoPtr implementation.
 *
 * @author Alexander Kozlov
 */
template < typename T, typename TDestructor = HELPER::TDelete<T> >
class AutoPtr
{
public:
    AutoPtr()
        : m_pT(NULL)
    {
    }

    explicit AutoPtr(T* pT)
        : m_pT(pT)
    {
    }

    ~AutoPtr(void)
    {
        Release();
    }

    void Release()
    {
        if (!m_pT)
            return;

        try
        {
            T* pT = m_pT;

            m_pT = 0;

            TDestructor::Destroy(pT);
        }
        catch (...)
        {
            ATLTRACE( _T("AutoPtr::Exception on Destroy object\n") );
            ATLASSERT(FALSE);
        }
    }

    void Attach(T* pT)
    {
        if (m_pT != pT)
        {
            Release();

            m_pT = pT;
        }
    }

    T* Detach()
    {
        T* pT = m_pT;

        m_pT = 0;

        return pT;
    }

    T& operator *  () const { ATLASSERT(m_pT); return *m_pT; }
    T* operator -> () const { ATLASSERT(m_pT); return  m_pT; }

    AutoPtr& operator = (T* pT)
    {
        Attach(pT);

        return *this;
    }

    bool operator ! () const
    {
        return !m_pT;
    }

    operator bool () const
    {
        return !!m_pT;
    }

public:
    T* m_pT;

private:
    AutoPtr(const AutoPtr &);
    AutoPtr& operator = (const AutoPtr &);
};


/**
 * AutoReleasePtr calls obj->Release() instead of delete.
 *
 * @author Alexander Kozlov
 */
template < typename T >
class AutoReleasePtr : public AutoPtr<T, HELPER::TRelease<T> >
{
public:
    AutoReleasePtr()
    {
    }

    explicit AutoReleasePtr(T* pT)
        : AutoPtr(pT)
    {
    }

    AutoReleasePtr& operator = (T* pT)
    {
        Attach(pT);

        return *this;
    }
};

/**
 * References counter implementation.
 *
 * @author Alexander Kozlov
 */
template <typename T, typename TDestructor>
struct TRefCounter
{
    T*            m_pT;
    LONG volatile m_nRef;

    explicit TRefCounter(T* pT)
        : m_pT(pT)
        , m_nRef(1)
    {
    }

    virtual ~TRefCounter()
    {
    }

    virtual LONG AddRef()
    {
        return ::InterlockedIncrement(&m_nRef);
    }

    virtual LONG Release()
    {
        LONG nRefCount = ::InterlockedDecrement(&m_nRef);
        if (nRefCount != 0)
            return nRefCount;

        ATLASSERT(m_pT);
        try
        {
            T* pT = m_pT;

            m_pT = 0;

            TDestructor::Destroy(pT);
        }
        catch (...)
        {
            ATLTRACE( _T("SmartPtr::TRefCounter::Exception on Destroy object\n") );
            ATLASSERT(FALSE);
        }

        try
        {
            delete this;
        }
        catch (...)
        {
            ATLTRACE( _T("SmartPtr::TRefCounter::Exception on delete itself\n") );
            ATLASSERT(FALSE);
        }

        return 0;
    }
};


/**
 * Week pointer keeps week reference to object.
 *
 * @author Alexander Kozlov
 */
template <typename SmartPtr>
class TWeekPtr
{
public:
    typedef typename SmartPtr::RefCounter  RefCounter;
    typedef typename SmartPtr::PointerType T;
    
private: friend typename SmartPtr;
    RefCounter* m_pRef;

public:
    TWeekPtr(RefCounter* pRef = 0)
        : m_pRef(pRef)
    {
    }

    TWeekPtr(const TWeekPtr &ptr)
        : m_pRef(ptr.m_pRef)
    {
    }

    TWeekPtr& operator = (const TWeekPtr &ptr)
    {
        m_pRef = ptr.m_pRef;
        return *this;
    }

    void AddRef()
    {
        if (m_pRef) m_pRef->AddRef();
    }

    void Release()
    {
        if (m_pRef && !m_pRef->Release())
            m_pRef = 0;
    }

    T* operator -> () const
    {
        ATLASSERT(m_pRef);
        return m_pRef->m_pT;
    }
};

/**
 * SmartPtr counts references to objects.
 *
 * @author Alexander Kozlov
 */
template < typename T, typename TDestructor = HELPER::TDelete<T> >
class SmartPtr
{
public:
    typedef TRefCounter<T, TDestructor> RefCounter;
    typedef TWeekPtr<SmartPtr>          WeekPtr;
    typedef T                           PointerType;

public:
    explicit SmartPtr(T* pT = 0)
        : m_pRef(0)
    {
        Acquire(pT);
    }

    SmartPtr(const SmartPtr &ptr)
        : m_pRef(0)
    {
        Acquire(ptr.m_pRef);
    }

    template <typename X, typename XDestructor>
    SmartPtr(const SmartPtr<X, XDestructor> &ptr)
        : m_pRef(0)
    {
        Acquire((RefCounter*)ptr.m_pRef);
    }

    SmartPtr(const WeekPtr &ptr)
        : m_pRef(0)
    {
        Acquire((RefCounter*)ptr.m_pRef);
    }

    ~SmartPtr(void)
    {
        Release();
    }

    T* Get() const
    {
        return m_pRef ? m_pRef->m_pT : 0;
    }

    LONG Release()
    {
        if (!m_pRef)
            return 0;

        LONG nRefCount = 0;
        try
        {
            RefCounter* pRef = m_pRef;

            m_pRef = 0;

            nRefCount = pRef->Release();
        }
        catch (...)
        {
            ATLTRACE( _T("SmartPtr::Exception on Release RefCounter\n") );
            ATLASSERT(FALSE);
        }
        return nRefCount;
    }

    SmartPtr& operator = (const SmartPtr &ptr)
    {
        if (*this == ptr)
            return *this;

        Release();
        Acquire(ptr.m_pRef);

        return *this;
    }

    T& operator *  () const { ATLASSERT(m_pRef); return *Get(); }
    T* operator -> () const { ATLASSERT(m_pRef); return Get();  }

    bool operator == (T* pObj) const { return Get() == pObj; }
    bool operator != (T* pObj) const { return Get() != pObj; }

    bool operator == (const SmartPtr& ptr) const { return Get() == ptr.Get(); }
    bool operator != (const SmartPtr& ptr) const { return Get() != ptr.Get(); }

    bool operator ! () const
    {
        return !m_pRef;
    }

    operator bool () const
    {
        return 0 != m_pRef;
    }

protected: template <class X, class XDestructor> friend class SmartPtr;
    RefCounter* m_pRef;

    void Acquire(T* pT)
    {
        ATLASSERT(0 == m_pRef);
        m_pRef = pT ? new RefCounter(pT) : 0;
    }

    void Acquire(RefCounter* pRef)
    {
        ATLASSERT(0 == m_pRef);

        if (m_pRef = pRef)
            m_pRef->AddRef();
    }

public:
    WeekPtr GetWeekPtr() const
    {
        return m_pRef;
    }
};

/**
 * SmartReleasePtr calls obj->Release() instead of delete.
 *
 * @author Alexander Kozlov
 */
template < typename T >
class SmartReleasePtr : public SmartPtr<T, HELPER::TRelease<T> >
{
public:
    explicit SmartReleasePtr(T* pT = 0)
        : SmartPtr<T, HELPER::TRelease<T> >(pT)
    {
    }

    SmartReleasePtr(const SmartReleasePtr &ptr)
        : SmartPtr<T, HELPER::TRelease<T> >(ptr)
    {
    }

    template <typename X>
    SmartReleasePtr(const SmartReleasePtr<X> &ptr)
        : SmartPtr<T, HELPER::TRelease<T> >(ptr)
    {
    }

    SmartReleasePtr(const WeekPtr &ptr)
        : SmartPtr<T, HELPER::TRelease<T> >(ptr)
    {
    }

    SmartReleasePtr& operator = (const SmartReleasePtr &ptr)
    {
        __super::operator = (ptr);

        return *this;
    }
};


/**
 * Increase/decrease reference count to object using week ptr.
 *
 * @author Alexander Kozlov
 */
template <typename T>
class WeekPtrLocker
{
private:
    T m_rT;

public:
    WeekPtrLocker(const T &rT)
        : m_rT(rT)
    {
        m_rT.AddRef();
    }

    ~WeekPtrLocker()
    {
        m_rT.Release();
    }
};

} // namespace TEMPL
