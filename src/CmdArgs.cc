#include "CmdArgs.hh"

#include "adt/Logger.hh"
#include "adt/Arena.hh"

using namespace adt;

int
main(int argc, char** argv)
{
    Logger logger {stderr, ILogger::LEVEL::DEBUG, 2048, true};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    LogInfo{"ArgvParser test...\n"};

    {
        Arena arena {SIZE_1G};
        defer( arena.freeAll() );

        ArgvParser cmd {&arena, argc, argv, {
            {
                .bNeedsValue = false,
                .sOneDash = "h",
                .sTwoDashes = "help",
                .sDescription = "display help text",
                .pfn = +[](void* pAny, const StringView svKey, const StringView svVal) noexcept {
                    LogInfo{"Display some help text here (key: '{}', val: '{}')\n", svKey, svVal};
                },
                .pAnyData {}
            }
        }};
        cmd.parse();
    }

    LogInfo{"ArgvParser test passed\n"};
}
