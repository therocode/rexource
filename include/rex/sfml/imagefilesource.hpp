#pragma once
#include <rex/filesource.hpp>
#include <SFML/Graphics/Image.hpp>

namespace rex
{
    namespace sf
    {
        class ImageFileSource : public rex::FileSource<::sf::Image>
        {
            public:
                using rex::FileSource<::sf::Image>::FileSource;
        
                ::sf::Image loadFromFile(const Path& path) const override
                {
                    ::sf::Image image;

                    if(!image.loadFromFile(path))
                    {
                        throw rex::InvalidResourceException("cannot open image file '" + path.str() + "'");
                    }

                    return image;
                }
        };
    }
}
