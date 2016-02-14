#pragma once
#include <rex/filesource.hpp>
#include "tree.hpp"

class TreeFileSource : public rex::FileSource<Tree>
{
    public:
        using rex::FileSource<Tree>::FileSource;

        Tree loadFromFile(const std::string& path) const override
        {
            return Tree();
        }
};
