#include <typeindex>
#include "gw/Hierarchy.hpp"
#include "gw/Reflection.hpp"
#include "gw/Transform.hpp"
#include "gw/World.hpp"
#include "gw/editor/app/EditorApp.hpp"

namespace {

gw::World buildDemoWorld() {
    gw::World world;

    gw::Entity a = world.createEntity();
    world.addComponent<gw::Transform>(a, gw::Transform{1.0f, 2.0f, 0.0f, 1.0f, 1.0f});

    gw::Entity b = world.createEntity();
    world.addComponent<gw::Transform>(b, gw::Transform{5.0f, 5.0f, 90.0f, 2.0f, 2.0f});
    world.addComponent<gw::Hierarchy>(b, gw::Hierarchy{a});

    gw::Entity c = world.createEntity();
    world.addComponent<gw::Transform>(c, gw::Transform{-3.0f, 0.0f, 180.0f, 1.0f, 1.0f});

    return world;
}

std::vector<gw::editor::KnownType> buildKnownTypes() {
    return {
        {std::type_index(typeid(gw::Transform)), "gw::Transform"},
        {std::type_index(typeid(gw::Hierarchy)), "gw::Hierarchy"},
    };
}

} // namespace

int main() {
    gw::World world = buildDemoWorld();
    gw::editor::EditorApp app(world, gw::ReflectionRegistry::instance(), buildKnownTypes(),
                               std::filesystem::current_path());
    app.run();
    return 0;
}
