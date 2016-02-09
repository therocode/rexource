#include <catch.hpp>
#include <map>
#include "helpers/person.hpp"
#include "helpers/tool.hpp"
#include "helpers/peoplesource.hpp"
#include "helpers/toolsource.hpp"
#include <rex/resourceprovider.hpp>

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
            rex::SourceView<PeopleSource> added = provider.addSource("people", PeopleSource("data/people"));

            THEN("the return value's id is the added source's name")
            {
                CHECK(added.id == "people");
            }

            THEN("the return value's 'source' attribute is the source object instance")
            {
                CHECK(added.source.path() == "data/people");
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

        WHEN("a source is added and removed")
        {
            provider.addSource("people", PeopleSource("data/people"));
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
                provider.addSource("people", PeopleSource("data/people"));
                CHECK_THROWS_AS(provider.addSource("people", PeopleSource("data/people")), rex::InvalidSourceException);
            }
        }

        WHEN("two sources are added and cleared, and subsequently accessed")
        {
            THEN("exceptions are thrown")
            {
                provider.addSource("people", PeopleSource("data/people"));
                provider.addSource("tools", ToolSource("data/tools"));
                provider.clearSources();

                CHECK_THROWS_AS(provider.source<PeopleSource>("people"), rex::InvalidSourceException);
                CHECK_THROWS_AS(provider.source<ToolSource>("tools"), rex::InvalidSourceException);
            }
        }
    }
}

SCENARIO("ResourceProvider can be used to access resources synchronously from sources")
{
    GIVEN("a resource provider with a source added")
    {
        rex::ResourceProvider provider;

        provider.addSource("people", PeopleSource("data/people"));

        WHEN("valid resources are gotten")
        {
            const Person& person = provider.get<Person>("people", "anders");

            THEN("the values are correct")
            {
                CHECK(person.name == "anders");
                CHECK(person.age == 47);
            }
        }

        WHEN("invalid resources are gotten")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.get<Person>("people", "ragnar"), rex::InvalidResourceException);
            }
        }

        WHEN("valid resources are gotten with the wrong type")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.get<Tool>("people", "ragnar"), rex::InvalidSourceException);
            }
        }

        WHEN("invalid resources are gotten with the wrong type")
        {
            THEN("an exception is thrown")
            {
                CHECK_THROWS_AS(provider.get<Tool>("people", "ragnar"), rex::InvalidSourceException);
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
