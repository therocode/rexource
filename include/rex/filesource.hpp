#pragma once
#include <regex>
#include <string>
#include <unordered_map>
#include <rex/config.hpp>
#include <rex/exceptions.hpp>
#include <rex/filelister.hpp>

namespace rex
{
    enum class Naming { NO_EXT, FILE_NAME, PATH };

    template <typename ResourceType>
    class FileSource
    {
        public:
            FileSource(const std::string& folder, const std::regex& regex = std::regex(".*"), Naming naming = Naming::NO_EXT);
            ResourceType load(const std::string& id) const;
            std::vector<std::string> list() const;
        protected:
            virtual ResourceType loadFromFile(const std::string& path) const = 0;
            std::string extractName(const std::string path, Naming naming);
            std::unordered_map<std::string, std::string> mFiles;
    };

    template <typename ResourceType>
    FileSource<ResourceType>::FileSource(const std::string& folder, const std::regex& regex, Naming naming)
    {
        FileLister lister(folder);
        auto pathList = lister.list();

        for(const auto& path : pathList)
        {
            if(std::regex_match(path, regex))
            {
                std::string name = extractName(path, naming);

                if(mFiles.count(name) == 0)
                    mFiles.emplace(name, path);
                else
                {
                    std::string collidingPath = mFiles.at(name);
                    throw AmbiguousNameException("FileSource loading from '" + folder + "' encountered name collision between '" + path + "' and '" + collidingPath + "' which both result in the name '" + name + "'. Change the naming strategy of the FileSource or rename your files.");
                }
            }
        }
    }

    template <typename ResourceType>
    ResourceType FileSource<ResourceType>::load(const std::string& id) const
    {
        return loadFromFile(mFiles.at(id));
    }

    template <typename ResourceType>
    std::vector<std::string> FileSource<ResourceType>::list() const
    {
        std::vector<std::string> result;

        for(const auto& file : mFiles)
            result.emplace_back(file.first);

        return result;
    }

    template <typename ResourceType>
    std::string FileSource<ResourceType>::extractName(const std::string path, Naming naming)
    {
        size_t lastSeparator = path.find_last_of('/');
        size_t lastPeriod = path.find_last_of('.');

        std::string name;

        if(naming == Naming::NO_EXT)
        {
            if(lastPeriod > lastSeparator)
                name = path.substr(lastSeparator + 1, lastPeriod - lastSeparator - 1);
            else
                name = path.substr(lastSeparator + 1);
        }
        else if(naming == Naming::FILE_NAME)
        {
            name = path.substr(lastSeparator + 1);
        }
        else if(naming == Naming::PATH)
        {
            return path;
        }

        return name;
    }
};
