#pragma once
#include <rex/filesource.hpp>
#include "tree.hpp"
#include <rex/json.hpp>
#include <fstream>

class TreeFileSource : public rex::FileSource<Tree>
{
    public:
        using rex::FileSource<Tree>::FileSource;

        Tree loadFromFile(const std::string& path) const override
        {
            std::ifstream treeFile(path);

            if(!treeFile)
                throw rex::InvalidResourceException("cannot open tree file '" + path + "'");

            nlohmann::json jsonTree;
            treeFile >> jsonTree;

            std::string barkType = jsonTree["bark_type"];
            std::string leafType = jsonTree["leaf_type"];
            float height = jsonTree["height"];
            float branchingFactor = jsonTree["branching_factor"];

            return Tree
            {
                barkType,
                leafType,
                height,
                branchingFactor
            };
        }
};
