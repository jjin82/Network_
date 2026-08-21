#pragma once

#include <windows.h>

namespace HNET
{
    namespace LIB
    {
        class StrParser 
        {
            class Token : public Memory
            {
            public:
                Token(char* pos) : _pos(pos), _next(NULL) {}

            public:
                char*   _pos;
                Token* _next;
            };

        public:
	        StrParser();
	        ~StrParser();

            int Parsing(char string[], const char* delimiters = "");
            const char* KeyValue(const char* key, const char* delimiters = "");

        private:
            char* _Find(const char* key);

        private:
            Token* _head;
            int    _count;
        };
    }
}