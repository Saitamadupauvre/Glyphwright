#pragma once
#include <dlfcn.h>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>

namespace gw {

enum class DLLoadError {
    None,
    FileNotFound,
    OpenFailed,
    EntryPointNotFound,
};

template <typename EntryPointFn>
class DLLoader {
public:
    DLLoader() = default;
    ~DLLoader() { close(); }

    DLLoader(const DLLoader&) = delete;
    DLLoader& operator=(const DLLoader&) = delete;

    DLLoader(DLLoader&& other) noexcept { *this = std::move(other); }
    DLLoader& operator=(DLLoader&& other) noexcept {
        if (this != &other) {
            close();
            _handle = std::exchange(other._handle, nullptr);
            _entryPoint = std::exchange(other._entryPoint, nullptr);
            _lastError = std::exchange(other._lastError, DLLoadError::None);
            _path = std::move(other._path);
        }
        return *this;
    }

    bool open(const std::filesystem::path& path, std::string_view entryPointSymbol) {
        close();
        if (!std::filesystem::exists(path)) {
            _lastError = DLLoadError::FileNotFound;
            return false;
        }
        _handle = ::dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!_handle) {
            _lastError = DLLoadError::OpenFailed;
            return false;
        }
        void* sym = ::dlsym(_handle, std::string(entryPointSymbol).c_str());
        if (!sym) {
            ::dlclose(_handle);
            _handle = nullptr;
            _lastError = DLLoadError::EntryPointNotFound;
            return false;
        }
        _entryPoint = reinterpret_cast<EntryPointFn>(sym);
        _path = path;
        _lastError = DLLoadError::None;
        return true;
    }

    void close() {
        if (_handle) {
            ::dlclose(_handle);
            _handle = nullptr;
        }
        _entryPoint = nullptr;
        _path.clear();
    }

    bool isOpen() const { return _handle != nullptr; }
    EntryPointFn entryPoint() const { return _entryPoint; }
    const std::filesystem::path& path() const { return _path; }
    DLLoadError lastError() const { return _lastError; }

    template <typename SymbolFn>
    SymbolFn symbol(std::string_view name) const {
        if (!_handle) return nullptr;
        void* sym = ::dlsym(_handle, std::string(name).c_str());
        return sym ? reinterpret_cast<SymbolFn>(sym) : nullptr;
    }

private:
    void* _handle = nullptr;
    EntryPointFn _entryPoint = nullptr;
    std::filesystem::path _path;
    DLLoadError _lastError = DLLoadError::None;
};

} // namespace gw
