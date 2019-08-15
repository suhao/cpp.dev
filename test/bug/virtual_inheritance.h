#pragma once

namespace virtual_inheritance{

struct Base {
    Base(int i = 0) : i(i) {}
    virtual ~Base() {}
    virtual Base *clone() const = 0;

protected:
    int i;
};

struct A : virtual public Base {
    A() {}
    virtual A *clone() const = 0;
};

struct B : public A {
    B() {}
    B *clone() const { return new B(*this); }

    /// MSVC debugger shows that 'b' is for some reason missing the Base
    /// portion of it's object ("Error: expression cannot be evaluated")
    /// and trying to access 'b.i' causes an unhandled exception.
    ///
    /// Note: This only seems to occur with MSVC
    B(const B &b) : Base(b.i), A() {}
};

void foo(const A &elem) {
    A *a = elem.clone();
    if (a) delete a;
}

}

void test_virtual_inheritance() {
    using namespace virtual_inheritance;
    A* a = new B;
    foo(*a);
    delete a;
}