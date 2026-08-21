#include "Common.h"

#pragma comment(lib, "odbc32.lib")

SQLLEN g_nts  = SQL_NTS;
SQLLEN g_zero = 0;
SQLLEN g_len  = 0;

#define DB_LOG(TYPE, FORMAT, ...) _log ? _log->Write(TYPE, FORMAT, __VA_ARGS__) : 0

namespace HNET
{
    namespace LIB
    {
        ODBC::ODBC(void)
            : _hEnv(SQL_NULL_HENV)
			, _hDbc(SQL_NULL_HDBC)
			, _hStmt(SQL_NULL_HSTMT)
            , _queryLen(0)
			, _paramCount(0)
			, _timeStampCount(0)
			, _log(NULL)
        {
            _connection[0] = '\0';
            _query[0]      = '\0';
			ZeroMemory(_timeStamps, sizeof(_timeStamps));
        }

        ODBC::~ODBC(void)
        {
			Disconnect();

			if (_log)
			{
				delete _log;
			}
        }

		bool ODBC::Connect(const wchar_t* driver, const wchar_t* database, const wchar_t* userId, const wchar_t* pw)
		{
			return Connect(L"localhost", driver, database, userId, pw);
		}

        bool ODBC::Connect(const wchar_t* ip, const wchar_t* driver, const wchar_t* database, const wchar_t* userId, const wchar_t* pw)
        {
            return Connect(ip, 0, driver, database, userId, pw);
        }

        bool ODBC::Connect(const wchar_t* ip, int port, const wchar_t* driver, const wchar_t* database, const wchar_t* userId, const wchar_t* pw)
        {
			if (NULL == _log)
			{
                wchar_t fileName[1024];
				API::StringPrintf(fileName, L"DB_%s", database);
				_log = new Logger(fileName);
			}

            // 커넥션 정보.
            wchar_t logDummy[1024] = L"";
            if (port)
            {
                API::StringPrintf(_connection, L"SERVER=%s;PORT=%d;DRIVER=%s;DATABASE=%s;UID=%s;PWD=%s", ip, port, driver, database, userId, pw);
                API::StringPrintf(logDummy, L"SERVER = \"%s\", PORT=\"%d\", DRIVER=\"%s\", DATABASE=\"%s\", UID=\"%s\"", ip, port, driver, database, userId);
            }
            else
            {
                API::StringPrintf(_connection, L"SERVER=%s;DRIVER=%s;DATABASE=%s;UID=%s;PWD=%s", ip, driver, database, userId, pw);
                API::StringPrintf(logDummy, L"SERVER = \"%s\", DRIVER=\"%s\", DATABASE=\"%s\", UID=\"%s\"", ip, driver, database, userId);
            }

			if (false == _Connect())
			{
				DB_LOG(LOG_TYPE_CRITICAL, L"DB connection failed. %s", logDummy);
				return false;
			}
				
			DB_LOG(LOG_TYPE_INFO, L"DB connection. %s", logDummy);

			return true;
        }

		void ODBC::Disconnect()
		{
			if (SQL_NULL_HSTMT != _hStmt)
			{
				SQLFreeHandle(SQL_HANDLE_STMT, _hStmt);
				_hStmt = SQL_NULL_HSTMT;
			}

			if (SQL_NULL_HDBC != _hDbc)
			{
				SQLDisconnect(_hDbc); 
				SQLFreeHandle(SQL_HANDLE_DBC, _hDbc);
				_hDbc = SQL_NULL_HDBC;
			}

			if (SQL_NULL_HENV != _hEnv)
			{
				SQLFreeHandle(SQL_HANDLE_ENV, _hEnv);
				_hEnv = SQL_NULL_HENV;
			}
		}

		bool ODBC::SetProcedure(const wchar_t* procedure)
        {
			if (false == API::StringPrintf(_query, L"{call %s}", procedure))
				return false;

			_paramCount = 0;
			_queryLen   = API::StringLenth(_query) - 1;

			_Prepare();
			
	        return true;
        }

		bool ODBC::Execute(const wchar_t* query, ...)
		{
            // 순수 쿼리 호출.
			if (query)
			{
				va_list Args;
				va_start(Args, query);
					_vsnwprintf_s(_query, sizeof(_query), _TRUNCATE, query, Args);
				va_end(Args);
                
                _Prepare();
			}
				
			SQLRETURN sqlReturn = SQLExecDirectW(_hStmt, _query, SQL_NTS);
			if (SQL_SUCCESS != sqlReturn && SQL_SUCCESS_WITH_INFO != sqlReturn)
				return _SqlError();

			return true;
		}

        bool ODBC::Fetch()
        {
	        if (SQL_SUCCESS != SQLFetch(_hStmt))
		        return false;

	        _paramCount = 0;

	        return true;
        }

        bool ODBC::NextRecord()
        {
	        if (SQL_SUCCESS != SQLMoreResults(_hStmt))
		        return false;

	        return true;
        }

	    void ODBC::In(char value)
        {
	        _AddParam();
	        SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &value, 0, &g_zero);
        }
	
	    void ODBC::In(unsigned char value)
        {
	        _AddParam();
	        SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_UTINYINT, SQL_TINYINT, 0, 0, &value, 0, &g_zero);
        }

        void ODBC::In(int value)
        {
	        _AddParam();
	        SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &value, 0, &g_zero);
        }

        void ODBC::In(unsigned int value)
        {
	        _AddParam();
	        SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &value, 0, &g_zero);
        }

	    void ODBC::In(long value)
        {
	        _AddParam();
	        SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &value, 0, &g_zero);
        }

        void ODBC::In(unsigned long value)
        {
	        _AddParam();
	        SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &value, 0, &g_zero);
        }

        void ODBC::In(short value)
        {
	        _AddParam();
	        SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_SHORT, SQL_SMALLINT, 0, 0, &value, 0, &g_zero);
        }

	    void ODBC::In(unsigned short value)
        {
	        _AddParam();
	        SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_USHORT, SQL_SMALLINT, 0, 0, &value, 0, &g_zero);
        }

        void ODBC::In(__int64 value)
        {
	        _AddParam();
	        SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, &value, 0, &g_zero);
        }

        void ODBC::In(unsigned __int64 value)
        {
	        _AddParam();
	        SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_UBIGINT, SQL_BIGINT, 0, 0, &value, 0, &g_zero);
        }

	    void ODBC::In(bool value)
        {
	        _AddParam();
	        SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &value, 0, &g_zero);
        }

        void ODBC::In(float value)
        {
	        _AddParam();
	        SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &value, 0, &g_zero);
        }

        void ODBC::In(double value)
        {
	        _AddParam();
	        SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_FLOAT, 0, 0, &value, 0, &g_zero);
        }

        void ODBC::In(SYSTEMTIME& value)
        {
            // 시간 변경.
	        TIMESTAMP_STRUCT* timeStamp = &_timeStamps[++_timeStampCount%MAX_TIMESTAMP_NUM];

			timeStamp->year     = value.wYear;
			timeStamp->month    = value.wMonth;
			timeStamp->day      = value.wDay;
			timeStamp->hour     = value.wHour;
			timeStamp->minute   = value.wMinute;
			timeStamp->second   = value.wSecond;
			timeStamp->fraction = 0;

            _AddParam();
	        SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 23, 0, timeStamp, 16, &g_zero);
        }

		void ODBC::In(char* string, SQLULEN size)
		{
			if (NULL == string)
				return;

			_AddParam();
			SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, size, 0, string, 0, &g_nts);
		}

		void ODBC::In(wchar_t* string, SQLULEN size)
		{
			if (NULL == string)
				return;

			_AddParam();
			SQLBindParameter(_hStmt, ++_paramCount, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, size, 0, string, 0, &g_nts);
		}

	    void ODBC::Out(OUT char& value)
        {
	        value = 0;
			
	        SQLGetData(_hStmt, ++_paramCount, SQL_C_TINYINT, &value, 0, &g_len);
        }

        void ODBC::Out(OUT unsigned char& value)
        {
	        value = 0;

	        SQLGetData(_hStmt, ++_paramCount, SQL_C_UTINYINT, &value, 0, &g_len);
        }

        void ODBC::Out(OUT int& value)
        {
	        value = 0;

	        SQLGetData(_hStmt, ++_paramCount, SQL_C_LONG, &value, 0, &g_len);
        }

        void ODBC::Out(OUT unsigned int& value)
        {
	        value = 0;

	        SQLGetData(_hStmt, ++_paramCount, SQL_C_ULONG, &value, 0, &g_len);
        }

	    void ODBC::Out(OUT long& value)
        {
	        value = 0;

	        SQLGetData(_hStmt, ++_paramCount, SQL_C_LONG, &value, 0, &g_len);
        }

        void ODBC::Out(OUT unsigned long& value)
        {
	        value = 0;

	        SQLGetData(_hStmt, ++_paramCount, SQL_C_ULONG, &value, 0, &g_len);
        }

        void ODBC::Out(OUT short& value)
        {
	        value = 0;

	        SQLGetData(_hStmt, ++_paramCount, SQL_C_SHORT, &value, 0, &g_len);
        }

	    void ODBC::Out(OUT unsigned short& value)
        {
	        value = 0;

	        SQLGetData(_hStmt, ++_paramCount, SQL_C_USHORT, &value, 0, &g_len);
        }

        void ODBC::Out(OUT __int64& value)
        {
	        value = 0;

	        SQLGetData(_hStmt, ++_paramCount, SQL_C_SBIGINT, &value, 0, &g_len);
        }

        void ODBC::Out(OUT unsigned __int64& value)
        {
	        value = 0;

	        SQLGetData(_hStmt, ++_paramCount, SQL_C_UBIGINT, &value, 0, &g_len);
        }

	    void ODBC::Out(OUT bool& value)
        {
	        value = false;

	        SQLGetData(_hStmt, ++_paramCount, SQL_C_TINYINT, &value, 0, &g_len);
        }

        void ODBC::Out(OUT float& value)
        {
	        value = 0.0f;

	        SQLGetData(_hStmt, ++_paramCount, SQL_C_FLOAT, &value, 0, &g_len);
        }

        void ODBC::Out(OUT double& value)
        {
	        value = 0.0;

	        SQLGetData(_hStmt, ++_paramCount, SQL_C_DOUBLE, &value, 0, &g_len);
        }

        void ODBC::Out(OUT SYSTEMTIME& value)
        {
	        TIMESTAMP_STRUCT TimeStamp;
	        ZeroMemory(&TimeStamp, sizeof(TimeStamp));

	        SQLRETURN Ret = SQLGetData(_hStmt, ++_paramCount, SQL_C_TYPE_TIMESTAMP, &TimeStamp, 0, &g_len);
	        if (SQL_SUCCESS == Ret) 
	        {
		        value.wYear         = TimeStamp.year;
		        value.wDay          = TimeStamp.day;
		        value.wHour         = TimeStamp.hour;
		        value.wMinute       = TimeStamp.minute;
		        value.wMonth        = TimeStamp.month;
		        value.wSecond       = TimeStamp.second;
		        value.wMilliseconds = 0;
		        value.wDayOfWeek    = 0;
	        }
        }

		void ODBC::_OutString(OUT char* string, SQLULEN size)
		{
			string[0] = '\0';

			SQLGetData(_hStmt, ++_paramCount, SQL_C_CHAR, string, size, &g_len);
		}

		void ODBC::_OutString(OUT wchar_t* string, SQLULEN size)
		{
			string[0] = '\0';

			SQLGetData(_hStmt, ++_paramCount, SQL_C_WCHAR, string, size, &g_len);
		}

        bool ODBC::_Connect()
		{
			Disconnect();

			if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_hEnv))
				return _SqlError();

			if (SQL_SUCCESS != SQLSetEnvAttr(_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER))
				return _SqlError();

			if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, _hEnv, &_hDbc))
				return _SqlError();

			wchar_t     out[4096]  = L"";
			SQLSMALLINT	connStrOut = 0;
			if (SQL_SUCCESS != SQLDriverConnectW(_hDbc, NULL, _connection, SQL_NTS, out, _countof(out), &connStrOut, SQL_DRIVER_NOPROMPT))
				return _SqlError();

			return true;
		}


		void ODBC::_Prepare()
		{
			if (SQL_NULL_HSTMT != _hStmt)
			{
				if (SQL_SUCCESS != SQLFreeHandle(SQL_HANDLE_STMT, _hStmt))
					_SqlError();

				_hStmt = SQL_NULL_HSTMT;
			}

			if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, _hDbc, &_hStmt))
				_SqlError();
		}

        void ODBC::_AddParam()
        {
			if (0 == _paramCount)
				_query[_queryLen++] = L'(';
			else
				_query[_queryLen++] = L',';

			// param.
			_query[_queryLen++] = L'?';
			_query[_queryLen]   = L')';
			
			// end.
			_query[_queryLen + 1] = L'}';
			_query[_queryLen + 2] = '\0';
        }

        // 에러라서 무조건 리턴 false
        bool ODBC::_SqlError()
        {
            wchar_t		state[4096] = L"";
            wchar_t		error[4096] = L"";
	        SQLINTEGER	nativeError = 0;
	        SQLSMALLINT	lenth       = 0;
	
	        SQLErrorW(_hEnv, _hDbc, _hStmt, state, &nativeError, error, _countof(error), &lenth);

			if (_query[0] == '\0')
				DB_LOG(LOG_TYPE_CRITICAL, L"error= %s, \"%s\"", state, error);
			else
				DB_LOG(LOG_TYPE_CRITICAL, L"error= %s, \"%s\", query=\"%s\"", state, error, _query);
	
	        // 접속 장애.
	        if (0 == API::StringCompare(state, L"00000") ||
				0 == API::StringCompare(state, L"08006") ||
				0 == API::StringCompare(state, L"08003") ||
				0 == API::StringCompare(state, L"08S01") ||
				0 == API::StringCompare(state, L"HYT00"))
	        {
				/*
		        while (false == _Connect())
                {
					DB_LOG(LOG_TYPE_CRITICAL, L"DB reconnection fail");
					HNET::API::Sleep(300);
                }
				
				DB_LOG(LOG_TYPE_INFO, L"DB reconnection success");
				*/
			}
			else if (0 == API::StringCompare(state, L"IM002"))
			{
                const wchar_t* p = L"32bit";
			#ifdef _WIN64
				p = L"64bit";
			#endif
			
				DB_LOG(LOG_TYPE_CRITICAL, L"error= %s, \"check installation Connector/ODBC ( %s )\"", state, p);
			}
	
	        return false;
        }
    }
}