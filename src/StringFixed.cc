#include "adt/Logger.hh"

using namespace adt;

int
main()
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    {
        const char* nts = "1234567890ABCD";
        StringFixed<10> sf10 {};
        sf10 = nts;

        ADT_ASSERT_ALWAYS(sf10.data()[9] == '\0', "");
        ADT_ASSERT_ALWAYS(sf10 == "123456789", "");
        LogDebug("sf: '{}'\n", sf10.data());
    }

    {
        StringFixed<5> sf5 = "12345_ABCF";
        StringFixed<5> sf5_2 = sf5;

        ADT_ASSERT_ALWAYS(sf5.data()[4] == '\0', "");
        ADT_ASSERT_ALWAYS(sf5 == sf5_2, "");
        ADT_ASSERT_ALWAYS(sf5 == "1234", "");
        LogDebug("sf5: '{}'\n", sf5);
    }
}
