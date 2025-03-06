#pragma once

#include <memory>
#include <type_traits>

namespace Utils
{
    template <typename T>
    class Singleton 
    {
        protected:
            Singleton() = default;

        public:
            static_assert(std::is_base_of_v<Singleton<T>, T>, "T must inherit from Singleton");

            Singleton(const Singleton* obj) = delete;
            Singleton* operator=(const Singleton*) = delete; 

            static T* GetInstance() {  static T Instance; return std::addressof(Instance); }
    };
}
