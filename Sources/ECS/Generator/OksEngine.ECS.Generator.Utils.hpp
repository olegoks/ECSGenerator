#include <ECS/Generator/auto_OksEngine.ECS.Generator.hpp>
#include <ECS/File/OksEngine.ECS.File.Utils.hpp>

namespace ECS::Generator {

	template<class System>
	auto GET_PROJECT_INCLUDE_FILE_NAME(System* system, ECS2::Entity::Id projectEntityId) {
		return "auto_" +
			std::filesystem::path{ system->template  GetComponent<ECS::Project::Path>(projectEntityId)->path_ }.stem().string() + ".Project";
	}

	template<class System>
	auto GET_PROJECT_INCLUDE_FILE_INCLUDE_PATH(System* system, ECS2::Entity::Id projectEntityId) {
		std::string fileName = GET_PROJECT_INCLUDE_FILE_NAME(system, projectEntityId);
		return std::filesystem::path{ fileName };
	}

	template<class System>
	auto GET_MODULE_INCLUDE_FILE_NAME(System* system, ECS2::Entity::Id moduleEntityId) {
		return "auto_" +
			std::filesystem::path{ system->template GetComponent<ECS::Module::Path>(moduleEntityId)->path_ }.stem().string() + ".Module";
	}

	template<class System>
	auto GET_MODULE_INCLUDE_FILE_INCLUDE_PATH(System* system, ECS2::Entity::Id projectEntityId, ECS2::Entity::Id moduleEntityId) {
		const std::filesystem::path modulePath = system->template GetComponent<ECS::Module::Path>(moduleEntityId)->path_;
		std::filesystem::path moduleIncludePath = system->template GetComponent<ECS::Module::IncludePath>(moduleEntityId)->path_;
		std::filesystem::path moduleRelativePath = modulePath.parent_path().lexically_relative(std::filesystem::path{ system->template GetComponent<ECS::Project::Path>(projectEntityId)->path_ }.parent_path());
		std::filesystem::path fileIncludePath = moduleRelativePath.lexically_relative(moduleIncludePath.relative_path());
		return fileIncludePath;
	}

}