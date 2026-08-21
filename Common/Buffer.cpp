#include "Buffer.h"

namespace HNET
{
    namespace LIB
    {
        Buffer::Buffer()
        {
            Clear();
        }

        Buffer::~Buffer()
        {

        }

        void Buffer::Clear()
        {
            _success = true;
            _in      = 0;
            _out     = 0;
            _trySize = 0;
        }

        USHORT Buffer::Offset()
        {
            return (USHORT)(sizeof(*this) - sizeof(_buf));
        }

        bool Buffer::InSuccess(USHORT& rollbackSize)
        {
            if (false == _success)
            {
                rollbackSize = _RollBack();

                // 롤백 후 성공 상태로.
                _success = true; 

                // 입력에 실패하였으니 false 리턴.
                return false;
            }

            _trySize = 0;

            return _success;
        }

        bool Buffer::OutEmpty()
        {
            return (_in <= _out);
        }

        Buffer& Buffer::operator = (Buffer& Buffer)
        {
            USHORT rollbackSize = 0;
            if (false == Buffer.InSuccess(rollbackSize))
                return *this;

            _success = Buffer._success;
            _in      = Buffer._in;
            _out     = Buffer._out;
            _trySize = 0;

            memcpy_s(_buf, BUFFER_SIZE, Buffer._buf, Buffer._in);

            return *this;
        }

        Buffer& Buffer::operator << (char* string)
        {
            _InString(string[0], strnlen_s(string, (BUFFER_SIZE - _in)));
            return *this;
        }

        Buffer& Buffer::operator << (const char* string)
        {
            _InString(string[0], strnlen_s(string, (BUFFER_SIZE - _in)));
            return *this;
        }

        Buffer& Buffer::operator << (wchar_t* string)
        {
            _InString(string[0], wcsnlen_s(string, (BUFFER_SIZE - _in)));
            return *this;
        }

        Buffer& Buffer::operator << (const wchar_t* string)
        {
            _InString(string[0], wcsnlen_s(string, (BUFFER_SIZE - _in)));
            return *this;
        }

        Buffer& Buffer::operator >> (OUT char*& string)
        {
            string[0] = '\0';
            _OutString(string);

            return *this;
        }

        Buffer& Buffer::operator >> (OUT wchar_t*& string)
        {
            string[0] = '\0';
            _OutString(string);

            return *this;
        }

        USHORT Buffer::In(char* string)
        {
            if(NULL == string) string = (char*)"";

            return _InString(string[0], strnlen_s(string, (BUFFER_SIZE - _in)));
        }

        USHORT Buffer::In(const char* string)
        {
            if (NULL == string) string = "";

            return _InString(string[0], strnlen_s(string, (BUFFER_SIZE - _in)));
        }

        USHORT Buffer::In(wchar_t* string)
        {
            if (NULL == string) string = (wchar_t*)L"";

            return _InString(string[0], wcsnlen_s(string, (BUFFER_SIZE - _in)));
        }

        USHORT Buffer::In(const wchar_t* string)
        {
            if (NULL == string) string = L"";

            return _InString(string[0], wcsnlen_s(string, (BUFFER_SIZE - _in)));
        }

        bool Buffer::Out(OUT char*& string)
        {
            return _OutString(string);
        }

        bool Buffer::Out(OUT wchar_t*& string)
        {
            return _OutString(string);
        }

        USHORT Buffer::_In(void* p, USHORT size)
        {
            if (false == _success)
                return 0;

            if (BUFFER_SIZE < _in + size)
            {
                _success = false;
                return 0;
            }

            memcpy_s(&_buf[_in], size, p, size);

            _in      += size;
            _trySize += size;

            return size;
        }

        USHORT Buffer::_InString(const char& string, size_t lenth)
        {
            if (false == _success)
                return 0;

            USHORT size      = (USHORT)((lenth + 1) * sizeof(char));
            USHORT totalSize = sizeof(USHORT) + size;

            if (BUFFER_SIZE < _in + totalSize)
            {
                _success = false;
                return 0;
            }

            // size.
            if (false == _In(&size, sizeof(size)))
            {
                _success = false;
                return 0;
            }

            // string.
            memcpy_s(&_buf[_in], size, &string, size);
            _in += size;

            _trySize += totalSize;

            return totalSize;
        }

        USHORT Buffer::_InString(const wchar_t& string, size_t lenth)
        {
            if (false == _success)
                return 0;

            USHORT size      = (USHORT)((lenth + 1) * sizeof(wchar_t));
            USHORT totalSize = sizeof(USHORT) + size;

            if (BUFFER_SIZE < _in + totalSize)
            {
                _success = false;
                return 0;
            }

            // size.
            if (false == _In(&size, sizeof(size)))
            {
                _success = false;
                return 0;
            }

            // string.
            memcpy_s(&_buf[_in], size, &string, size);
            _in += size;

            _trySize += totalSize;

            return totalSize;
        }

        bool Buffer::_Out(void* p, USHORT size)
        {
            if (OutEmpty())
                return false;

            if ((_out + size) > _in)
                return false;

            memcpy_s(p, size, &_buf[_out], size);

            _out += size;

            return true;
        }

        bool Buffer::_OutString(OUT char*& string, int count)
        {
            USHORT size = 0;
            if (false == _Out(&size, sizeof(size)))
            {
                Clear();
                return (_success = false);
            }

            if (0 == size)
            {
                Clear();
                return (_success = false);
            }

            // '\0' 포함된 길이.
            int lenth = size / sizeof(char);
            if (0 == lenth)
            {
                Clear();
                return (_success = false);
            }

            if ((_out + size) > _in)
            {
                Clear();
                return (_success = false);
            }

            if (0 == count)
            {
                string = (char*)&_buf[_out];
                string[lenth - 1] = '\0';
            }
            else
            {
                size_t copySize = size;
                if (lenth > count)
                {
                    copySize = count * sizeof(char);
                    lenth    = count;
                }

                memcpy_s(string, copySize, &_buf[_out], copySize);
                string[lenth - 1] = '\0';
            }

            _out += size;

            return true;
        }

        bool Buffer::_OutString(OUT wchar_t*& string, int count)
        {
            USHORT size = 0;
            if (false == _Out(&size, sizeof(size)))
            {
                Clear();
                return (_success = false);
            }

            if (0 == size)
            {
                Clear();
                return (_success = false);
            }

            // '\0' 포함된 길이.
            int lenth = size / sizeof(wchar_t);
            if (0 == lenth)
            {
                Clear();
                return (_success = false);
            }

            if ((_out + size) > _in)
            {
                Clear();
                return (_success = false);
            }

            if (0 == count)
            {
                string            = (wchar_t*)&_buf[_out];
                string[lenth - 1] = '\0';
            }
            else
            {
                size_t copySize = size;
                if (lenth > count)
                {
                    copySize = count * sizeof(wchar_t);
                    lenth    = count;
                }

                memcpy_s(string, copySize, &_buf[_out], copySize);
                string[lenth - 1] = '\0';
            }

            _out += size;

            return true;
        }

        USHORT Buffer::_RollBack()
        {
            USHORT rollBackSize = _trySize;

            _in     -= _trySize;
            _trySize = 0;

            if (BUFFER_SIZE < _in)
            {
                rollBackSize += _in;
                _in           = 0;
            }

            return rollBackSize;
        }
    }
}