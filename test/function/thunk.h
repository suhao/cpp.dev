#pragma once

#include <stdio.h>
#include <functional>

#include <atlstdthunk.h>

class ThunkTest {
public:
    CStdCallThunk thunk_;

    static void Test1(void* obj, int p1) {
        auto p = reinterpret_cast<ThunkTest*>(obj);
        p->Print();
    }

    void Print() {
        printf("Test ±»ÐÞ¸ÄÁË");
    }


    template<typename P>
    void Init() {
        thunk_.Init((DWORD_PTR)(ThunkTest::Test1), this);
    }

    void* GetAddress() { return thunk_.GetCodeAddress(); }


};

template< typename TDst, typename TSrc >
TDst  UnionCastType(TSrc src) {
    union {
        TDst  uDst;
        TSrc  uSrc;
    } uMedia;
    uMedia.uSrc = src;
    return uMedia.uDst;
}

#pragma pack( push, 1 )
struct  MemFunToStdCallThunk {
    BYTE          m_mov;
    DWORD      m_this;   //mov ecx pThis
    BYTE          m_jmp;
    DWORD      m_relproc;  //jmp

    BOOL  Init(DWORD_PTR proc, void* pThis) {
        m_mov = 0xB9;   // mov ecx
        m_this = PtrToUlong(pThis);
        m_jmp = 0xe9;          //jmp

        m_relproc = DWORD((INT_PTR)proc - ((INT_PTR)this + sizeof(MemFunToStdCallThunk)));
        ::FlushInstructionCache(::GetCurrentProcess(), this, sizeof(MemFunToStdCallThunk));
        return TRUE;
    }

    void* GetCodeAddress() {
        return this;
    }
};
#pragma  pack( pop )


template <typename T>
struct Callback;

template <typename Ret, typename... Params>
struct Callback<Ret(Params...)> {
    template <typename... Args>
    static Ret callback(Args... args) {
        func(args...);
    }
    static std::function<Ret(Params...)> func;
};
// Initialize the static member.
template <typename Ret, typename... Params>
std::function<Ret(Params...)> Callback<Ret(Params...)>::func;

void register_with_library(int(*func)(int *k, int *e)) {
    int x = 0, y = 1;
    int o = func(&x, &y);
    printf("Value: %i\n", o);
}

class A {
public:
    A() {
        // Callback<int(int*, int*)>::func = std::bind(&A::e, this, std::placeholders::_1, std::placeholders::_2);
         //callback_t func = static_cast<callback_t>(Callback<int(int*, int*)>::callback);
         //register_with_library(func);
    }
    ~A() {

    }
    int e(int *k, int *j) {
        return *k - *j;
    }
};

typedef int(*callback_t)(int*, int*);


void thunk_test() {
    // 1: ThunkTest
    ThunkTest test;
    test.Init<int>();
    typedef void(*TestFunc)(int);
    TestFunc func = (TestFunc)(test.GetAddress());
    func(1000);
    int a = 0;

    // 2: MemFunToStdCallThunk


    // 3: std::bind() std::function target();

    // 4: template<T* pointer, Params params> classs { static void Test(); }
    // also could instend of function.

    // 5: class { operator(); }


    // 6: with static
    struct Foo {
        void print(int* x) { // Some member function.
            std::cout << *x << std::endl;
        }
    };
    Foo foo; // Create instance of Foo.
    // Store member function and the instance using std::bind.
    Callback<void(int*)>::func = std::bind(&Foo::print, foo, std::placeholders::_1);
    // Convert callback-function to c-pointer.
    //void(*c_func)(int*) = static_cast<decltype(c_func)>(Callback<void(int*)>::callback);

    // Use in any way you wish.
    //std::unique_ptr<int> iptr{ new int(5) };
    //c_func(iptr.get());

    // 7:
//     typedef void(*voidCCallback)();
//     template<typename T>
//     voidCCallback makeCCallback(void (T::*method)(), T* r) {
//         Callback<void()>::func = std::bind(method, r);
//         void(*c_function_pointer)() = static_cast<decltype(c_function_pointer)>(Callback<void()>::callback);
//         return c_function_pointer;
//     }

    // 8: std::mem_fn


    // 9:
// #include <type_traits>
// 
//     template<typename T>
//     struct ActualType {
//         typedef T type;
//     };
//     template<typename T>
//     struct ActualType<T*> {
//         typedef typename ActualType<T>::type type;
//     };
// 
//     template<typename T, unsigned int n, typename CallerType>
//     struct Callback;
// 
//     template<typename Ret, typename ... Params, unsigned int n, typename CallerType>
//     struct Callback<Ret(Params...), n, CallerType> {
//         typedef Ret(*ret_cb)(Params...);
//         template<typename ... Args>
//         static Ret callback(Args ... args) {
//             func(args...);
//         }
// 
//         static ret_cb getCallback(std::function<Ret(Params...)> fn) {
//             func = fn;
//             return static_cast<ret_cb>(Callback<Ret(Params...), n, CallerType>::callback);
//         }
// 
//         static std::function<Ret(Params...)> func;
// 
//     };
// 
//     template<typename Ret, typename ... Params, unsigned int n, typename CallerType>
//     std::function<Ret(Params...)> Callback<Ret(Params...), n, CallerType>::func;
// 
// #define GETCB(ptrtype,callertype) Callback<ActualType<ptrtype>::type,__COUNTER__,callertype>::getCallback

    // using
//     typedef void (cb_type)(uint8_t, uint8_t);
//     class testfunc {
//     public:
//         void test(int x) {
//             std::cout << "in testfunc.test " << x << std::endl;
//         }
// 
//         void test1(int x) {
//             std::cout << "in testfunc.test1 " << x << std::endl;
//         }
// 
//     };
// 
//     cb_type* f = GETCB(cb_type, testfunc)(std::bind(&testfunc::test, tf, std::placeholders::_2));
// 
//     cb_type* f1 = GETCB(cb_type, testfunc)(
//         std::bind(&testfunc::test1, tf, std::placeholders::_2));
// 
// 
//     f(5, 4);
//     f1(5, 7);


}