// system
#include <windows.h>

// c++
#include <cstdio>
#include <vector>
#include <string_view>
#include <atomic>

// local
#include "com_ptr.hpp"
#include "foo.h"

using namespace std;
using namespace rap;

static FILE* g_logger = nullptr;

class FooImpl : public IFoo
{
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
        FTRACE(g_logger);
        if (ppvObject == nullptr)
        {
            return E_INVALIDARG;
        }
        *ppvObject = nullptr;

        if (riid == IID_IUnknown)
        {
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else if(riid == IID_IFoo)
        {
            *ppvObject = static_cast<IFoo*>(this);
        }
        else
        {
            return E_INVALIDARG;
        }
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        FTRACE(g_logger);
        return ++_refcount;
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        FTRACE(g_logger);
        auto retval = --_refcount;
        if (retval == 0)
        {
            delete this;
        }
        return retval;
    }

    HRESULT STDMETHODCALLTYPE DoBar()
    {
        FTRACE(g_logger);
        return E_NOTIMPL;
    }
private:
    atomic_size_t _refcount = 0;
};

class FooFactoryImpl : public IFooFactory
{
public:
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
    {
        FTRACE(g_logger);
        if (ppvObject == nullptr)
        {
            return E_INVALIDARG;
        }
        *ppvObject = nullptr;

        if (riid == IID_IUnknown)
        {
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else if(riid == IID_IFooFactory)
        {
            *ppvObject = static_cast<IFooFactory*>(this);
        }
        else
        {
            return E_INVALIDARG;
        }
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        FTRACE(g_logger);
        return ++_refcount;
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        FTRACE(g_logger);
        auto retval = --_refcount;
        if (retval == 0)
        {
            delete this;
        }
        return retval;
    }

    HRESULT STDMETHODCALLTYPE GetFoo(IFoo** ppFoo) override
    {
        FTRACE(g_logger);
        return E_NOTIMPL;
    }

private:
    atomic_size_t _refcount = 0;
};

int main(int argc, char** argv)
{
    g_logger = fopen("E:\\mingw_com\\log.txt", "wb");
    FTRACE_MSG(g_logger, "init");

    vector<string_view> args;
    args.insert(args.begin(), argv, argv + argc);

    bool embedding = false;
    for(auto arg : args)
    {
        if (arg.find("Embedding") == string_view::npos) continue;
        embedding = true; break;
    }

    if (embedding)
    {
        com_ptr<ITypeLib> typelib;
        THROW_IF_FAILED(LoadTypeLibEx(L"fooproxy.dll", REGKIND_REGISTER, &typelib));
        fprintf(g_logger, "registered typelib\n");

        FooFactoryImpl fooFactory;

        DWORD regId = 0;
        THROW_IF_FAILED(CoRegisterClassObject(
                            CLSID_FooFactoryImpl,
                            (IFooFactory*)&fooFactory,
                            CLSCTX_LOCAL_SERVER,
                            REGCLS_MULTIPLEUSE,
                            &regId));

        MSG ms;
        while(GetMessage(&ms, 0, 0, 0))
        {
            TranslateMessage(&ms);
            DispatchMessage(&ms);
        }

        CoRevokeClassObject(regId);
        CoUninitialize();
    }
    else
    {
        FTRACE_MSG(g_logger, "not embedding");
    }
}