#include "ModLoader.h"

#include "CodeLoader.h"
#include "Context.h"
#include "Types.h"
#include "Utilities.h"

HOOK(void, __fastcall, InitRomDirectoryPaths, sigInitRomDirectoryPaths())
{
    originalInitRomDirectoryPaths();

    // Get the address of the vector from the lea instruction that loads it.
    uint8_t* instrAddr = (uint8_t*)sigInitRomDirectoryPaths() + 0x30;
    const auto romDirectoryPaths = (prj::vector<prj::string>*)(instrAddr + readUnalignedU32(instrAddr + 0x3) + 0x7);

    std::vector<std::string> modRomDirectoryPaths;

    // Check for every rom folder within mod directory paths.
    for (auto& modDirectoryPath : ModLoader::modDirectoryPaths)
    {
        for (auto& romDirectoryPath : *romDirectoryPaths)
            modRomDirectoryPaths.push_back(modDirectoryPath + "\\" + romDirectoryPath.c_str());
    }

    // Cleanse directories that do not exist.
    processDirectoryPaths(modRomDirectoryPaths, false);

    if (modRomDirectoryPaths.empty())
        return;

    LOG("ROM:")

    for (auto& modRomDirectoryPath : modRomDirectoryPaths)
        LOG(" - %s", modRomDirectoryPath.c_str());

    // Insert to the beginning of the game's rom directory paths.
    // The first item in the vector has the highest priority.
    romDirectoryPaths->insert(romDirectoryPaths->begin(), modRomDirectoryPaths.begin(), modRomDirectoryPaths.end());
}

std::vector<std::string> ModLoader::modDirectoryPaths;

void ModLoader::initMod(const std::filesystem::path& path)
{
    const std::string modDirectoryPath = path.string();
    const std::string configFilePath = modDirectoryPath + "\\config.toml";

    toml::table config;

    try
    {
        config = toml::parse_file(configFilePath);
    }

    catch (std::exception& exception)
    {
        LOG(" - Failed to load \"%s\": %s", getRelativePath(configFilePath).c_str(), exception.what())
        return;
    }

    if (!config["enabled"].value_or(true))
        return;

    LOG(" - %s", config["name"].value_or(path.filename().string().c_str()))

    if (toml::array* includeArr = config["include"].as_array())
    {
        for (auto& includeElem : *includeArr)
        {
            const std::string include = includeElem.value_or("");

            if (!include.empty())
                modDirectoryPaths.push_back(modDirectoryPath + "\\" + include);
        }
    }

    if (toml::array* dllArr = config["dll"].as_array())
    {
        for (auto& dllElem : *dllArr)
        {
            const std::string dll = dllElem.value_or("");

            if (!dll.empty())
                CodeLoader::dllFilePaths.push_back(path.wstring() + L"\\" + convertMultiByteToWideChar(dll));
        }
    }
}

void ModLoader::init()
{
    LOG("Mods: \"%s\"", getRelativePath(Config::modsDirectoryPath).c_str())

    if (Config::priorityPaths.size() > 0)
    {
        LOG("Using priority array")
        for (auto& path : Config::priorityPaths)
        {
            const std::string modDirectory = Config::modsDirectoryPath + "\\" + path;
            if (std::filesystem::is_directory(modDirectory))
                initMod(modDirectory);
        }
    }
    else
    {
        LOG("Using alphanumeric folder name order for priority")
        for (auto& modDirectory : std::filesystem::directory_iterator(Config::modsDirectoryPath))
        {
            if (std::filesystem::is_directory(modDirectory))
                initMod(modDirectory.path());
        }
    }
    if (!modDirectoryPaths.empty())
        INSTALL_HOOK(InitRomDirectoryPaths);
}