#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <cctype>
#include <limits>

// Dear ImGui headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Window manager header
#include <GLFW/glfw3.h>

const size_t KEY_WORD_INDEX = 0;
const size_t ITEM_NAME_INDEX = 0;
const int SOURCE_INDEX = -1; 
const size_t TYPE_INDEX = 1;
const size_t ATTUNEMENT_INDEX = 1;
const size_t PRICE_INDEX = 2;

using CSVRow = std::vector<std::string>;
// A struct that bundles the items hash map along with an insertion-order list
struct OrderedDictionary {
    std::unordered_map<std::string, CSVRow> itemMap;
    std::vector<std::string> insertionOrder; // Tracks top-to-bottom CSV sequence
    
    // Helper function to reset both structures seamlessly
    void clear() {
        itemMap.clear();
        insertionOrder.clear();
    }
};
using MasterDictionary = std::unordered_map<std::string, OrderedDictionary>;

struct PairKey {
    std::string rarity;
    std::string itemName;
};

// --- Core Helper Functions ---
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return str;
}

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

CSVRow parseCSVLine(const std::string& line) {
    CSVRow row;
    std::string cell;
    bool inQuotes = false;
    for (size_t i = 0; i < line.length(); ++i) {
        char ch = line[i];
        if (ch == '"') {
            inQuotes = !inQuotes; 
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

OrderedDictionary readDictionary(const std::string& filename, size_t keyWordIndex) {
    OrderedDictionary dict;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open " << filename << "\n";
        return dict;
    }
    std::string line;
    if (std::getline(file, line)) { /* Skip Header */ }

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        CSVRow row = parseCSVLine(line);
        if (keyWordIndex < row.size()) {
            std::string key = row[keyWordIndex];
            
            // Only add to the order list if it's a unique new item name
            if (dict.itemMap.find(key) == dict.itemMap.end()) {
                dict.insertionOrder.push_back(key);
            }
            dict.itemMap[key] = row;
        }
    }
    return dict;
}

OrderedDictionary readList(const std::string& filename) {
    OrderedDictionary dict;
    std::ifstream file(filename);
    if (!file.is_open()) return dict;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        CSVRow row = parseCSVLine(line);
        if (!row.empty() && !row[0].empty()) { 
            std::string key = row[0];          
            if (dict.itemMap.find(key) == dict.itemMap.end()) {
                dict.insertionOrder.push_back(key);
            }
            dict.itemMap[key] = row;
        }
    }
    return dict;
}

void appendMasterDictionary(MasterDictionary& masterDict, const OrderedDictionary& dict, const std::string& rarity) {
    masterDict[rarity] = dict;
}
// --- Lookup Logic Core ---
OrderedDictionary rarityCheck(std::string rarity, const MasterDictionary& masterDict) {
    rarity = toLower(trim(rarity));
    for (const auto& [rarityKey, subDict] : masterDict) {
        if (toLower(rarityKey) == rarity) {
            return subDict;
        }
    }
    return {};
}

std::vector<std::pair<PairKey, CSVRow>> sourceCheck(std::string source, const MasterDictionary& masterDict) {
    std::vector<std::pair<PairKey, CSVRow>> sourceItems;
    source = toLower(trim(source));
    for (const auto& [rarity, items] : masterDict) {
        for (const std::string& itemName : items.insertionOrder) {
            const CSVRow& itemData = items.itemMap.at(itemName);
            if (itemData.empty()) continue;
            size_t targetIndex = itemData.size() - 1; 
            if (toLower(itemData[targetIndex]) == source) {
                sourceItems.push_back({{rarity, itemName}, itemData});
            }
        }
    }
    return sourceItems;
}

std::vector<std::pair<PairKey, CSVRow>> typeCheck(std::string type, const MasterDictionary& masterDict) {
    std::vector<std::pair<PairKey, CSVRow>> typeItems;
    type = toLower(trim(type));
    for (const auto& [rarity, items] : masterDict) {
        for (const std::string& itemName : items.insertionOrder) {
            const CSVRow& itemData = items.itemMap.at(itemName);
            if (itemData.size() > TYPE_INDEX) {
                if (toLower(itemData[TYPE_INDEX]) == type) {
                    typeItems.push_back({{rarity, itemName}, itemData});
                }
            }
        }
    }
    return typeItems;
}

std::unordered_map<std::string, CSVRow> nameCheck(std::string itemName, const MasterDictionary& masterDict) {
    std::unordered_map<std::string, CSVRow> userItems;
    itemName = toLower(trim(itemName));
    for (const auto& [rarity, items] : masterDict) {
        if (items.itemMap.find(itemName) != items.itemMap.end()) {
        userItems[rarity] = items.itemMap.at(itemName);
        }
    }
    return userItems;
}

void ApplyDndVaultTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    style.WindowRounding = 6.0f;       
    style.FrameRounding = 4.0f;        
    style.ScrollbarRounding = 12.0f;   
    style.FramePadding = ImVec2(6, 6); 
    colors[ImGuiCol_Text]                  = ImVec4(0.92f, 0.88f, 0.78f, 1.00f); 
    colors[ImGuiCol_WindowBg]              = ImVec4(0.09f, 0.09f, 0.10f, 1.00f); 
    colors[ImGuiCol_ChildBg]               = ImVec4(0.12f, 0.12f, 0.14f, 1.00f); 
    colors[ImGuiCol_Border]                = ImVec4(0.74f, 0.58f, 0.32f, 0.50f); 
    colors[ImGuiCol_FrameBg]               = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.24f, 0.24f, 0.27f, 1.00f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.48f, 0.11f, 0.11f, 1.00f); 
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.64f, 0.15f, 0.15f, 1.00f); 
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.35f, 0.08f, 0.08f, 1.00f); 
    colors[ImGuiCol_Header]                = ImVec4(0.38f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.48f, 0.11f, 0.11f, 1.00f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.28f, 0.06f, 0.06f, 1.00f);
}

int main() {
    MasterDictionary masterDictionary;
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Non Magic Items.csv", KEY_WORD_INDEX), "Non Magic");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Common.csv", KEY_WORD_INDEX), "Common");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Uncommon.csv", KEY_WORD_INDEX), "Uncommon");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Rare.csv", KEY_WORD_INDEX), "Rare");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Very Rare.csv", KEY_WORD_INDEX), "Very Rare");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Legendary.csv", KEY_WORD_INDEX), "Legendary");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Artifact.csv", KEY_WORD_INDEX), "Artifact");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Materials.csv", KEY_WORD_INDEX), "Materials");
    appendMasterDictionary(masterDictionary, readDictionary("Master Magic Item List - Dragonmarks.csv", KEY_WORD_INDEX), "Dragonmark");

    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "DND Vault GUI", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); 

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ApplyDndVaultTheme();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    static char queryBuffer[256] = "";
    int currentMethod = 0; 
    const char* searchMethods[] = { "NAME", "TYPE", "SOURCE", "RARITY" };

    std::unordered_map<std::string, CSVRow> nameResults;
    std::vector<std::pair<PairKey, CSVRow>> structuredResults; 
    OrderedDictionary rarityResults;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); 

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("DND Vault Search Panel", nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::Combo("Search By", &currentMethod, searchMethods, IM_ARRAYSIZE(searchMethods));
        ImGui::InputText("Query", queryBuffer, IM_ARRAYSIZE(queryBuffer));
        ImGui::Separator();

        if (ImGui::Button("Execute Search", ImVec2(150, 30))) {
            std::string searchQuery(queryBuffer);
            
            nameResults.clear();
            structuredResults.clear();
            rarityResults.clear(); // Fixed: Successfully calls custom inner clear mapping

            if (currentMethod == 0) {       
                nameResults = nameCheck(searchQuery, masterDictionary);
            } else if (currentMethod == 1) { 
                structuredResults = typeCheck(searchQuery, masterDictionary);
            } else if (currentMethod == 2) { 
                structuredResults = sourceCheck(searchQuery, masterDictionary);
            } else if (currentMethod == 3) { 
                rarityResults = rarityCheck(searchQuery, masterDictionary);
            }
        }

        ImGui::Text("Results:");
        ImGui::BeginChild("ScrollingResultRegion", ImVec2(0, 400), true);

        bool hasResults = !nameResults.empty() || !structuredResults.empty() || !rarityResults.itemMap.empty();

        if (hasResults) {
            ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
            
            if (ImGui::BeginTable("DndVaultItemTable", 5, tableFlags)) {
                ImGui::TableSetupColumn("Item Name", ImGuiTableColumnFlags_WidthFixed, 220.0f);
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 180.0f);
                ImGui::TableSetupColumn("Attunement", ImGuiTableColumnFlags_WidthFixed, 140.0f);
                ImGui::TableSetupColumn("Price", ImGuiTableColumnFlags_WidthFixed, 110.0f);
                ImGui::TableSetupColumn("Source Book", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                // Shared layout lambda to safely render column fields into the grid table 
                auto DisplayRowInTable = [](const CSVRow& data) {
                    ImGui::TableNextRow();
                    
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(data.size() > 0 ? data[0].c_str() : "Unknown");

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(data.size() > 1 ? data[1].c_str() : "-");

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(data.size() > 2 ? data[2].c_str() : "-");

                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextUnformatted(data.size() > 3 ? data[3].c_str() : "-");

                    ImGui::TableSetColumnIndex(4);
                    if (!data.empty()) {
                        int lastValidIndex = data.size() - 1;
                        while (lastValidIndex > 0 && data[lastValidIndex].empty()) {
                            lastValidIndex--;
                        }
                        ImGui::TextUnformatted(data[lastValidIndex].c_str());
                    } else {
                        ImGui::TextUnformatted("-");
                    }
                };

                // Populate Name results rows
                for (const auto& [rarity, data] : nameResults) {
                    DisplayRowInTable(data);
                }
                
                // Populate Type & Source results rows (Maintains top-to-bottom sorting)
                for (const auto& [key, data] : structuredResults) {
                    DisplayRowInTable(data);
                }
                
                // Fixed Render Loop 3: Pull rows via chronological entry tracking vector index sequences
                for (const std::string& itemName : rarityResults.insertionOrder) {
                    DisplayRowInTable(rarityResults.itemMap.at(itemName));
                }

                ImGui::EndTable();
            }
        } else {
            ImGui::Text("No items match the current search query criteria.");
        }

        ImGui::EndChild();
        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.09f, 0.09f, 0.10f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}