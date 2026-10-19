#pragma once
#include <ECS\Project\auto_OksEngine.ECS.Project.hpp>
#include <Resource/OksEngine.ResourceSystem.Utils.hpp>

namespace ECS
{
	namespace Project
	{
		void Create::Update(
			ECS2::Entity::Id entity0id,
			const CommandLineParameters* commandLineParameters0,
			const ECS::Project::Path* eCS__Project__Path0) {

			ECS2::Entity::Id projectEntityId = CreateEntity();
			CreateComponent<ECS::Project::Tag>(projectEntityId);
			CreateComponent<ECS::Project::Path>(projectEntityId, eCS__Project__Path0->path_);
			const auto projectFilePath = std::filesystem::path{ eCS__Project__Path0->path_ };
			std::vector<Common::Byte> data = Resource::LoadFileAndGetContent(projectFilePath.string());
			CreateComponent<Behaviour::LuaScript>(projectEntityId, std::string{ data.data(), data.size() });

		};

	}

} // namespace ECS
