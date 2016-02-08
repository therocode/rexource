#include <catch.hpp>
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

            THEN("accessing the source from the provider gives the same source object as when returned")
            {
                CHECK(provider.source<PeopleSource>("people") == added);
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
