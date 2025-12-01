# cc::ecs usage examples

```cpp
#include <cc/ecs/ecs.hpp>
#include <cc/math/math.hpp>
#include <iostream>

using namespace cc;
using namespace cc::ecs;

//NOTE: Example components
struct Transform {
    cc::vec3f position{0.0f};
    cc::vec3f scale{1.0f};
};

struct Velocity {
    cc::vec3f value{0.0f};
};

struct Health {
    float value{100.0f};
};

int main() {
    Registry registry;

    //NOTE: create entities
    Entity e1 = registry.Create();
    Entity e2 = registry.Create();
    Entity e3 = registry.Create();

    //NOTE: attach components
    registry.Emplace<Transform>(e1, Transform{
        .position = {0.0f, 0.0f, 0.0f},
        .scale    = {1.0f, 1.0f, 1.0f}
    });
    registry.Emplace<Velocity>(e1, Velocity{
        .value = {1.0f, 0.0f, 0.0f}
    });
    registry.Emplace<Health>(e1, Health{100.0f});

    registry.Emplace<Transform>(e2, Transform{
        .position = {5.0f, 0.0f, 0.0f},
        .scale    = {1.0f, 1.0f, 1.0f}
    });
    registry.Emplace<Velocity>(e2, Velocity{
        .value = {-1.0f, 0.0f, 0.0f}
    });

    registry.Emplace<Transform>(e3, Transform{
        .position = {0.0f, 5.0f, 0.0f},
        .scale    = {1.0f, 1.0f, 1.0f}
    });
    registry.Emplace<Health>(e3, Health{50.0f});

    //NOTE: basic access
    if (registry.Has<Transform>(e1)) {
        auto& t = registry.Get<Transform>(e1);
        t.position.x += 10.0f;
    }

    //NOTE: simple update loop
    const float dt = 1.0f / 60.0f;

    for (int frame = 0; frame < 3; ++frame) {
        //NOTE: movement system: Transform + Velocity
        for (auto [e, transform, velocity] : registry.View<Transform, Velocity>()) {
            transform.position += velocity.value * dt;
        }

        //NOTE: damage system: Health only
        for (auto [e, health] : registry.View<Health>()) {
            health.value -= 1.0f;
        }

        //NOTE: destruction example
        for (auto [e, health] : registry.View<Health>()) {
            if (health.value <= 0.0f) {
                registry.Destroy(e);
            }
        }
    }

    //NOTE: printing final positions
    for (auto [e, transform] : registry.View<Transform>()) {
        std::cout << "Entity " << e.index
                  << " pos=("
                  << transform.position.x << ", "
                  << transform.position.y << ", "
                  << transform.position.z << ")\n";
    }

    return 0;
}
```
