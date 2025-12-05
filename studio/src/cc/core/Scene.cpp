
#include "Scene.hpp"
#include <utility>



namespace cc{
Scene::Scene(std::string name) 
:name_(std::move(name)) {



};


[[nodiscard]] std::string Scene::GetSceneName() const{
    return name_;
}



}


