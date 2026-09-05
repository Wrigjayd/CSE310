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
using ItemDictionary = std::unordered_map<std::string, CSVRow>;
using MasterDictionary = std::unordered_map<std::string, ItemDictionary>;

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

ItemDictionary readDictionary(const std::string& filename, size_t keyWordIndex) {
    ItemDictionary dict;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open " << filename << "\n";
        return dict;
    }
    std::string line;
    if (std::getline(file, line)) { }

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

void appendMasterDictionary(MasterDictionary& masterDict, const ItemDictionary& dict, const std::string& rarity) {
    masterDict[rarity] = dict;
}

// --- Lookup Logic Core ---
ItemDictionary rarityCheck(std::string rarity, const MasterDictionary& masterDict) {
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
        for (const auto& [itemNameKey, itemData] : items) {
            if (itemData.empty()) continue;
            size_t targetIndex = itemData.size() - 1; 
            if (toLower(itemData[targetIndex]) == source) {
                sourceItems.push_back({{rarity, itemNameKey}, itemData});
            }
        }
    }
    return sourceItems;
}

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

    // Fixed: Standardized result structures to ensure types clear completely on search reload
    std::unordered_map<std::string, CSVRow> nameResults;
    std::vector<std::pair<PairKey, CSVRow>> structuredResults; 
    ItemDictionary rarityResults;

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
            rarityResults.clear();

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

        // Rendering Loop 1: NAME Matches
        for (const auto& [rarity, data] : nameResults) {
            ImGui::TextColored(ImVec4(0.74f, 0.58f, 0.32f, 1.00f), "[%s] ", rarity.c_str());
            ImGui::SameLine();
            for (size_t i = 0; i < data.size(); ++i) {
                ImGui::Text("%s %s", data[i].c_str(), (i < data.size() - 1) ? "|" : "");
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
            
            // Rendering Loop 2: TYPE & SOURCE Matches (Fixed structured vector extraction)
        for (const auto& [key, data] : structuredResults) {
            ImGui::TextColored(ImVec4(0.74f, 0.58f, 0.32f, 1.00f), "[%s] ", key.rarity.c_str());
            ImGui::SameLine();
            for (size_t i = 0; i < data.size(); ++i) {
                ImGui::Text("%s %s", data[i].c_str(), (i < data.size() - 1) ? "|" : "");
                ImGui::SameLine();
            }
            ImGui::NewLine();
        }
            // Rendering Loop 3: RARITY Matches (Fixed nested row processing)
        for (const auto& [name, data] : rarityResults) {
            ImGui::TextColored(ImVec4(0.74f, 0.58f, 0.32f, 1.00f), "[%s] ", name.c_str());
            ImGui::SameLine();
            for (size_t i = 0; i < data.size(); ++i) {
                ImGui::Text("%s %s", data[i].c_str(), (i < data.size() - 1) ? "|" : "");
                ImGui::SameLine();
            }

            ImGui::NewLine();
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