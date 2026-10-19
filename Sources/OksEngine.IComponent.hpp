#pragma once 

#include <ECS2.Component.hpp>

template<class Component>

struct IComponent : ECS2::IComponent<Component> {

};


template<class Component>
inline void Edit(std::shared_ptr<ECS2::World>, ECS2::Entity::Id entityId, Component* component) {

}
