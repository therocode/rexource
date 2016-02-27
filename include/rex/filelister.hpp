#pragma once
#include <rex/config.hpp>
#include <vector>
#include <string>
#include <rex/exceptions.hpp>
#include <rex/path.hpp>
#include <rex/tinydir.hpp>

namespace rex
{
    class FileLister
    {
        public:
            FileLister(Path folderPath);
            std::vector<Path> list() const;
        private:
            void fetchFilesRecursively(const Path& folder, std::vector<Path>& output) const;
            Path mFolderPath;
    };

    inline FileLister::FileLister(Path folderPath):
        mFolderPath(std::move(folderPath))
    {
        tinydir_dir folder;
        int32_t result = tinydir_open(&folder, mFolderPath.str().c_str());

        if(result != 0)
            throw InvalidFileException("given path '" + mFolderPath.str() + "' is not a valid directory");
        else
            tinydir_close(&folder);
    }

    inline std::vector<Path> FileLister::list() const
    {
        std::vector<Path> result;
        fetchFilesRecursively(mFolderPath, result);

        return result;
    }

    inline void FileLister::fetchFilesRecursively(const Path& folderPath, std::vector<Path>& output) const
    {
        tinydir_dir folder;
        int32_t result = tinydir_open(&folder, folderPath.str().c_str());

        if(result != 0)
            throw InvalidFileException("given path '" + folderPath.str() + "' is not a valid directory");

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

        tinydir_close(&folder);
    }
}
