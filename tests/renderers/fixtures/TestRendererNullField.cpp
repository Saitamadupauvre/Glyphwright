#include "gw/RendererVTable.hpp"

namespace {

void* create() { return nullptr; }
void destroy(void*) {}
bool init(void*, gw::RendererRect) { return true; }
void shutdown(void*) {}
void setRect(void*, gw::RendererRect) {}
void beginFrame(void*) {}
void endFrame(void*) {}

} // namespace

extern "C" const gw::RendererVTable* get_renderer_vtable() {
    static const gw::RendererVTable table{
        gw::kRendererAbiVersion,
        &create,
        &destroy,
        &init,
        &shutdown,
        &setRect,
        &beginFrame,
        nullptr, // drawBatch intentionally missing
        &endFrame,
        nullptr,
        nullptr,
        nullptr,
    };
    return &table;
}
