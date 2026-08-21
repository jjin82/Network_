#pragma once

#include <sql.h>
#include <sqlext.h>
#include "RWLock.h"

namespace HNET
{
    namespace LIB
    {
        class ODBC : public RWLock
        {
	        enum { MAX_DNS = 1024, MAX_USER = 1024, MAX_TIMESTAMP_NUM = 1024 };

        public:
	        ODBC();
	        ~ODBC();

			bool Connect(const wchar_t* driver, const wchar_t* database, const wchar_t* userId, const wchar_t* pw);
			bool Connect(const wchar_t* ip, const wchar_t* driver, const wchar_t* database, const wchar_t* userId, const wchar_t* pw);
            bool Connect(const wchar_t* ip, int port, const wchar_t* driver, const wchar_t* database, const wchar_t* userId, const wchar_t* pw);
			void Disconnect();

			bool SetProcedure(const wchar_t* procedure);
			bool Execute(const wchar_t* query = NULL, ...);
	        bool Fetch();
	        bool NextRecord();

			void In(char value);
			void In(unsigned char value);
			void In(int value);
			void In(unsigned int value);
			void In(long value);
			void In(unsigned long value);
			void In(short value);
			void In(unsigned short value);
			void In(__int64 value);
			void In(unsigned __int64 value);
			void In(bool value);
			void In(float value);
			void In(double value);
			void In(SYSTEMTIME& value);
			void In(char* string, SQLULEN size);
			void In(wchar_t* string, SQLULEN size);
	
		    void Out(OUT char& value);
	        void Out(OUT unsigned char& value);
	        void Out(OUT int& value);
	        void Out(OUT unsigned int& value);
		    void Out(OUT long& value);
	        void Out(OUT unsigned long& value);
	        void Out(OUT short& value);
		    void Out(OUT unsigned short& value);
	        void Out(OUT __int64& value);
	        void Out(OUT unsigned __int64& value);
	        void Out(OUT bool& value);
	        void Out(OUT float& value);
	        void Out(OUT double& value);
	        void Out(OUT SYSTEMTIME& value);
			template<int N> void Out(OUT char(&string)[N])    { _OutString(string, N); }
			template<int N> void Out(OUT wchar_t(&string)[N]) { _OutString(string, N); }
		
		private:
			void _OutString(OUT char* string, SQLULEN size);
			void _OutString(OUT wchar_t* string, SQLULEN size);
	
        private:
			bool _Connect();
			void _Prepare();
	        void _AddParam();
	        bool _SqlError();

        private:
	        SQLHENV          _hEnv;
	        SQLHDBC          _hDbc;
	        SQLHSTMT         _hStmt;

            wchar_t          _connection[1024];
            wchar_t          _query[16384];
			size_t           _queryLen;
	        SQLUSMALLINT	 _paramCount;

	        LONG             _timeStampCount;
	        TIMESTAMP_STRUCT _timeStamps[MAX_TIMESTAMP_NUM];

	        Logger*          _log;
        };
    }
}