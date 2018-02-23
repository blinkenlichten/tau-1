#ifndef SCOPED_FN_H
#define SCOPED_FN_H

#ifdef __cplusplus
#include <utility>

struct VoidOp { void operator()(){ }};

template<class UnlockFn, class LockFn = VoidOp >
class ScopedInvoke
{
    ScopedInvoke();
    ScopedInvoke(const ScopedInvoke&);
    ScopedInvoke& operator = (const ScopedInvoke&);
public:
    UnlockFn m_unlock_fn;
    LockFn m_lock_fn;
    bool m_released = false;

    ScopedInvoke(ScopedInvoke&& rhs) = default;

    ScopedInvoke(UnlockFn&& fnUnlock) : m_unlock_fn(fnUnlock)
    {  }

    ScopedInvoke(UnlockFn&& fnUnlock, LockFn&& fnLock) : m_unlock_fn(std::move(fnUnlock)), m_lock_fn(std::move(fnLock))
    { m_lock_fn.operator()(); }

    UnlockFn&& Release()
    {
        m_released = true;
        return std::move(m_unlock_fn);
    }
    virtual ~ScopedInvoke()
    { if (!m_released) m_unlock_fn.operator()(); }
};

#define ON_SCOPE_EXIT(NAME, ON_EXIT_STATEMENT) \
  auto fn_scoped_##NAME = [&](){ ON_EXIT_STATEMENT; }; \
  ScopedInvoke<decltype(fn_scoped_##NAME )>(std::move(fn_scoped_##NAME));

/*using namespace std;

int main()
{

    auto fnHi = [](){cout << "Hello World!" << endl;};
    auto fnLow = []() { cout << "Bye, Bye!" << endl;};
    {
        // executes only "Bye"
        ScopedInvoke<decltype(fnLow)> guard(std::move(fnLow));
    }
    cout << "----------\n";
    // executes "Hello" and "Bye"
    ScopedInvoke<decltype(fnLow), decltype(fnHi)> guard(std::move(fnLow), std::move(fnHi));
    return 0;
}*/

template<typename PtrType, class FnDisposer>
class TDisposableUniquePointer
{
public:
    typedef TDisposableUniquePointer<PtrType, FnDisposer> __TBase;

    PtrType m_ptr = nullptr;
    FnDisposer m_func;
    TDisposableUniquePointer() = default;
    TDisposableUniquePointer(PtrType object) : m_ptr(object)  {   }

    TDisposableUniquePointer(PtrType object, FnDisposer&& disp)
        : m_ptr(object), m_func(std::move(disp))
    {   }
    PtrType get() const { return m_ptr; }
    bool empty() const { return nullptr == m_ptr;}
    void reset(PtrType object)
    {
        if (m_ptr)
            m_func(release());
        m_ptr = object;
    }
    PtrType release() { PtrType val = nullptr; std::swap(m_ptr, val); return val;}

    virtual ~TDisposableUniquePointer()
    { if (m_ptr) m_func(release()); }
};

#endif //__cplusplus

#endif // SCOPED_FN_H
