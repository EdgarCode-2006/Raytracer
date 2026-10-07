#ifndef _DLLOADER_HPP_
    #define _DLLOADER_HPP_
    #include <stdexcept>
    #include <string>

// Platform specific part of the loader (dlopen on Linux, LoadLibrary on Windows).
// It lives in DLLoader.cpp so that <windows.h> is never included next to raylib.h.
namespace DynamicLibrary {
    #ifdef _WIN32
        inline constexpr const char *EXTENSION = "dll";
    #else
        inline constexpr const char *EXTENSION = "so";
    #endif

    void *open(const std::string &path);
    void *getSymbol(void *handle, const std::string &symbol);
    void close(void *handle) noexcept;
}

template <typename T>
class DLLoader {
    private:
        void *_handle = nullptr;
    public:
        DLLoader() = default;
        DLLoader(const DLLoader&) = delete;
        DLLoader& operator=(const DLLoader&) = delete;

        // Move constructor
        DLLoader(DLLoader&& other) noexcept {
            this->_handle = other._handle;
            other._handle = nullptr;
        }

        // Move assignement
        DLLoader& operator=(DLLoader&& other) noexcept {
            if (this != &other) {
                this->closeLib();
                this->_handle = other._handle;
                other._handle = nullptr;
            }
            return *this;
        }

        void loadNewLib(const std::string &path)
        {
            _handle = DynamicLibrary::open(path);
        }

        void closeLib() {
            if (_handle) {
                DynamicLibrary::close(_handle);
                _handle = nullptr;
            }
        }

        ~DLLoader() {
            this->closeLib();
        }

        T* getInstance(const std::string& entryPointSymbol) {

            using FactoryFunc = T* (*)();

            void* sym = DynamicLibrary::getSymbol(_handle, entryPointSymbol);

            FactoryFunc func = reinterpret_cast<FactoryFunc>(sym);

            return func();
        }
};
#endif
