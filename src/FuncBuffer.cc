#include "adt/FuncBuffer.hh"
#include "adt/logs.hh"
#include "adt/Logger.hh"

using namespace adt;

int
main()
{
    {
        int i0 = 666;
        int i1 = 999;
        int i2 = 133345;

        FuncBuffer<void, 12> fn0 {[=] { LOG_GOOD("HELLO: {}, {}, {}\n", i0, i1, i2); }};
        static_assert(sizeof(fn0) == 24);

        fn0();

        FuncBuffer<void> fn1 {
            [](void* p) {
                ADT_ASSERT_ALWAYS(*static_cast<int*>(p) == 666, "{}", *static_cast<int*>(p));
            },
            i0
        };
        static_assert(sizeof(fn1) == 16);

        fn1();

        FuncBuffer<void> fn2 {
            [] { LOG_WARN("NULL\n"); }
        };
        static_assert(sizeof(fn2) == 16);

        fn2();
    }

    {
        int i0 = 999;

        FuncBuffer<int> fn0 {[&] { return i0 = 666; }};
        (void)fn0();

        ADT_ASSERT_ALWAYS(i0 == 666, "{}", i0);
    }
}
