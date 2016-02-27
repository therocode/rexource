#pragma once

namespace rex
{
    class Path
    {
        public:
            Path(std::string path);
            Path(const char* path);
            operator const std::string&() const;
            const std::string& str() const;
            const std::string& fileName() const;
            const std::string& stem() const;
            const std::string& extension() const;
        private:
            std::string toGoodSlash(std::string path) const;
            std::string stripTrailingSlash(std::string path) const;
            size_t fileNameStart(const std::string& path) const;
            size_t extensionStart(const std::string& fileName) const;

            std::string mPath;
            std::string mFileName;
            std::string mStem;
            std::string mExtension;
    };

    inline Path::Path(std::string path)
    {
        mPath = std::move(path);
        mPath = toGoodSlash(mPath);
        mPath = stripTrailingSlash(mPath);

        size_t fileStart = fileNameStart(mPath);

        mFileName = mPath.substr(fileStart);

        size_t _extensionStart = extensionStart(mFileName);

        if(_extensionStart != std::string::npos)
        {
            mExtension = mFileName.substr(_extensionStart);
            mStem = mFileName.substr(0, _extensionStart - 1); 
        }
        else
        {
            mStem = mFileName; 
        }
    }

    inline Path::Path(const char* path):
        Path::Path(std::string(path))
    {

    }

    inline Path::operator const std::string&() const
    {
        return mPath;
    }

    inline const std::string& Path::str() const
    {
        return *this;
    }

    inline const std::string& Path::fileName() const
    {
        return mFileName;
    }

    inline const std::string& Path::stem() const
    {
        return mStem;
    }

    inline const std::string& Path::extension() const
    {
        return mExtension;
    }

    inline std::string Path::toGoodSlash(std::string path) const
    {
        for(size_t backslashPos = path.find_first_of('\\'); backslashPos != std::string::npos;)
        {
            path[backslashPos] = '/';
            
            backslashPos = path.find_first_of('\\');
        }

        return path;
    }

    inline std::string Path::stripTrailingSlash(std::string path) const
    {
        bool endsWithSlash = path.back() == '/';

        if(endsWithSlash)
            path.pop_back();

        return path;
    }

    inline size_t Path::fileNameStart(const std::string& path) const
    {
        size_t lastSlash = path.find_last_of('/');

        if(lastSlash != std::string::npos)
            return lastSlash + 1;
        else
            return 0;
    }
    
    inline size_t Path::extensionStart(const std::string& fileName) const
    {
        bool containsOtherThanDot = fileName.find_first_not_of('.') != std::string::npos;

        if(!containsOtherThanDot)
            return std::string::npos;

        size_t lastSlash = fileName.find_last_of('.');

        if(lastSlash != std::string::npos)
            return lastSlash + 1;
        else
            return std::string::npos;
    }
}
