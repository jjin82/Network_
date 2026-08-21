#pragma once

#include <windows.h>

#pragma pack(push, 1)

namespace HNET
{
	namespace LIB
	{
		class Buffer
		{
            enum { BUFFER_SIZE = 60000 };

		public:
			Buffer();
			~Buffer();

			void Clear();
			USHORT Offset();

            bool InSuccess(USHORT& rollbackSize);
			bool OutEmpty();

            Buffer& operator = (Buffer& Buffer);

            template<typename T> Buffer& operator << (T v);
            Buffer& operator << (char* string);
            Buffer& operator << (const char* string);
            Buffer& operator << (wchar_t* string);
            Buffer& operator << (const wchar_t* string);

            template<typename T> Buffer& operator >> (T& v);
            Buffer& operator >> (OUT char*& string);
            template<int N> Buffer& operator >> (OUT char(&string)[N]);
            Buffer& operator >> (OUT wchar_t*& string);
            template<int N> Buffer& operator >> (OUT wchar_t(&string)[N]);

            template<typename T> USHORT In(T v);
            USHORT In(char* string);
            USHORT In(const char* string);
            USHORT In(wchar_t* string);
            USHORT In(const wchar_t* string);

            template<typename T> bool Out(T& v);
            bool Out(OUT char*& string);
            template<int N> bool Out(OUT char(&string)[N]);
            bool Out(OUT wchar_t*& string);
            template<int N> bool Out(OUT wchar_t(&string)[N]);

		private:
            USHORT _In(void* p, USHORT size);
            USHORT _InString(const char& string, size_t lenth);
            USHORT _InString(const wchar_t& string, size_t lenth);
            
            bool _Out(void* p, USHORT size);
            bool _OutString(OUT char*& string, int count = 0);
            bool _OutString(OUT wchar_t*& string, int count = 0);
            
            USHORT _RollBack();

		private:
			bool   _success;            // 성공, 실패.
			USHORT _in;                 // 입력 위치.
			USHORT _out;                // 추출 위치.
			USHORT _trySize;            // begin()과 end() 사이에 등록이 시도 된 크기. (_RollBack() 에서 사용)
            BYTE   _buf[BUFFER_SIZE];   // 버퍼.
		};

        template<typename T>
        Buffer& Buffer::operator << (T v)
        {
            _In(&v, (USHORT)sizeof(T));
            return *this;
        }

        template<typename T>
        Buffer& Buffer::operator >> (T& v)
        {
            _Out(&v, (USHORT)sizeof(T));
            return *this;
        }

        template<int N>
        Buffer& Buffer::operator >> (OUT char(&string)[N])
        {
            string[0] = '\0';
            char* s = string;
            _OutString(s, N);

            return *this;
        }

        template<int N>
        Buffer& Buffer::operator >> (OUT wchar_t(&string)[N])
        {
            string[0] = '\0';
            wchar_t* s = string;
            _OutString(s, N);

            return *this;
        }

        template<int N>
        bool Buffer::Out(OUT char(&string)[N])
        {
            string[0] = '\0';
            char* s = string;

            return _OutString(s, N);;
        }

        template<int N>
        bool Buffer::Out(OUT wchar_t(&string)[N])
        {
            string[0] = '\0';
            wchar_t* s = string;

            return _OutString(s, N);;
        }

        template<typename T>
        USHORT Buffer::In(T v)
        {
            return _In(&v, (USHORT)sizeof(T));
        }

        template<typename T> 
        bool Buffer::Out(T& v)
        {
            return _Out(&v, (USHORT)sizeof(T));
        }
    }
}

#pragma pack(pop)