#pragma once
#include <rex/config.hpp>
#include <vector>
#include <string>
#include <rex/exceptions.hpp>
#include <rex/tinydir.hpp>

namespace rex
{
    class FileLister
    {
        public:
            FileLister(const std::string& folderPath);
            std::vector<std::string> list() const;
        private:
            void fetchFilesRecursively(const std::string& folder, std::vector<std::string>& output) const;
            std::string mFolderPath;
    };

    inline FileLister::FileLister(const std::string& folderPath):
        mFolderPath(folderPath)
    {
        tinydir_dir folder;
        int32_t result = tinydir_open(&folder, folderPath.c_str());

        if(result != 0)
            throw InvalidFileException("given path '" + folderPath + "' is not a directory");
    }

    inline std::vector<std::string> FileLister::list() const
    {
        std::vector<std::string> result;
        fetchFilesRecursively(mFolderPath, result);

        return result;
    }

    inline void FileLister::fetchFilesRecursively(const std::string& folderPath, std::vector<std::string>& output) const
    {
        tinydir_dir folder;
        tinydir_open(&folder, folderPath.c_str());

        while(folder.has_next)
        {
            tinydir_file file;
            tinydir_readfile(&folder, &file);
            tinydir_next(&folder);

            std::string path(file.path);
            size_t last = path.size() - 1;

            bool skip = false;

            if(path[last] == '.')
            {
                if(path[last - 1] == '/')
                    skip = true;
                else if(path[last - 1] == '.' && path[last - 2] == '/')
                    skip = true;
            }

            if(skip)
            {
                continue;
            }

            if(file.is_dir)
            {
                fetchFilesRecursively(path, output);
            }
            else
            {
                output.push_back(path);
            }
        }
    }
}
