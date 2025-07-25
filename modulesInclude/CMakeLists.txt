add_library(adtlib)
set_target_properties(adtlib PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF) # Or ON if you need GNU extensions
target_sources(adtlib PUBLIC FILE_SET CXX_MODULES FILES
    adt.cppm
    Vec.cppm
    sort.cppm
    utils.cppm
    String.cppm
    print.cppm
    StdAllocator.cppm
    IException.cppm
    assert.cppm
    IAllocator.cppm
    Thread.cppm
)
