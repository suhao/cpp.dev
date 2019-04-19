#pragma once

#include <iostream>
#include <vector>
#include <functional>
#include <map>


int DoSomeThing() {
    return 0;
}


class Base {
public:
    virtual void f1() {
        std::wcout << L"base::f1" << std::endl;
    };
    virtual void f2() = 0;
    virtual void f3(int a) = 0;

    void f4() {
        std::wcout << L"base::f4" << std::endl;
    }
};

class Driver : public Base {
public:
    Driver() {}
    virtual void f1() {
        std::wcout << L"Driver::f1" << std::endl;
        Base::f1();
    };
    virtual void f2() {
        std::wcout << L"Driver::f2" << std::endl;
    }
    virtual void f3(int a) {
        std::wcout << L"Driver::f3=" << a << std::endl;
    };

    void f4() {
        std::wcout << L"Driver::f4" << std::endl;
    }
};

template<typename T, typename P>
T function_cast(P p) {

    auto ppt =  *reinterpret_cast<T*>(static_cast<void*>(&p));
    return ppt;
    //auto aaaa = dynamic_cast<T*>(ppt);
    //if (aaaa != nullptr) return ppt;
    //return nullptr;
}

// p must &class::func
template<typename T, typename P>
T pointer_cast(P p) {
    return *static_cast<T*>(static_cast<void*>(&p));
}

// Must keep p and t size equal.
template<typename T, typename P>
T union_cast(P p) {
    union { P p; T t; } var;
    var.p = p;
    return var.t;
}

// win32, win64 should modify the eax.
// template<typename T, typename P>
// __declspec(naked) void* __cdecl cdecl_cast(...) {
//     __asm {
//         mov eax, dword ptr[esp+4]
//         ret
//     }
// }

// must after vc6.0
#define asm_cast(var, addr) \
{                           \
__asm {                     \
    mov var, offset addr    \
    }                       \
}



template<class Observer>
class Dispatcher
{
public:
    explicit Dispatcher() {}
    virtual ~Dispatcher() {}

    template<typename P1>
    class CallbackBase {
    public:
        CallbackBase(std::function<void(P1)> callback) : callback_(callback) {

        }

        void Run(P1 p1) {
            callback_(p1);
        }

        std::function<void(P1)> callback_;
    };

    class CallbackBase1 {
    public:
        CallbackBase1(std::function<void()> callback) : callback_(callback) {

        }

        void Run() {
            callback_();
        }

        std::function<void()> callback_;
    };


    template<typename T>
    void AddObserver(T method, std::function<void()> callback, bool repeating) {
        void* func = reinterpret_cast<void*>(new CallbackBase1(callback));
        dispatcher_.insert(std::make_pair(function_cast<void*>(method), func));
    }

    template<typename T, typename P1>
    void AddObserver(T method, std::function<void(P1)> callback, bool repeating) {
        void* func = reinterpret_cast<void*>(new CallbackBase<P1>(callback));
        dispatcher_.insert(std::make_pair(function_cast<void*>(method), func));
    }

    template<typename T>
    void Run(T method) {
        auto range = dispatcher_.equal_range(function_cast<void*>(method));
        while (range.first != range.second) {
            auto child = range.first++->second;
            CallbackBase1* func = reinterpret_cast<CallbackBase1*>((child));
            func->Run();
        }
    }

    template<typename T, typename P1>
    void Run(T method, P1 p1) {
        auto range = dispatcher_.equal_range(function_cast<void*>(method));
        while (range.first != range.second) {
            auto child = range.first++->second;
            CallbackBase<P1>* func = reinterpret_cast<CallbackBase<P1>*>((child));
            func->Run(p1);
        }
    }

private:

    std::multimap<void*, void*> dispatcher_;
};




void function_test() {
    auto f1 = &Base::f1;
    auto f2 = &Base::f2;
    auto f3 = &Base::f3;

    auto f11 = static_cast<void*>(&f1);
    auto f22 = static_cast<void*>(&f2);
    auto f33 = static_cast<void*>(&f3);


    auto ff1 = function_cast<void*>(&Base::f1);
    auto ff2 = function_cast<void*>(&Base::f2);
    auto ff3 = function_cast<void*>(&Base::f3);

    auto ffd1 = function_cast<void*>(&Driver::f1);
    auto ffd2 = function_cast<void*>(&Driver::f2);
    auto ffd3 = function_cast<void*>(&Driver::f3);


    typedef void(Base::*BaseFuncPTR)();
    typedef void(Base::*DriverFuncPTR)();
    auto fptrb1 = function_cast<BaseFuncPTR>(ff1);
    auto fptrd1 = function_cast<DriverFuncPTR>(ffd1);

    auto fptrb2 = function_cast<BaseFuncPTR>(ff2);
    auto fptrd2 = function_cast<DriverFuncPTR>(ffd2);

    auto fptrb3 = function_cast<BaseFuncPTR>(ff3);
    auto fptrd3 = function_cast<DriverFuncPTR>(ffd3);


    auto f4b = &Base::f4;
    auto f4d = &Driver::f4;
    auto f4fb = function_cast<void*>(&Base::f4);
    auto f4fd = function_cast<void*>(&Driver::f4);

    auto f4bob = function_cast<BaseFuncPTR>(f4fb);
    auto f4bod = function_cast<DriverFuncPTR>(f4fb);

    auto f4dob = function_cast<BaseFuncPTR>(f4fb);
    auto f4dod = function_cast<DriverFuncPTR>(f4fd);

    std::vector<void*>  v;
    v.push_back(function_cast<void*>(&Base::f1));
    v.push_back(function_cast<void*>(&Base::f2));
    v.push_back(function_cast<void*>(&Base::f3));

    Driver d, c;


    Dispatcher<Base> dispatcher;
    dispatcher.AddObserver(&Base::f1, std::bind(&Driver::f1, &d), true);
    dispatcher.AddObserver(&Base::f1, std::bind(&Base::f1, &d), true);

    dispatcher.AddObserver(&Base::f1, std::bind(&Driver::f1, &c), true);
    dispatcher.AddObserver(&Base::f1, std::bind(&Base::f1, &c), true);


    dispatcher.AddObserver(&Base::f2, std::bind(&Driver::f2, &d), true);
    dispatcher.AddObserver(&Base::f2, std::bind(&Base::f2, &d), true);
    std::function<void(int)> fc;
    fc = [&](int a) {
        d.f3(a);
    };
    /* = std::bind(&Driver::f3, &d)*/;
    dispatcher.AddObserver(&Base::f3, fc, true);

    dispatcher.Run(&Base::f1);
    dispatcher.Run(&Base::f3, 10);

    auto f = std::bind(&Driver::f1, &d);
    f();


    typedef void(*FuncPTR)();
    auto pt = &DoSomeThing;

    auto ss = function_cast<void*>(&DoSomeThing);
    auto dd = reinterpret_cast<void*>(&DoSomeThing);
    auto ffff = static_cast<void*>(&DoSomeThing);

    auto it = reinterpret_cast<FuncPTR>(&DoSomeThing);
    auto itss = function_cast<FuncPTR>(ss);
    auto itdd = reinterpret_cast<FuncPTR>(dd);
    auto itff = static_cast<FuncPTR>(ffff);


    //auto p1 = reinterpret_cast<FuncPTR>(&Driver::f1);
    auto p2 = &Driver::f1;
    auto p3 = function_cast<void*>(&Driver::f1);
    // auto p4 = reinterpret_cast<void*>(&Driver::f1);
    // auto p5 = static_cast<void*>(&Driver::f1);
}


