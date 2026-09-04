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

//code for multi key searching/results
struct PairKey {
    std::string rarity;
    std::string itemName;
};

//trim whitespaces from strings
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// helpful bit of code to allow for the conversion of the users input into lowercase.
std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return str;
}

//code for splitting the strings of the csv into dynamic lists by commas
std::vector<std::string> splitByComma(const std::string& input) {
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string item;
    while (std::getline(ss, item, ',')) {
        std::string trimmed = trim(item);
        if (!trimmed.empty()) {
            result.push_back(trimmed);
        }
    }
    return result;
}

//custom csv parser to allow for third party library constraints to be mitigated
CSVRow parseCSVLine(const std::string& line) {
    CSVRow row;
    std::string cell;
    bool inQuotes = false;
    for (size_t i = 0; i < line.length(); ++i) {
        char ch = line[i];
        if (ch == '"') {
            inQuotes = !inQuotes; // Toggle quote block state
        } else if (ch == ',' && !inQuotes) {
            row.push_back(trim(cell));
            cell.clear();
        } else {
            cell += ch;
        }
    }
    row.push_back(trim(cell));
    return row;
}

//code fo reading standard csv files containing header
ItemDictionary readDictionary(const std::string& filename, size_t keyWordIndex) {
    ItemDictionary dict;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open " << filename << "\n";
        return dict;
    }
    std::string line;
    if (std::getline(file, line)) { /* Skip the CSV header row */ }

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        CSVRow row = parseCSVLine(line);
        if (keyWordIndex < row.size()) {
            std::string key = row[keyWordIndex];
            dict[key] = row;
        }
    }
    return dict;
}

//code to read custom manually loaded CSVs lacking headers

ItemDictionary readList(const std::string& filename) {
    ItemDictionary dict;
    std::ifstream file(filename);
    if (!file.is_open()) return dict;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        CSVRow row = parseCSVLine(line);
        if (!row.empty() && !row[0].empty()) { 
            std::string key = row[0];          
            dict[key] = row;
        }
    }
    return dict;
}

// adds the standard item dictionary to the global matrix
void appendMasterDictionary(MasterDictionary& masterDict, const ItemDictionary& dict, const std::string& rarity) {
    masterDict[rarity] = dict;
}

//rarity check against map structure
ItemDictionary rarityCheck(std::string rarity, const MasterDictionary& masterDict) {
    rarity = toLower(trim(rarity));
    for (const auto& [rarityKey, subDict] : masterDict) {
        if (toLower(rarityKey) == rarity) {
            return subDict;
        }
    }
    return {};
}

//getting the source book from negative lookup indexes
std::vector<std::pair<PairKey, CSVRow>> sourceCheck(std::string source, const MasterDictionary& masterDict) {
    std::vector<std::pair<PairKey, CSVRow>> sourceItems;
    source = toLower(trim(source));

    for (const auto& [rarity, items] : masterDict) {
        for (const auto& [itemNameKey, itemData] : items) {
            if (itemData.empty()) continue;
            
            // Replicating negative indexing logic (Python's SOURCE_INDEX = -1)
            size_t targetIndex = itemData.size() - 1; 
            
            if (toLower(itemData[targetIndex]) == source) {
                sourceItems.push_back({{rarity, itemNameKey}, itemData});
            }
        }
    }
    return sourceItems;
}

//extract item types using string standards
std::vector<std::pair<PairKey, CSVRow>> typeCheck(std::string type, const MasterDictionary& masterDict) {
    std::vector<std::pair<PairKey, CSVRow>> typeItems;
    type = toLower(trim(type));

    for (const auto& [rarity, items] : masterDict) {
        for (const auto& [itemNameKey, itemData] : items) {
            if (itemData.size() > TYPE_INDEX) {
                if (toLower(itemData[TYPE_INDEX]) == type) {
                    typeItems.push_back({{rarity, itemNameKey}, itemData});
                }
            }
        }
    }
    return typeItems;
}


//check global item titles across all structure boundaries
std::unordered_map<std::string, CSVRow> nameCheck(std::string itemName, const MasterDictionary& masterDict) {
    std::unordered_map<std::string, CSVRow> userItems;
    itemName = toLower(trim(itemName));

    for (const auto& [rarity, items] : masterDict) {
        for (const auto& [itemNameKey, itemData] : items) {
            if (toLower(itemNameKey) == itemName) {
                userItems[rarity] = itemData;
            }
        }
    }
    return userItems;
}

//utility format tool to output text of the standard vector elements
void printCSVRow(const CSVRow& row) {
    std::cout << "[";
    for (size_t i = 0; i < row.size(); ++i) {
        std::cout << "'" << row[i] << "'" << (i < row.size() - 1 ? ", " : "");
    }
    std::cout << "]\n";
}

//random item lookup gen code
std::string randomItem(const std::vector<std::string>& list) {
    if (list.empty()) return "";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, list.size() - 1);
    return list[dis(gen)];
}

//randomizer
std::vector<std::string> randomizeList(std::vector<std::string> list) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(list.begin(), list.end(), g);
    return list;
}

int main() {
    MasterDictionary masterDictionary;

    // Load base data pipeline paths
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Non Magic Items.csv", KEY_WORD_INDEX), "Non Magic");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Common.csv", KEY_WORD_INDEX), "Common");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Uncommon.csv", KEY_WORD_INDEX), "Uncommon");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Rare.csv", KEY_WORD_INDEX), "Rare");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Very Rare.csv", KEY_WORD_INDEX), "Very Rare");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Legendary.csv", KEY_WORD_INDEX), "Legendary");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Artifact.csv", KEY_WORD_INDEX), "Artifact");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Materials.csv", KEY_WORD_INDEX), "Materials");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Dragonmarks.csv", KEY_WORD_INDEX), "Dragonmark");

    std::string lastLoadedRarity = "";

    std::cout << "DND Vault Engine loaded successfully.\n";

    while(true) {
        std::cout << "Select Search Method \n";
        std::cout << "1.Search by Name\n";
        std::cout << "2. Search by Type\n";
        std::cout << "3. Search by Source\n";
        std::cout << "4. Search by Rarity\n";
        std::cout << "5. Exit\n";
        std::cout << "Enter choice (1-5): ";

        int choice;
        if (!(std::cin >> choice)){
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid Selection. Try Again. \n\n";
            continue;
        }

        if (choice == 5){
            std::cout << "Exiting DND Vault.\n";
            break;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //clear newline
        std::cout <<"Enter Search Query: ";
        std::string query;
        std::getline(std::cin, query);

        std::cout << "\n--- Search Results ---\n";
        std::cout << "\nRarity : Item Name : Type : Attunement : Price : Source :\n";

        if (choice == 1){ //searching by name 
            auto results = nameCheck(query, masterDictionary);
            if (results.empty()) std::cout << "No Matching Items Found. \n";
            for (const auto& [rarity, data] : results) {
                std:: cout << rarity << ": ";
                printCSVRow(data);
            }
        }
        else if(choice == 2){ //searching by type
            auto results = typeCheck(query, masterDictionary);
            if (results.empty()) std::cout << "No Matching Items Found.\n";
            for (const auto& [key, data] : results) {
                std::cout << key.rarity << " - " << key.itemName << ": ";
                printCSVRow(data);
            }
        }
        else if (choice == 3){//searching by source
            auto results = sourceCheck(query, masterDictionary);
            if (results.empty()) std::cout << "No Matching Items Found.\n";
            for (const auto& [key, data]: results){
                std::cout << key.rarity << " - " << key.itemName << ": ";
                printCSVRow(data);
            }
        }
        else if (choice == 4){//searching by rarity
            auto results = rarityCheck(query, masterDictionary);
            if (results.empty()) std::cout << "No Matching Items Found.\n";
            for (const auto& [name, data]: results) {
                std::cout << name << ": ";
                printCSVRow(data);
            }
        }
        else {
            std::cout << "Invalid Choice.\n";
        }

        std::cout << "------------------------\n\n";

    }

    return 0;
}