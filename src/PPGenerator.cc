#include "adt/print.hh"

static const char* ntsExample =
R"(/* NOTE: MSVC needs '/Zc:preprocessor'. */

/* example:
 *
 * #define ENTITY_PP_BIND_I(TYPE, NAME) , &Entity::NAME
 * #define ENTITY_PP_BIND(TUPLE) ENTITY_PP_BIND_I TUPLE
 * #define ENTITY_FIELDS \
 *     (adt::StringFixed<128>, sfName),
 *     (adt::math::V4, color),
 *     (adt::math::V3, pos),
 *     (adt::math::Qt, rot),
 *     (adt::math::V3, scale),
 *     (adt::math::V3, vel),
 *     (adt::i16, assetI),
 *     (adt::i16, modelI),
 *     (bool, bNoDraw)
 * ADT_SOA_GEN_STRUCT_ZERO(Entity, Bind, ENTITY_FIELDS);
 *
 * expands to:
 *
 * struct Entity {
 *   struct Bind {
 *     adt::StringFixed<128>& sfName;
 *     adt::math::V4& color;
 *     adt::math::V3& pos;
 *     adt::math::Qt& rot;
 *     adt::math::V3& scale;
 *     adt::math::V3& vel;
 *     adt::i16& assetI;
 *     adt::i16& modelI;
 *     bool& bNoDraw;
 *   };
 *   adt::StringFixed<128> sfName;
 *   adt::math::V4 color;
 *   adt::math::V3 pos;
 *   adt::math::Qt rot;
 *   adt::math::V3 scale;
 *   adt::math::V3 vel;
 *   adt::i16 assetI;
 *   adt::i16 modelI;
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

int
main(const int argc, const char** argv)
{
    parseArgs(argc, argv);

    printf("#pragma once\n");
    printf("/* Generated with PPGenerator.cc. */\n\n");
    printf("%s\n", ntsExample);

    printf("#define ADT_PP_RSEQ_N() %d", s_max);
    for (int i = s_max - 1; i >= 0; --i)
        printf(",%d", i);

    printf("\n");

    printf("#define ADT_PP_ARG_N(");
    for (int i = 1; i <= s_max; ++i)
        printf("_%d,", i);
    printf("N,...) N\n");

    printf("#define ADT_PP_NARG_(...) ADT_PP_ARG_N(__VA_ARGS__)\n");
    printf("#define ADT_PP_NARG(...) ADT_PP_NARG_(__VA_ARGS__, ADT_PP_RSEQ_N())\n");
    printf("\n");
    printf("#define ADT_PP_CONCAT(A, B) ADT_PP_CONCAT_I(A, B)\n");
    printf("#define ADT_PP_CONCAT_I(A, B) A##B\n");
    printf("\n");
    printf("#define ADT_PP_FOR_EACH(MACRO, ...) ADT_PP_CONCAT(ADT_PP_FOR_EACH_, ADT_PP_NARG(__VA_ARGS__))(MACRO, __VA_ARGS__)\n");
    printf("\n");

    for (int i = 1; i <= s_max; ++i)
    {
        printf("#define ADT_PP_FOR_EACH_%d(m", i);
        for (int j = 1; j <= i; ++j)
            printf(", x%d", j);
        printf(") \\\n");
        printf("\t");
        printf("m(x1)");
        for (int j = 2; j <= i; ++j)
            printf(" m(x%d)", j);
        printf("\n");
    }

    printf("\n");
    printf("#define ADT_DECL_FIELD_ZERO(TUPLE) ADT_DECL_FIELD_ZERO_I TUPLE;\n");
    printf("#define ADT_DECL_FIELD_ZERO_I(TYPE, NAME) TYPE NAME {}\n");
    printf("\n");
    printf("#define ADT_DECL_FIELD(TUPLE) ADT_DECL_FIELD_I TUPLE;\n");
    printf("#define ADT_DECL_FIELD_I(TYPE, NAME) TYPE NAME\n");
    printf("\n");
    printf("#define ADT_DECL_FIELD_REF(TUPLE) ADT_DECL_FIELD_REF_I TUPLE;\n");
    printf("#define ADT_DECL_FIELD_REF_I(TYPE, NAME) TYPE& NAME\n");
    printf("%s", ntsADT_SOA_GEN_STRUCT);
    printf("%s", ntsADT_SOA_GEN_STRUCT_ZERO);
    
}
