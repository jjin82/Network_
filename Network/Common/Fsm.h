#pragma once

#include <typeinfo>
#include <map>

class FsmParam
{

};

template<typename T>
class FsmState
{
public:
    FsmState(int type)
        : _type(type)
        , _preState(NULL) {}
    ~FsmState() {}

    virtual void Enter(T* owner, FsmParam* param = NULL) = 0;
    virtual void Exit(T* owner) = 0;
    virtual bool Excute(T* owner) = 0;      // 리턴: true(실행), false(중지)

public:
    int			  _type;       // 타입.      
    FsmState<T>* _preState;   // 이전의 상태.
};

template<typename T>
class Fsm
{
public:
    Fsm(T* owner)
		: _owner(owner)
    {
    }

    bool Regster(FsmState<T>* state, bool defaultState = false)
    {
		_states[state->_type] = state;

        // 기본 상태면 세팅.
        if (true == defaultState)
        {
            // 기본 상태 진입 처리.
            state->Enter(_owner);

            // 새로운 상태 세팅.
            _curState = state;
        }

        return true;
    }

    void Update()
    {
        if (0 == _states.size())
            return;

        _transition = false;

        // 실행 확인.(단 Excute 에서 상태가 변경이 없었을 때)
		if (_curState->Excute(_owner))
            return;

        // 상태가 변경 되서 이전 상태로 원복은 패스.
        if (_transition)
            return;

        // 이전 상태로 원복.
        RevertState();
    }

    void Change(int type, FsmParam* param = NULL)
    {
        // 같은 상태면 리턴.
		if (_curState->_type == type)
        {
			_curState->Exit(_owner);
			_curState->Enter(_owner, param);
            return;
        }

		// 돌아가야 될 상태와 같다면.
		if (_curState->_preState && _curState->_preState->_type == type)
		{
			_curState->Exit(_owner);
			_curState->_preState->Enter(_owner, param);
			_curState = _curState->_preState;
			return;
		}

        // 상태 변경 중이면 리턴.
        if (_transition)
            return;

        // 새로운 상태 획득.
		FsmState<T>* newState = GetState(type);
        if (NULL == newState)
            return;

        // 상태 변경 중.
        _transition = true;

        // 새로운 상태에 현재 상태를 이전 상태로 세팅.
        newState->_preState = _curState;

        // 현재 상태 퇴장 처리.
		_curState->Exit(_owner);

        // 새로운 상태 진입 처리.
		newState->Enter(_owner, param);

        // 새로운 상태 세팅.
        _curState = newState;
    }

    void RevertState()
    {
        _transition = true;

        // 현재 상태 퇴장 처리.
		_curState->Exit(_owner);

        // 원복 상태 진입 처리.
		_curState->_preState->Enter(_owner);

        // 원복 상태.
        _curState = _curState->_preState;
    }

    T Owner()
    {
        return _owner;
    }

    int FsmCurStateType()
    {
        return _curState->_type;
    }

    FsmState<T>* GetState(int type)
    {
        auto found = _states.find(type);
        if (found == _states.end())
            return NULL;

        return found->second;
    }

    /*
    string StateToString(int type)
    {
        FsmState state = GetState(type);
        return state.ToString();
    }
    */

private:
    T* _owner;

    bool _transition = false;
    FsmState<T>* _curState;
    std::map<int, FsmState<T>*> _states;
};

