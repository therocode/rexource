#include <catch.hpp>
#include <rex/threadpool.hpp>

SCENARIO("ThreadPool can be used to enqueue work which finishes asynchronously")
{
    GIVEN("a threadPool")
    {
        rex::ThreadPool threadPool(1);

        WHEN("a task is given")
        {
            std::string text;

            std::future<void> result = threadPool.enqueue([&text]
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                text = "hello";
            });

            THEN("the task is first not finished, but is later resolved")
            {
                CHECK_FALSE(text == "hello");
                CHECK(result.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout);

                result.wait();

                CHECK(result.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready);

                CHECK(text == "hello");
            }
        }
    }
}
