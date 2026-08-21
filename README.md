# C++ IOCP Network Library

Windows 환경에서 게임 서버 및 실시간 서버 통신을 위해 직접 설계하고 개발한
**C++ IOCP 기반 비동기 TCP 네트워크 라이브러리**입니다.

Winsock2와 IOCP(I/O Completion Port)를 기반으로 다수의 클라이언트 연결을 비동기 방식으로 처리합니다.

세션 관리, 패킷 송수신, 패킷 분할/병합 처리, Gathering Send, 패킷 디스패치 등의 기능을 네트워크 계층 내부에서 처리하도록 구성했습니다.

## 주요 기술

* C++
* Windows
* Winsock2
* TCP/IP
* IOCP (I/O Completion Port)
* Multi-Threading
* Overlapped I/O
* AcceptEx
* WSARecv / WSASend
* IPv4 / IPv6

## Architecture

```text
                    Application
                        │
              ┌─────────┴─────────┐
              │                   │
        HNetAcceptor        HNetConnector
          (Server)              (Client)
              │                   │
              └─────────┬─────────┘
                        │
                       HNet
                        │
          ┌─────────────┼─────────────┐
          │             │             │
     HNetSession    HNetFunctor   HNetPacket
          │
        HNetIo
          │
   ┌──────┼────────────────────────────┐
   │      │      │      │      │       │
 Accept  Recv   Send  Connect  Reconnect Disconnect
   │
 HNetIocp
   │
 Windows IOCP
```

## 주요 구성

### HNet

네트워크 시스템의 기본 클래스입니다.

IOCP, Session, Packet Handler 등의 네트워크 시스템을 통합 관리합니다.

주요 역할:

* IOCP 생성 및 관리
* Session 관리
* Packet Handler 등록
* Send / Broadcast
* 연결 및 연결 종료 처리
* Single / Multi Thread 처리 구조 지원

### HNetIocp

Windows IOCP를 관리합니다.

```text
Socket
   │
Overlapped I/O
   │
IO Completion Port
   │
Worker Thread
   │
HNetIo::Completion()
   │
Accept / Recv / Send
```

`CreateIoCompletionPort()`를 이용해 Completion Port를 생성하고, 복수의 Worker Thread에서 `GetQueuedCompletionStatus()`를 통해 완료된 I/O를 처리합니다.

I/O 완료 이벤트는 `HNetIo`를 통해 각각의 작업으로 전달됩니다.

### HNetSession

하나의 TCP Connection을 관리하는 Session 객체입니다.

Session 내부에서 다음 네트워크 I/O 객체를 관리합니다.

```text
HNetSession
 ├─ Listen
 ├─ Accept
 ├─ Connect
 ├─ Reconnect
 ├─ Recv
 ├─ Send
 └─ Disconnect
```

각 Session은 고유한 `NetId`를 가지며 서버에서는 이를 이용하여 연결된 클라이언트를 식별합니다.

## Asynchronous I/O

네트워크 I/O는 Windows Overlapped I/O 방식으로 동작합니다.

`HNetIo`가 `OVERLAPPED`를 상속하며 각 I/O 작업을 구분합니다.

```cpp
enum class IO_TYPE
{
    DISCONNECT,
    LISTEN,
    ACCEPT,
    CONNECT,
    RECONNECT,
    RECV,
    SEND,
    ACCEPT_WS,
    RECV_WS
};
```

IOCP에서 작업이 완료되면 해당 I/O Type에 따라 Session으로 전달됩니다.

```text
GetQueuedCompletionStatus()
            │
            ▼
 HNetIo::Completion()
            │
    ┌───────┼────────┐
    ▼       ▼        ▼
  Recv     Send    Accept
```

## TCP Packet 처리

TCP는 메시지 단위 프로토콜이 아니기 때문에 하나의 패킷이 여러 번에 나누어 수신되거나 여러 패킷이 한 번에 수신될 수 있습니다.

이를 처리하기 위해 Session별 Receive Buffer에서 수신 위치를 관리합니다.

```text
Receive Buffer

┌──────────┬──────────┬──────────────┐
│ Packet A │ Packet B │ Partial C    │
└──────────┴──────────┴──────────────┘
                         │
                         ▼
                 다음 Receive와 결합
```

완전한 패킷이 만들어지지 않은 경우 데이터를 보존하고 다음 Receive 결과와 연결합니다.

하나의 Receive Buffer에 여러 패킷이 포함된 경우에는 패킷 크기를 기준으로 반복해서 분리합니다.

이를 통해 TCP Packet Fragmentation / Coalescing 상황을 처리합니다.

## Packet Structure

기본 TCP 패킷은 다음과 같은 구조로 구성됩니다.

```text
┌──────────────────────┐
│ Size     : USHORT    │
├──────────────────────┤
│ Type     : USHORT    │
├──────────────────────┤
│ Payload              │
│ ...                  │
└──────────────────────┘
```

Packet Header에는 Packet Size와 Packet Type 정보를 저장합니다.

```cpp
class HNetPacket
{
protected:
    USHORT _size;
    USHORT _type;
};
```

Packet Type을 기반으로 수신 데이터를 Application의 Message Handler와 연결합니다.

## Packet Dispatch

패킷을 수신할 때마다 `switch` 문으로 분기하는 대신 Packet Type과 처리 함수를 미리 등록하는 방식으로 구성했습니다.

```text
Packet Type
    │
    ▼
Function Table
    │
    ├─ Type 1 → Handler A
    ├─ Type 2 → Handler B
    ├─ Type 3 → Handler C
    └─ ...
```

이를 통해 패킷 처리 로직과 네트워크 수신 로직을 분리했습니다.

## Gathering Send

작은 패킷이 연속해서 발생할 경우 패킷마다 `WSASend()`를 호출하면 Socket I/O 호출 횟수가 증가합니다.

이를 줄이기 위해 Send Queue에 들어온 데이터를 Gathering Buffer에 결합한 뒤 한 번의 I/O로 전송하도록 구현했습니다.

```text
Packet A ┐
Packet B ├─→ Gathering Buffer ─→ WSASend
Packet C │
Packet D ┘
```

작은 패킷이 빈번하게 발생하는 게임 서버 환경에서 Send I/O 호출 횟수를 줄이는 것을 목적으로 합니다.

## Connection Management

### Server

`HNetAcceptor`가 Server 역할을 담당합니다.

서버 시작 시 지정된 Session 수만큼 Session 객체를 생성하고 `AcceptEx()`를 요청합니다.

연결이 종료된 Session은 다시 Accept 상태로 전환하여 재사용합니다.

### Client

`HNetConnector`는 서버 접속 기능을 담당합니다.

주요 기능:

* Connect
* Disconnect
* Packet Send
* Packet Receive
* Reconnect

## Multi-Threading

IOCP Worker Thread가 완료된 네트워크 이벤트를 처리하는 구조입니다.

```text
                 IOCP
                   │
        ┌──────────┼──────────┐
        ▼          ▼          ▼
     Worker 1   Worker 2   Worker 3
        │          │          │
        └──────────┼──────────┘
                   ▼
             Network Event
```

Session의 Send Queue에는 동기화 처리를 적용하여 여러 Thread에서 동시에 Send가 요청되는 상황을 처리합니다.

## Single Thread Message Processing

네트워크 I/O와 Application Message 처리를 분리할 수 있도록 Job Queue 구조를 제공합니다.

```text
IOCP Worker Thread
        │
        ▼
     IO Job Queue
        │
        ▼
     HNet::Update()
        │
        ▼
 Application Thread
```

게임 서버에서 Network Thread와 Logic Thread를 분리해야 하는 구조에 적용할 수 있도록 설계했습니다.

## WebSocket

TCP Socket 외에 WebSocket 연결을 처리하기 위한 기능도 포함되어 있습니다.

* WebSocket Accept
* HTTP Upgrade Handshake
* `Sec-WebSocket-Key` 처리
* SHA1 / Base64 기반 `Sec-WebSocket-Accept`
* Binary Frame
* Mask 처리
* Ping / Pong
* Disconnect Frame

관련 클래스:

```text
HNetWsAcceptor
HNetIoWsAccept
HNetIoWsRecv
HNetWsPacket
```

## File Structure

```text
Network_
│
├─ HNetwork.h
│
├─ HNet.h / .cpp
│   └─ Network Core
│
├─ HNetIocp.h / .cpp
│   └─ IOCP / Worker Thread
│
├─ HNetSession.h / .cpp
│   └─ TCP Session
│
├─ HNetIo.h / .cpp
│   └─ Async I/O
│
├─ HNetPacket.h / .cpp
│   └─ Packet / Serialization
│
├─ HNetFunctor.h / .cpp
│   └─ Packet Handler Dispatch
│
├─ HNetAcceptor.h / .cpp
│   └─ TCP Server
│
├─ HNetConnector.h / .cpp
│   └─ TCP Client
│
├─ HNetWsAcceptor.h / .cpp
│   └─ WebSocket Server
│
└─ Common/
    └─ Common Utility
```

## Development Environment

* **OS:** Windows
* **Language:** C++
* **Compiler:** Microsoft Visual C++
* **Network:** Winsock2 / TCP/IP
* **I/O Model:** IOCP
* **Threading:** Multi-Threading

## Design Goals

단순한 Socket Wrapper가 아니라 게임 서버에서 반복적으로 필요한 네트워크 처리 영역을 하나의 계층으로 분리하는 것을 목표로 개발했습니다.

주요 설계 요소는 다음과 같습니다.

* IOCP 기반 비동기 네트워크 처리
* 다중 Session 관리
* TCP 패킷 경계 처리
* Packet Type 기반 Message Dispatch
* Gathering Send를 통한 Send I/O 최적화
* Network Thread와 Logic Thread 분리 가능 구조
* Session 재사용
* Client Reconnect
* IPv4 / IPv6 지원
* WebSocket 지원

상위 Application에서는 Socket이나 IOCP의 세부 구현을 직접 처리하지 않고 네트워크 라이브러리가 제공하는 인터페이스를 통해 연결 및 메시지를 처리할 수 있도록 구성했습니다.
