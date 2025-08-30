add_executable(
    adt-alloc
    allocTest.cc
)

add_executable(
    adt-print
    printTest.cc
)

add_executable(
    adt-vec
    VecTest.cc
)

add_executable(
    adt-heap
    HeapTest.cc
)

add_executable(
    adt-string
    StringTest.cc
)

add_executable(
    adt-pool
    PoolTest.cc
)

add_executable(
    adt-map
    MapTest.cc
)

add_executable(
    adt-threadpool
    ThreadPoolTest.cc
)

add_executable(
    adt-queue
    QueueTest.cc
)

add_executable(
    adt-list
    ListTest.cc
)

add_executable(
    adt-rbtree
    RBTreeTest.cc
)

add_executable(
    adt-arr
    ArrTest.cc
)

add_executable(
    adt-json
    JSONTest.cc
    json/Parser.cc
    json/Lexer.cc
)

add_executable(
    adt-assert
    AssertTest.cc
)

add_executable(
    adt-threads
    threadsTest.cc
)

add_executable(
    adt-soa
    SOATest.cc
)

add_executable(
    adt-soa2
    SOATest2.cc
)

add_executable(
    QueueSPSC
    QueueSPSCTest.cc
)

add_executable(
    BinarySearch
    BinarySearch.cc
)

add_executable(
    SList
    SList.cc
)

add_executable(
    StringFixed
    StringFixed.cc
)

add_executable(
    Set
    Set.cc
)

add_executable(
    SOA3
    SOA3.cc
)

add_executable(
    Pipeline
    Pipeline.cc
)

add_executable(
    Sort
    Sort.cc
)

add_executable(
    ParallelFor
    ParallelFor.cc
)

add_executable(
    QueueMPMC
    QueueMPMC.cc
)

add_executable(
    PPGenerator
    PPGenerator.cc
)

add_executable(
    SplayTree
    SplayTree.cc
)

add_executable(
    RefCount
    RefCount.cc
)

add_executable(
    Rope
    Rope.cc
)

add_executable(
    FuncBuffer
    FuncBuffer.cc
)

add_executable(
    time
    time.cc
)

add_executable(
    Logger
    Logger.cc
)
add_library(LoggerUser SHARED
    LoggerUser.cc
)
# target_link_libraries(Logger PRIVATE LoggerUser)

add_executable(PieceList
    PieceList.cc
)

add_executable(Arena2
    Arena2.cc
)

add_executable(CmdArgs
    CmdArgs.cc
)
