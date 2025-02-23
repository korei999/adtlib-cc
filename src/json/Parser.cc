#include "Parser.hh"

#include "adt/logs.hh"

using namespace adt;

namespace json
{

#define OK_OR_RET(RES) if (!RES) return false;

bool
Parser::parse(IAllocator* pAlloc, StringView sJson)
{
    m_pAlloc = pAlloc;
    m_lex = Lexer(sJson);

    m_tCurr = m_lex.next();
    m_tNext = m_lex.next();

    if ((m_tCurr.eType != TOKEN_TYPE::L_BRACE) && (m_tCurr.eType != TOKEN_TYPE::L_BRACKET))
    {
        CERR("wrong first token\n");
        return false;
    }

    do
    {
        m_aObjects.push(m_pAlloc, {});
        if (!parseNode(&m_aObjects.last()))
        {
            LOG_WARN("parseNode() failed\n");
            return false;
        }
    }
    while (m_tCurr.eType == TOKEN_TYPE::L_BRACE); /* some json files have multiple root objects */

    return true;
}

bool
Parser::printNodeError()
{
    const auto& tok = m_tCurr;
    CERR("({}, {}): unexpected token: '{}'\n",
        tok.row, tok.column, m_tCurr.eType
    );
    return false;
}

void
Parser::next()
{
    m_tCurr = m_tNext;
    m_tNext = m_lex.next();
}

bool
Parser::expect(TOKEN_TYPE t)
{
    const auto& tok = m_tCurr;

    if (!!(tok.eType & t))
    {
        return true;
    }
    else
    {
        CERR("({}, {}): unexpected token: expected: '{}', got '{}' ('{}')\n",
             tok.row, tok.column, t, m_tCurr.eType, m_tCurr.sLiteral
        );
        return false;
    }
}

bool
Parser::expectNot(TOKEN_TYPE t)
{
    const auto& tok = m_tCurr;

    if (!!(tok.eType & t))
    {
        CERR("({}, {}): unexpected token: not expected: '{}', got '{}' ('{}')\n",
             tok.row, tok.column, t, m_tCurr.eType, m_tCurr.sLiteral
        );
        return false;
    }
    else return true;
}

bool
Parser::parseNode(Node* pNode)
{
    switch (m_tCurr.eType)
    {
        default:
        return printNodeError();
        break;

        case TOKEN_TYPE::QUOTED_STRING:
        parseIdent(&pNode->tagVal);
        break;

        case TOKEN_TYPE::STRING:
        parseString(&pNode->tagVal);
        break;

        case TOKEN_TYPE::NUMBER:
        parseNumber(&pNode->tagVal);
        break;

        case TOKEN_TYPE::FLOAT:
        parseFloat(&pNode->tagVal);
        break;

        case TOKEN_TYPE::L_BRACE:
        next(); /* skip brace */
        OK_OR_RET(parseObject(pNode));
        break;

        case TOKEN_TYPE::L_BRACKET:
        next(); /* skip bracket */
        OK_OR_RET(parseArray(pNode));
        break;
    }

    return true;
}

void
Parser::parseString(TagVal* pTV)
{
    const auto& sLit = m_tCurr.sLiteral;

    if (sLit == "null")
        *pTV = {.eTag = TAG::NULL_, .val = {nullptr}};
    else if (sLit == "true")
        *pTV = {.eTag = TAG::BOOL, .val = {.b = true}};
    else if (sLit == "false")
        *pTV = {.eTag = TAG::BOOL, .val = {.b = false}};
    else
        *pTV = {.eTag = TAG::STRING, .val {.s = m_tCurr.sLiteral}};

    next();
}

void
Parser::parseIdent(TagVal* pTV)
{
    *pTV = {.eTag = TAG::STRING, .val {.s = m_tCurr.sLiteral}};
    next();
}

void
Parser::parseNumber(TagVal* pTV)
{
    *pTV = {.eTag = TAG::LONG, .val = {.l = atoll(m_tCurr.sLiteral.m_pData)}};
    next();
}

void
Parser::parseFloat(TagVal* pTV)
{
    *pTV = {.eTag = TAG::DOUBLE, .val = {.d = atof(m_tCurr.sLiteral.m_pData)}};
    next();
}

bool
Parser::parseObject(Node* pNode)
{
    pNode->tagVal.eTag = TAG::OBJECT;
    pNode->tagVal.val.o = Vec<Node>(m_pAlloc);
    auto& aObjs = getObject(pNode);

    while (m_tCurr.eType != TOKEN_TYPE::R_BRACE)
    {
        /* make sure key is quoted */
        OK_OR_RET(expect(TOKEN_TYPE::QUOTED_STRING));

        aObjs.push(m_pAlloc, {.svKey = m_tCurr.sLiteral, .tagVal = {}});

        /* skip identifier and ':' */
        next();
        OK_OR_RET(expect(TOKEN_TYPE::COLON));
        next();

        OK_OR_RET(parseNode(&aObjs.last()));

        if (m_tCurr.eType != TOKEN_TYPE::COMMA)
        {
            OK_OR_RET(expect(TOKEN_TYPE::R_BRACE));
            next();
            break;
        }
        else
        {
            next();
            OK_OR_RET(expectNot(TOKEN_TYPE::R_BRACE | TOKEN_TYPE::STRING));
        }
    }

    if (aObjs.empty())
        next();

    return true;
}

bool
Parser::parseArray(Node* pNode)
{
    pNode->tagVal.eTag = TAG::ARRAY;
    pNode->tagVal.val.a = Vec<Node>(m_pAlloc);
    auto& aTVs = getArray(pNode);

    /* collect each key/value pair inside array */
    while (m_tCurr.eType != TOKEN_TYPE::R_BRACKET)
    {
        aTVs.push(m_pAlloc, {});

        switch (m_tCurr.eType)
        {
            default:
            case TOKEN_TYPE::QUOTED_STRING:
            case TOKEN_TYPE::STRING:
            parseString(&aTVs.last().tagVal);
            break;

            case TOKEN_TYPE::NUMBER:
            parseNumber(&aTVs.last().tagVal);
            break;

            case TOKEN_TYPE::FLOAT:
            parseFloat(&aTVs.last().tagVal);
            break;

            case TOKEN_TYPE::L_BRACE:
            next();
            OK_OR_RET(parseObject(&aTVs.last()));
            break;
        }

        if (m_tCurr.eType != TOKEN_TYPE::COMMA)
        {
            OK_OR_RET(expect(TOKEN_TYPE::R_BRACKET));
            next();
            break;
        }
        else
        {
            next();
            OK_OR_RET(expectNot(TOKEN_TYPE::L_BRACKET));
        }
    }

    if (aTVs.empty())
        next();

    return true;
}

void
Parser::destroy()
{
    auto fn = +[](Node* p, void* a) -> bool {
        auto* pAlloc = (IAllocator*)a;

        if (p->tagVal.eTag == TAG::ARRAY || p->tagVal.eTag == TAG::OBJECT)
            p->freeData(pAlloc);

        return false;
    };

    traverse(fn, m_pAlloc, TRAVERSAL_ORDER::POST);
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
printNode(FILE* fp, Node* pNode, StringView sEnd, int depth)
{
    StringView key = pNode->svKey;

    switch (pNode->tagVal.eTag)
    {
        default: break;

        case TAG::OBJECT:
        {
            auto& obj = getObject(pNode);
            StringView q0, q1, objName0, objName1;

            if (key.getSize() == 0)
            {
                q0 = q1 = objName1 = objName0 = "";
            }
            else
            {
                objName0 = key;
                objName1 = ": ";
                q1 = q0 = "\"";
            }

            print::toFILE(fp, "{:{}}{}{}{}{}{\n", depth, "", q0, objName0, q1, objName1);
            for (u32 i = 0; i < obj.getSize(); i++)
            {
                StringView slE = (i == obj.getSize() - 1) ? "\n" : ",\n";
                printNode(fp, &obj[i], slE, depth + 2);
            }
            print::toFILE(fp, "{:{}}}{}", depth, "", sEnd);
        }
        break;

        case TAG::ARRAY:
        {
            auto& arr = getArray(pNode);
            StringView q0, q1, arrName0, arrName1;

            if (key.getSize() == 0)
            {
                q0 =  q1 = arrName1 = arrName0 = "";
            }
            else
            {
                arrName0 = key;
                arrName1 = ": ";
                q1 = q0 = "\"";
            }

            print::toFILE(fp, "{:{}}", depth, "");

            if (arr.getSize() == 0)
            {
                print::toFILE(fp, "{}{}{}{}[]{}", q0, arrName0, q1, arrName1, sEnd);
                break;
            }

            print::toFILE(fp, "{}{}{}{}[\n", q0, arrName0, q1, arrName1);
            for (u32 i = 0; i < arr.getSize(); i++)
            {
                StringView slE = (i == arr.getSize() - 1) ? "\n" : ",\n";

                switch (arr[i].tagVal.eTag)
                {
                    default:
                    case TAG::STRING:
                    {
                        StringView sl = getString(&arr[i]);
                        print::toFILE(fp, "{:{}}\"{}\"{}", depth + 2, "", sl, slE);
                    }
                    break;

                    case TAG::NULL_:
                    print::toFILE(fp, "{:{}}{}{}", depth + 2, "", "null", slE);
                    break;

                    case TAG::LONG:
                    {
                        long num = getLong(&arr[i]);
                        print::toFILE(fp, "{:{}}{}{}", depth + 2, "", num, slE);
                    }
                    break;

                    case TAG::DOUBLE:
                    {
                        double dnum = getDouble(&arr[i]);
                        print::toFILE(fp, "{:{}}{}{}", depth + 2, "", dnum, slE);
                    }
                    break;

                    case TAG::BOOL:
                    {
                        bool b = getBool(&arr[i]);
                        print::toFILE(fp, "{:{}}{}{}", depth + 2, "", b ? "true" : "false", slE);
                    }
                    break;

                    case TAG::OBJECT:
                    printNode(fp, &arr[i], slE, depth + 2);
                    break;
                }
            }
            print::toFILE(fp, "{:{}}]{}", depth, "", sEnd);
        }
        break;

        case TAG::DOUBLE:
        {
            f64 f = getDouble(pNode);
            print::toFILE(fp, "{:{}}\"{}\": {}{}", depth, "", key, f, sEnd);
        }
        break;

        case TAG::LONG:
        {
            long i = getLong(pNode);
            print::toFILE(fp, "{:{}}\"{}\": {}{}", depth, "", key, i, sEnd);
        }
        break;

        case TAG::NULL_:
        print::toFILE(fp, "{:{}}\"{}\": {}{}", depth, "", key, "null", sEnd);
        break;

        case TAG::STRING:
        {
            StringView sl = getString(pNode);
            print::toFILE(fp, "{:{}}\"{}\": \"{}\"{}", depth, "", key, sl, sEnd);
        }
        break;

        case TAG::BOOL:
        {
            bool b = getBool(pNode);
            print::toFILE(fp, "{:{}}\"{}\": {}{}", depth, "", key, b ? "true" : "false", sEnd);
        }
        break;
    }
}

static void
traverseNodePRE(Node* pNode, bool (*pfn)(Node* p, void* pFnArgs), void* pArgs)
{
    if (pfn(pNode, pArgs))
        return;

    switch (pNode->tagVal.eTag)
    {
        default: break;

        case TAG::ARRAY:
        {
            auto& obj = getArray(pNode);

            for (ssize i = 0; i < obj.getSize(); i++)
                traverseNodePRE(&obj[i], pfn, pArgs);
        }
        break;

        case TAG::OBJECT:
        {
            auto& obj = getObject(pNode);

            for (ssize i = 0; i < obj.getSize(); i++)
                traverseNodePRE(&obj[i], pfn, pArgs);
        }
        break;
    }
}

static void
traverseNodePOST(Node* pNode, bool (*pfn)(Node* p, void* pFnArgs), void* pArgs)
{
    switch (pNode->tagVal.eTag)
    {
        default: break;

        case TAG::ARRAY:
        {
            auto& obj = getArray(pNode);

            for (ssize i = 0; i < obj.getSize(); i++)
                traverseNodePOST(&obj[i], pfn, pArgs);
        }
        break;

        case TAG::OBJECT:
        {
            auto& obj = getObject(pNode);

            for (ssize i = 0; i < obj.getSize(); i++)
                traverseNodePOST(&obj[i], pfn, pArgs);
        }
        break;
    }

    if (pfn(pNode, pArgs))
        return;
}

void
traverseNode(Node* pNode, bool (*pfn)(Node* p, void* pFnArgs), void* pArgs, TRAVERSAL_ORDER eOrder)
{
    switch (eOrder)
    {
        case TRAVERSAL_ORDER::PRE:
        traverseNodePRE(pNode, pfn, pArgs);
        break;

        case TRAVERSAL_ORDER::POST:
        traverseNodePOST(pNode, pfn, pArgs);
        break;
    }
}

Vec<Node>&
Parser::getRoot()
{
    ADT_ASSERT(m_aObjects.getSize() > 0, "empty");

    if (m_aObjects.getSize() == 1)
        return getObject(&m_aObjects.first());
    else return m_aObjects;
}

const Vec<Node>&
Parser::getRoot() const
{
    ADT_ASSERT(m_aObjects.getSize() > 0, "empty");

    if (m_aObjects.getSize() == 1)
        return getObject(&m_aObjects.first());
    else return m_aObjects;
}

void
Parser::traverse(bool (*pfn)(Node* p, void* pFnArgs), void* pArgs, TRAVERSAL_ORDER eOrder)
{
    switch (eOrder)
    {
        case TRAVERSAL_ORDER::PRE:
        for (auto& obj : m_aObjects)
            traverseNodePRE(&obj, pfn, pArgs);
        break;

        case TRAVERSAL_ORDER::POST:
        for (auto& obj : m_aObjects)
            traverseNodePOST(&obj, pfn, pArgs);
        break;
    }
}

} /* namespace json */
