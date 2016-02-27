#pragma once
#include <rex/filesource.hpp>
#include <SFML/Graphics/Font.hpp>

namespace rex
{
    namespace sf
    {
        class FontFileSource : public rex::FileSource<::sf::Font>
        {
            public:
                using rex::FileSource<::sf::Font>::FileSource;
        
                ::sf::Font loadFromFile(const Path& path) const override
                {
                    ::sf::Font font;

                    if(!font.loadFromFile(path))
                    {
                        throw rex::InvalidResourceException("cannot open font file '" + path.str() + "'");
                    }

                    return font;
                }
        };
    }
}
