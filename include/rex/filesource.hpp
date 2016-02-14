#pragma once
#include <rex/config.hpp>
#include <rex/filelister.hpp>
#include <unordered_map>
#include <string>
#include <iostream>

namespace rex
{
    template <typename ResourceType>
    class FileSource
    {
        public:
            FileSource(const std::string& folder, const std::string& pattern);
            ResourceType load(const std::string& id) const;
            std::vector<std::string> list() const;
        protected:
            virtual ResourceType loadFromFile(const std::string& path) const = 0;
            std::unordered_map<std::string, std::string> mFiles;
    };

    template <typename ResourceType>
    FileSource<ResourceType>::FileSource(const std::string& folder, const std::string& pattern)
    {
        FileLister lister(folder);
        auto pathList = lister.list();

        for(const auto& path : pathList)
        {
            size_t lastSeparator = path.find_last_of('/');
            size_t lastPeriod = path.find_last_of('.');

            std::string name;

            if(lastPeriod > lastSeparator)
                name = path.substr(lastSeparator + 1, lastPeriod - lastSeparator - 1);
            else
                name = path.substr(lastSeparator + 1);

            mFiles.emplace(name, path);
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
};
