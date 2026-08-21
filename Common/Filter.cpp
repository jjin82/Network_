#include "Common.h"

namespace HNET
{
    namespace LIB
    {
        Filter::Filter()
        {
	
        }

        Filter::~Filter()
        {
	        Filter* filter;
	        POSITION Pos = GetStartPosition();
	        while (Pos)
	        {
		        filter = GetNextValue(Pos);
		        delete filter;
	        }

	        RemoveAll();
        }

        bool Filter::Register(const wchar_t* string)
        { 
	        // 문자열 끝인 경우 추가.
	        if ('\0' == *string)
	        {
		        SetAt(*string, NULL);
		        return true;
	        }

	        Filter* word = NULL;
	        CAtlMap<wchar_t, Filter*>::CPair* Pair = Lookup(*string);
	        if (NULL == Pair)
	        {
				word = new(std::nothrow) Filter;
		        if (NULL == word)
			        return false;

		        POSITION Pos = SetAt(*string, word);
		        if (NULL == Pos) 
			        return false;
	        }
	        else
	        {
				word = Pair->m_value;
	        }
	
	        return word->Register((string + 1));
        }

        bool Filter::IsExist(const wchar_t* string, OUT wchar_t* filter, size_t lenth)
        {
	        // 최초 호출 길이 획득.
	        if (*string && 0 == lenth)
		        lenth = wcslen(string);

	        CAtlMap<wchar_t, Filter*>::CPair* pair = Lookup('\0');
	        if (pair)
		        return true;

	        if (0 == GetCount())
		        return false;

	        if ('\0' == *string)
		        return false;

	        int     count = 0;
	        Filter* word  = NULL;
	        while (lenth)
	        {
				pair = Lookup(*(string + count));
		        if (pair)
		        {
					word = pair->m_value;
	
			        if (filter)
			        {
				        *filter     = *(string + count);
				        *(++filter) = '\0';
			        }

			        return word->IsExist((string + (++count)), filter, --lenth);
		        }
		        ++count;
		        --lenth;
	        }

	        return false;
        }
    }
}