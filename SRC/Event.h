#ifndef __EVENT_H__
#define __EVENT_H__

#include <unordered_set>
#include <functional>
#include <memory>

template <typename T>
class Event
{
    std::unordered_set<std::function<void(T)>> subscribers;

public:
    Event& operator+=(void(*func)(T))
    {
        subscribers.insert(func);
        return *this;
    }

    template<typename Obj>
    void subscribe(Obj* obj, void(Obj::* memFunc)(T))
    {
        subscribers.insert([=](T param) {
            (obj->*memFunc)(param);
            });
    }

    void Invoke(T param)
    {
        for (const auto& sub : subscribers)
        {
            sub(param);
        }
    }
};
#endif