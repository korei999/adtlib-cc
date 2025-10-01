#include "adt/Logger.hh"
#include "adt/ThreadPool.hh"

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

using namespace adt;

static void
test()
{
}

int
main()
{
    ThreadPool ztp {SIZE_1M * 64};
    IThreadPool::setGlobal(&ztp);
    defer(
        ztp.destroy();
        IThreadPool::setGlobal(nullptr);
    );

    Logger logger {2, ILogger::LEVEL::DEBUG, SIZE_1K*4};
    ILogger::setGlobal(&logger);
    defer( logger.destroy() );

    test();
}
