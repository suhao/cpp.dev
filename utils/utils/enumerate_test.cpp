#include <Windows.h>
#include "utils/enumerate.h"
#include "utils/scoped_object.h"

#ifdef TEST

DECLARE_ENUM_WITH_TYPE(TestEnumClass, int32_t, ZERO = 0x00, TWO = 0x02, ONE = 0x01, THREE = 0x03, FOUR);

int ENUM_TEST(void) {
    TestEnumClass first, second;
    first = TestEnumClass::FOUR;
    second = TestEnumClass::TWO;

    std::cout << first << "(" << static_cast<uint32_t>(first) << ")" << std::endl; // FOUR(4)

    std::string strOne;
    strOne = *first;
    std::cout << strOne << std::endl; // FOUR

    std::string strTwo;
    strTwo = ("Enum-" + second) + (TestEnumClass::THREE + "-test");
    std::cout << strTwo << std::endl; // Enum-TWOTHREE-test

    std::string strThree("TestEnumClass: ");
    strThree += second;
    std::cout << strThree << std::endl; // TestEnumClass: TWO
    std::cout << "Enum count=" << *first << std::endl;

    bool has_key = ContainsKey<TestEnumClass>(100);

    return 0;
}

#endif // _DEBUG