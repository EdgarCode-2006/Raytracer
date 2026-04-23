#ifndef _DLLOADER_HPP_
    #define _DLLOADER_HPP_
    #include <stdexcept>
    #include <string>
    #include <dlfcn.h>

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
            _handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL | RTLD_NODELETE);
            if (!_handle) {
                throw std::runtime_error(dlerror());
            }
        }

        void closeLib() {
            if (_handle) {
                dlclose(_handle);
                _handle = nullptr;
            }
        }

        ~DLLoader() {
            this->closeLib();
        }

        T* getInstance(const std::string& entryPointSymbol) {

            using FactoryFunc = T* (*)();

            void* sym = dlsym(_handle, entryPointSymbol.c_str());

            char* error = dlerror();
            if (error != nullptr)
                throw std::runtime_error(error);

            FactoryFunc func = reinterpret_cast<FactoryFunc>(sym);

            return func();
        }
};
#endif