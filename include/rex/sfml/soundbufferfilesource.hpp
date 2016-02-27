#pragma once
#include <rex/filesource.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

namespace rex
{
    namespace sf
    {
        class SoundBufferFileSource : public rex::FileSource<::sf::SoundBuffer>
        {
            public:
                using rex::FileSource<::sf::SoundBuffer>::FileSource;
        
                ::sf::SoundBuffer loadFromFile(const Path& path) const override
                {
                    ::sf::SoundBuffer soundBuffer;

                    if(!soundBuffer.loadFromFile(path))
                    {
                        throw rex::InvalidResourceException("cannot open sound buffer file '" + path.str() + "'");
                    }

                    return soundBuffer;
                }
        };
    }
}
