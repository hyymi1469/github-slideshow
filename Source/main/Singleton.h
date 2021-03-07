#pragma once

template< typename T >
class Singleton
{
    static T* _Singleton;

public:
    Singleton(){}
    virtual ~Singleton(){}

 
    static T* GetSingleton()
    {
        if ( nullptr == _Singleton )
            _Singleton = new T;

        return _Singleton;
    }

    static void Release()
    {
        delete _Singleton;
        _Singleton = nullptr;
    }
};

template<typename T> T* Singleton<T>::_Singleton = nullptr;
