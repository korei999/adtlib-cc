#include "Parser.hh"

#include "adt/logs.hh"

using namespace adt;

namespace json
{

#define OK_OR_RET(RES) do { if (RES == RESULT::FAIL) return FAIL; } while(false)

static RESULT expect(Parser*s, TOKEN_TYPE t, adt::String sFile, int line);
static void next(Parser* s);
static RESULT parseNode(Parser* s, Object* pNode);
static RESULT parseObject(Parser* s, Object* pNode);
static RESULT parseArray(Parser* s, Object* pNode); /* arrays are same as objects */
static void parseString(Parser* s, TagVal* pTV);
static void parseIdent(Parser* s, TagVal* pTV);
static void parseNumber(Parser* s, TagVal* pTV);
static void parseFloat(Parser* s, TagVal* pTV);
static void parseNull(Parser* s, TagVal* pTV);
static void parseTrue(Parser* s, TagVal* pTV);
static void parseFalse(Parser* s, TagVal* pTV);

RESULT
Parser::load(adt::String path)
{
    m_sName = path;
    m_lex = Lexer(path);

    m_tCurr = m_lex.next();
    m_tNext = m_lex.next();

    if ((m_tCurr.eType != TOKEN_TYPE::L_BRACE) && (m_tCurr.eType != TOKEN_TYPE::L_BRACKET))
    {
        CERR("wrong first token\n");
        return RESULT::FAIL;
    }

    return RESULT::OK;
}

RESULT
Parser::parse()
{
    do
    {
        m_aObjects.push(m_pAlloc, {});
        if (parseNode(this, &m_aObjects.last()) == FAIL)
        {
            LOG_WARN("parseNode() failed\n");
            return FAIL;
        }

    } while (m_tCurr.eType == TOKEN_TYPE::L_BRACE);

    return OK;
}

RESULT
Parser::loadParse(adt::String path)
{
    if (load(path) == RESULT::FAIL)
        return FAIL;

    parse();
    return OK;
}

static RESULT
expect(Parser* s, TOKEN_TYPE t, adt::String svFile, int line)
{
    if (s->m_tCurr.eType != t)
    {
        CERR("('{}', at {}): ({}): unexpected token: expected: '{}', got '{}'\n",
             svFile, line, s->m_sName, char(t), char(s->m_tCurr.eType));
        return FAIL;
    }

    return OK;
}

static void
next(Parser* s)
{
    s->m_tCurr = s->m_tNext;
    s->m_tNext = s->m_lex.next();
}

static RESULT
parseNode(Parser* s, Object* pNode)
{
    switch (s->m_tCurr.eType)
    {
        default:
            next(s);
            break;

        case json::TOKEN_TYPE::QUOTED_STRING:
            parseIdent(s, &pNode->tagVal);
            break;

        case json::TOKEN_TYPE::STRING:
            parseString(s, &pNode->tagVal);
            break;

        case TOKEN_TYPE::NUMBER:
            parseNumber(s, &pNode->tagVal);
            break;

        case TOKEN_TYPE::FLOAT:
            parseFloat(s, &pNode->tagVal);
            break;

        case TOKEN_TYPE::L_BRACE:
            next(s); /* skip brace */
            OK_OR_RET(parseObject(s, pNode));
            break;

        case TOKEN_TYPE::L_BRACKET:
            next(s); /* skip bracket */
            OK_OR_RET(parseArray(s, pNode));
            break;
    }

    return OK;
}

static void
parseString(Parser* s, TagVal* pTV)
{
    const auto& sLit = s->m_tCurr.sLiteral;

    if (sLit == "null")
    {
        parseNull(s, pTV);
    }
    else if (sLit == "true")
    {
        parseTrue(s, pTV);
    }
    else if (sLit == "false")
    {
        parseFalse(s, pTV);
    }
    else
    {
        parseIdent(s, pTV);
    }
}

static void
parseIdent(Parser* s, TagVal* pTV)
{
    *pTV = {.tag = TAG::STRING, .val {.sv = s->m_tCurr.sLiteral}};
    next(s);
}

static void
parseNumber(Parser* s, TagVal* pTV)
{
    *pTV = TagVal{.tag = TAG::LONG, .val = {.l = atol(s->m_tCurr.sLiteral.m_pData)}};
    next(s);
}

static void
parseFloat(Parser* s, TagVal* pTV)
{
    *pTV = TagVal{.tag = TAG::DOUBLE, .val = {.d = atof(s->m_tCurr.sLiteral.m_pData)}};
    next(s);
}

static RESULT
parseObject(Parser* s, Object* pNode)
{
    pNode->tagVal.tag = TAG::OBJECT;
    pNode->tagVal.val.o = adt::VecBase<Object>(s->m_pAlloc, 1);
    auto& aObjs = getObject(pNode);

    for (; s->m_tCurr.eType != TOKEN_TYPE::R_BRACE; next(s))
    {
        /*OK_OR_RET(expect(s, TOKEN_TYPE::STRING, ADT_LOGS_FILE, __LINE__));*/

        Object ob {.svKey = s->m_tCurr.sLiteral, .tagVal = {}};
        aObjs.push(s->m_pAlloc, ob);

        /* skip identifier and ':' */
        next(s);
        OK_OR_RET(expect(s, TOKEN_TYPE::COLON, ADT_LOGS_FILE, __LINE__));
        next(s);

        OK_OR_RET(parseNode(s, &aObjs.last()));

        if (s->m_tCurr.eType != TOKEN_TYPE::COMMA)
        {
            next(s);
            break;
        }
    }

    if (aObjs.getSize() == 0) next(s);

    return OK;
}

static RESULT
parseArray(Parser* s, Object* pNode)
{
    pNode->tagVal.tag = TAG::ARRAY;
    pNode->tagVal.val.a = adt::VecBase<Object>(s->m_pAlloc, 1);
    auto& aTVs = getArray(pNode);

    /* collect each key/value pair inside array */
    for (; s->m_tCurr.eType != TOKEN_TYPE::R_BRACKET; next(s))
    {
        aTVs.push(s->m_pAlloc, {});

        switch (s->m_tCurr.eType)
        {
            default:
            case TOKEN_TYPE::QUOTED_STRING:
            case TOKEN_TYPE::STRING:
                parseString(s, &aTVs.last().tagVal);
                break;

            case TOKEN_TYPE::NUMBER:
                parseNumber(s, &aTVs.last().tagVal);
                break;

            case TOKEN_TYPE::FLOAT:
                parseFloat(s, &aTVs.last().tagVal);
                break;

            case TOKEN_TYPE::L_BRACE:
                next(s);
                OK_OR_RET(parseObject(s, &aTVs.last()));
                break;
        }

        if (s->m_tCurr.eType != TOKEN_TYPE::COMMA)
        {
            next(s);
            break;
        }
    }

    if (aTVs.getSize() == 0) next(s);

    return OK;
}

static void
parseNull(Parser* s, TagVal* pTV)
{
    *pTV = {.tag = TAG::NULL_, .val = {nullptr}};
    next(s);
}

static void
parseTrue(Parser* s, TagVal* pTV)
{
    *pTV = {.tag = TAG::BOOL, .val = {.b = true}};
    next(s);
}

static void
parseFalse(Parser* s, TagVal* pTV)
{
    *pTV = {.tag = TAG::BOOL, .val = {.b = false}};
    next(s);
}

void
Parser::destroy()
{
    auto fn = +[](Object* p, void* a) -> bool {
        auto* pAlloc = (IAllocator*)a;

        if (p->tagVal.tag == TAG::ARRAY || p->tagVal.tag == TAG::OBJECT)
            pAlloc->free(p->tagVal.val.a.m_pData);

        return false;
    };

    traverseAll(fn, m_pAlloc);
    m_aObjects.destroy(m_pAlloc);
}

void
Parser::print(FILE* fp)
{
    for (auto& obj : m_aObjects)
    {
        printNode(fp, &obj, "", 0);
        fputc('\n', fp);
    }
}

void
printNode(FILE* fp, Object* pNode, adt::String svEnd, int depth)
{
    adt::String key = pNode->svKey;

    switch (pNode->tagVal.tag)
    {
        default: break;

        case TAG::OBJECT:
            {
                auto& obj = getObject(pNode);
                adt::String q0, q1, objName0, objName1;

                if (key.m_size == 0)
                {
                    q0 = q1 = objName1 = objName0 = "";
                }
                else
                {
                    objName0 = key;
                    objName1 = ": ";
                    q1 = q0 = "\"";
                }

                fprintf(fp, "%*s", depth, "");
                /*COUT("{: >{}}", "", depth);*/
                print::toFILE(fp, "{}{}{}{}{\n", q0, objName0, q1, objName1);
                for (u32 i = 0; i < obj.getSize(); i++)
                {
                    adt::String slE = (i == obj.getSize() - 1) ? "\n" : ",\n";
                    printNode(fp, &obj[i], slE, depth + 2);
                }
                fprintf(fp, "%*s", depth, "");
                /*COUT("{: >{}}", "", depth);*/
                print::toFILE(fp, "}{}", svEnd);
            } break;

        case TAG::ARRAY:
            {
                auto& arr = getArray(pNode);
                adt::String q0, q1, arrName0, arrName1;

                if (key.m_size == 0)
                {
                    q0 =  q1 = arrName1 = arrName0 = "";
                }
                else
                {
                    arrName0 = key;
                    arrName1 = ": ";
                    q1 = q0 = "\"";
                }

                fprintf(fp, "%*s", depth, "");
                /*COUT("{: >{}}", "", depth);*/

                if (arr.getSize() == 0)
                {
                    print::toFILE(fp, "{}{}{}{}[", q0, arrName0, q1, arrName1);
                    print::toFILE(fp, "]{}", svEnd);
                    break;
                }

                print::toFILE(fp, "{}{}{}{}[\n", q0, arrName0, q1, arrName1);
                for (u32 i = 0; i < arr.getSize(); i++)
                {
                    adt::String slE = (i == arr.getSize() - 1) ? "\n" : ",\n";

                    switch (arr[i].tagVal.tag)
                    {
                        default:
                        case TAG::STRING:
                            {
                                adt::String sl = getString(&arr[i]);
                                fprintf(fp, "%*s", depth + 2, "");
                                /*COUT("{: >{}}", "", depth + 2);*/
                                print::toFILE(fp, "\"{}\"{}", sl, slE);
                            } break;

                        case TAG::NULL_:
                                fprintf(fp, "%*s", depth + 2, "");
                                /*COUT("{: >{}}", "", depth + 2);*/
                                print::toFILE(fp, "{}{}", "null", slE);
                            break;

                        case TAG::LONG:
                            {
                                long num = getLong(&arr[i]);
                                fprintf(fp, "%*s", depth + 2, "");
                                /*COUT("{: >{}}", "", depth + 2);*/
                                print::toFILE(fp, "{}{}", num, slE);
                            } break;

                        case TAG::DOUBLE:
                            {
                                double dnum = getDouble(&arr[i]);
                                fprintf(fp, "%*s", depth + 2, "");
                                /*COUT("{: >{}}", "", depth + 2);*/
                                print::toFILE(fp, "{}{}", dnum, slE);
                            } break;

                        case TAG::BOOL:
                            {
                                bool b = getBool(&arr[i]);
                                fprintf(fp, "%*s", depth + 2, "");
                                /*COUT("{: >{}}", "", depth + 2);*/
                                print::toFILE(fp, "{}{}", b ? "true" : "false", slE);
                            } break;

                        case TAG::OBJECT:
                                printNode(fp, &arr[i], slE, depth + 2);
                            break;
                    }
                }
                fprintf(fp, "%*s", depth, "");
                /*COUT("{: >{}}", "", depth);*/
                print::toFILE(fp, "]{}", svEnd);
            } break;

        case TAG::DOUBLE:
            {
                double f = getDouble(pNode);
                fprintf(fp, "%*s", depth, "");
                /*COUT("{: >{}}", "", depth);*/
                print::toFILE(fp, "\"{}\": {}{}", key, f, svEnd);
            } break;

        case TAG::LONG:
            {
                long i = getLong(pNode);
                fprintf(fp, "%*s", depth, "");
                /*COUT("{: >{}}", "", depth);*/
                print::toFILE(fp, "\"{}\": {}{}", key, i, svEnd);
            } break;

        case TAG::NULL_:
                fprintf(fp, "%*s", depth, "");
                /*COUT("{: >{}}", "", depth);*/
                print::toFILE(fp, "\"{}\": {}{}", key, "null", svEnd);
            break;

        case TAG::STRING:
            {
                adt::String sl = getString(pNode);
                fprintf(fp, "%*s", depth, "");
                /*COUT("{: >{}}", "", depth);*/
                print::toFILE(fp, "\"{}\": \"{}\"{}", key, sl, svEnd);
            } break;

        case TAG::BOOL:
            {
                bool b = getBool(pNode);
                fprintf(fp, "%*s", depth, "");
                /*COUT("{: >{}}", "", depth);*/
                print::toFILE(fp, "\"{}\": {}{}", key, b ? "true" : "false", svEnd);
            } break;
    }
}

void
Parser::traverse(Object* pNode, bool (*pfn)(Object* p, void* pFnArgs), void* pArgs)
{
    switch (pNode->tagVal.tag)
    {
        default: break;

        case TAG::ARRAY:
        {
            auto& obj = getArray(pNode);

            for (u32 i = 0; i < obj.getSize(); i++)
                traverse(&obj[i], pfn, pArgs);
        }
        break;

        case TAG::OBJECT:
        {
            auto& obj = getObject(pNode);

            for (u32 i = 0; i < obj.getSize(); i++)
                traverse(&obj[i], pfn, pArgs);
        }
        break;
    }

    if (pfn(pNode, pArgs)) return;
}

adt::VecBase<Object>&
Parser::getRoot()
{
    assert(m_aObjects.m_size > 0 && "[Parser]: this json is empty");

    if (m_aObjects.m_size == 1) return getObject(&m_aObjects.first());
    else return m_aObjects;
}

} /* namespace json */
