#pragma once 

#include <Common/ECS2/ECS2.Component.hpp>
#include <Common/Lua/Lua.Common.hpp>

template<class Component>

struct IComponent : ECS2::IComponent<Component> {

};



template<class ComponentType>
bool IsBindable() {
	return ComponentType::IsBindable();
}

template<class ComponentType>
bool IsSerializable() {
	return ComponentType::IsSerializable();
}

template <class Component>
void Edit(std::shared_ptr<ECS2::World>, ECS2::Entity::Id entityId, Component* component) { }

template <class Component>
std::string Serialize(Component* component) { return ""; };

template <class Component>
Component Parse(luabridge::LuaRef& luaRef) { return Component{}; };

template <class Component>
void Bind(lua_State* state) { };