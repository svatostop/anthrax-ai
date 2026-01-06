#pragma once
#include <stdio.h>
#include <deque>
#include <functional>
#include <utility>
#include "anthraxAI/utils/singleton.h"
#include <algorithm>
namespace Core
{
    typedef std::pair<int, std::function<void()>> DeletorPair;
    static inline DeletorPair Pair(int n, std::function<void()> function) { return std::make_pair(n, function); }
    class Deletor : public Utils::Singleton<Deletor>
    {
        std::deque<DeletorPair> Deletors;
        public:
            void Push(int n, std::function<void()>&& function) {
                Deletors.push_back(Core::Pair(n, function));
            }

            void CleanIf(int val) {
                auto it = std::remove_if(
                  Deletors.begin(), Deletors.end(),
                  [&](const DeletorPair &pair) {
                  if (pair.first == val) {
                      pair.second();
                      return true;
                  }
                  else {
                    return false;
                  }});
                Deletors.resize(std::distance(
                  Deletors.begin(), it));
            }

            void CleanAll() {
                for (auto it = Deletors.rbegin(); it != Deletors.rend(); it++) {
                    (*it).second();
                }
                Deletors.clear();
            }

        enum Type {
            NONE = 0,
            PIPELINE = 1,
            CMD,
            SYNC,
            DESC,
            SKINNING_DESC
        };

    };
}
