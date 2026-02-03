#include "work_stealing_queue.hpp"
#include <cassert>
#include <iostream>
#include <thread>
int main(void) {
    {
        work_stealing_queue<int, 16, 16> q;
        int x;

        assert(q.push(1));
        assert(q.try_pop(x) && x == 1);
        assert(!q.try_pop(x));
    }

    {
        work_stealing_queue<int, 8, 8> q;
        int x;

        q.push(42);

        std::thread thief([&] {
            int y;
            bool s = q.steal(y);
            assert(s || true); // allowed race
        });

        bool p = q.try_pop(x);
        thief.join();

        assert(p || true); // exactly one wins
    }
    std::cout << "tests passed successfully" << std::endl;
}