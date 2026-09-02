#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <cctype>

//a bunch of constants for creating order of the csv files. They match the original index rules from the python code.
const size_t KEY_WORD_INDEX = 0;
const size_t ITEM_NAME_INDEX = 0;
const int SOURCE_INDEX = -1; // Negative index means "last element"
const size_t TYPE_INDEX = 1;
const size_t ATTUNEMENT_INDEX = 1;
const size_t PRICE_INDEX = 2;

//creating custom layers for sorting and searching the csv files. The row into the Rarity csv, and those csvs into the master csv which has all of the rows from all of the csvs in one
using CSVRow = std::vector<std::string>;
using ItemDictionary = std::unordered_map<std::string, CSVRow>;
using MasterDictionary = std::unordered_map<std::string, ItemDictionary>;

// helpful bit of code to allow for the conversion of the users input into lowercase.
std::string toLower(std:string str) {
    std:transform(str.begin(), str.end(), str.begin(), [](unsigned char c){
        return std::toLower(c);

    });
    return str;
}
