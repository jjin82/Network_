#pragma once

#include "Memory.h"

unsigned long GetEntityKey();

namespace HNET
{
    namespace LIB
    {
        //■=============================================================================================■
        //   오브젝트
        //   - 각 오브젝트는 고유키가 발급됨.
        //■=============================================================================================■
        template<typename T>
        class Entity : public Memory
        {
        public:
	        Entity();
	        ~Entity();

	        const unsigned long GetKey()  { return _key; }
	
	        bool operator==(Entity &e) { return (_key == e._key); }
	        bool operator!=(Entity &e) { return (_key != e._key); }

        private: 
	        const unsigned long _key;
        };

        template<typename T>
        Entity<T>::Entity()
            : _key(GetEntityKey())
        {

        }

        template<typename T>
        Entity<T>::~Entity()
        {

        }
    }
}