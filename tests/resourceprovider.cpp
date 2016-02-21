#include <catch.hpp>
#include <map>
#include "helpers/person.hpp"
#include "helpers/tool.hpp"
#include "helpers/peoplesource.hpp"
#include "helpers/toolsource.hpp"
#include <rex/resourceprovider.hpp>

#include "helpers/treefilesource.hpp"

SCENARIO("ResourceProvider can manage sources")
{
    GIVEN("an empty resource provider")
    {
        rex::ResourceProvider provider;

        WHEN("no source is added")
        {
            THEN("accessing a source causes an exception")
            {
                CHECK_THROWS_AS(provider.source<PeopleSource>("people"), rex::InvalidSourceException);
            }
        }

        WHEN("a source is added")
        {
            rex::SourceView<PeopleSource> added = provider.addSource("people", PeopleSource("tests/data/people", false));

            THEN("the return value's id is the added source's name")
            {
                CHECK(added.id == "people");
            }

            THEN("the return value's 'source' attribute is the source object instance")
            {
                CHECK(added.source.path() == "tests/data/people");
            }

            THEN("accessing the source from the provider gives a view with same source id as when returned")
            {
                CHECK(provider.source<PeopleSource>("people").id == added.id);
            }

            THEN("accessing the source with the wrong type causes an exception")
            {
                CHECK_THROWS_AS(provider.source<ToolSource>("people"), rex::InvalidSourceException);
            }
        }

        WHEN("two sources are added and listed")
        {
            provider.addSource("people", PeopleSource("tests/data/people", false));
            provider.addSource("tools", ToolSource("tests/data/tools"));

            std::vector<std::string> sources = provider.sources();

            THEN("the list contains the sources added before")
            {
                std::set<std::string> ordered
                {
                    sources[0],
                    sources[1],
                };

                REQUIRE(sources.size() == 2);
                CHECK(ordered.count("people") == 1);
                CHECK(ordered.count("tools") == 1);
            }

        }

        WHEN("a source is added and removed")
        {
            provider.addSource("people", PeopleSource("tests/data/people", false));
            bool removed = provider.removeSource("people");

            THEN("the function reports a removal")
            {
                CHECK(removed);
            }

            THEN("accessing that source throws an exception")
            {
                CHECK_THROWS_AS(provider.source<PeopleSource>("people"), rex::InvalidSourceException);
            }
        }

        WHEN("a source that doesn't exist is removed")
        {
            bool removed = provider.removeSource("people");

            THEN("the function does not report a removal")
            {
                CHECK_FALSE(removed);
            }
        }

        WHEN("a source is added twice")
        {
            THEN("an exception is thrown")
            {
                provider.addSource("people", PeopleSource("tests/data/people", false));
                CHECK_THROWS_AS(provider.addSource("people", PeopleSource("tests/data/people", false)), rex::InvalidSourceException);
            }
        }

        WHEN("two sources are added and cleared, and subsequently accessed")
        {
            THEN("exceptions are thrown")
            {
                provider.addSource("people", PeopleSource("tests/data/people", false));
                provider.addSource("tools", ToolSource("tests/data/tools"));
                provider.clearSources();

                CHECK_THROWS_AS(provider.source<PeopleSource>("people"), rex::InvalidSourceException);
                CHECK_THROWS_AS(provider.source<ToolSource>("tools"), rex::InvalidSourceException);
            }
        }
    }
}

SCENARIO("ResourceProvider can be used to list available resources in a source")
{
    GIVEN("a resource provider with a source added")
    {
        rex::ResourceProvider provider;

        provider.addSource("people", PeopleSource("tests/data/people", false));
        
        WHEN("the provider is asked to list all resources within the source")
        {
            std::vector<std::string> list = provider.list("people");

            std::set<std::string> sortedList =
            {
                {list[0]},
                {list[1]},
                {list[2]},
            };

            CHECK(sortedList.size() == 3);
            CHECK(sortedList.count("kalle") != 0);
            CHECK(sortedList.count("anders") != 0);
            CHECK(sortedList.count("torsten") != 0);
        }

        WHEN("resources are listed from an invalid source")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.list("tools"), rex::InvalidSourceException);
            }
        }
    }
}

SCENARIO("ResourceProvider can be used to access resources synchronously from sources")
{
    GIVEN("a resource provider with a source added")
    {
        rex::ResourceProvider provider;

        provider.addSource("people", PeopleSource("tests/data/people", false));

        WHEN("valid resources are gotten")
        {
            const Person& person = provider.get<Person>("people", "anders");

            THEN("the values are correct")
            {
                CHECK(person.name == "anders");
                CHECK(person.age == 47);
            }
        }

        WHEN("more than one valid resource are gotten")
        {
            std::vector<rex::ResourceView<Person>> people = provider.get<Person>("people", std::vector<std::string>{"anders", "torsten"});

            REQUIRE(people.size() == 2);

            std::map<std::string, int32_t> peopleSorted =
            {
                {people[0].resource.name, people[0].resource.age},
                {people[1].resource.name, people[1].resource.age},
            };

            REQUIRE(peopleSorted.size() == 2);

            auto iter = peopleSorted.begin();
            CHECK(iter->first == "anders");
            CHECK(iter->second == 47);
            ++iter;
            CHECK(iter->first == "torsten");
            CHECK(iter->second == 94);
        }

        WHEN("invalid resources are gotten")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.get<Person>("people", "asdf"), rex::InvalidResourceException);
            }

            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.get<Person>("people", std::vector<std::string>{"anders", "asdf"}), rex::InvalidResourceException);
            }
        }

        WHEN("valid resources are gotten with the wrong type")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.get<Tool>("people", "anders"), rex::InvalidSourceException);
            }
        }

        WHEN("resources are gotten from invalid sources")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.get<Tool>("tools", "hammer"), rex::InvalidSourceException);
            }
        }

        WHEN("all resources are gotten from a valid source")
        {
            std::vector<rex::ResourceView<Person>> people = provider.getAll<Person>("people");

            THEN("all resources are gotten with valid values")
            {
                REQUIRE(people.size() == 3);

                std::map<std::string, int32_t> peopleSorted =
                {
                    {people[0].resource.name, people[0].resource.age},
                    {people[1].resource.name, people[1].resource.age},
                    {people[2].resource.name, people[2].resource.age},
                };

                REQUIRE(peopleSorted.size() == 3);

                auto iter = peopleSorted.begin();
                CHECK(iter->first == "anders");
                CHECK(iter->second == 47);
                ++iter;
                CHECK(iter->first == "kalle");
                CHECK(iter->second == 19);
                ++iter;
                CHECK(iter->first == "torsten");
                CHECK(iter->second == 94);
            }
        }

        WHEN("all resources are gotten from a valid source, but with wrong type")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.getAll<Tool>("people"), rex::InvalidSourceException);
            }
        }

        WHEN("all resources are gotten from an invalid source")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.getAll<Tool>("tools"), rex::InvalidSourceException);
            }
        }
    }
}

#ifndef REX_DISABLE_ASYNC
SCENARIO("ResourceProvider can be used to access resources asynchronously from sources")
{
    GIVEN("a resource provider with a source added")
    {
        rex::ResourceProvider provider;

        provider.addSource("people", PeopleSource("tests/data/people", true));

        WHEN("valid resources are accessed asynchronously")
        {
            rex::AsyncResourceView<Person> personView = provider.asyncGet<Person>("people", "anders");

            THEN("initially the name is correct and the value is a waiting std::future")
            {
                CHECK(personView.identifier == "anders");
                CHECK(personView.future.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout);
            }

            THEN("when the resource is loaded, it can be properly accessed with the std::future")
            {
                REQUIRE(personView.future.wait_for(std::chrono::milliseconds(150)) == std::future_status::ready);

                const Person& person = personView.future.get();
                CHECK(person.name == "anders");
                CHECK(person.age == 47);
            }
        }

        WHEN("more than one valid resource are accessed asynchronously")
        {
            std::vector<rex::AsyncResourceView<Person>> persons = provider.asyncGet<Person>("people", std::vector<std::string>{"anders", "kalle"});

            THEN("initially the name is correct and the value is a waiting std::future")
            {
                bool valid1 = persons[0].identifier == "anders" || persons[0].identifier == "kalle";
                bool valid2 = persons[1].identifier == "anders" || persons[1].identifier == "kalle";

                CHECK(valid1);
                CHECK(valid2);
                CHECK(persons[0].identifier != persons[1].identifier);
            }

            THEN("when the async accesses are resolved, the contained resources have correct values")
            {
                REQUIRE(persons[0].future.wait_for(std::chrono::milliseconds(200)) == std::future_status::ready);
                REQUIRE(persons[1].future.wait_for(std::chrono::milliseconds(200)) == std::future_status::ready);

                std::map<std::string, int32_t> peopleSorted =
                {
                    {persons[0].future.get().name, persons[0].future.get().age},
                    {persons[1].future.get().name, persons[1].future.get().age},
                };
                
                REQUIRE(peopleSorted.size() == 2);

                auto iter = peopleSorted.begin();
                CHECK(iter->first == "anders");
                CHECK(iter->second == 47);
                ++iter;
                CHECK(iter->first == "kalle");
                CHECK(iter->second == 19);
            }
        }

        WHEN("invalid resources are accessed asynchronously")
        {
            rex::AsyncResourceView<Person> personView = provider.asyncGet<Person>("people", "asdf");

            THEN("initially the name is correct and the value is a waiting std::future")
            {
                CHECK(personView.identifier == "asdf");
                CHECK(personView.future.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout);
            }

            THEN("when the resource is failed to load, accessing it throws an exception")
            {
                REQUIRE(personView.future.wait_for(std::chrono::milliseconds(150)) == std::future_status::ready);

                CHECK_THROWS_AS(personView.future.get(), rex::InvalidResourceException);
            }
        }

        WHEN("many resources are accessed asynchronously and at least one is invalid")
        {
            std::vector<rex::AsyncResourceView<Person>> people = provider.asyncGet<Person>("people", std::vector<std::string>{"kalle", "asdf"});

            REQUIRE(people[0].future.wait_for(std::chrono::milliseconds(200)) == std::future_status::ready);
            REQUIRE(people[1].future.wait_for(std::chrono::milliseconds(200)) == std::future_status::ready);

            THEN("when the resources are finished loading, the valid resource can be accessed normally and the other one throws an exception")
            {
                std::map<std::string, std::shared_future<const Person&>> sorted
                {
                    { people[0].identifier, people[0].future },
                    { people[1].identifier, people[1].future },
                };

                auto kalle = sorted.at("kalle");
                auto invalid = sorted.at("asdf");

                CHECK(kalle.get().name == "kalle");
                CHECK(kalle.get().age == 19);
                CHECK_THROWS_AS(invalid.get(), rex::InvalidResourceException);
            }
        }

        WHEN("valid resources are accessed asynchronously with the wrong type")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.asyncGet<Tool>("people", "anders"), rex::InvalidSourceException);
            }
        }

        WHEN("resources are accessed asynchronously from invalid sources")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.asyncGet<Tool>("tools", "hammer"), rex::InvalidSourceException);
            }
        }
    }
}

SCENARIO("ResourceProvider can be used to access the same resources synchronously and asynchronously in a mixed way")
{
    GIVEN("a resource provider with a source added")
    {
        rex::ResourceProvider provider;

        provider.addSource("people", PeopleSource("tests/data/people", true));

        WHEN("resources are accessed asynchronously after being accessed synchronously")
        {
            provider.get<Person>("people", "anders");
            rex::AsyncResourceView<Person> personView = provider.asyncGet<Person>("people", "anders");

            THEN("the async access is resolved pretty much directly with correct values")
            {
                CHECK(personView.identifier == "anders");
                REQUIRE(personView.future.wait_for(std::chrono::milliseconds(10)) == std::future_status::ready);

                const Person& person = personView.future.get();
                CHECK(person.name == "anders");
                CHECK(person.age == 47);
            }
        }

        WHEN("resources are accessed asynchronously after being accessed asynchronously")
        {
            rex::AsyncResourceView<Person> personView1 = provider.asyncGet<Person>("people", "anders");
            rex::AsyncResourceView<Person> personView2 = provider.asyncGet<Person>("people", "anders");

            THEN("both async views are initially not ready but resolve later into the same value")
            {
                CHECK(personView1.future.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout);
                CHECK(personView2.future.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout);

                REQUIRE(personView1.future.wait_for(std::chrono::milliseconds(100)) == std::future_status::ready);
                REQUIRE(personView2.future.wait_for(std::chrono::milliseconds(100)) == std::future_status::ready);

                const Person& person1 = personView1.future.get();
                CHECK(person1.name == "anders");
                CHECK(person1.age == 47);

                const Person& person2 = personView2.future.get();
                CHECK(person2.name == "anders");
                CHECK(person2.age == 47);
            }
        }
    }
}

SCENARIO("ResourceProvider can be used to access all resources from a source in an asynchronous way")
{
    GIVEN("a resource provider with a source added")
    {
        rex::ResourceProvider provider;

        provider.addSource("people", PeopleSource("tests/data/people", true));

        WHEN("all resources are accessed asynchronously")
        {
            std::vector<rex::AsyncResourceView<Person>> allPeople = provider.asyncGetAll<Person>("people");

            THEN("the async access is resolved pretty much directly with correct values")
            {
                REQUIRE(allPeople.size() == 3);

                bool name1Good = allPeople[0].identifier == "anders" || allPeople[0].identifier == "kalle" || allPeople[0].identifier == "torsten";
                bool name2Good = allPeople[1].identifier == "anders" || allPeople[1].identifier == "kalle" || allPeople[1].identifier == "torsten";
                bool name3Good = allPeople[2].identifier == "anders" || allPeople[2].identifier == "kalle" || allPeople[2].identifier == "torsten";

                CHECK(name1Good);
                CHECK(name2Good);
                CHECK(name3Good);

                CHECK(allPeople[0].identifier != allPeople[1].identifier);
                CHECK(allPeople[0].identifier != allPeople[2].identifier);
                CHECK(allPeople[1].identifier != allPeople[2].identifier);
            }

            THEN("when the async accesses are resolved, the contained resources have correct values")
            {
                REQUIRE(allPeople[0].future.wait_for(std::chrono::milliseconds(200)) == std::future_status::ready);
                REQUIRE(allPeople[1].future.wait_for(std::chrono::milliseconds(200)) == std::future_status::ready);
                REQUIRE(allPeople[2].future.wait_for(std::chrono::milliseconds(200)) == std::future_status::ready);

                std::map<std::string, int32_t> peopleSorted =
                {
                    {allPeople[0].future.get().name, allPeople[0].future.get().age},
                    {allPeople[1].future.get().name, allPeople[1].future.get().age},
                    {allPeople[2].future.get().name, allPeople[2].future.get().age},
                };

                REQUIRE(peopleSorted.size() == 3);

                auto iter = peopleSorted.begin();
                CHECK(iter->first == "anders");
                CHECK(iter->second == 47);
                ++iter;
                CHECK(iter->first == "kalle");
                CHECK(iter->second == 19);
                ++iter;
                CHECK(iter->first == "torsten");
                CHECK(iter->second == 94);
            }
        }

        WHEN("all resources are accessed asynchronously with the wrong type")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.asyncGetAll<Tool>("people"), rex::InvalidSourceException);
            }
        }

        WHEN("all resources are accessed asynchronously from an invalid source")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.asyncGetAll<Tool>("tools"), rex::InvalidSourceException);
            }
        }
    }
}
#endif

SCENARIO("Resources in valid sources can be marked as unused")
{
    GIVEN("a resource provider with a source added and resources loaded")
    {
        rex::ResourceProvider provider;

        provider.addSource("people", PeopleSource("tests/data/people", false));
        provider.get<Person>("people", "kalle");
        provider.get<Person>("people", "anders");

        WHEN("existing and non-existing resources are marked as unused in a valid source")
        {
            THEN("no exception is thrown")
            {
                CHECK_NOTHROW(provider.markUnused("people", "kalle"));
                CHECK_NOTHROW(provider.markUnused("people", "anders"));
                CHECK_NOTHROW(provider.markUnused("people", "asdfa"));
            }
        }

        WHEN("resources are marked as unused in an invalid source")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.markUnused("tools", "hammer"), rex::InvalidSourceException);
            }
        }
    }
}

SCENARIO("All resources in a source can be marked as invalid")
{
    GIVEN("a resource provider with a source added and resources loaded")
    {
        rex::ResourceProvider provider;

        provider.addSource("people", PeopleSource("tests/data/people", false));
        provider.get<Person>("people", "kalle");
        provider.get<Person>("people", "anders");

        WHEN("all resources are marked as unused in a valid source")
        {
            THEN("no exception is thrown")
            {
                CHECK_NOTHROW(provider.markAllUnused("people"));
            }
        }

        WHEN("all resources are marked as unused in an invalid source")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.markAllUnused("tools"), rex::InvalidSourceException);
            }
        }
    }
}
