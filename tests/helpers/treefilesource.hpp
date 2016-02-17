#pragma once
#include <rex/filesource.hpp>
#include "tree.hpp"
#include <rex/json.hpp>
#include <fstream>
#include <thread>

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

            size_t numberStartAt = path.find_first_of("1234567890");
            std::string startsWithNumber = path.substr(numberStartAt);
            int32_t number = std::stoi(startsWithNumber);
            std::this_thread::sleep_for(std::chrono::milliseconds(number / 100));
            treeFile.close();

            return Tree
            {
                barkType,
                leafType,
                height,
                branchingFactor
            };
        }
};
