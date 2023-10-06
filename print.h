#include <shared_mutex>
#include <string>
#include <vector>
#include <functional>

namespace Print
{
    class Buffer
    {
        public:
            void Add(const std::string& line);
            void Dump();
            void Clear();
            bool Available();
        private:
            mutable std::shared_mutex buffLock;
            std::vector<std::string> buff;
    };
}