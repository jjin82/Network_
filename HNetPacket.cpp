#include "HNetPacket.h"

const long   HNET_PACKET_DEFAULT_REFERENCE  = 10000000;

//■=============================================================================================■
//   HNetPacket. 
//■=============================================================================================■
HNetPacket::HNetPacket(USHORT type)
    : _type(type)
{
    if (MAX_TYPE < type)
    {
        _type = INVALID_TYPE;
        HAPI::MsgBox(L"error", L"invalid packet type= %d (max type= %d)", type, MAX_TYPE);
    }
}

HNetPacket::~HNetPacket()
{

}

bool HNetPacket::Check() const
{
    if (   INVALID_TYPE       == _type
        || sizeof(HNetPacket)  > _size
        || MAX_SIZE            < _size)
    {
        return false;
    }

    return true;
}

USHORT HNetPacket::Size() const
{
    return _size;
}

USHORT HNetPacket::Type() const
{
    return _type;
}

USHORT HNetPacket::Lenth() const
{
    return _size - sizeof(*this);
}

WsaSend* HNetPacket::operator < (WsaSend* wsaSend)
{
	if (false == Check())
		return NULL;

	wsaSend->len = _size;

	return wsaSend;
}

void* HNetPacket::operator new(size_t size, WsaSend*& wsaSend)
{
    wsaSend            = (WsaSend*)HAPI::Alloc(sizeof(WsaSend) + size);
    wsaSend->buf       = (char*)(wsaSend + 1);
    wsaSend->_refCount = HNET_PACKET_DEFAULT_REFERENCE;
    wsaSend->_count    = 0;
    wsaSend->_encrypt  = false;

    *(USHORT*)(wsaSend->buf) = (USHORT)size;

    return wsaSend->buf;
}

//■=============================================================================================■
//   HNetPacketEx. 
//■=============================================================================================■
HNetPacketEx::HNetPacketEx()
	: HNetPacket(0)
{
	if (sizeof(HNetPacketEx) == _size)
	{
		_size = (sizeof(HNetPacket) + _buf.Offset());
		return;
	}

    // 어떠한 멤버 변수도 가지면 안된다
    HAPI::MsgBox(L"error", L"do not have a member variable(packet type= %d)", _type);
}

HNetPacketEx::HNetPacketEx(USHORT type)
	: HNetPacket(type)
{
    if (sizeof(HNetPacketEx) == _size)
    {
        _size = (sizeof(HNetPacket) + _buf.Offset());
        return;
    }

    // 어떠한 멤버 변수도 가지면 안된다
    HAPI::MsgBox(L"error", L"do not have a member variable(packet type= %d)", _type);
}

bool HNetPacketEx::Success()
{
    USHORT rollbackSize = 0;
    if (false == _buf.InSuccess(rollbackSize))
    {
        _size -= rollbackSize;
        return false; 
    }

    return true;
}

bool HNetPacketEx::Empty()
{
	return _buf.OutEmpty();
}

//■=============================================================================================■
//   HNetStreamEx. 
//■=============================================================================================■
HNetStreamEx::HNetStreamEx(USHORT type)
    : HNetPacket(type)
{
    if (sizeof(HNetStreamEx) == _size)
    {
        _size = sizeof(HNetPacket);
        return;
    }

    // 어떠한 멤버 변수도 가지면 안된다
    HAPI::MsgBox(L"error", L"do not have a member variable(packet type= %d)", _type);
}

bool HNetStreamEx::Set(byte* buf, USHORT lenth)
{
    if (LENTH < lenth)
        return false;

    memcpy(_buf, buf, lenth);

    _size += lenth;

    return true;
}

HNetStreamEx::operator HNetStream()
{
    HNetStream stream;
    stream._type  = _type;
    stream._buf   = _buf;
    stream._lenth = (_size - sizeof(HNetPacket));
    
    return stream;
}

//■=============================================================================================■
//   Header.
//■=============================================================================================■
enum HNET_WS_HEADER_INFO
{
    MASK_BYTES          = 4,                   // 마스트 크기.

    EXTEND_126          = 126,                 // 확장 플레그 값. (2byte)
    EXTEND_BYTES_126    = 2,                   // 확장 크기. (2byte) 
    PAYLOAD_LEN_126     = 126,                 // 126 이상이면 확장 크기. (2byte)

    EXTEND_127          = 127,                 // 대용량 확장 플레그 값. (8byte)
    EXTEND_BYTES_127    = 8,                   // 확장 크기. (8byte)
    PAYLOAD_LEN_127     = SHRT_MAX,            // SHRT_MAX 이상이면 확장 크기. (8byte)

    EXTEND_MAX_BYTES    = EXTEND_BYTES_127     // 확장시 최대 증가 크기. (8byte)
};

void HNetWsPacket::Header::Initialize(USHORT dataSize)
{
    // +---------------------------------------------------------------+
    //                1               2               3               4| ( byte )
    //  1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8| ( bit  )
    // +-+-+-+-+-------+-+-------------+-------------------------------+
    // |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
    // |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
    // |N|V|V|V|       |S|             |   (if payload len==126/127)   |
    // | |1|2|3|       |K|             |                               |
    // +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
    // |     Extended payload length continued, if payload len == 127  |
    // + - - - - - - - - - - - - - - - +-------------------------------+
    // |                               |Masking-key, if MASK set to 1  |
    // +-------------------------------+-------------------------------+
    // | Masking-key (continued)       |          Payload Data         |
    // +-------------------------------- - - - - - - - - - - - - - - - +
    // :                     Payload Data continued ...                :
    // + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
    // |                     Payload Data continued ...                |
    // +---------------------------------------------------------------+

    _fin_opcode = BINARY_TYPE;

    // 확장해야 되는가 아닌가.
    if (PAYLOAD_LEN_127 <= dataSize)
    {
        _mask_lenth = EXTEND_127;
        __int64* size = (__int64*)(this + 1);
        *size = htonll(dataSize);
    }
    else if (PAYLOAD_LEN_126 <= dataSize)
    {
        _mask_lenth = EXTEND_126;
        USHORT* size = (USHORT*)(this + 1);
        *size = htons((USHORT)dataSize);
    }
    else
    {
        _mask_lenth = (byte)dataSize;
    }
}

bool HNetWsPacket::Header::Check() const
{
    USHORT size = Size();

    // 기본 헤더 크기 검사.
    if (sizeof(Header) > size)
        return false;

    if (HNetPacket::MAX_SIZE < size)
        return false;

    return true;
}

USHORT HNetWsPacket::Header::Size() const
{
    // 기본 헤더의 크기.
    USHORT headerSize = sizeof(Header);

    // 확장된 헤더라면 크기 추가.
    if (IsExtend_126())
    {
        headerSize += EXTEND_BYTES_126;
    }
    else if (IsExtend_127())
    {
        headerSize += EXTEND_BYTES_127;
    }

    // 마스크 값이 있는 헤더라면 크기 추가.
    if (IsMask())
        headerSize += MASK_BYTES;

    return (headerSize + DataSize());
}

USHORT HNetWsPacket::Header::DataSize() const
{
    // _mask_lenth의 뒤 7비트가 사이즈이다.
    // 그래서 0x7F( 0111 1111 ) 이거로 비트 연산하여 크기를 얻어낸다.
    USHORT size = (_mask_lenth & 0x7F);

    if (EXTEND_126 == size)
    {
        USHORT* p = (USHORT*)(this + 1);
        size = ntohs(*p);
    }
    else if (EXTEND_127 == size)
    {
        __int64* p = (__int64*)(this + 1);
        __int64 ll = ntohll(*p);
        if (ll > 0xFFFFFFFF)
            return 0;

        size = (USHORT)ll;
    }

    return size;
}

byte HNetWsPacket::Header::Opcode() const
{
    return (_fin_opcode & 0x0F);
}

size_t HNetWsPacket::Header::MaxHeaderSize()
{
    // header의 최대 크기(크기 정보가 최대 EXTEND_MAX_BYTES(8byte) 까지 확장된다)
    return (sizeof(Header) + EXTEND_MAX_BYTES);
}

char* HNetWsPacket::Header::Decrypt()
{
    // 기본 데이터 위치.
    char* data = (char*)(this + 1);

    // 패킷의 헤더를 확장해야되는가.
    if (IsExtend_126())
    {
        data += EXTEND_BYTES_126;
    }
    else if (IsExtend_127())
    {
        data += EXTEND_BYTES_127;
    }

    // 마스크 값이 있는가.
    if (IsMask())
    {
        char* maskKey = data;
        data += MASK_BYTES;

        // data decrypt.
        USHORT dataSize = DataSize();
        for (USHORT i = 0; i < dataSize; ++i)
        {
            data[i] ^= maskKey[i % MASK_BYTES];
        }
    }

    return data;
}

HNetWsPacket* HNetWsPacket::Header::ToPacket()
{
    // 데이터 위치.
    char* data = Decrypt();

    // HNetWsPacket 포멧에 맞게 데이터 앞쪽 sizeof(USHORT) 크기의 버퍼를 패킷 크기로 사용.
    data -= sizeof(USHORT);
    *(USHORT*)data = Size();

    return (HNetWsPacket*)data;
}

bool HNetWsPacket::Header::IsMask() const
{
    return 0 < (_mask_lenth & 0x80);
}

bool HNetWsPacket::Header::IsExtend() const
{
    return (IsExtend_126() || IsExtend_127());
}

bool HNetWsPacket::Header::IsExtend_126() const
{
    // _mask_lenth의 뒤 7비트가 사이즈이다.
    // 그래서 0x7F( 0111 1111 ) 이거로 비트 연산하여 크기를 얻어낸다.

    return (EXTEND_126 == (_mask_lenth & 0x7F));
}

bool HNetWsPacket::Header::IsExtend_127() const
{
    // _mask_lenth의 뒤 7비트가 사이즈이다.
    // 그래서 0x7F( 0111 1111 ) 이거로 비트 연산하여 크기를 얻어낸다.

    return (EXTEND_127 == (_mask_lenth & 0x7F));
}

bool HNetWsPacket::Header::IsFinish() const
{
    return 0 < (_fin_opcode & 0x80);
}

bool HNetWsPacket::Header::IsDisconnect() const
{
    return (OPCODE_DISCONNECT == Opcode());
}

bool HNetWsPacket::Header::IsPing() const
{
    return (OPCODE_PING == Opcode());
}

bool HNetWsPacket::Header::IsPong() const
{
    return (OPCODE_PONG == Opcode());
}

//■=============================================================================================■
//   HNetWsPacket. 
//■=============================================================================================■
HNetWsPacket::HNetWsPacket(USHORT type)
    : _size(0)
    , _type(type)
{

}

HNetWsPacket::~HNetWsPacket()
{

}

bool HNetWsPacket::Check() const
{
    if (sizeof(HNetWsPacket) > _size)
        return false;

    if (HNetPacket::MAX_SIZE < _size)
        return false;

    if (HNetPacket::INVALID_TYPE == _type)
        return false;

    return true;
}

USHORT HNetWsPacket::Size() const
{
    return _size;
}

USHORT HNetWsPacket::Type() const
{
    return _type;
}

WsaSend* HNetWsPacket::operator < (WsaSend* wsaSend)
{
    if (false == Check())
        return NULL;

    if (NULL == wsaSend->buf)
    {
        // +----------------------------------------------------------------+
        // |   WsaSend   |   header(10byte)   |   type(2byte)   |   buffer  |
        // |                                  |               data          |
        // +----------------------------------------------------------------+

        // 헤더의 기본 크기.
        int headerSize = sizeof(Header);

        // 데이터 크기. (NetSize는 내부에서만 사용하고 송신하지 않으므로 데이터 크기에서 제외시켜준다)
        USHORT dataSize = (_size - sizeof(USHORT));

        // 5. 크기에 따른 헤더 확장 시 "헤더의 위치 변경"과 "송신 크기" 변경.
        if (PAYLOAD_LEN_127 <= dataSize)
        {
            headerSize += EXTEND_BYTES_127;
        }
        else if (PAYLOAD_LEN_126 <= dataSize)
        {
            headerSize += EXTEND_BYTES_126;
        }

        // 1. header 위치로.
        wsaSend->buf = (char*)(wsaSend + 1);

        // 2. header의 크기에 맞게 메모리 위치 및 크기 세팅.
        wsaSend->buf += (Header::MaxHeaderSize() - headerSize);
        wsaSend->len  = (headerSize + dataSize);

        // 웹소켓 규약에 맞게 패킷 초기화.
        ((Header*)wsaSend->buf)->Initialize(dataSize);
    }

    return wsaSend;
}

void* HNetWsPacket::operator new(size_t size, WsaSend*& wsaSend)
{
    // +----------------------------------------------------------------+
    // |   WsaSend   |   header(10byte)   |   type(2byte)   |   buffer  |
    // |                                  |               data          |
    // +----------------------------------------------------------------+

    // 메모리 할당 및 초기화.
    wsaSend = (WsaSend*)HAPI::Alloc(sizeof(WsaSend) + Header::MaxHeaderSize() + size);
    wsaSend->Initialize();

    // 1. header 위치로.
    char* packet = (char*)(wsaSend + 1);

    // 2. type 위치로.
    packet += Header::MaxHeaderSize();

    // 3. 내부에서 사용되는 패킷의 _size 변수를 header의 메모리를 사용하도록 이동.
    packet -= sizeof(USHORT);

    // 4. 패킷의 _size 변수에 값 세팅.
    ((HNetWsPacket*)packet)->_size = (USHORT)size;

    return packet;
}

//■=============================================================================================■
//   HNetWsPacketEx. 
//■=============================================================================================■
HNetWsPacketEx::HNetWsPacketEx()
    : HNetWsPacket(0)
{
    if (sizeof(HNetWsPacketEx) == _size)
    {
        _size = (sizeof(HNetWsPacketEx) + _buf.Offset());
        return;
    }

    // 어떠한 멤버 변수도 가지면 안된다
    HAPI::MsgBox(L"error", L"do not have a member variable(packet type= %d)", _type);
}

HNetWsPacketEx::HNetWsPacketEx(USHORT type)
    : HNetWsPacket(type)
{
    if (sizeof(HNetWsPacketEx) == _size)
    {
        _size = (sizeof(HNetWsPacketEx) + _buf.Offset());
        return;
    }

    // 어떠한 멤버 변수도 가지면 안된다
    HAPI::MsgBox(L"error", L"do not have a member variable(packet type= %d)", _type);
}

bool HNetWsPacketEx::Success()
{
    USHORT rollbackSize = 0;
    if (false == _buf.InSuccess(rollbackSize))
    {
        _size -= rollbackSize;
        return false;
    }

    return true;
}

bool HNetWsPacketEx::Empty()
{
    return _buf.OutEmpty();
}

//■=============================================================================================■
//   WsaSend. 
//■=============================================================================================■
WsaSend::~WsaSend()
{
    if (0 == InterlockedAdd(&_refCount, -(HNET_PACKET_DEFAULT_REFERENCE - _count)))
        HAPI::Dealloc(this);
}

void WsaSend::Initialize(CHAR* wsaBuf)
{
    WSABUF::buf = wsaBuf;
    WSABUF::len = 0;
    _refCount   = HNET_PACKET_DEFAULT_REFERENCE;
    _count      = 0;
    _encrypt    = false;
}

void WsaSend::Reference()
{
    ++_count;
}

void WsaSend::Release()
{ 
    if (0 == InterlockedDecrement(&_refCount))
        HAPI::Dealloc(this);
}

bool WsaSend::NotEncrypt()
{
    if (_encrypt)
        return false;
 
    return (_encrypt = true);
}

USHORT WsaSend::Type()
{
    return *(USHORT*)(buf + sizeof(USHORT));
}

char* WsaSend::Body()
{
    return (buf + sizeof(HNetPacket));
}

USHORT WsaSend::BodyLenth()
{
    return (USHORT)(len - sizeof(HNetPacket));
}