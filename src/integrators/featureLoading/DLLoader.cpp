#include "integrators/featureLoader/DLLoader.hpp"

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #include <filesystem>
#else
    #include <dlfcn.h>
#endif

#ifdef _WIN32

static std::string lastErrorMessage()
{
    const DWORD code = GetLastError();
    char *buffer = nullptr;
    const DWORD size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, code, 0, reinterpret_cast<LPSTR>(&buffer), 0, nullptr);
    std::string message = size ? std::string(buffer, size) : "error code " + std::to_string(code);

    LocalFree(buffer);
    while (!message.empty() && (message.back() == '\n' || message.back() == '\r' || message.back() == ' '))
        message.pop_back();
    return message;
}

void *DynamicLibrary::open(const std::string &path)
{
    // With an absolute path, LOAD_WITH_ALTERED_SEARCH_PATH makes Windows look for the
    // plugin's own dependencies next to it, like an rpath would.
    const std::filesystem::path fullPath = std::filesystem::absolute(path).make_preferred();
    HMODULE handle = LoadLibraryExW(fullPath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);

    if (!handle)
        throw std::runtime_error(path + ": " + lastErrorMessage());

    // Same as RTLD_NODELETE: the features can be destroyed after their loader,
    // so their code has to stay mapped until the process exits.
    HMODULE pinned = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
        reinterpret_cast<LPCWSTR>(handle), &pinned);
    return handle;
}

void *DynamicLibrary::getSymbol(void *handle, const std::string &symbol)
{
    FARPROC sym = GetProcAddress(static_cast<HMODULE>(handle), symbol.c_str());

    if (!sym)
        throw std::runtime_error(symbol + ": " + lastErrorMessage());
    return reinterpret_cast<void *>(sym);
}

void DynamicLibrary::close(void *handle) noexcept
{
    FreeLibrary(static_cast<HMODULE>(handle));
}

#else

void *DynamicLibrary::open(const std::string &path)
{
    void *handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL | RTLD_NODELETE);

    if (!handle)
        throw std::runtime_error(dlerror());
    return handle;
}

void *DynamicLibrary::getSymbol(void *handle, const std::string &symbol)
{
    dlerror();
    void *sym = dlsym(handle, symbol.c_str());

    const char *error = dlerror();
    if (error != nullptr)
        throw std::runtime_error(error);
    return sym;
}

void DynamicLibrary::close(void *handle) noexcept
{
    dlclose(handle);
}

#endif
