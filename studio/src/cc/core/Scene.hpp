#include <cc/ecs/ecs.hpp>
#include <string>

namespace cc{

class Scene {
public:

    Scene(std::string name);

    [[nodiscard]] std::string GetSceneName() const;


    cc::ecs::Entity CreateEntity(){
        return registry_.Create();
    }


    template<typename T, typename... Args>
    T& AddComponent(cc::ecs::Entity entity, Args&&... args){
        return registry_.Emplace<T>(entity, std::forward<Args>(args)...);
    }


    template<typename T>
    T* GetComponent(cc::ecs::Entity entity){
        return registry_.Get<T>(entity);
    }

    template<typename T>
    bool HasComponent(cc::ecs::Entity entity){
        return registry_.Has<T>(entity);
    }

    cc::ecs::Registry& GetRegistry(){
        return registry_;
    }


    template<typename... Components>
    auto View(){
        return registry_.View<Components...>();
    }


private:
    cc::ecs::Registry registry_;
    std::string name_;
};

}
