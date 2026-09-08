#include "gw/RendererLoader.hpp"

namespace gw {

RendererLoader::~RendererLoader() { unload(); }

bool RendererLoader::validateVTable(const RendererVTable* vtable, RendererLoadError& err) {
    if (!vtable) {
        err = RendererLoadError::NullVTable;
        return false;
    }
    if (vtable->abiVersion != kRendererAbiVersion) {
        err = RendererLoadError::AbiVersionMismatch;
        return false;
    }
    if (!vtable->create || !vtable->destroy || !vtable->init || !vtable->shutdown ||
        !vtable->setRect || !vtable->beginFrame || !vtable->drawBatch || !vtable->endFrame) {
        err = RendererLoadError::MissingRequiredFn;
        return false;
    }
    const bool hasSerialize = vtable->serializeState != nullptr;
    const bool hasFree = vtable->freeState != nullptr;
    const bool hasDeserialize = vtable->deserializeState != nullptr;
    if (hasSerialize != hasFree || hasFree != hasDeserialize) {
        err = RendererLoadError::InconsistentStateFns;
        return false;
    }
    return true;
}

bool RendererLoader::openAndValidate(const std::filesystem::path& path, RendererRect rect, LoadedLibrary& out) {
    if (!out.dl.open(path, "get_renderer_vtable")) {
        switch (out.dl.lastError()) {
            case DLLoadError::FileNotFound: _lastError = RendererLoadError::FileNotFound; break;
            case DLLoadError::OpenFailed: _lastError = RendererLoadError::DlopenFailed; break;
            case DLLoadError::EntryPointNotFound: _lastError = RendererLoadError::SymbolNotFound; break;
            case DLLoadError::None: _lastError = RendererLoadError::DlopenFailed; break;
        }
        return false;
    }

    out.vtable = out.dl.entryPoint()();
    if (!validateVTable(out.vtable, _lastError)) {
        out.dl.close();
        out.vtable = nullptr;
        return false;
    }

    out.instance = out.vtable->create();
    if (!out.instance) {
        _lastError = RendererLoadError::InitFailed;
        out.dl.close();
        out.vtable = nullptr;
        return false;
    }

    if (!out.vtable->init(out.instance, rect)) {
        _lastError = RendererLoadError::InitFailed;
        out.vtable->destroy(out.instance);
        out.instance = nullptr;
        out.dl.close();
        out.vtable = nullptr;
        return false;
    }

    _lastError = RendererLoadError::None;
    return true;
}

void RendererLoader::destroyLoaded(LoadedLibrary& lib) {
    if (lib.instance) {
        lib.vtable->shutdown(lib.instance);
        lib.vtable->destroy(lib.instance);
        lib.instance = nullptr;
    }
    lib.vtable = nullptr;
    lib.dl.close();
}

bool RendererLoader::load(const std::filesystem::path& libraryPath, RendererRect rect) {
    LoadedLibrary next;
    if (!openAndValidate(libraryPath, rect, next)) {
        return false;
    }
    destroyLoaded(_current);
    _current = std::move(next);
    _rect = rect;
    return true;
}

bool RendererLoader::swap(const std::filesystem::path& newLibraryPath) {
    LoadedLibrary next;
    if (!openAndValidate(newLibraryPath, _rect, next)) {
        return false;
    }

    const bool bothSupportState = _current.instance && _current.vtable->serializeState &&
                                   next.vtable->deserializeState;
    bool stateTransferred = false;
    if (bothSupportState) {
        StateBuffer buf = _current.vtable->serializeState(_current.instance);
        stateTransferred = next.vtable->deserializeState(next.instance, buf.data, buf.size);
        _current.vtable->freeState(buf);
    }
    if (!stateTransferred) {
        if (!next.vtable->init(next.instance, _rect)) {
            _lastError = RendererLoadError::InitFailed;
            destroyLoaded(next);
            return false;
        }
    }

    destroyLoaded(_current);
    _current = std::move(next);
    _lastError = RendererLoadError::None;
    return true;
}

void RendererLoader::unload() {
    destroyLoaded(_current);
}

void RendererLoader::setRect(RendererRect rect) {
    _rect = rect;
    if (isLoaded()) {
        _current.vtable->setRect(_current.instance, rect);
    }
}

void RendererLoader::beginFrame() {
    if (isLoaded()) {
        _current.vtable->beginFrame(_current.instance);
    }
}

void RendererLoader::drawBatch(const DrawCommand* commands, size_t count) {
    if (isLoaded()) {
        _current.vtable->drawBatch(_current.instance, commands, count);
    }
}

void RendererLoader::endFrame() {
    if (isLoaded()) {
        _current.vtable->endFrame(_current.instance);
    }
}

} // namespace gw
