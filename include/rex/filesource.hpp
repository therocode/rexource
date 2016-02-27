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
            FileSource(const Path& folder, const std::regex& regex = std::regex(".*"), Naming naming = Naming::NO_EXT);
            ResourceType load(const std::string& id) const;
            std::vector<std::string> list() const;
        protected:
            virtual ResourceType loadFromFile(const Path& path) const = 0;
            std::string extractName(const Path& path, Naming naming);
            std::unordered_map<std::string, Path> mFiles;
    };

    template <typename ResourceType>
    FileSource<ResourceType>::FileSource(const Path& folder, const std::regex& regex, Naming naming)
    {
        FileLister lister(folder, FileLister::FILES);
        auto pathList = lister.list();

        for(const auto& path : pathList)
        {
            if(std::regex_match(path.str(), regex))
            {
                std::string name = extractName(path.str(), naming);

                if(mFiles.count(name) == 0)
                    mFiles.emplace(name, path);
                else
                {
                    std::string collidingPath = mFiles.at(name);
                    throw AmbiguousNameException("FileSource loading from '" + folder.str() + "' encountered name collision between '" + path.str() + "' and '" + collidingPath + "' which both result in the name '" + name + "'. Change the naming strategy of the FileSource or rename your files.");
                }
            }
        }
    }

    template <typename ResourceType>
    ResourceType FileSource<ResourceType>::load(const std::string& id) const
    {
    	try 
    	{   
    	    return loadFromFile(mFiles.at(id));
    	}   
    	catch(const std::exception& e)
    	{   
    	    throw rex::InvalidResourceException("With resource '" + id + "', " + e.what());
    	}   
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
    std::string FileSource<ResourceType>::extractName(const Path& path, Naming naming)
    {
        if(naming == Naming::NO_EXT)
        {
            return path.stem();
        }
        else if(naming == Naming::FILE_NAME)
        {
            return path.fileName();
        }
        else if(naming == Naming::PATH)
        {
            return path;
        }

        return "";
    }
};
