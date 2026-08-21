#include "Common.h"

namespace HNET
{
    namespace ETC
    {
        const char* DELIMITERS = " \t\n\r,;:][-";
    }
}

namespace HNET
{
    namespace LIB
    {
        StrParser::StrParser()
            : _head(NULL)
            , _count(0)
        {

        }

        StrParser::~StrParser()
        {
            while (_head)
            {
                Token* removeToken = _head;
                _head = removeToken->_next;

                delete removeToken;
            }
        }

        int StrParser::Parsing(char string[], const char* delimiters)
        {
            if ('\0' == delimiters[0])
                delimiters = HNET::ETC::DELIMITERS;

            char*    next  = string;
            Token** token = &_head;

            while (next)
            {
                char* pCur = strtok_s(next, delimiters, &next);
                if (NULL == pCur)
                    break;

                *token = new Token(pCur);
                if (NULL == *token)
                    break;

                token = &(*token)->_next;

                ++_count;
            }

            return _count;
        }

        const char* StrParser::KeyValue(const char* key, const char* delimiters)
        {
            if ('\0' == delimiters[0])
                delimiters = HNET::ETC::DELIMITERS;

            if (NULL == key)
                return NULL;

            char* token = _Find(key);
            if (NULL == token)
                return NULL;

            char* string = NULL;
            strtok_s(token, delimiters, &string);
            if (NULL == string)
                return NULL;
            
            return string;
        }

        char* StrParser::_Find(const char* key)
        {
            for (Token* token = _head; token; token = token->_next)
            {
                char* found = strstr(token->_pos, key);
                if (NULL == found)
                    continue;

                return found;
            }

            return NULL;
        }
    }
}