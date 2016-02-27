#pragma once
#include <rex/filesource.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace rex
{
    namespace sf
    {
        class TextureFileSource : public rex::FileSource<::sf::Texture>
        {
            public:
                using rex::FileSource<::sf::Texture>::FileSource;
        
                ::sf::Texture loadFromFile(const Path& path) const override
                {
                    ::sf::Texture texture;

                    if(!texture.loadFromFile(path))
                    {
                        throw rex::InvalidResourceException("cannot open texture file '" + path.str() + "'");
                    }

                    return texture;
                }
        };
    }
}
