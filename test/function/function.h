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

template<typename R, typename Receive, typename... Args>
struct FunctorTraits{
    typedef R(Receive::* Functor)(Args...);
    using RunType = std::function<bool(Args...)>;
    FunctorTraits(Functor functor, RunType runner) : functor_(functor), runner_(runner) {}

    Functor functor_;
    RunType runner_;

};

template<typename R, typename Receive, typename P1>
struct FunctorTrait{
    typedef R(Receive::* Functor)(P1);
    using RunType = std::function<bool(P1)>;
    FunctorTrait(Functor functor, RunType runner) : functor_(functor), runner_(runner) {}

    Functor functor_;
    RunType runner_;

};

//template<typename... Args>
//class RunnableAdapter;

//template<typename R, typename... Args>
//class RunnableAdapter<R(*)(Args...)> {
//public:
//    typedef R(*Runnable)(Args...);
//    using RunType = std::function<bool(Args...)>;
//    static constexpr bool is_method = false;
//    static constexpr bool is_function = true;
//    RunnableAdapter(Runnable function) : function_(function) {}
//    void* get() const { return function_cast(function_); }
//
//private:
//    Runnable function_ = nullptr const;
//};

template<typename R, typename T, typename... Args>
class RunnableAdapter/*<R(T::*)(Args...)>*/ {
public:
    typedef R(T::*Runnable)(Args...);
    using RunType = std::function<bool(Args...)>;
    static constexpr bool is_method = true;
    static constexpr bool is_function = false;
    RunnableAdapter(Runnable method) : method_(method) {}
    void* get() const { return function_cast(method_); }

private:
    Runnable method_ = nullptr const;
};


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


    //template<typename R, typename... Args>
    //class Functor {
    //public:
    //    explicit Functor(R(*function)(Args&&...)) : function_(function) {}
    //    void* get() { return function_cast<void*>(function); }
    //private:
    //    R(*function_)(Args...);
    //};

 

   /* template<typename T, typename Function>
    bool Connect(T , Function call) {
        return true;
    }*/


    //template<typename R, typename T, typename... Args>
    //bool Connect(FunctorTraits<R, T, Args...> callback) {
    //    return true;
    //}

    template<typename Function, typename R, typename Receive, typename... Args>
    bool Connect(R( Receive::*method)(Args...) , Function function) {
        using RunType = RunnableAdapter<R, Receive, Args...>::RunType;
        using Runnable = RunnableAdapter<R, Receive, Args...>::Runnable;
        std::shared_ptr<void> value((reinterpret_cast<void*>(new RunType(function))));

        void* p = nullptr;
        auto key = function_cast<void*>(method);
        slots_[key].insert((std::make_pair(p, value)));
        auto& list = slots_[key];
        for (auto it = list.begin(); it != list.end(); ) {
            auto& fc = it->second;
            if (!fc) continue;
            auto fctt = reinterpret_cast<RunType*>(fc.get());
            if (!fctt || !(*fctt)) continue;
            if ((*fctt)(333)) list.erase(it++);
            else ++it;
        }

        //auto key = method.get();
        return true;
    }

    


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

    std::map<void* /*key*/, std::map<void* /*obj*/, std::shared_ptr<void> /*callback*/>> slots_;
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


    //dispatcher.Connect(FunctorTraits<void, Base, int>(&Base::f3, [](int a) {
    //    return true;
    //    }));


    dispatcher.Connect(&Base::f3, [](int t) {
        t += 199;

        std::cout << t;
        return true;;
        });

    auto ddd = [](int) {
        return;
    };

    auto bbb = [](std::wstring) {
        return;
    };

    //dispatcher.Connect(&Base::f3, bbb);



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

// 观察者模式：
// 1. google base老版本的实现方式
// 2. boost.signal基础上实现
// 3. google base新版本的实现方式
// 4. 自我实现一个：结合google base新老版本和boost.signal的基础上实现，可以利用https://isocpp.org/blog/2015/04/making-boost.signals2-more-oop-friendly-pavel-frolov的方式
//  主要考虑到可以在被通知时能够自删除（原有的想法是利用weakPtrs实现，指针有效时进行调用，然后在执行返回后删除）
//  考虑到接口多个函数和继承的方式需要实现所有的接口函数，不确定有没有更好的办法利用宏来定义所有的接口，从而实现接口的定义和调用自动化
//  LambdaOBS接口实现了单个接口的归一化绑定和调用，但是针对SDK的接口需要利用观察者模式重新实现来处理
// 所以，观察者模式在c++中时无法离开的；具体的实现待详细设计以及调优和兼容性处理

template<typename Callback, typename MethodName, typename MethodSignature>
struct ObserverMethod {

};


