#include "adt/ArgvParser.hh"

#include "adt/Arena.hh"
#include "adt/StdAllocator.hh"
#include "adt/logs.hh"

using namespace adt;

int
main(int argc, char** argv)
{
    LOG_NOTIFY("ArgvParser test...\n");

    {
        // Arena arena {SIZE_1G};
        // defer( arena.freeAll() );

        ArgvParser cmd {StdAllocator::inst(), stderr, "(more usage...)", argc, argv, {
            {
                .bNeedsValue = false,
                .sOneDash = "h",
                .sTwoDashes = "help",
                .sUsage = "display help text",
                .pfn = [](ArgvParser*, void*, const StringView svKey, const StringView svVal) {
                    print::out("Showing some help text here (key: '{}', val: '{}')\n", svKey, svVal);
                    return ArgvParser::RESULT::SHOW_ALL_USAGE;
                },
                .pAnyData {}
            },
            {
                .bNeedsValue = true,
                .sOneDash = "c",
                .sTwoDashes = "config",
                .sUsage = "config file and stuff",
                .pfn = [](ArgvParser*, void*, const StringView svKey, const StringView svVal) {
                    print::out("setting fake config file (key: '{}', val: '{}')\n", svKey, svVal);
                    return ArgvParser::RESULT::GOOD;
                },
                .pAnyData {}
            },
        }};
        defer( cmd.destroy() );

        cmd.parse();
        // cmd.printUsage(StdAllocator::inst());
    }

    LOG_GOOD("ArgvParser test passed\n");
}
