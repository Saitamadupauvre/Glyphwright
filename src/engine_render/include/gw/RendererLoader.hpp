#pragma once
#include <filesystem>
#include "gw/DLLoader.hpp"
#include "gw/RendererVTable.hpp"

namespace gw {

enum class RendererLoadError {
    None,
    FileNotFound,
    DlopenFailed,
    SymbolNotFound,
    NullVTable,
    AbiVersionMismatch,
    MissingRequiredFn,
    InconsistentStateFns,
    InitFailed,
};

class RendererLoader {
public:
    RendererLoader() = default;
    ~RendererLoader();

    RendererLoader(const RendererLoader&) = delete;
    RendererLoader& operator=(const RendererLoader&) = delete;

    bool load(const std::filesystem::path& libraryPath, RendererRect rect);
    bool swap(const std::filesystem::path& newLibraryPath);
    void unload();

    bool isLoaded() const { return _current.instance != nullptr; }
    RendererLoadError lastError() const { return _lastError; }

    void setRect(RendererRect rect);
    void beginFrame();
    void drawBatch(const DrawCommand* commands, size_t count);
    void endFrame();

private:
    struct LoadedLibrary {
        DLLoader<GetRendererVTableFn> dl;
        const RendererVTable* vtable = nullptr;
        void* instance = nullptr;
    };

    static bool validateVTable(const RendererVTable* vtable, RendererLoadError& err);
    bool openAndValidate(const std::filesystem::path& path, RendererRect rect, LoadedLibrary& out);
    static void destroyLoaded(LoadedLibrary& lib);

    LoadedLibrary _current;
    RendererRect _rect{};
    RendererLoadError _lastError = RendererLoadError::None;
};

} // namespace gw
