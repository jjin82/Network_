#pragma once

#include "HNetCommon.h"

class WsaSend;

#pragma pack(push, 1)

//■=============================================================================================■
//   HNetPacket. 
//  
//   - 무조건 상속을 해야 사용 가능.
//■=============================================================================================■
class HNetPacket
{
public:
    static const USHORT MAX_SIZE     = 32500;
    static const USHORT MAX_TYPE     = (USHRT_MAX - 1);
    static const USHORT INVALID_TYPE = USHRT_MAX;

private:
	friend class HNetPacketEx;
    friend class HNetStreamEx;
    T2(PACKET, T) friend class RefPacket;

protected:
    HNetPacket(USHORT type);
    ~HNetPacket();

private:
	WsaSend* operator < (WsaSend* wsaSend);
	void* operator new(size_t size, WsaSend*& wsaSend);

public:
	bool Check() const;

    USHORT Size() const;
    USHORT Type() const;
    USHORT Lenth() const;

protected:
    USHORT _size;
    USHORT _type;
};


//■=============================================================================================■
//   HNetPacketEx. 
//■=============================================================================================■
class HNetPacketEx : public HNetPacket
{
    T2(P, T) friend class RefPacket;

protected:
	HNetPacketEx();
    HNetPacketEx(USHORT type);

public:
    bool Success();
	bool Empty();

    T1(T) void In(T v)        
    { 
        _size += _buf.In(v); 
    }

    T1(T) void Out(T& v) const 
    { 
        _buf.Out(v); 
    }

private:
    mutable HLIB::Buffer _buf;
};


//■=============================================================================================■
//   HNetStream. 
//■=============================================================================================■
class HNetStream
{
public:
    USHORT _type;
    byte*  _buf;
    USHORT _lenth;
};


//■=============================================================================================■
//   HNetStreamEx. 
//■=============================================================================================■
class HNetStreamEx : public HNetPacket
{
    T2(P, T) friend class RefPacket;
    enum { LENTH = 4000 };

protected:
    HNetStreamEx(USHORT type);

public:
    bool Set(byte* buf, USHORT lenth);
    operator HNetStream();

private:
    byte _buf[LENTH];
};


//■=============================================================================================■
//   HNetWsPacket.
//  
//   - 무조건 상속을 해야 사용 가능.
//■=============================================================================================■
class HNetWsPacket
{
    friend class HNetWsPacketEx;
    T2(P, T) friend class RefPacket;

public:
    class Header
    {
    public:
        enum WS_HEADER_INFO
        {
            STRING_TYPE       = 0x81,  // 문자열 타입.
            BINARY_TYPE       = 0x82,  // 바이너리 타입.
            PING_TYPE         = 0x89,  // 핑 타입.
            OPCODE_DISCONNECT = 8,     // 접속 종료.
            OPCODE_PING       = 9,     // 핑.
            OPCODE_PONG       = 10,    // 퐁.
        };

        static size_t MaxHeaderSize();

    public:
        void Initialize(USHORT lenth);
        bool Check() const;

        USHORT Size() const;
        USHORT DataSize() const;
        byte Opcode() const;

        char* Decrypt();
        HNetWsPacket* ToPacket();

        bool IsMask() const;
        bool IsExtend() const;
        bool IsExtend_126() const;
        bool IsExtend_127() const;
        bool IsFinish() const;
        bool IsDisconnect() const;
        bool IsPing() const;
        bool IsPong() const;

    private:
        byte _fin_opcode;
        byte _mask_lenth;
    };

protected:
    HNetWsPacket(USHORT type);
    ~HNetWsPacket();

private:
	WsaSend* operator < (WsaSend* wsaSend);
	void* operator new(size_t size, WsaSend*& wsaSend);
    
public:
	bool Check() const;

    USHORT Size() const;
    USHORT Type() const;
	
private:
    USHORT _size;
    USHORT _type;
};

//■=============================================================================================■
//   HNetWsPacketEx. 
//■=============================================================================================■
class HNetWsPacketEx : public HNetWsPacket
{
    T2(P, T) friend class RefPacket;

protected:
	HNetWsPacketEx();
    HNetWsPacketEx(USHORT type);

public:
    bool Success();
    bool Empty();

    T1(T) void In(T v)         
    { 
        _size += _buf.In(v); 
    }

    T1(T) void Out(T& v) const 
    { 
        _buf.Out(v); 
    }

private:
    mutable HLIB::Buffer _buf;
};

#pragma pack(pop)

//■=============================================================================================■
//   RefPacket: 패킷 레퍼.
//■=============================================================================================■
T2(P, T) class RefPacket
{
    friend class HNetAcceptor;
    friend class HNetWsAcceptor;
    friend class HNetConnector;

public:
    RefPacket()          
        : _packet(new(_wsaSend) P)
    { 
        // 검증 코드.(컴파일 에러)
        static T* p = _packet;
    }
	
    RefPacket(USHORT type)
        : _packet(new(_wsaSend) P(type))
    { 
        // 검증 코드.(컴파일 에러)
        static T* p = _packet; 
    }

#if defined(_MSC_VER)
    #if (_MSC_VER >= 1910)
        template<typename... V>
        RefPacket(V... v)
            : _packet(new(_wsaSend) P(v...))
        {
            // 검증 코드.(컴파일 에러)
            static T* p = _packet;
        }
    #endif
#endif

    ~RefPacket()
    { 
        _wsaSend->~WsaSend();
    }
    
    P* operator->()
    { 
        return _packet; 
    }

    void Reuse()
    { 
        USHORT type = _packet->_type;
        _wsaSend->~WsaSend();
        
        // 할당과 패킷 타입 복구.
        _packet        = new(_wsaSend) P;
        _packet->_type = type;
    }

private: 
    operator WsaSend*()
    { 
        return *_packet < _wsaSend; 
    }

private:
    P*       _packet;
    WsaSend* _wsaSend;
};

namespace HNET
{
    // 패킷 인터페이스.
    typedef HNetPacket       Packet;
    typedef HNetPacketEx     PacketEx;

    //■=============================================================================================■
    //   NewPacketT: 템플릿 패킷 생성.
    //■=============================================================================================■
    T1(P) class NewPacketT : public RefPacket<P, HNetPacket>
    {
    public:
        NewPacketT() {}
        NewPacketT(USHORT type) : RefPacket<P, HNetPacket>(type) {}

#if defined(_MSC_VER)
    #if (_MSC_VER >= 1910)
        template<typename... V> 
        NewPacketT(V... v) : RefPacket<P, HNetPacket>(v...) {}
    #endif
#endif
    };

    //■=============================================================================================■
    //   NewPacket: HNetPacketEx 패킷 생성.
    //■=============================================================================================■
    class NewPacket : public NewPacketT<HNetPacketEx>
    {
    public:
        NewPacket(USHORT type) : NewPacketT(type) {}
    };

    //■=============================================================================================■
    //   NewStream: HNetStreamEx 패킷 생성.
    //■=============================================================================================■
    class NewStream : public NewPacketT<HNetStreamEx>
    {
    public:
        NewStream(USHORT type) : NewPacketT(type) {}
    };
}

#ifdef HNETWORK_FULL_SOURCE

namespace HNET
{
	// 패킷 인터페이스.
    typedef HNetWsPacket WsPacket;
    
    //■=============================================================================================■
    //   WsNewPacket: 웹 패킷 생성.
    //■=============================================================================================■
    T1(P) class NewWsPacketT : public RefPacket<P, HNET::WsPacket>
    {
    public:
        NewWsPacketT() {}
        NewWsPacketT(USHORT type) : RefPacket<P, HNET::WsPacket>(type) {}
    };

    //■=============================================================================================■
    //   NewPacket: HNetPacketEx 패킷 생성.
    //■=============================================================================================■
    class NewWsPacket : public NewWsPacketT<HNetWsPacketEx>
    {
    public:
        NewWsPacket(USHORT type) : NewWsPacketT(type) {}
    };
}
#endif

//■=============================================================================================■
//   WsaSend.
//■=============================================================================================■
class WsaSend : public WSABUF
{
public:
    ~WsaSend();
    
    void Initialize(CHAR* wsaBuf = NULL);

    void Reference();
    void Release();

    bool NotEncrypt();

    USHORT Type();
    char*  Body();
    USHORT BodyLenth();

public:
    long _refCount;
    long _count;
    bool _encrypt;
};