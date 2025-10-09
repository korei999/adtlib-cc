#include "adt/print.hh"
#include "adt/Logger.hh"

static const char* ntsExample =
R"(/* NOTE: MSVC needs '/Zc:preprocessor'. */

/* example:
 *
 * #define ENTITY_PP_BIND_I(TYPE, NAME) , &Entity::NAME
 * #define ENTITY_PP_BIND(TUPLE) ENTITY_PP_BIND_I TUPLE
 * #define ENTITY_FIELDS \
 *     (StringFixed<128>, sfName),\
 *     (math::V4, color),\
 *     (math::V3, pos),\
 *     (math::Qt, rot),\
 *     (math::V3, scale),\
 *     (math::V3, vel),\
 *     (i16, assetI),\
 *     (i16, modelI),\
 *     (bool, bNoDraw)
 * ADT_SOA_GEN_STRUCT_ZERO(Entity, Bind, ENTITY_FIELDS);
 *
 * expands to:
 *
 * struct Entity {
 *   struct Bind {
 *     StringFixed<128>& sfName;
 *     math::V4& color;
 *     math::V3& pos;
 *     math::Qt& rot;
 *     math::V3& scale;
 *     math::V3& vel;
 *     i16& assetI;
 *     i16& modelI;
 *     bool& bNoDraw;
 *   };
 *   StringFixed<128> sfName;
 *   math::V4 color;
 *   math::V3 pos;
 *   math::Qt rot;
 *   math::V3 scale;
 *   math::V3 vel;
 *   i16 assetI;
 *   i16 modelI;
 *   bool bNoDraw;
 * }; */
)";


static const char* ntsADT_SOA_GEN_STRUCT =
R"(
#define ADT_SOA_GEN_STRUCT(NAME, BIND, ...)                                                                            \
    struct NAME                                                                                                        \
    {                                                                                                                  \
        struct BIND                                                                                                    \
        {                                                                                                              \
            ADT_PP_FOR_EACH(ADT_DECL_FIELD_REF, __VA_ARGS__)                                                           \
        };                                                                                                             \
                                                                                                                       \
        ADT_PP_FOR_EACH(ADT_DECL_FIELD, __VA_ARGS__)                                                                   \
    }
)";

static const char* ntsADT_SOA_GEN_STRUCT_ZERO =
R"(
#define ADT_SOA_GEN_STRUCT_ZERO(NAME, BIND, ...)                                                                       \
    struct NAME                                                                                                        \
    {                                                                                                                  \
        struct BIND                                                                                                    \
        {                                                                                                              \
            ADT_PP_FOR_EACH(ADT_DECL_FIELD_REF, __VA_ARGS__)                                                           \
        };                                                                                                             \
                                                                                                                       \
        ADT_PP_FOR_EACH(ADT_DECL_FIELD_ZERO, __VA_ARGS__)                                                              \
    }
)";

using namespace adt;

static int s_max = 10;

static void
parseArgs(const int argc, const char** argv)
{
    for (int i = 1; i < argc; ++i)
    {
        const StringView svArg {argv[i]};
        if (svArg.beginsWith("--"))
        {
            if (svArg == "--max")
            {
                if (i + 1 < argc)
                {
                    s_max = StringView(argv[i + 1]).toI64();
                }
                else
                {
                    print::err("no arg after --max");
                    exit(1);
                }
            }
        }
    }
}


template<typename ...ARGS>
static isize
out(const StringView fmt, const ARGS&... args)
{
    return print::toFILE(Gpa::inst(), stdout, fmt, args...);
}

int
main(const int argc, const char** argv)
{
    parseArgs(argc, argv);

    out("#pragma once\n");
    out("/* Generated with PPGenerator.cc. */\n\n");
    out("{}\n", ntsExample);

    out("#define ADT_PP_RSEQ_N() {}", s_max);
    for (int i = s_max - 1; i >= 0; --i)
        out(",{}", i);

    out("\n");

    out("#define ADT_PP_ARG_N(");
    for (int i = 1; i <= s_max; ++i)
        out("_{},", i);
    out("N,...) N\n");

    out("#define ADT_PP_NARG_(...) ADT_PP_ARG_N(__VA_ARGS__)\n");
    out("#define ADT_PP_NARG(...) ADT_PP_NARG_(__VA_ARGS__, ADT_PP_RSEQ_N())\n");
    out("\n");
    out("#define ADT_PP_CONCAT(A, B) ADT_PP_CONCAT_I(A, B)\n");
    out("#define ADT_PP_CONCAT_I(A, B) A##B\n");
    out("\n");
    out("#define ADT_PP_FOR_EACH(MACRO, ...) ADT_PP_CONCAT(ADT_PP_FOR_EACH_, ADT_PP_NARG(__VA_ARGS__))(MACRO, __VA_ARGS__)\n");
    out("\n");

    for (int i = 1; i <= s_max; ++i)
    {
        out("#define ADT_PP_FOR_EACH_{}(m", i);
        for (int j = 1; j <= i; ++j)
            out(", x{}", j);
        out(") \\\n");
        out("\t");
        out("m(x1)");
        for (int j = 2; j <= i; ++j)
            out(" m(x{})", j);
        out("\n");
    }

    out("\n");
    out("#define ADT_DECL_FIELD_ZERO(TUPLE) ADT_DECL_FIELD_ZERO_I TUPLE;\n");
    out("#define ADT_DECL_FIELD_ZERO_I(TYPE, NAME) TYPE NAME {}\n");
    out("\n");
    out("#define ADT_DECL_FIELD(TUPLE) ADT_DECL_FIELD_I TUPLE;\n");
    out("#define ADT_DECL_FIELD_I(TYPE, NAME) TYPE NAME\n");
    out("\n");
    out("#define ADT_DECL_FIELD_REF(TUPLE) ADT_DECL_FIELD_REF_I TUPLE;\n");
    out("#define ADT_DECL_FIELD_REF_I(TYPE, NAME) TYPE& NAME\n");
    out("{}", ntsADT_SOA_GEN_STRUCT);
    out("{}", ntsADT_SOA_GEN_STRUCT_ZERO);
}
