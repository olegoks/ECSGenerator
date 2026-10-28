#pragma once
#include <ECS\Generator\OksEngine.ECS.Generator.Utils.hpp>
#include <Resource/OksEngine.ResourceSystem.Utils.hpp>
#include <CPP/OksEngine.CPP.Tree.Utils.hpp>
#include <ECS/File/OksEngine.ECS.File.Utils.hpp>
#include <ECS/File/auto_OksEngine.ECS.File.Archetype.hpp>

#include <queue>
#include <tuple>

#include <CMakeTemplates_Generated/Help.h>
#include <CPPTemplates_Generated/auto_CPP.hpp>

extern "C" {
#include <graphviz/gvc.h>
}

#include <Common/OS/OS.FileSystem.TextFile.hpp>


//#define ECS__GENERATOR__CREATE_WORLD_PARAMETER()\
//	[this](){\
//		ECS2::Entity::Id worldParameterEntityId = CPP__TREE__DECL__CREATE_PARAMETER(\
//			CPP__TREE__TYPE__CREATE_TEMPLATE_SPECIALIZATION_TYPE(\
//				CPP__TREE__TYPE__CREATE_NAMED_TYPE("std::shared_ptr"),\
//				std::vector{ CPP__TREE__TYPE__CREATE_NAMED_TYPE("ECS2::World") }),\
//			"world");\
//	return worldParameterEntityId;\
//	}()
//
//#define CREATE_BEGIN_PROFILE_EXPR_STATEMENT(str)\
//CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(\
//	CPP__TREE__EXPR__CREATE_MACRO_INVOCATION(\
//		"BEGIN_PROFILE",\
//		CPP__TREE__CREATE_ENTITIES_VECTOR(\
//			CPP__TREE__EXPR__CREATE_LITERAL_EXPR(str, "")\
//		),\
//		ECS2::Entity::Id::invalid_\
//	)\
//)\
//
//#define CREATE_END_PROFILE_EXPR_STATEMENT() \
//CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(\
//	CPP__TREE__EXPR__CREATE_MACRO_INVOCATION(\
//		"END_PROFILE",\
//		CPP__TREE__CREATE_ENTITIES_VECTOR(),\
//		ECS2::Entity::Id::invalid_\
//	)\
//)

/* ======================== GENERATOR / PROFILE HELPERS ======================== */

template<class System>
ECS2::Entity::Id ECS__GENERATOR__CREATE_WORLD_PARAMETER(System* system) {
	return CPP__TREE__DECL__CREATE_PARAMETER(
		system,
		CPP__TREE__TYPE__CREATE_TEMPLATE_SPECIALIZATION_TYPE(
			system,
			CPP__TREE__TYPE__CREATE_NAMED_TYPE(system, "std::shared_ptr"),
			std::vector<ECS2::Entity::Id>{
		CPP__TREE__TYPE__CREATE_NAMED_TYPE(system, "ECS2::World")
	}),
		"world");
}

template<class System>
ECS2::Entity::Id CREATE_BEGIN_PROFILE_EXPR_STATEMENT(
	System* system,
	const std::string& str) {
	return CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(
		system,
		CPP__TREE__EXPR__CREATE_MACRO_INVOCATION(
			system,
			"BEGIN_PROFILE",
			std::vector<ECS2::Entity::Id>{
		CPP__TREE__EXPR__CREATE_LITERAL_EXPR(system, str, "")
	},
			ECS2::Entity::Id::invalid_));
}

template<class System>
ECS2::Entity::Id CREATE_END_PROFILE_EXPR_STATEMENT(System* system) {
	return CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(
		system,
		CPP__TREE__EXPR__CREATE_MACRO_INVOCATION(
			system,
			"END_PROFILE",
			std::vector<ECS2::Entity::Id>{},
			ECS2::Entity::Id::invalid_));
}

template<class System>
std::tuple<
	std::vector<ECS2::Entity::Id>,
	std::vector<ECS2::Entity::Id>,
	std::vector<ECS2::Entity::Id>,
	std::vector<ECS2::Entity::Id>,
	std::vector<ECS2::Entity::Id>,
	std::vector<ECS2::Entity::Id>,
	std::vector<ECS2::Entity::Id>> ECS__GENERATOR__GET_FILE_TABLES(System* system, ECS2::Entity::Id fileEntityId) {

	std::vector<ECS2::Entity::Id> fileStructs;
	std::vector<ECS2::Entity::Id> fileConstants;
	std::vector<ECS2::Entity::Id> fileComponents;
	std::vector<ECS2::Entity::Id> fileSystems;
	std::vector<ECS2::Entity::Id> fileEnums;
	std::vector<ECS2::Entity::Id> fileArchetypes;//deprecated
	std::vector<ECS2::Entity::Id> fileBundles;

	std::function<void(ECS2::Entity::Id)> getFileTables = [&](ECS2::Entity::Id entityId) {
		const ECS2::ComponentsFilter tableComponentsFilter = system->GetComponentsFilter(entityId);

		//Table is namespace.
		if (tableComponentsFilter.IsSet<ECS::File::Table::Namespace::Tag>()) {
			for (ECS2::Entity::Id childTableEntityId : system->template GetComponent<ECS::File::Table::ChildTablesEntityIds>(entityId)->entityIds_) {
				getFileTables(childTableEntityId);
			}
		}
		else if (tableComponentsFilter.IsSet<ECS::File::Table::System::Tag>()) {
			fileSystems.push_back(entityId);
		}
		else if (tableComponentsFilter.IsSet<ECS::File::Table::Component::Tag>()) {
			fileComponents.push_back(entityId);
		}
		else if (tableComponentsFilter.IsSet<ECS::File::Table::Struct::Tag>()) {
			fileStructs.push_back(entityId);
		}
		else if (tableComponentsFilter.IsSet<ECS::File::Table::Constant::Tag>()) {
			fileConstants.push_back(entityId);
		}
		//deprecated
		else if (tableComponentsFilter.IsSet<ECS::File::Table::Archetype::Tag>()) {
			fileArchetypes.push_back(entityId);
		}
		else if (tableComponentsFilter.IsSet<ECS::File::Table::Bundle::Tag>()) {
			fileBundles.push_back(entityId);
		}
		else if (tableComponentsFilter.IsSet<ECS::File::Table::Enum::Tag>()) {
			fileEnums.push_back(entityId);
		}
		else {
			ASSERT_FAIL();
		}
		};

	auto* fileTables = system->template GetComponent<ECS::File::Table::EntityIds>(fileEntityId);

	for (ECS2::Entity::Id tableEntityId : fileTables->entityIds_) {
		getFileTables(tableEntityId);
	}


	//Sort structs by theire definition order.
	std::sort(fileStructs.begin(), fileStructs.end(),
		[&](ECS2::Entity::Id first, ECS2::Entity::Id second) {
			const ECS2::ComponentsFilter firstCF = system->GetComponentsFilter(first);
			const Common::Index firstOrderIndex = system->template GetComponent<ECS::File::Table::DefinitionOrder>(first)->orderIndex_;
			const ECS2::ComponentsFilter secondCF = system->GetComponentsFilter(second);
			const Common::Index secondOrderIndex = system->template GetComponent<ECS::File::Table::DefinitionOrder>(second)->orderIndex_;
			return firstOrderIndex < secondOrderIndex;
		});
	std::sort(fileArchetypes.begin(), fileArchetypes.end(),
		[&](ECS2::Entity::Id first, ECS2::Entity::Id second) {
			const ECS2::ComponentsFilter firstCF = system->GetComponentsFilter(first);
			const Common::Index firstOrderIndex = system->template GetComponent<ECS::File::Table::DefinitionOrder>(first)->orderIndex_;
			const ECS2::ComponentsFilter secondCF = system->GetComponentsFilter(second);
			const Common::Index secondOrderIndex = system->template GetComponent<ECS::File::Table::DefinitionOrder>(second)->orderIndex_;
			return firstOrderIndex < secondOrderIndex;
		});

	return { fileStructs, fileConstants, fileComponents, fileSystems, fileEnums, fileArchetypes, fileBundles };

}

template<class System>
std::vector<ECS2::Entity::Id> ECS__GENERATOR__GENERATE_HPP_INCLUDES(
	System* system,
	ECS2::Entity::Id fileEntityId,
	const std::filesystem::path& projectFilePath,
	const std::vector<ECS2::Entity::Id>& fileStructs,
	const std::vector<ECS2::Entity::Id>& fileComponents,
	const std::vector<ECS2::Entity::Id>& fileConstants,
	const std::vector<ECS2::Entity::Id>& fileArchetypes,
	const std::vector<ECS2::Entity::Id>& fileSystems) {

	std::vector<Common::Byte> data = Resource::LoadFileAndGetContent(projectFilePath.string());

	::Lua::Context context;
	::Lua::Script script{ std::string{ data.data(), data.size() } };
	context.LoadScript(script);
	luabridge::LuaRef ecsTable = luabridge::getGlobal(context.state_, "ECS");
	ASSERT_FMSG(ecsTable.isTable(), "");
	luabridge::LuaRef includesTable = ecsTable["Includes"];


	std::unordered_set<std::string> includes;

	includes.insert("lua.hpp");
	includes.insert("Common/ECS2/ECS2.hpp");
	includes.insert("chrono");
	includes.insert("tuple");
	includes.insert("OksEngine.IComponent.hpp");
	includes.insert("Common/Common.hpp");
	includes.insert("Common/Common.Bitset.hpp");

	includes.insert("luabridge3/LuaBridge/LuaBridge.h");
	includes.insert("Common/OS/OS.Assert.hpp");

	auto getIncludesByTypeName = [&](const std::string& typeName) {
		std::vector<std::string> includes;
		if (includesTable.isTable()) {

			for (luabridge::Iterator it(includesTable); !it.isNil(); ++it) {
				// Каждый элемент таблицы — это LuaRef, который должен быть таблицей
				luabridge::LuaRef ruleTable = it.value();

				if (ruleTable.isTable()) {

					bool needToIncludeHeader = false;
					// Извлекаем массив "Entries"
					luabridge::LuaRef entriesRef = ruleTable["Entries"];
					if (entriesRef.isTable()) {
						for (luabridge::Iterator entIt(entriesRef); !entIt.isNil(); ++entIt) {
							// Значение должно быть строкой
							if (entIt.value().isString()) {
								std::string key = entIt.value().cast<std::string>().value();
								if (typeName.find(key) != std::string::npos) {
									needToIncludeHeader = true;
								}
							}
						}
					}

					// Аналогично извлекаем массив "Headers"
					luabridge::LuaRef headersRef = ruleTable["Headers"];
					if (headersRef.isTable()) {
						for (luabridge::Iterator hdrIt(headersRef); !hdrIt.isNil(); ++hdrIt) {
							if (hdrIt.value().isString() && needToIncludeHeader) {
								includes.push_back(hdrIt.value().cast<std::string>().value());
							}
						}
					}
				}
			}
		}
		return includes;
		};

	for (ECS2::Entity::Id ecsStructEntityId : fileStructs) {
		if (system->template IsComponentExist<ECS::File::Table::Struct::Field::EntityIds>(ecsStructEntityId)) {
			for (ECS2::Entity::Id fieldEntityId : system->template GetComponent<ECS::File::Table::Struct::Field::EntityIds>(ecsStructEntityId)->entityIds_) {
				auto structIncludes = getIncludesByTypeName(system->template GetComponent<ECS::File::Table::Struct::Field::Type>(fieldEntityId)->name_);
				for (auto include : structIncludes) {
					includes.insert(include);
				}
			}
		}
	}

	for (ECS2::Entity::Id ecsComponentEntityId : fileComponents) {
		if (system->template IsComponentExist<ECS::File::Table::Component::Field::EntityIds>(ecsComponentEntityId)) {
			for (ECS2::Entity::Id fieldEntityId : system->template GetComponent<ECS::File::Table::Component::Field::EntityIds>(ecsComponentEntityId)->entityIds_) {
				auto componentIncludes = getIncludesByTypeName(system->template GetComponent<ECS::File::Table::Component::Field::Type>(fieldEntityId)->name_);
				for (auto include : componentIncludes) {
					includes.insert(include);
				}
			}
		}
	}

	for (ECS2::Entity::Id ecsConstantEntityId : fileConstants) {
		const std::string typeName = system->template GetComponent<ECS::File::Table::Constant::TypeName>(ecsConstantEntityId)->type_;
		auto contantIncludes = getIncludesByTypeName(typeName);
		for (auto include : contantIncludes) {
			includes.insert(include);
		}
	}


	for (ECS2::Entity::Id archetypeEntityId : fileArchetypes) {
		const ECS2::ComponentsFilter archetypeCF = system->GetComponentsFilter(archetypeEntityId);
		ASSERT(archetypeCF.IsSet<ECS::File::Table::Archetype::Tag>());

		std::vector<ECS2::Entity::Id> componentEntityIds;
		if (archetypeCF.IsSet<ECS::File::Table::Archetype::Components>()) {
			auto* components = system-> template GetComponent<ECS::File::Table::Archetype::Components>(archetypeEntityId);
			for (std::string component : components->components_) {
				ECS2::Entity::Id componentEntityId = ECS::File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<System, ECS::File::Table::Component::Tag>(system, archetypeEntityId, component);
				ECS2::Entity::Id ecsFileEntityId = ECS::File::Table::ECS__FILE__TABLE__GET_TABLE_FILE_ENTITY(system, componentEntityId);
				const std::filesystem::path ecsFileIncludePath = ECS::File::Table::ECS__FILE__GET_FILE_INCLUDE_PATH(system, ecsFileEntityId, projectFilePath, ".hpp");

				includes.insert(ecsFileIncludePath.string());
			}
		}

		std::vector<ECS2::Entity::Id> archetypeEntityIds;
		if (archetypeCF.IsSet<ECS::File::Table::Archetype::Archetypes>()) {
			auto* archetypes = system->template GetComponent<ECS::File::Table::Archetype::Archetypes>(archetypeEntityId);
			for (std::string archetype : archetypes->archetypes_) {
				ECS2::Entity::Id refArchetypeEntityId = ECS::File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<System, ECS::File::Table::Archetype::Tag>(system, archetypeEntityId, archetype);
				ECS2::Entity::Id ecsFileEntityId = ECS::File::Table::ECS__FILE__TABLE__GET_TABLE_FILE_ENTITY(system, refArchetypeEntityId);
				const std::filesystem::path ecsFileIncludePath = ECS::File::Table::ECS__FILE__GET_FILE_INCLUDE_PATH(system, ecsFileEntityId, projectFilePath, ".hpp");

				includes.insert(ecsFileIncludePath.string());
			}
		}
	}

	//Generate system component includes.
	for (ECS2::Entity::Id systemEntityId : fileSystems) {

		//if (ECS__FILE__TABLE__SYSTEM__IS_DISABLED(systemEntityId)) {
		//	continue;
		//}
		const std::string systemFullName = ECS::File::Table::ECS__FILE__TABLE__GET_FULL_NAME(system, systemEntityId, "::", false);

		if (system->template IsComponentExist<ECS::File::Table::System::UpdateMethod::EntityId>(systemEntityId)) {
			ECS2::Entity::Id updateMethodEntityId
				= system->template GetComponent<ECS::File::Table::System::UpdateMethod::EntityId>(systemEntityId)->id_;
			if (system->template IsComponentExist<ECS::File::Table::System::UpdateMethod::Access::Entity::EntityIds>(updateMethodEntityId)) {
				::std::vector<ECS2::Entity::Id> accessesEntities
					= system->template GetComponent<ECS::File::Table::System::UpdateMethod::Access::Entity::EntityIds>(updateMethodEntityId)->entityIds_;
				for (ECS2::Entity::Id accessEntity : accessesEntities) {
					//archetype
					if (system->template IsComponentExist<ECS::File::Table::System::UpdateMethod::Access::Entity::Archetype>(accessEntity)) {
						const std::string archetypeLocalName = system->template GetComponent<ECS::File::Table::System::UpdateMethod::Access::Entity::Archetype>(accessEntity)->name_;
						ECS2::Entity::Id archetypeEntityId = ECS::File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<System, ECS::File::Table::Archetype::Tag>(system, systemEntityId, archetypeLocalName);
						ECS2::Entity::Id fileEntityId = ECS::File::Table::ECS__FILE__TABLE__GET_TABLE_FILE_ENTITY(system, archetypeEntityId);
						includes.insert(ECS::File::Table::ECS__FILE__GET_FILE_INCLUDE_PATH(system, fileEntityId, projectFilePath, ".hpp").string());
					}
				}
			}
		}

		std::vector<ECS2::Entity::Id> allComponents = ECS::File::Table::ECS__FILE__TABLE__SYSTEM__GET_COMPONENTS_ENTITY_IDS(system, systemEntityId);

		for (const ECS2::Entity::Id componentEntityId : allComponents) {
			ECS2::Entity::Id ecsFileEntityId = ECS::File::Table::ECS__FILE__TABLE__GET_TABLE_FILE_ENTITY(system, componentEntityId);
			if (ecsFileEntityId == fileEntityId) {
				continue;
			}
			const std::filesystem::path ecsFileIncludePath = ECS::File::Table::ECS__FILE__GET_FILE_INCLUDE_PATH(system, ecsFileEntityId, projectFilePath, ".hpp");

			includes.insert(ecsFileIncludePath.string());
		}
	}

	std::vector<ECS2::Entity::Id> includeNodeEntities;
	for (const std::string& include : includes) {
		includeNodeEntities.push_back(CPP__TREE__PREPROCESSOR__CREATE_INCLUDE(system, include));
	}

	return includeNodeEntities;
}


template<class System>
std::vector<ECS2::Entity::Id> ECS__GENERATOR__GENERATE_CPP__INCLUDES(System* system, const std::filesystem::path& projectFilePath, ECS2::Entity::Id fileEntityId) {

	std::vector<ECS2::Entity::Id> includeNodeEntities;

	const std::filesystem::path filePath = system->template GetComponent<ECS::File::Path>(fileEntityId)->path_;
	const std::filesystem::path correspondingHppFileIncludePath = ECS::File::Table::ECS__FILE__GET_FILE_INCLUDE_PATH(system, fileEntityId, projectFilePath, ".hpp");
	std::string correspondingHppFileIncludePathStr = correspondingHppFileIncludePath.generic_string();
	std::replace(correspondingHppFileIncludePathStr.begin(), correspondingHppFileIncludePathStr.end(), '\\', '/');
	includeNodeEntities.push_back(CPP__TREE__PREPROCESSOR__CREATE_INCLUDE(system, correspondingHppFileIncludePathStr));
	includeNodeEntities.push_back(CPP__TREE__PREPROCESSOR__CREATE_INCLUDE(system, "imgui.h"));
	includeNodeEntities.push_back(CPP__TREE__PREPROCESSOR__CREATE_INCLUDE(system, "misc/cpp/imgui_stdlib.h"));

	return includeNodeEntities;
}


namespace ECS::Generator
{


	//template<class System>
	//std::vector<ECS2::Entity::Id> GENERATE_STRUCTS_FORWARD_DECLARATIONS(System* system, const std::vector<ECS2::Entity::Id>& fileStructs) {

	//	std::vector<ECS2::Entity::Id> structForwardDeclNodes;

	//	//Generate structs forward declarations .
	//	for (ECS2::Entity::Id structEntityId : fileStructs) {

	//		const ECS2::ComponentsFilter structCF = system->GetComponentsFilter(structEntityId);
	//		ASSERT(structCF.IsSet<File::Table::Struct::Tag>());

	//		const ECS2::Entity::Id cppStructEntityId = system->CreateEntity();
	//		const auto ecsParentTables = ECS::File::Table::GET_TABLE_PARENTS(system, structEntityId);
	//		const auto [firstNamespaceEntityId, lastNamespaceEntityId] = CPP__TREE__CREATE_CPP_NAMESPACES(system, ecsParentTables, std::vector<ECS2::Entity::Id>{cppStructEntityId});

	//		system->template CreateComponent<CPP::Tree::Node::Tag>(cppStructEntityId);
	//		system->template CreateComponent<CPP::Tree::Decl::Tag>(cppStructEntityId);
	//		system->template CreateComponent<CPP::Tree::Decl::Class_>(cppStructEntityId);
	//		system->template CreateComponent<CPP::Tree::Node::Name>(cppStructEntityId, system->template GetComponent<File::Table::Name>(structEntityId)->name_);

	//		if (lastNamespaceEntityId.IsValid()) {
	//			system->template CreateComponent<CPP::Tree::Node::ParentEntityId>(cppStructEntityId, lastNamespaceEntityId);
	//		}
	//		if (firstNamespaceEntityId.IsValid()) {
	//			structForwardDeclNodes.push_back(firstNamespaceEntityId);
	//		}
	//		else {
	//			structForwardDeclNodes.push_back(cppStructEntityId);
	//		}
	//	}

	//	return structForwardDeclNodes;

	//}
	//template<class System>
	//std::vector<ECS2::Entity::Id> GENERATE_CONSTANTS(System* system, const std::vector<ECS2::Entity::Id>& fileConstants) {

	//	std::vector<ECS2::Entity::Id> constantNodes;

	//	//Generate constants code tree.
	//	for (ECS2::Entity::Id constantEntityId : fileConstants) {
	//		const ECS2::ComponentsFilter constantCF = system->GetComponentsFilter(constantEntityId);
	//		ASSERT(constantCF.IsSet<File::Table::Constant::Tag>());

	//		const ECS2::Entity::Id cppConstantEntityId = system->CreateEntity();
	//		const auto parentTables = ECS__FILE__TABLE__GET_TABLE_PARENTS(system, constantEntityId);
	//		const auto [firstNamespaceEntityId, lastNamespaceEntityId] = CPP__TREE__CREATE_CPP_NAMESPACES(system, parentTables, std::vector<ECS2::Entity::Id>{cppConstantEntityId});

	//		CPP__TREE__DECL__CREATE_VARIABLE_COMPONENTS(system,
	//			cppConstantEntityId,
	//			CPP__TREE__TYPE__CREATE_NAMED_TYPE(system->template GetComponent<File::Table::Constant::TypeName>(constantEntityId)->type_),
	//			system->template GetComponent<File::Table::Name>(constantEntityId)->name_,
	//			CPP__TREE__EXPR__CREATE_LITERAL_EXPR(system->template GetComponent<File::Table::Constant::Value>(constantEntityId)->value_, ""),
	//			lastNamespaceEntityId
	//		);

	//		system->template GetComponent<File::Table::Constant::TypeName>(constantEntityId)->type_;

	//		if (firstNamespaceEntityId.IsValid()) {
	//			constantNodes.push_back(firstNamespaceEntityId);
	//		}
	//		else {
	//			constantNodes.push_back(cppConstantEntityId);
	//		}
	//	}

	//	return constantNodes;
	//}

	//template<class System>
	//std::vector<ECS2::Entity::Id> GENERATE_ENUMS(System* system, const std::vector<ECS2::Entity::Id>& fileEnums) {
	//	std::vector<ECS2::Entity::Id> enumsNodes;
	//	//Generate Enums code tree.
	//	for (ECS2::Entity::Id enumEntityId : fileEnums) {
	//		const ECS2::ComponentsFilter enumCF = system->GetComponentsFilter(enumEntityId);
	//		ASSERT(enumCF.IsSet<File::Table::Enum::Tag>());

	//		const ECS2::Entity::Id cppEnumEntityId = system->CreateEntity();
	//		const auto ecsParentTables = ECS::File::Table::GET_TABLE_PARENTS(system, enumEntityId);
	//		const auto [firstNamespaceEntityId, lastNamespaceEntityId] = CPP__TREE__CREATE_CPP_NAMESPACES(ecsParentTables, std::vector<ECS2::Entity::Id>{cppEnumEntityId});

	//		system->template CreateComponent<CPP::Tree::Node::Tag>(cppEnumEntityId);
	//		system->template CreateComponent<CPP::Tree::Decl::Tag>(cppEnumEntityId);
	//		system->template CreateComponent<CPP::Tree::Decl::Enum_>(cppEnumEntityId);
	//		system->template CreateComponent<CPP::Tree::Node::Name>(cppEnumEntityId, system->template GetComponent<File::Table::Name>(enumEntityId)->name_);

	//		std::vector<ECS2::Entity::Id> enumeratorEntityIds;
	//		for (const std::string value : system->template GetComponent<ECS::File::Table::Enum::Values>(enumEntityId)->values_) {
	//			ECS2::Entity::Id enumeratorEntityId = system->CreateEntity();
	//			system->template CreateComponent<CPP::Tree::Node::Tag>(enumeratorEntityId);
	//			system->template CreateComponent<CPP::Tree::Decl::Tag>(enumeratorEntityId);
	//			system->template CreateComponent<CPP::Tree::Decl::Enumerator>(enumeratorEntityId, value, ECS2::Entity::Id::invalid_); // Without 
	//			system->template CreateComponent<CPP::Tree::Node::Name>(enumeratorEntityId, value);
	//			system->template CreateComponent<CPP::Tree::Node::ParentEntityId>(enumeratorEntityId, cppEnumEntityId);
	//			enumeratorEntityIds.push_back(enumeratorEntityId);
	//		}
	//		system->template CreateComponent<CPP::Tree::Node::ChildEntityIds>(cppEnumEntityId, enumeratorEntityIds);

	//		if (lastNamespaceEntityId.IsValid()) {
	//			system->template CreateComponent<CPP::Tree::Node::ParentEntityId>(cppEnumEntityId, lastNamespaceEntityId);
	//		}
	//		if (firstNamespaceEntityId.IsValid()) {
	//			enumsNodes.push_back(firstNamespaceEntityId);
	//		}
	//		else {
	//			enumsNodes.push_back(cppEnumEntityId);
	//		}
	//	}
	//	return enumsNodes;
	//}

	//template<class System>
	//std::vector<ECS2::Entity::Id> GENERATE_STRUCTS(System* system, const std::vector<ECS2::Entity::Id>& fileStructs) {

	//	std::vector<ECS2::Entity::Id> structDeclNodes;

	//	//Generate structs code tree.
	//	for (ECS2::Entity::Id structEntityId : fileStructs) {

	//		const ECS2::ComponentsFilter structCF = system->GetComponentsFilter(structEntityId);
	//		ASSERT(structCF.IsSet<File::Table::Struct::Tag>());

	//		const ECS2::Entity::Id cppStructEntityId = system->CreateEntity();
	//		const auto ecsParentTables = ECS::File::Table::GET_TABLE_PARENTS(structEntityId);
	//		const auto [firstNamespaceEntityId, lastNamespaceEntityId] = CPP__TREE__CREATE_CPP_NAMESPACES(system, ecsParentTables, std::vector<ECS2::Entity::Id>{cppStructEntityId});

	//		{
	//			system->template CreateComponent<CPP::Tree::Node::Tag>(cppStructEntityId);
	//			system->template CreateComponent<CPP::Tree::Decl::Tag>(cppStructEntityId);
	//			system->template CreateComponent<CPP::Tree::Decl::Class_>(cppStructEntityId);
	//			system->template CreateComponent<CPP::Tree::Node::Name>(cppStructEntityId, system->template GetComponent<File::Table::Name>(structEntityId)->name_);

	//			std::vector<ECS2::Entity::Id> childEntityIds;
	//			std::vector<ECS2::Entity::Id> fieldEntityIds;
	//			if (system->template IsComponentExist<ECS::File::Table::Struct::Field::EntityIds>(structEntityId)) {
	//				for (ECS2::Entity::Id fieldEntityId : system->template GetComponent<ECS::File::Table::Struct::Field::EntityIds>(structEntityId)->entityIds_) {
	//					ASSERT(system->template IsComponentExist<ECS::File::Table::Struct::Field::Tag>(fieldEntityId));

	//					ECS2::Entity::Id variableEntityId = CPP__TREE__DECL__CREATE_VARIABLE(
	//						system,
	//						CPP__TREE__TYPE__CREATE_NAMED_TYPE(system, system->template GetComponent<File::Table::Struct::Field::Type>(fieldEntityId)->name_),
	//						system->template GetComponent<File::Table::Struct::Field::Name>(fieldEntityId)->name_ + "_",
	//						(system->template IsComponentExist<File::Table::Struct::Field::DefaultValue>(fieldEntityId))
	//						? (CPP__TREE__EXPR__CREATE_LITERAL_EXPR(system, system->template GetComponent<File::Table::Struct::Field::DefaultValue>(fieldEntityId)->value_, ""))
	//						: (ECS2::Entity::Id::invalid_),
	//						cppStructEntityId
	//					);
	//					system->template CreateComponent<CPP::Tree::Access::Public_>(variableEntityId);

	//					fieldEntityIds.push_back(variableEntityId);
	//					childEntityIds.push_back(variableEntityId);
	//				}
	//			}
	//			//Default constructor.
	//			{
	//				ECS2::Entity::Id constructorEntityId = CPP__TREE__DECL__CREATE_EMPTY_CONSTRUCTOR(
	//					system,
	//					system->template GetComponent<File::Table::Name>(structEntityId)->name_,
	//					cppStructEntityId, ECS2::Entity::Id::invalid_);
	//				system->template CreateComponent<CPP::Tree::Decl::Constructor>(constructorEntityId);
	//				system->template CreateComponent<CPP::Tree::Access::Public_>(constructorEntityId);

	//				childEntityIds.push_back(constructorEntityId);
	//			}

	//			//Constructor with parameters.
	//			if (!fieldEntityIds.empty()) {

	//				const ECS2::Entity::Id constructorRealizationEntityId = system->CreateEntity();

	//				const auto [firstConstructorRealizationNamespaceEntityId, lastConstructorRealizationNamespaceEntityId] = CPP__TREE__CREATE_CPP_NAMESPACES(
	//					ecsParentTables,
	//					CPP__TREE__CREATE_ENTITIES_VECTOR(
	//						constructorRealizationEntityId
	//					)
	//				);

	//				CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(
	//					system,
	//					constructorRealizationEntityId,
	//					ECS2::Entity::Id::invalid_,
	//					CPP__TREE__CREATE_ENTITIES_VECTOR(),
	//					system->template GetComponent<File::Table::Name>(structEntityId)->name_,
	//					CPP__TREE__CREATE_ENTITIES_VECTOR(),
	//					lastConstructorRealizationNamespaceEntityId,
	//					[&]() {
	//						std::vector<ECS2::Entity::Id> parameterEntityIds;
	//						for (ECS2::Entity::Id fieldEntityId : system->template GetComponent<ECS::File::Table::Struct::Field::EntityIds>(structEntityId)->entityIds_) {
	//							ECS2::Entity::Id parameterEntityId = system->CreateEntity();
	//							system->template CreateComponent<CPP::Tree::Node::Tag>(parameterEntityId);
	//							system->template CreateComponent<CPP::Tree::Decl::Tag>(parameterEntityId);

	//							//Type entity id.
	//							ECS2::Entity::Id typeEntityId = CPP__TREE__TYPE__CREATE_NAMED_TYPE(system, system->template GetComponent<File::Table::Struct::Field::Type>(fieldEntityId)->name_);

	//							system->template CreateComponent<CPP::Tree::Decl::Parameter>(parameterEntityId,
	//								system->template GetComponent<ECS::File::Table::Struct::Field::Name>(fieldEntityId)->name_,
	//								typeEntityId,
	//								ECS2::Entity::Id::invalid_, // There is no initializer.
	//								false
	//							);
	//							parameterEntityIds.push_back(parameterEntityId);
	//						}
	//						parameterEntityIds.push_back(CPP__TREE__STMT__CREATE_EMPTY_COMPOUND_STATEMENT(system));
	//						return parameterEntityIds;


	//					}()
	//						);

	//				system->template CreateComponent<CPP::Tree::Decl::Constructor>(constructorRealizationEntityId, cppStructEntityId);
	//				system->template CreateComponent<CPP::Tree::ConstructorInitList>(
	//					constructorRealizationEntityId,
	//					[&]() {

	//						auto ecsStructFields = system->template GetComponent<ECS::File::Table::Struct::Field::EntityIds>(structEntityId)->entityIds_;
	//						ASSERT(ecsStructFields.size() == fieldEntityIds.size());
	//						std::vector<ECS2::Entity::Id> initializerEntityIds;
	//						for (Common::Index i = 0; i < ecsStructFields.size(); i++) {
	//							ECS2::Entity::Id ecsFieldEntityId = ecsStructFields[i];
	//							const std::string ecsFieldName = system->template GetComponent<ECS::File::Table::Struct::Field::Name>(ecsFieldEntityId)->name_;
	//							initializerEntityIds.push_back(CPP__TREE__CREATE_CONSTRUCTOR_INITIALIZER(
	//								system, 
	//								fieldEntityIds[i],
	//								CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(system, ecsFieldName)
	//							));
	//						}
	//						return initializerEntityIds;
	//					}());

	//				if (firstConstructorRealizationNamespaceEntityId.IsValid()) {
	//					structMethodNodes.push_back(firstConstructorRealizationNamespaceEntityId);
	//				}
	//				else {
	//					structMethodNodes.push_back(constructorRealizationEntityId);
	//				}


	//				ECS2::Entity::Id constructorPrototypeEntityId = CPP__TREE__DECL__CREATE_CONSTRUCTOR(
	//					system->template GetComponent<File::Table::Name>(structEntityId)->name_,
	//					CPP__TREE__CREATE_ENTITIES_VECTOR(),
	//					lastNamespaceEntityId,
	//					ECS2::Entity::Id::invalid_,
	//					[&]() {
	//						std::vector<ECS2::Entity::Id> parameterEntityIds;
	//						for (ECS2::Entity::Id fieldEntityId : system->template GetComponent<ECS::File::Table::Struct::Field::EntityIds>(structEntityId)->entityIds_) {
	//							ECS2::Entity::Id parameterEntityId = system->CreateEntity();
	//							system->template CreateComponent<CPP::Tree::Node::Tag>(parameterEntityId);
	//							system->template CreateComponent<CPP::Tree::Decl::Tag>(parameterEntityId);

	//							//Type entity id.
	//							ECS2::Entity::Id typeEntityId = CPP__TREE__TYPE__CREATE_NAMED_TYPE(system->template GetComponent<File::Table::Struct::Field::Type>(fieldEntityId)->name_);

	//							system->template CreateComponent<CPP::Tree::Decl::Parameter>(parameterEntityId,
	//								system->template GetComponent<ECS::File::Table::Struct::Field::Name>(fieldEntityId)->name_,
	//								typeEntityId,
	//								ECS2::Entity::Id::invalid_, // There is no initializer.
	//								false
	//							);
	//							parameterEntityIds.push_back(parameterEntityId);
	//						}
	//						//parameterEntityIds.push_back(CPP__TREE__STMT__CREATE_EMPTY_COMPOUND_STATEMENT());
	//						return parameterEntityIds;


	//					}());

	//				childEntityIds.push_back(constructorPrototypeEntityId);

	//			}

	//			system->template CreateComponent<CPP::Tree::Node::ChildEntityIds>(cppStructEntityId, childEntityIds);
	//		}
	//		if (lastNamespaceEntityId.IsValid()) {
	//			system->template CreateComponent<CPP::Tree::Node::ParentEntityId>(cppStructEntityId, lastNamespaceEntityId);
	//		}
	//		if (firstNamespaceEntityId.IsValid()) {
	//			structDeclNodes.push_back(firstNamespaceEntityId);
	//		}
	//		else {
	//			structDeclNodes.push_back(cppStructEntityId);
	//		}
	//	}
	//	return structDeclNodes;
	//}

	//.ecs file to .hpp file.
	void GenerateTrees::Update(
		ECS2::Entity::Id entity0id,
		const ECS::Project::Tag* eCS__Project__Tag0,
		const ECS::Project::Path* eCS__Project__Path0,
		const Behaviour::LuaScript* luaScript0) {


		const auto projectFilePath = std::filesystem::path{ eCS__Project__Path0->path_ };


		const auto allFiles = world_->GetComponents<ECS__FILE__FILE>();
		const Common::Size allFilesNumber = world_->GetEntitiesNumber<ECS__FILE__FILE>();

		Common::Size componentTypeId = 0;

		for (Common::Index i = 0; i < allFilesNumber; i++) {

			const auto* path = std::get<ECS::File::Path*>(allFiles) + i;
			ECS2::Entity::Id fileEntityId = *(std::get<ECS2::Entity::Id*>(allFiles) + i);

			//All file TABLES

			auto [fileStructs,
				fileConstants,
				fileComponents,
				fileSystems,
				fileEnums,
				fileArchetypes,
				fileBundles] = ECS__GENERATOR__GET_FILE_TABLES(this, fileEntityId);


			//HPP File
			ECS2::Entity::Id hppFileEntityId = CreateEntity<CPP__FILE__FILE>();
			std::vector<ECS2::Entity::Id> hppFileNodeEntityIds;

			//HPP File Add includes.
			{
				CreateComponent<CPP::File::Tag>(hppFileEntityId);
				CreateComponent<CPP::File::Type::Hpp>(hppFileEntityId);
				//File PATH and NAME
				CreateComponent<CPP::File::Path>(hppFileEntityId, std::filesystem::path{ path->path_ }.parent_path().string());
				CreateComponent<CPP::File::Name>(hppFileEntityId, "auto_" + std::filesystem::path{ path->path_ }.stem().string());

				std::vector<ECS2::Entity::Id> includeNodeEntities = ECS__GENERATOR__GENERATE_HPP_INCLUDES(
					this,
					fileEntityId,
					std::filesystem::path{ eCS__Project__Path0->path_ },
					fileStructs,
					fileComponents,
					fileConstants,
					fileArchetypes,
					fileSystems);

				includeNodeEntities.insert(includeNodeEntities.begin(), CPP__TREE__PREPROCESSOR__CREATE_PRAGMA_ONCE(this));

				hppFileNodeEntityIds.insert(hppFileNodeEntityIds.end(), includeNodeEntities.begin(), includeNodeEntities.end());
			}

			//CPP File
			ECS2::Entity::Id cppFileEntityId = CreateEntity<CPP__FILE__FILE>();
			std::vector<ECS2::Entity::Id> cppFileNodeEntityIds;
			{
				CreateComponent<CPP::File::Tag>(cppFileEntityId);
				CreateComponent<CPP::File::Type::Cpp>(cppFileEntityId);

				//File PATH and NAME
				CreateComponent<CPP::File::Path>(cppFileEntityId, std::filesystem::path{ path->path_ }.parent_path().string());
				CreateComponent<CPP::File::Name>(cppFileEntityId, "auto_" + std::filesystem::path{ path->path_ }.stem().string());

				std::vector<ECS2::Entity::Id> includeNodeEntities = ECS__GENERATOR__GENERATE_CPP__INCLUDES(this, std::filesystem::path{ eCS__Project__Path0->path_ }, fileEntityId);
				cppFileNodeEntityIds.insert(cppFileNodeEntityIds.end(), includeNodeEntities.begin(), includeNodeEntities.end());
			}

			//Generate structs forward declarations .
			for (ECS2::Entity::Id structEntityId : fileStructs) {

				const ECS2::ComponentsFilter structCF = GetComponentsFilter(structEntityId);
				ASSERT(structCF.IsSet<File::Table::Struct::Tag>());

				const ECS2::Entity::Id cppStructEntityId = CreateEntity();
				const auto ecsParentTables = ECS::File::Table::GET_TABLE_PARENTS(this, structEntityId);
				const auto [firstNamespaceEntityId, lastNamespaceEntityId] = CPP__TREE__CREATE_CPP_NAMESPACES(this, ecsParentTables, std::vector<ECS2::Entity::Id>{cppStructEntityId});

				CreateComponent<CPP::Tree::Node::Tag>(cppStructEntityId);
				CreateComponent<CPP::Tree::Decl::Tag>(cppStructEntityId);
				CreateComponent<CPP::Tree::Decl::Class_>(cppStructEntityId);
				CreateComponent<CPP::Tree::Node::Name>(cppStructEntityId, GetComponent<File::Table::Name>(structEntityId)->name_);

				if (lastNamespaceEntityId.IsValid()) {
					CreateComponent<CPP::Tree::Node::ParentEntityId>(cppStructEntityId, lastNamespaceEntityId);
				}
				if (firstNamespaceEntityId.IsValid()) {
					hppFileNodeEntityIds.push_back(firstNamespaceEntityId);
				}
				else {
					hppFileNodeEntityIds.push_back(cppStructEntityId);
				}
			}


			//Generate constants code tree.
			for (ECS2::Entity::Id constantEntityId : fileConstants) {
				const ECS2::ComponentsFilter constantCF = GetComponentsFilter(constantEntityId);
				ASSERT(constantCF.IsSet<File::Table::Constant::Tag>());

				const ECS2::Entity::Id cppConstantEntityId = CreateEntity();
				const auto parentTables = ECS::File::Table::GET_TABLE_PARENTS(this, constantEntityId);
				const auto [firstNamespaceEntityId, lastNamespaceEntityId] = CPP__TREE__CREATE_CPP_NAMESPACES(this, parentTables, std::vector<ECS2::Entity::Id>{cppConstantEntityId});

				CPP__TREE__DECL__CREATE_VARIABLE_COMPONENTS(
					this,
					cppConstantEntityId,
					CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, GetComponent<File::Table::Constant::TypeName>(constantEntityId)->type_),
					GetComponent<File::Table::Name>(constantEntityId)->name_,
					CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, GetComponent<File::Table::Constant::Value>(constantEntityId)->value_, ""),
					lastNamespaceEntityId
				);

				GetComponent<File::Table::Constant::TypeName>(constantEntityId)->type_;

				if (firstNamespaceEntityId.IsValid()) {
					hppFileNodeEntityIds.push_back(firstNamespaceEntityId);
				}
				else {
					hppFileNodeEntityIds.push_back(cppConstantEntityId);
				}
			}


			//Generate Enums code tree.
			for (ECS2::Entity::Id enumEntityId : fileEnums) {
				const ECS2::ComponentsFilter enumCF = GetComponentsFilter(enumEntityId);
				ASSERT(enumCF.IsSet<File::Table::Enum::Tag>());

				const ECS2::Entity::Id cppEnumEntityId = CreateEntity();
				const auto ecsParentTables = ECS::File::Table::GET_TABLE_PARENTS(this, enumEntityId);
				const auto [firstNamespaceEntityId, lastNamespaceEntityId] = CPP__TREE__CREATE_CPP_NAMESPACES(this, ecsParentTables, std::vector<ECS2::Entity::Id>{cppEnumEntityId});

				CreateComponent<CPP::Tree::Node::Tag>(cppEnumEntityId);
				CreateComponent<CPP::Tree::Decl::Tag>(cppEnumEntityId);
				CreateComponent<CPP::Tree::Decl::Enum_>(cppEnumEntityId);
				CreateComponent<CPP::Tree::Node::Name>(cppEnumEntityId, GetComponent<File::Table::Name>(enumEntityId)->name_);

				std::vector<ECS2::Entity::Id> enumeratorEntityIds;
				for (const std::string value : GetComponent<ECS::File::Table::Enum::Values>(enumEntityId)->values_) {
					ECS2::Entity::Id enumeratorEntityId = CreateEntity();
					CreateComponent<CPP::Tree::Node::Tag>(enumeratorEntityId);
					CreateComponent<CPP::Tree::Decl::Tag>(enumeratorEntityId);
					CreateComponent<CPP::Tree::Decl::Enumerator>(enumeratorEntityId, value, ECS2::Entity::Id::invalid_); // Without 
					CreateComponent<CPP::Tree::Node::Name>(enumeratorEntityId, value);
					CreateComponent<CPP::Tree::Node::ParentEntityId>(enumeratorEntityId, cppEnumEntityId);
					enumeratorEntityIds.push_back(enumeratorEntityId);
				}
				CreateComponent<CPP::Tree::Node::ChildEntityIds>(cppEnumEntityId, enumeratorEntityIds);

				if (lastNamespaceEntityId.IsValid()) {
					CreateComponent<CPP::Tree::Node::ParentEntityId>(cppEnumEntityId, lastNamespaceEntityId);
				}
				if (firstNamespaceEntityId.IsValid()) {
					hppFileNodeEntityIds.push_back(firstNamespaceEntityId);
				}
				else {
					hppFileNodeEntityIds.push_back(cppEnumEntityId);
				}
			}

			//Generate structs code tree.
			for (ECS2::Entity::Id structEntityId : fileStructs) {

				const ECS2::ComponentsFilter structCF = GetComponentsFilter(structEntityId);
				ASSERT(structCF.IsSet<File::Table::Struct::Tag>());

				const ECS2::Entity::Id cppStructEntityId = CreateEntity();
				const auto ecsParentTables = ECS::File::Table::GET_TABLE_PARENTS(this, structEntityId);
				const auto [firstNamespaceEntityId, lastNamespaceEntityId] = CPP__TREE__CREATE_CPP_NAMESPACES(this, ecsParentTables, std::vector<ECS2::Entity::Id>{cppStructEntityId});

				{
					CreateComponent<CPP::Tree::Node::Tag>(cppStructEntityId);
					CreateComponent<CPP::Tree::Decl::Tag>(cppStructEntityId);
					CreateComponent<CPP::Tree::Decl::Class_>(cppStructEntityId);
					CreateComponent<CPP::Tree::Node::Name>(cppStructEntityId, GetComponent<File::Table::Name>(structEntityId)->name_);

					std::vector<ECS2::Entity::Id> childEntityIds;
					std::vector<ECS2::Entity::Id> fieldEntityIds;
					if (IsComponentExist<ECS::File::Table::Struct::Field::EntityIds>(structEntityId)) {
						for (ECS2::Entity::Id fieldEntityId : GetComponent<ECS::File::Table::Struct::Field::EntityIds>(structEntityId)->entityIds_) {
							ASSERT(IsComponentExist<ECS::File::Table::Struct::Field::Tag>(fieldEntityId));

							ECS2::Entity::Id variableEntityId = CPP__TREE__DECL__CREATE_VARIABLE(
								this,
								CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, GetComponent<File::Table::Struct::Field::Type>(fieldEntityId)->name_),
								GetComponent<File::Table::Struct::Field::Name>(fieldEntityId)->name_ + "_",
								(IsComponentExist<File::Table::Struct::Field::DefaultValue>(fieldEntityId))
								? (CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, GetComponent<File::Table::Struct::Field::DefaultValue>(fieldEntityId)->value_, ""))
								: (ECS2::Entity::Id::invalid_),
								cppStructEntityId
							);
							CreateComponent<CPP::Tree::Access::Public_>(variableEntityId);

							fieldEntityIds.push_back(variableEntityId);
							childEntityIds.push_back(variableEntityId);
						}
					}
					//Default constructor.
					{
						ECS2::Entity::Id constructorEntityId = CPP__TREE__DECL__CREATE_EMPTY_CONSTRUCTOR(
							this,
							GetComponent<File::Table::Name>(structEntityId)->name_,
							cppStructEntityId, ECS2::Entity::Id::invalid_);
						//CreateComponent<CPP::Tree::Decl::Constructor>(constructorEntityId);
						CreateComponent<CPP::Tree::Access::Public_>(constructorEntityId);

						childEntityIds.push_back(constructorEntityId);
					}

					//Constructor with parameters.
					if (!fieldEntityIds.empty()) {

						const ECS2::Entity::Id constructorRealizationEntityId = CreateEntity();

						const auto [firstConstructorRealizationNamespaceEntityId, lastConstructorRealizationNamespaceEntityId] = CPP__TREE__CREATE_CPP_NAMESPACES(
							this,
							ecsParentTables,
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								constructorRealizationEntityId
							)
						);

						CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(
							this,
							constructorRealizationEntityId,
							ECS2::Entity::Id::invalid_,
							CPP__TREE__CREATE_ENTITIES_VECTOR(),
							GetComponent<File::Table::Name>(structEntityId)->name_,
							CPP__TREE__CREATE_ENTITIES_VECTOR(),
							lastConstructorRealizationNamespaceEntityId,
							[&]() {
								std::vector<ECS2::Entity::Id> parameterEntityIds;
								for (ECS2::Entity::Id fieldEntityId : GetComponent<ECS::File::Table::Struct::Field::EntityIds>(structEntityId)->entityIds_) {
									ECS2::Entity::Id parameterEntityId = CreateEntity();
									CreateComponent<CPP::Tree::Node::Tag>(parameterEntityId);
									CreateComponent<CPP::Tree::Decl::Tag>(parameterEntityId);

									//Type entity id.
									ECS2::Entity::Id typeEntityId = CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, GetComponent<File::Table::Struct::Field::Type>(fieldEntityId)->name_);

									CreateComponent<CPP::Tree::Decl::Parameter>(parameterEntityId,
										GetComponent<ECS::File::Table::Struct::Field::Name>(fieldEntityId)->name_,
										typeEntityId,
										ECS2::Entity::Id::invalid_, // There is no initializer.
										false
									);
									parameterEntityIds.push_back(parameterEntityId);
								}
								parameterEntityIds.push_back(CPP__TREE__STMT__CREATE_EMPTY_COMPOUND_STATEMENT(this));
								return parameterEntityIds;


							}()
								);

						CreateComponent<CPP::Tree::Decl::Constructor>(constructorRealizationEntityId, cppStructEntityId);
						CreateComponent<CPP::Tree::ConstructorInitList>(
							constructorRealizationEntityId,
							[&]() {

								auto ecsStructFields = GetComponent<ECS::File::Table::Struct::Field::EntityIds>(structEntityId)->entityIds_;
								ASSERT(ecsStructFields.size() == fieldEntityIds.size());
								std::vector<ECS2::Entity::Id> initializerEntityIds;
								for (Common::Index i = 0; i < ecsStructFields.size(); i++) {
									ECS2::Entity::Id ecsFieldEntityId = ecsStructFields[i];
									const std::string ecsFieldName = GetComponent<ECS::File::Table::Struct::Field::Name>(ecsFieldEntityId)->name_;
									initializerEntityIds.push_back(CPP__TREE__CREATE_CONSTRUCTOR_INITIALIZER(
										this,
										fieldEntityIds[i],
										CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, ecsFieldName)
									));
								}
								return initializerEntityIds;
							}());

						if (firstConstructorRealizationNamespaceEntityId.IsValid()) {
							cppFileNodeEntityIds.push_back(firstConstructorRealizationNamespaceEntityId);
						}
						else {
							cppFileNodeEntityIds.push_back(constructorRealizationEntityId);
						}


						ECS2::Entity::Id constructorPrototypeEntityId = CPP__TREE__DECL__CREATE_CONSTRUCTOR(
							this,
							GetComponent<File::Table::Name>(structEntityId)->name_,
							CPP__TREE__CREATE_ENTITIES_VECTOR(),
							lastNamespaceEntityId,
							ECS2::Entity::Id::invalid_,
							[&]() {
								std::vector<ECS2::Entity::Id> parameterEntityIds;
								for (ECS2::Entity::Id fieldEntityId : GetComponent<ECS::File::Table::Struct::Field::EntityIds>(structEntityId)->entityIds_) {
									ECS2::Entity::Id parameterEntityId = CreateEntity();
									CreateComponent<CPP::Tree::Node::Tag>(parameterEntityId);
									CreateComponent<CPP::Tree::Decl::Tag>(parameterEntityId);

									//Type entity id.
									ECS2::Entity::Id typeEntityId = CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,GetComponent<File::Table::Struct::Field::Type>(fieldEntityId)->name_);

									CreateComponent<CPP::Tree::Decl::Parameter>(parameterEntityId,
										GetComponent<ECS::File::Table::Struct::Field::Name>(fieldEntityId)->name_,
										typeEntityId,
										ECS2::Entity::Id::invalid_, // There is no initializer.
										false
									);
									parameterEntityIds.push_back(parameterEntityId);
								}
								//parameterEntityIds.push_back(CPP__TREE__STMT__CREATE_EMPTY_COMPOUND_STATEMENT());
								return parameterEntityIds;


							}());

						childEntityIds.push_back(constructorPrototypeEntityId);

					}


					CreateComponent<CPP::Tree::Node::ChildEntityIds>(cppStructEntityId, childEntityIds);
				}
				if (lastNamespaceEntityId.IsValid()) {
					CreateComponent<CPP::Tree::Node::ParentEntityId>(cppStructEntityId, lastNamespaceEntityId);
				}
				if (firstNamespaceEntityId.IsValid()) {
					hppFileNodeEntityIds.push_back(firstNamespaceEntityId);
				}
				else {
					hppFileNodeEntityIds.push_back(cppStructEntityId);
				}
			}

			//Generate components code tree.
			for (ECS2::Entity::Id ecsComponentEntityId : fileComponents) {

				const ECS2::ComponentsFilter componentCF = GetComponentsFilter(ecsComponentEntityId);
				ASSERT(componentCF.IsSet<File::Table::Component::Tag>());

				const auto ecsParentTables = ECS::File::Table::GET_TABLE_PARENTS(this, ecsComponentEntityId);

				const auto [hppFirstNamespaceEntityId, hppLastNamespaceEntityId] = CPP__TREE__CREATE_CPP_NAMESPACES(
					this,
					ecsParentTables,
					CPP__TREE__CREATE_ENTITIES_VECTOR()// hppLastNamespaceEntityId will not have ChildEntityIds, need to create manual
				);
				std::vector<ECS2::Entity::Id> hppChilds{};

				const auto [cppFirstNamespaceEntityId, cppLastNamespaceEntityId] = CPP__TREE__CREATE_CPP_NAMESPACES(
					this,
					ecsParentTables,
					CPP__TREE__CREATE_ENTITIES_VECTOR()); // cppLastNamespaceEntityId will not have ChildEntityIds, need to create manual
				std::vector<ECS2::Entity::Id> cppChilds;

				// Component class 
				{
					const ECS2::Entity::Id hppComponentClassEntityId = CreateEntity();
					CreateComponent<CPP::Tree::Node::Tag>(hppComponentClassEntityId);
					CreateComponent<CPP::Tree::Decl::Tag>(hppComponentClassEntityId);
					CreateComponent<CPP::Tree::Decl::Class_>(hppComponentClassEntityId);
					CreateComponent<CPP::Tree::Node::Name>(hppComponentClassEntityId, GetComponent<File::Table::Name>(ecsComponentEntityId)->name_);

					std::vector<ECS2::Entity::Id> childEntityIds;
					std::vector<ECS2::Entity::Id> cppFieldEntityIds;
					std::vector<ECS2::Entity::Id> ecsFieldEntityIds;

					if (IsComponentExist<ECS::File::Table::Component::Field::EntityIds>(ecsComponentEntityId)) {
						for (ECS2::Entity::Id fieldEntityId : GetComponent<ECS::File::Table::Component::Field::EntityIds>(ecsComponentEntityId)->entityIds_) {
							ASSERT(IsComponentExist<ECS::File::Table::Component::Field::Tag>(fieldEntityId));
							ecsFieldEntityIds.push_back(fieldEntityId);
							ECS2::Entity::Id variableEntityId = CPP__TREE__DECL__CREATE_VARIABLE(
								this,
								CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, GetComponent<File::Table::Component::Field::Type>(fieldEntityId)->name_),				/*Type*/
								GetComponent<File::Table::Component::Field::Name>(fieldEntityId)->name_ + "_",												/*Name*/
								(IsComponentExist<File::Table::Component::Field::DefaultValue>(fieldEntityId))												/*Initialize if need*/
								? (CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, GetComponent<File::Table::Component::Field::DefaultValue>(fieldEntityId)->value_, ""))
								: (ECS2::Entity::Id::invalid_),
								hppComponentClassEntityId																										/*Parent*/
							);

							CreateComponent<CPP::Tree::Access::Public_>(variableEntityId);
							cppFieldEntityIds.push_back(variableEntityId);
							childEntityIds.push_back(variableEntityId);
						}
					}
					//Default constructor.
					{
						childEntityIds.push_back(
							CPP__TREE__DECL__CREATE_PUBLIC_EMPTY_CONSTRUCTOR(
								this,
								GetComponent<File::Table::Name>(ecsComponentEntityId)->name_,

								hppComponentClassEntityId,
								ECS2::Entity::Id::invalid_
							)

						);
					}

					//Constructor with parameters.

					if (!ecsFieldEntityIds.empty()) {
						ECS2::Entity::Id constructorWithParametersEntityId = CPP__TREE__DECL__CREATE_CONSTRUCTOR(
							this,
							GetComponent<File::Table::Name>(ecsComponentEntityId)->name_,
							[&]() {
								std::vector<ECS2::Entity::Id> initializerEntityIds;
								for (Common::Index i = 0; i < ecsFieldEntityIds.size(); i++) {
									ECS2::Entity::Id fieldEntityId = ecsFieldEntityIds[i];
									//fieldEntityIds[i]{ parameterEntityIds[i] }
									ECS2::Entity::Id initializer = CPP__TREE__CREATE_CONSTRUCTOR_INITIALIZER(
										this,
										cppFieldEntityIds[i],
										(IsComponentExist<File::Table::Component::Field::Copyable>(fieldEntityId))
										? (CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(
											this, GetComponent<ECS::File::Table::Component::Field::Name>(fieldEntityId)->name_))
										: (CPP__TREE__EXPR__CREATE_CALL_EXPR(
											this,
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "std::move"),
											CPP__TREE__CREATE_ENTITIES_VECTOR(),
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
													GetComponent<ECS::File::Table::Component::Field::Name>(fieldEntityId)->name_
												)
											)
										))
									);
									initializerEntityIds.push_back(initializer);
								}
								return initializerEntityIds;
							}(),
								hppComponentClassEntityId,
								ECS2::Entity::Id::invalid_,
								[&]() {
								std::vector<ECS2::Entity::Id> parameterEntityIds;
								if (IsComponentExist<ECS::File::Table::Component::Field::EntityIds>(ecsComponentEntityId)) {
									for (ECS2::Entity::Id fieldEntityId : GetComponent<ECS::File::Table::Component::Field::EntityIds>(ecsComponentEntityId)->entityIds_) {
										parameterEntityIds.push_back(
											CPP__TREE__DECL__CREATE_PARAMETER(this,

												(IsComponentExist<File::Table::Component::Field::Copyable>(fieldEntityId))
												? (CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, GetComponent<File::Table::Component::Field::Type>(fieldEntityId)->name_))
												: (CPP__TREE__TYPE__CREATE_REFERENCE_TYPE(this,
													CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, GetComponent<File::Table::Component::Field::Type>(fieldEntityId)->name_),
													true
												)),
												GetComponent<ECS::File::Table::Component::Field::Name>(fieldEntityId)->name_
											)
										);
									}
								}
								//Add empty body.
								parameterEntityIds.push_back(CPP__TREE__STMT__CREATE_EMPTY_COMPOUND_STATEMENT(this));
								return parameterEntityIds;
								}()
									);
						CreateComponent<CPP::Tree::Access::Public_>(constructorWithParametersEntityId);

						childEntityIds.push_back(constructorWithParametersEntityId);
					}




					//GetName static Function
					//static const char* GetFullName()
					//{
					//	return "Config::Tag";
					//};
					{
						ECS2::Entity::Id getNameFunctionEntityId = CPP__TREE__DECL__CREATE_PUBLIC_STATIC_METHOD(this,
							CPP__TREE__TYPE__CREATE_POINTER_TYPE(this, CPP__TREE__TYPE__CREATE_CONST_CHAR_TYPE(this)),
							"GetFullName",
							hppComponentClassEntityId,
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
									CPP__TREE__CREATE_ENTITIES_VECTOR(
										CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
											CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"" + File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "::", false) + "\"", "")
										)
									)
								)
							)
						);
						childEntityIds.push_back(getNameFunctionEntityId);
					}
					//GetName static Function
					//static const char* GetName()
					//{
					//	return "Tag";
					//};
					{
						ECS2::Entity::Id getNameFunctionEntityId = CPP__TREE__DECL__CREATE_PUBLIC_STATIC_METHOD(this,
							CPP__TREE__TYPE__CREATE_POINTER_TYPE(this, CPP__TREE__TYPE__CREATE_CONST_CHAR_TYPE(this)),
							"GetName",
							hppComponentClassEntityId,
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this, 
									CPP__TREE__CREATE_ENTITIES_VECTOR(
										CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this, 
											CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"" + File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false) + "\"", "")
										)
									)
								)
							)
						);
						childEntityIds.push_back(getNameFunctionEntityId);
					}
					//IsBindable static Function
					//constexpr static bool IsBindable()
					//{
					//	return true;
					//};
					{
						ECS2::Entity::Id isBindableFunctionEntityId = CPP__TREE__DECL__CREATE_PUBLIC_STATIC_METHOD(this,
							CPP__TREE__TYPE__CREATE_BOOL_TYPE(this),
							"IsBindable",
							hppComponentClassEntityId,
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
									CPP__TREE__CREATE_ENTITIES_VECTOR(
										CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
											CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, (IsComponentExist<ECS::File::Table::Component::Bindable>(ecsComponentEntityId) ? ("true") : ("false")), "")
										)
									)
								)
							)
						);
						CreateComponent<CPP::Tree::Constexpr_>(isBindableFunctionEntityId);
						childEntityIds.push_back(isBindableFunctionEntityId);
					}

					//IsSerializable static Function
					//constexpr static bool IsSerializable()
					//{
					//	return true;
					//};
					{
						ECS2::Entity::Id isSerializableFunctionEntityId = CPP__TREE__DECL__CREATE_PUBLIC_STATIC_METHOD(
							this,
							CPP__TREE__TYPE__CREATE_BOOL_TYPE(this),
							"IsSerializable",
							hppComponentClassEntityId,
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
									CPP__TREE__CREATE_ENTITIES_VECTOR(
										CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
											CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, (IsComponentExist<ECS::File::Table::Component::Serializable>(ecsComponentEntityId) ? ("true") : ("false")), "")
										)
									)
								)
							)
						);
						CreateComponent<CPP::Tree::Constexpr_>(isSerializableFunctionEntityId);
						childEntityIds.push_back(isSerializableFunctionEntityId);
					}

					//GetTypeId static Function
					//static inline ECS2::ComponentTypeId GetTypeId()
					//{
					//	return 211;
					//};
					{
						ECS2::Entity::Id getTypeIdFunctionEntityId = CPP__TREE__DECL__CREATE_PUBLIC_STATIC_INLINE_METHOD(this,
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::ComponentTypeId"),
							"GetTypeId",
							hppComponentClassEntityId,
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
									CPP__TREE__CREATE_ENTITIES_VECTOR(
										CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this, CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, std::to_string(++componentTypeId), ""))
									)
								)
							)
						);
						CreateComponent<ECS::File::Table::Component::TypeId>(ecsComponentEntityId, componentTypeId);

						childEntityIds.push_back(getTypeIdFunctionEntityId);
					}

					CreateComponent<CPP::Tree::Node::ChildEntityIds>(hppComponentClassEntityId, childEntityIds);

					hppChilds.push_back(hppComponentClassEntityId);

				}

				//Edit function
				{
					ECS2::Entity::Id hppEditComponentFunctionPrototypeEntityId = CreateEntity();
					std::vector<ECS2::Entity::Id> hppFunctionChilds;
					std::vector<ECS2::Entity::Id> cppFunctionChilds;

					//Parameter: std::shared_ptr<ECS2::World> world
					ECS2::Entity::Id worldParameterEntityId = ECS__GENERATOR__CREATE_WORLD_PARAMETER(this);

					ECS2::Entity::Id ecsEntityIdParameterEntityId = CPP__TREE__DECL__CREATE_PARAMETER(this,
						CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::Entity::Id"), "ecsEntityId");
					ECS2::Entity::Id componentPointerParameter = CPP__TREE__DECL__CREATE_PARAMETER(this,
						CPP__TREE__TYPE__CREATE_POINTER_TYPE(this, 
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,
								File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "::", false)
							)
						),
						File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
					);

					hppFunctionChilds.push_back(worldParameterEntityId);
					hppFunctionChilds.push_back(ecsEntityIdParameterEntityId);
					hppFunctionChilds.push_back(componentPointerParameter);


					//Body. if manualEditFunction create only prototype else generate prototype(hpp) and realization(cpp).
					if (!componentCF.IsSet<ECS::File::Table::Component::ManualEditFunction>()) {
						ECS2::Entity::Id cppEditComponentFunctionRealizationEntityId = CreateEntity();

						cppFunctionChilds.push_back(worldParameterEntityId);
						cppFunctionChilds.push_back(ecsEntityIdParameterEntityId);
						cppFunctionChilds.push_back(componentPointerParameter);

						ECS2::Entity::Id bodyCompoundStmtEntityId = CreateEntity();

						std::vector<ECS2::Entity::Id> bodyChilds;
						bodyChilds.push_back(CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
							CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
								CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "ImGui::PushID"),
								CPP__TREE__CREATE_ENTITIES_VECTOR(),
								CPP__TREE__CREATE_ENTITIES_VECTOR(
									CPP__TREE__EXPR__CREATE_CALL_EXPR_NO_PARAMETERS(this,
										(File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false) + "::" + "GetTypeId")
									)
								)
							)));

						if (IsComponentExist<ECS::File::Table::Component::Field::EntityIds>(ecsComponentEntityId)) {

							for (ECS2::Entity::Id fieldEntityId : GetComponent<ECS::File::Table::Component::Field::EntityIds>(ecsComponentEntityId)->entityIds_) {
								ASSERT(IsComponentExist<ECS::File::Table::Component::Field::Tag>(fieldEntityId));
								const std::string typeName = GetComponent<File::Table::Component::Field::Type>(fieldEntityId)->name_;
								const std::string fieldName = GetComponent<File::Table::Component::Field::Name>(fieldEntityId)->name_;
								if (typeName == "float") {
									bodyChilds.push_back(CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
										CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "ImGui::InputScalar"),
											CPP__TREE__CREATE_ENTITIES_VECTOR(),
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, ("\"" + fieldName + "\""), ""),
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "ImGuiDataType_Float"),
												CPP__TREE__EXPR__CREATE_UNARY_OPERATION(this,
													CPP::Tree::Expr::UnaryOpType::AddressOf,
													CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
														CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
															File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
														),
														fieldName + "_",
														true
													)
												)
											)
										)
									));
								}
								else if (typeName == "double") {
									bodyChilds.push_back(CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
										CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "ImGui::InputScalar"),
											CPP__TREE__CREATE_ENTITIES_VECTOR(),
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, ("\"" + fieldName + "\""), ""),
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "ImGuiDataType_Double"),
												CPP__TREE__EXPR__CREATE_UNARY_OPERATION(this,
													CPP::Tree::Expr::UnaryOpType::AddressOf,
													CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
														CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
															File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
														),
														fieldName + "_",
														true
													)
												)
											)
										)
									));
								}
								else if (typeName == "Common::UInt64") {
									bodyChilds.push_back(CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
										CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "ImGui::InputScalar"),
											CPP__TREE__CREATE_ENTITIES_VECTOR(),
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, ("\"" + fieldName + "\""), ""),
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "ImGuiDataType_U64"),
												CPP__TREE__EXPR__CREATE_UNARY_OPERATION(this,
													CPP::Tree::Expr::UnaryOpType::AddressOf,
													CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
														CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
															File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
														),
														fieldName + "_",
														true
													)
												)
											)
										)
									));
								}
								else if (typeName == "std::string") {
									bodyChilds.push_back(CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
										CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "ImGui::InputText"),
											CPP__TREE__CREATE_ENTITIES_VECTOR(),
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, ("\"##" + fieldName + "\""), ""),
												CPP__TREE__EXPR__CREATE_UNARY_OPERATION(this,
													CPP::Tree::Expr::UnaryOpType::AddressOf,
													CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
														CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
															File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
														),
														fieldName + "_",
														true
													)
												)
											)
										)
									));
								}
								else {
									//Can't generate imgui edit code for the type.
									/*NOT_IMPLEMENTED();*/
								}
							}
						}
						bodyChilds.push_back(CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
							CPP__TREE__EXPR__CREATE_CALL_EXPR_NO_PARAMETERS(this, "ImGui::PopID")
						)
						);

						bodyCompoundStmtEntityId = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT_COMPONENTS(this,
							bodyCompoundStmtEntityId,
							bodyChilds);

						cppFunctionChilds.push_back(bodyCompoundStmtEntityId);

						CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
							cppEditComponentFunctionRealizationEntityId,
							CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
							CPP__TREE__CREATE_ENTITIES_VECTOR(),
							GetComponent<File::Table::Name>(ecsComponentEntityId)->name_ + "Edit",
							CPP__TREE__CREATE_ENTITIES_VECTOR(),
							cppLastNamespaceEntityId,
							cppFunctionChilds
						);

						cppChilds.push_back(cppEditComponentFunctionRealizationEntityId);

					}


					CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
						hppEditComponentFunctionPrototypeEntityId,
						CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
						CPP__TREE__CREATE_ENTITIES_VECTOR(),
						GetComponent<File::Table::Name>(ecsComponentEntityId)->name_ + "Edit",
						CPP__TREE__CREATE_ENTITIES_VECTOR(),
						hppLastNamespaceEntityId,
						hppFunctionChilds
					);

					hppChilds.push_back(hppEditComponentFunctionPrototypeEntityId);
				}


				//Serialize
				if (IsComponentExist<ECS::File::Table::Component::Serializable>(ecsComponentEntityId)) {
					ECS2::Entity::Id hppSerializeComponentFunctionPrototypeEntityId = CreateEntity();

					std::vector<ECS2::Entity::Id> hppFunctionChilds;
					std::vector<ECS2::Entity::Id> cppFunctionChilds;

					ECS2::Entity::Id componentPointerParameter = CPP__TREE__DECL__CREATE_PARAMETER(this,
						CPP__TREE__TYPE__CREATE_POINTER_TYPE(this,
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,
								File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "::", false)
							)
						),
						File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
					);

					hppFunctionChilds.push_back(componentPointerParameter);

					if (!componentCF.IsSet<ECS::File::Table::Component::ManualSerializeFunction>()) {
						ECS2::Entity::Id cppSerializeComponentFunctionRealizationEntityId = CreateEntity();
						cppFunctionChilds.push_back(componentPointerParameter);

						ECS2::Entity::Id bodyCompoundStmtEntityId = CreateEntity();

						std::vector<ECS2::Entity::Id> bodyChilds;

						bodyChilds.push_back(CPP__TREE__DECL__CREATE_USING_DIRECTIVE(this, "std::string_literals"));


						bodyChilds.push_back(
							CPP__TREE__DECL__CREATE_VARIABLE(this,
								CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::string"),
								"string",
								CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"" + File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false) + " =\"", ""),
								cppSerializeComponentFunctionRealizationEntityId
							)
						);

						auto addString = [this, &bodyChilds](ECS2::Entity::Id rightOperand) {
							bodyChilds.push_back(CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
								CPP__TREE__EXPR__CREATE_BINARY_OPERATION(this,
									CPP::Tree::Expr::BinaryOpType::PlusAssign,
									CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "string"),
									rightOperand
								)
							)
							);

							};

						bool isFirst = true;
						addString(CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"{\"", ""));
						if (IsComponentExist<ECS::File::Table::Component::Field::EntityIds>(ecsComponentEntityId)) {
							for (ECS2::Entity::Id fieldEntityId : GetComponent<ECS::File::Table::Component::Field::EntityIds>(ecsComponentEntityId)->entityIds_) {
								ASSERT(IsComponentExist<ECS::File::Table::Component::Field::Tag>(fieldEntityId));
								const std::string typeName = GetComponent<File::Table::Component::Field::Type>(fieldEntityId)->name_;
								const std::string fieldName = GetComponent<File::Table::Component::Field::Name>(fieldEntityId)->name_;

								if (!isFirst) {
									isFirst = false;
									addString(CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, ",", ""));
								}
								addString(CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"" + fieldName + "=\"", ""));

								std::unordered_set<std::string> std_to_stringAvailableTypes = {
									"int", "long", "long long", "unsigned", "unsigned long",
									"Common::UInt64", "float", "double",
								};
								if (std_to_stringAvailableTypes.contains(typeName)) {
									addString(CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
										CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "std::to_string"),
										CPP__TREE__CREATE_ENTITIES_VECTOR(),
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
													File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
												),
												fieldName + "_",
												true
											)
										)
									));
								}
								else if (typeName == "std::string") {
									addString(CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"\\\"\"", ""));
									addString(
										CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
												File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
											),
											fieldName + "_",
											true
										));
									addString(CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"\\\"\"", ""));
								}
								else {
									//Can't serialize current type.
									//NOT_IMPLEMENTED();
								}

							}
						}
						addString(CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"}\"", ""));

						bodyChilds.push_back(
							CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
								CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "string")
							)
						);

						bodyCompoundStmtEntityId = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT_COMPONENTS(this,
							bodyCompoundStmtEntityId,
							bodyChilds
						);

						cppFunctionChilds.push_back(bodyCompoundStmtEntityId);

						CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
							cppSerializeComponentFunctionRealizationEntityId,
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::string"),
							CPP__TREE__CREATE_ENTITIES_VECTOR(),
							GetComponent<File::Table::Name>(ecsComponentEntityId)->name_ + "Serialize",
							CPP__TREE__CREATE_ENTITIES_VECTOR(),
							cppLastNamespaceEntityId,
							cppFunctionChilds
						);
						cppChilds.push_back(cppSerializeComponentFunctionRealizationEntityId);
					}

					CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
						hppSerializeComponentFunctionPrototypeEntityId,
						CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::string"),
						CPP__TREE__CREATE_ENTITIES_VECTOR(),
						GetComponent<File::Table::Name>(ecsComponentEntityId)->name_ + "Serialize",
						CPP__TREE__CREATE_ENTITIES_VECTOR(),
						hppLastNamespaceEntityId,
						hppFunctionChilds
					);

					hppChilds.push_back(hppSerializeComponentFunctionPrototypeEntityId);
				}

				//Parse
				if (IsComponentExist<ECS::File::Table::Component::Serializable>(ecsComponentEntityId)) {
					ECS2::Entity::Id hppParseComponentFunctionPrototypeEntityId = CreateEntity();

					std::vector<ECS2::Entity::Id> hppFunctionChilds;
					std::vector<ECS2::Entity::Id> cppFunctionChilds;

					ECS2::Entity::Id luaRefParameter = CPP__TREE__DECL__CREATE_PARAMETER(this,
						CPP__TREE__TYPE__CREATE_REFERENCE_TYPE(this,
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "luabridge::LuaRef"), false
						),
						File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
					);

					hppFunctionChilds.push_back(luaRefParameter);


					if (!componentCF.IsSet<ECS::File::Table::Component::ManualSerializeFunction>()) {
						ECS2::Entity::Id cppParseComponentFunctionRealizationEntityId = CreateEntity();
						cppFunctionChilds.push_back(luaRefParameter);

						ECS2::Entity::Id bodyCompoundStmtEntityId = CreateEntity();

						std::vector<ECS2::Entity::Id> bodyChilds;

						bodyChilds.push_back(
							CPP__TREE__DECL__CREATE_VARIABLE(this,
								CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false)),
								"component",
								ECS2::Entity::Id::invalid_,
								cppParseComponentFunctionRealizationEntityId
							)
						);

						if (IsComponentExist<ECS::File::Table::Component::Field::EntityIds>(ecsComponentEntityId)) {
							for (ECS2::Entity::Id fieldEntityId : GetComponent<ECS::File::Table::Component::Field::EntityIds>(ecsComponentEntityId)->entityIds_) {
								ASSERT(IsComponentExist<ECS::File::Table::Component::Field::Tag>(fieldEntityId));
								const std::string typeName = GetComponent<File::Table::Component::Field::Type>(fieldEntityId)->name_;
								const std::string fieldName = GetComponent<File::Table::Component::Field::Name>(fieldEntityId)->name_;


								//position3DRef["x"]
								ECS2::Entity::Id valueAccess = CPP__TREE__EXPR__CREATE_ARRAY_SUBSCRIPT(this,
									CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
										File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
									),
									CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"" + fieldName + "\"", "")
								);

								//position3DRef["x"].cast<float>()
								ECS2::Entity::Id castCallEntityId = CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
									CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
										valueAccess,
										"cast",
										false
									),
									std::vector<ECS2::Entity::Id>{ CPP__TREE__TYPE__CREATE_FLOAT_TYPE(this) },
									std::vector<ECS2::Entity::Id>{}
								);

								//position3DRef["x"].cast<float>().value()
								ECS2::Entity::Id valueCallEntityId = CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
									CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
										castCallEntityId,
										"value",
										false
									),
									std::vector<ECS2::Entity::Id>{},
									std::vector<ECS2::Entity::Id>{}
								);


								//position3D.x_ = position3DRef["x"].cast<float>().value();
								ECS2::Entity::Id saveCastValueEntityId = CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
									CPP__TREE__EXPR__CREATE_BINARY_OPERATION(this,
										CPP::Tree::Expr::BinaryOpType::Assign,
										CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
												"component"//ECS__FILE__TABLE__GET_NAME(ecsComponentEntityId, false)
											),
											fieldName + "_",
											false
										),
										valueCallEntityId
									)
								);

								bodyChilds.push_back(
									saveCastValueEntityId
								);

							}
						}
						bodyChilds.push_back(
							CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
								CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
									"component"
								)
							)
						);

						bodyCompoundStmtEntityId = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT_COMPONENTS(this,
							bodyCompoundStmtEntityId,
							bodyChilds
						);

						cppFunctionChilds.push_back(bodyCompoundStmtEntityId);

						CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
							cppParseComponentFunctionRealizationEntityId,
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false)),
							CPP__TREE__CREATE_ENTITIES_VECTOR(),
							GetComponent<File::Table::Name>(ecsComponentEntityId)->name_ + "Parse",
							CPP__TREE__CREATE_ENTITIES_VECTOR(),
							cppLastNamespaceEntityId,
							cppFunctionChilds
						);
						cppChilds.push_back(cppParseComponentFunctionRealizationEntityId);
					}

					CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
						hppParseComponentFunctionPrototypeEntityId,
						CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,
							File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false)
						),
						CPP__TREE__CREATE_ENTITIES_VECTOR(),
						File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false) + "Parse",
						CPP__TREE__CREATE_ENTITIES_VECTOR(),
						hppLastNamespaceEntityId,
						hppFunctionChilds
					);

					hppChilds.push_back(hppParseComponentFunctionPrototypeEntityId);


				}

				//Bind
				if (IsComponentExist<ECS::File::Table::Component::Bindable>(ecsComponentEntityId)) {
					ECS2::Entity::Id hppBindComponentFunctionPrototypeEntityId = CreateEntity();

					std::vector<ECS2::Entity::Id> hppFunctionChilds;
					std::vector<ECS2::Entity::Id> cppFunctionChilds;

					ECS2::Entity::Id luaStateParameter = CPP__TREE__DECL__CREATE_PARAMETER(this,
						CPP__TREE__TYPE__CREATE_POINTER_TYPE(this,
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "lua_State")
						),
						"state"
					);

					hppFunctionChilds.push_back(luaStateParameter);


					if (!componentCF.IsSet<ECS::File::Table::Component::ManualBindFunction>()) {
						ECS2::Entity::Id cppParseComponentFunctionRealizationEntityId = CreateEntity();
						cppFunctionChilds.push_back(luaStateParameter);

						ECS2::Entity::Id bodyCompoundStmtEntityId = CreateEntity();

						std::vector<ECS2::Entity::Id> bodyChilds;

						//luabridge::Namespace globalNamespace = luabridge::getGlobalNamespace(state);
						bodyChilds.push_back(CPP__TREE__DECL__CREATE_VARIABLE(this,
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "luabridge::Namespace"),
							"namespaceGlobal",
							CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
								CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "luabridge::getGlobalNamespace"),
								CPP__TREE__CREATE_ENTITIES_VECTOR(),
								CPP__TREE__CREATE_ENTITIES_VECTOR(
									CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "state")
								)
							),
							bodyCompoundStmtEntityId
						));

						std::vector<ECS2::Entity::Id> namespaceEntityIds = File::Table::ECS__FILE__TABLE__GET_NAMESPACE_ENTITY_IDS(this, ecsComponentEntityId);

						std::vector<std::string> namespaces;
						namespaces.push_back("Global");

						for (ECS2::Entity::Id namespaceEntityId : namespaceEntityIds) {
							//luabridge::Namespace
							bodyChilds.push_back(
								CPP__TREE__DECL__CREATE_VARIABLE(this,
									CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "luabridge::Namespace"),
									("namespace" + File::Table::ECS__FILE__TABLE__GET_NAME(this, namespaceEntityId, false)),
									CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
										CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "namespace" + namespaces.back()),
											"beginNamespace",
											false
										),
										CPP__TREE__CREATE_ENTITIES_VECTOR(),
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"" + namespaces.back() + "\"", "")
										)
									),
									bodyCompoundStmtEntityId
								)
							);
							namespaces.push_back(File::Table::ECS__FILE__TABLE__GET_NAME(this, namespaceEntityId, false));
						}

						//auto classTest2 = namespaceTest2.beginClass<Test2::Test2>("Test2");
						bodyChilds.push_back(
							CPP__TREE__DECL__CREATE_VARIABLE(this,
								CPP__TREE__TYPE__CREATE_AUTO_TYPE(this),
								("class" + File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false)),
								CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
									CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
										CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "namespace" + namespaces.back()),
										"beginClass",
										false
									),
									CPP__TREE__CREATE_ENTITIES_VECTOR(
										CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,
											File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false)
										)
									),
									CPP__TREE__CREATE_ENTITIES_VECTOR(
										CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this,
											"\"" + File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false) + "\"", ""
										)
									)
								),
								bodyCompoundStmtEntityId
							)
						);


						if (IsComponentExist<ECS::File::Table::Component::Field::EntityIds>(ecsComponentEntityId)) {
							for (ECS2::Entity::Id fieldEntityId : GetComponent<ECS::File::Table::Component::Field::EntityIds>(ecsComponentEntityId)->entityIds_) {
								ASSERT(IsComponentExist<ECS::File::Table::Component::Field::Tag>(fieldEntityId));
								const std::string typeName = GetComponent<File::Table::Component::Field::Type>(fieldEntityId)->name_;
								const std::string fieldName = GetComponent<File::Table::Component::Field::Name>(fieldEntityId)->name_;

								//classTest2.addProperty("x", &Test2::x_);
								bodyChilds.push_back(
									CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
										CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
											CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "class" + File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false)),
												"addProperty",
												false
											),
											CPP__TREE__CREATE_ENTITIES_VECTOR(),
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"" + fieldName + "\"", ""),
												CPP__TREE__EXPR__CREATE_MEMBER_PTR_EXPR(this,
													CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,
														File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false)
													),
													fieldName + "_"
												)
											)
										)
									)
								);
							}
						}

						//classTest2.endClass();
						bodyChilds.push_back(
							CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
								CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
									CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
										CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "class" + File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false)),
										"endClass",
										false
									),
									CPP__TREE__CREATE_ENTITIES_VECTOR(),
									CPP__TREE__CREATE_ENTITIES_VECTOR()
								)
							)
						);

						//namespaceName.endNamespace()
						for (auto it = namespaces.rbegin(); it != namespaces.rend(); it++) {

							bodyChilds.push_back(
								CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
									CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
										CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
												("namespace" + *it)),
											"endNamespace",
											false
										),
										CPP__TREE__CREATE_ENTITIES_VECTOR(),
										CPP__TREE__CREATE_ENTITIES_VECTOR()
									)
								)
							);
						}

						bodyCompoundStmtEntityId = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT_COMPONENTS(this,
							bodyCompoundStmtEntityId,
							bodyChilds
						);

						cppFunctionChilds.push_back(bodyCompoundStmtEntityId);

						CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
							cppParseComponentFunctionRealizationEntityId,
							CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
							CPP__TREE__CREATE_ENTITIES_VECTOR(),
							GetComponent<File::Table::Name>(ecsComponentEntityId)->name_ + "Bind",
							CPP__TREE__CREATE_ENTITIES_VECTOR(),
							cppLastNamespaceEntityId,
							cppFunctionChilds
						);
						cppChilds.push_back(cppParseComponentFunctionRealizationEntityId);
					}

					CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
						hppBindComponentFunctionPrototypeEntityId,
						CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
						CPP__TREE__CREATE_ENTITIES_VECTOR(),
						File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false) + "Bind",
						CPP__TREE__CREATE_ENTITIES_VECTOR(),
						hppLastNamespaceEntityId,
						hppFunctionChilds
					);

					hppChilds.push_back(hppBindComponentFunctionPrototypeEntityId);
				}


				if (hppLastNamespaceEntityId.IsValid()) {
					CreateComponent<CPP::Tree::Node::ChildEntityIds>(hppLastNamespaceEntityId, hppChilds);
					hppFileNodeEntityIds.push_back(hppFirstNamespaceEntityId);
				}
				else {
					//TODO: reserve
					hppFileNodeEntityIds.insert(hppFileNodeEntityIds.end(), hppChilds.begin(), hppChilds.end());
				}
				if (cppLastNamespaceEntityId.IsValid()) {
					CreateComponent<CPP::Tree::Node::ChildEntityIds>(cppLastNamespaceEntityId, cppChilds);
					cppFileNodeEntityIds.push_back(cppFirstNamespaceEntityId);
				}
				else {
					cppFileNodeEntityIds.insert(cppFileNodeEntityIds.end(), cppChilds.begin(), cppChilds.end());
				}


				//Template Edit Specialization.
				ECS2::Entity::Id templateEditSpecialization = CreateEntity();
				{
					std::vector<ECS2::Entity::Id> functionChilds;

					ECS2::Entity::Id worldParameterEntityId = ECS__GENERATOR__CREATE_WORLD_PARAMETER(this);
					ECS2::Entity::Id ecsEntityIdParameterEntityId = CPP__TREE__DECL__CREATE_PARAMETER(this,
						CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::Entity::Id"), "ecsEntityId");
					ECS2::Entity::Id componentPointerParameter = CPP__TREE__DECL__CREATE_PARAMETER(this,
						CPP__TREE__TYPE__CREATE_POINTER_TYPE(this,
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,
								File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "::", false)
							)
						),
						File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
					);

					functionChilds.push_back(worldParameterEntityId);
					functionChilds.push_back(ecsEntityIdParameterEntityId);
					functionChilds.push_back(componentPointerParameter);
					//Body
					{
						ECS2::Entity::Id bodyCompoundStmtEntityId = CreateEntity();

						std::vector<ECS2::Entity::Id> bodyChilds;

						ECS2::Entity::Id editCallExprEntityId = CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
							CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
								File::Table::ECS__FILE__TABLE__GET_FULL_NAMESPACE_STRING(this, ecsComponentEntityId, "::", false) + "::" + File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false) + "Edit"
							),
							std::vector<ECS2::Entity::Id>{},
							(std::vector<ECS2::Entity::Id>{
							CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world"),
								CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "ecsEntityId"),
								CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false))
						})
						);
						bodyCompoundStmtEntityId = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT_COMPONENTS(this,
							bodyCompoundStmtEntityId,
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
									editCallExprEntityId
								)
							)
						);

						functionChilds.push_back(bodyCompoundStmtEntityId);
					}
					CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
						templateEditSpecialization,
						CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
						std::vector<ECS2::Entity::Id>{ },
						"Edit",
						std::vector<ECS2::Entity::Id>{
						CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,
							File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "::", false)
						)
					},
						hppLastNamespaceEntityId,
						functionChilds
					);
					CreateComponent<CPP::Tree::Inline_>(templateEditSpecialization);
					hppFileNodeEntityIds.push_back(templateEditSpecialization);
				}

				//Template Serialize Specialization.

				ECS2::Entity::Id templateSerializeSpecialization = CreateEntity();
				if (IsComponentExist<ECS::File::Table::Component::Serializable>(ecsComponentEntityId))
				{
					std::vector<ECS2::Entity::Id> functionChilds;
					ECS2::Entity::Id componentPointerParameter = CPP__TREE__DECL__CREATE_PARAMETER(this,
						CPP__TREE__TYPE__CREATE_POINTER_TYPE(this,
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,
								File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "::", false)
							)
						),
						File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
					);

					functionChilds.push_back(componentPointerParameter);
					//Body
					{
						ECS2::Entity::Id bodyCompoundStmtEntityId = CreateEntity();

						std::vector<ECS2::Entity::Id> bodyChilds;

						ECS2::Entity::Id serializeCallExprEntityId = CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
							CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
								File::Table::ECS__FILE__TABLE__GET_FULL_NAMESPACE_STRING(this, ecsComponentEntityId, "::", false) + "::" + File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false) + "Serialize"
							),
							CPP__TREE__CREATE_ENTITIES_VECTOR(),
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
									File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
								)
							)
						);

						bodyCompoundStmtEntityId = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT_COMPONENTS(this,
							bodyCompoundStmtEntityId,
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
									serializeCallExprEntityId
								)
							)
						);

						functionChilds.push_back(bodyCompoundStmtEntityId);
					}
					CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
						templateSerializeSpecialization,
						CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::string"),
						CPP__TREE__CREATE_ENTITIES_VECTOR(),
						"Serialize",
						CPP__TREE__CREATE_ENTITIES_VECTOR(
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,
								File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "::", false)
							)
						),
						hppLastNamespaceEntityId,
						functionChilds
					);
					CreateComponent<CPP::Tree::Inline_>(templateSerializeSpecialization);
					hppFileNodeEntityIds.push_back(templateSerializeSpecialization);
				}

				//Template Parse Specialization.
				ECS2::Entity::Id templateParseSpecialization = CreateEntity();
				if (IsComponentExist<ECS::File::Table::Component::Serializable>(ecsComponentEntityId))
				{
					std::vector<ECS2::Entity::Id> functionChilds;

					ECS2::Entity::Id luaRefParameter = CPP__TREE__DECL__CREATE_PARAMETER(this,
						CPP__TREE__TYPE__CREATE_REFERENCE_TYPE(this,
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "luabridge::LuaRef"), false
						),
						File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
					);

					functionChilds.push_back(luaRefParameter);
					//Body
					{
						ECS2::Entity::Id bodyCompoundStmtEntityId = CreateEntity();

						std::vector<ECS2::Entity::Id> bodyChilds;

						ECS2::Entity::Id parseCallExprEntityId = CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
							CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
								File::Table::ECS__FILE__TABLE__GET_FULL_NAMESPACE_STRING(this, ecsComponentEntityId, "::", false) + "::" + File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false) + "Parse"
							),
							CPP__TREE__CREATE_ENTITIES_VECTOR(),
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
									File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "__", false)
								)
							)
						);

						bodyCompoundStmtEntityId = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT_COMPONENTS(this,
							bodyCompoundStmtEntityId,
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this, parseCallExprEntityId)
							)
						);

						functionChilds.push_back(bodyCompoundStmtEntityId);
					}
					CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
						templateParseSpecialization,
						CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,
							File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "::", false)
						),
						CPP__TREE__CREATE_ENTITIES_VECTOR(),
						"Parse",
						CPP__TREE__CREATE_ENTITIES_VECTOR(
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,
								File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "::", false)
							)
						),
						hppLastNamespaceEntityId,
						functionChilds
					);
					CreateComponent<CPP::Tree::Inline_>(templateParseSpecialization);
					hppFileNodeEntityIds.push_back(templateParseSpecialization);
				}


				//Template Bind Specialization.
				ECS2::Entity::Id templateBindSpecialization = CreateEntity();
				if (IsComponentExist<ECS::File::Table::Component::Bindable>(ecsComponentEntityId)) {
					std::vector<ECS2::Entity::Id> functionChilds;

					ECS2::Entity::Id luaRefParameter = CPP__TREE__DECL__CREATE_PARAMETER(this,
						CPP__TREE__TYPE__CREATE_POINTER_TYPE(this,
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "lua_State")
						),
						"state"
					);

					functionChilds.push_back(luaRefParameter);
					//Body
					{
						ECS2::Entity::Id bodyCompoundStmtEntityId = CreateEntity();

						std::vector<ECS2::Entity::Id> bodyChilds;

						ECS2::Entity::Id parseCallExprEntityId = CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
							CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
								File::Table::ECS__FILE__TABLE__GET_FULL_NAMESPACE_STRING(this, ecsComponentEntityId, "::", false) + "::" + File::Table::ECS__FILE__TABLE__GET_NAME(this, ecsComponentEntityId, false) + "Bind"
							),
							CPP__TREE__CREATE_ENTITIES_VECTOR(),
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
									"state"
								)
							)
						);

						bodyCompoundStmtEntityId = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT_COMPONENTS(this,
							bodyCompoundStmtEntityId,
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this, parseCallExprEntityId)
							)
						);

						functionChilds.push_back(bodyCompoundStmtEntityId);
					}
					CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
						templateBindSpecialization,
						CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
						CPP__TREE__CREATE_ENTITIES_VECTOR(),
						"Bind",
						CPP__TREE__CREATE_ENTITIES_VECTOR(
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,
								File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, ecsComponentEntityId, "::", false)
							)
						),
						hppLastNamespaceEntityId,
						functionChilds
					);
					CreateComponent<CPP::Tree::Inline_>(templateBindSpecialization);
					hppFileNodeEntityIds.push_back(templateBindSpecialization);
				}
			}

			//Generate archetypes macroses
			for (ECS2::Entity::Id archetypeEntityId : fileArchetypes) {

				const ECS2::ComponentsFilter archetypeCF = GetComponentsFilter(archetypeEntityId);
				ASSERT(archetypeCF.IsSet<File::Table::Archetype::Tag>());

				std::vector<ECS2::Entity::Id> componentEntityIds;
				if (archetypeCF.IsSet<ECS::File::Table::Archetype::Components>()) {
					auto* components = GetComponent<ECS::File::Table::Archetype::Components>(archetypeEntityId);
					for (std::string component : components->components_) {
						ECS2::Entity::Id componentEntityId = File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<GenerateTrees, ECS::File::Table::Component::Tag>(this, archetypeEntityId, component);
						componentEntityIds.push_back(componentEntityId);
					}
				}

				std::vector<ECS2::Entity::Id> archetypeEntityIds;
				if (archetypeCF.IsSet<ECS::File::Table::Archetype::Archetypes>()) {
					auto* archetypes = GetComponent<ECS::File::Table::Archetype::Archetypes>(archetypeEntityId);
					for (std::string archetype : archetypes->archetypes_) {
						ECS2::Entity::Id refArchetypeEntityId = File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<GenerateTrees, ECS::File::Table::Archetype::Tag>(this, archetypeEntityId, archetype);
						archetypeEntityIds.push_back(refArchetypeEntityId);
					}
				}

				const std::string componentFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, archetypeEntityId, "::", false);
				const std::string namespaceStr = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, archetypeEntityId, "__", true);

				std::string macrosBody;
				bool isFirst = true;
				for (ECS2::Entity::Id componentEntityId : componentEntityIds) {
					const std::string componentFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, componentEntityId, "::", false);
					if (!isFirst) {
						macrosBody += ",";
					}
					isFirst = false;
					macrosBody += componentFullName;
				}
				for (ECS2::Entity::Id archetypeEntityId : archetypeEntityIds) {
					const std::string archetypeFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, archetypeEntityId, "__", true);
					if (!isFirst) {
						macrosBody += ",";
					}
					isFirst = false;
					macrosBody += archetypeFullName;
				}

				ECS2::Entity::Id defineEntityId = CreateEntity();
				CreateComponent<CPP::Tree::Preprocessor::Tag>(defineEntityId);
				CreateComponent<CPP::Tree::Preprocessor::Define_>(defineEntityId, namespaceStr, std::vector<std::string>{}, macrosBody, ECS2::Entity::Id::invalid_, false);

				hppFileNodeEntityIds.push_back(CPP__TREE__COMMENT__CREATE_CLANG_FORMAT_OFF_COMMENT(this));
				hppFileNodeEntityIds.push_back(defineEntityId);
				hppFileNodeEntityIds.push_back(CPP__TREE__COMMENT__CREATE_CLANG_FORMAT_ON_COMMENT(this));
			}

			//Generate archetypes macroses
			for (ECS2::Entity::Id bundleEntityId : fileBundles) {

				const ECS2::ComponentsFilter bundleCF = GetComponentsFilter(bundleEntityId);
				ASSERT(bundleCF.IsSet<File::Table::Bundle::Tag>());

				std::vector<ECS2::Entity::Id> componentEntityIds;
				if (bundleCF.IsSet<ECS::File::Table::Bundle::Components>()) {
					auto* components = GetComponent<ECS::File::Table::Bundle::Components>(bundleEntityId);
					for (std::string component : components->components_) {
						ECS2::Entity::Id componentEntityId = File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<GenerateTrees, ECS::File::Table::Component::Tag>(this, bundleEntityId, component);
						componentEntityIds.push_back(componentEntityId);
					}
				}

				std::vector<ECS2::Entity::Id> bundleEntityIds;
				if (bundleCF.IsSet<ECS::File::Table::Bundle::Bundles>()) {
					auto* bundles = GetComponent<ECS::File::Table::Bundle::Bundles>(bundleEntityId);
					for (std::string bundle : bundles->bundles_) {
						ECS2::Entity::Id refBundleEntityId = File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<GenerateTrees, ECS::File::Table::Bundle::Tag>(this, bundleEntityId, bundle);
						bundleEntityIds.push_back(refBundleEntityId);
					}
				}

				const std::string componentFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, bundleEntityId, "::", false);
				const std::string namespaceStr = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, bundleEntityId, "__", true);

				std::string macrosBody;
				bool isFirst = true;
				for (ECS2::Entity::Id componentEntityId : componentEntityIds) {
					const std::string componentFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, componentEntityId, "::", false);
					if (!isFirst) {
						macrosBody += ",";
					}
					isFirst = false;
					macrosBody += componentFullName;
				}
				for (ECS2::Entity::Id bundleEntityId : bundleEntityIds) {
					const std::string bundleFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, bundleEntityId, "__", true);
					if (!isFirst) {
						macrosBody += ",";
					}
					isFirst = false;
					macrosBody += bundleFullName;
				}


				ECS2::Entity::Id defineEntityId = CreateEntity();
				CreateComponent<CPP::Tree::Preprocessor::Tag>(defineEntityId);
				CreateComponent<CPP::Tree::Preprocessor::Define_>(defineEntityId, namespaceStr, std::vector<std::string>{}, macrosBody, ECS2::Entity::Id::invalid_, false);

				hppFileNodeEntityIds.push_back(defineEntityId);
			}

			//Generate Systems code tree.
			BEGIN_PROFILE("Generate systems code tree for file %s", path->path_.c_str());
			for (ECS2::Entity::Id systemEntityId : fileSystems) {


				const ECS2::ComponentsFilter systemCF = GetComponentsFilter(systemEntityId);
				ASSERT(systemCF.IsSet<File::Table::System::Tag>());
				const std::string systemName = GetComponent<File::Table::Name>(systemEntityId)->name_;
				const std::string systemFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, systemEntityId, "::", false);
				if (systemName == "CreateProjectIncludeFile") {
					Common::BreakPointLine();
				}
				//HPP file code.
				{
					const ECS2::Entity::Id runSystemFunctionPrototypeEntityId = CreateEntity();
					const ECS2::Entity::Id cppSystemClassEntityId = CreateEntity();
					std::vector<ECS2::Entity::Id> namespaceChilds{ cppSystemClassEntityId, runSystemFunctionPrototypeEntityId };

					const auto [firstNamespaceEntityId, lastNamespaceEntityId] = CPP__TREE__CREATE_CPP_NAMESPACES(this,
						ECS::File::Table::GET_TABLE_PARENTS(this, systemEntityId),
						namespaceChilds);

					//System class 
					{
						std::vector<ECS2::Entity::Id> systemClassChildEntityIds;
						/*if (!ECS__FILE__TABLE__SYSTEM__IS_DISABLED(systemEntityId)) {*/
							//Default constructor.
							{
								ECS2::Entity::Id constructorEntityId = CPP__TREE__DECL__CREATE_EMPTY_CONSTRUCTOR(this,
									GetComponent<File::Table::Name>(systemEntityId)->name_,
									cppSystemClassEntityId, ECS2::Entity::Id::invalid_);
								CreateComponent<CPP::Tree::Access::Public_>(constructorEntityId);
								systemClassChildEntityIds.push_back(constructorEntityId);
							}
							//std::shared_ptr<ECS2::World> world_{ nullptr } member
							ECS2::Entity::Id worldMemberEntityId = CPP__TREE__DECL__CREATE_VARIABLE(this,
								CPP__TREE__TYPE__CREATE_TEMPLATE_SPECIALIZATION_TYPE(this,
									CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::shared_ptr"),
									std::vector{ CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,"ECS2::World") }),
								"world_",
								CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "nullptr", ""),
								cppSystemClassEntityId
							);
							CreateComponent<CPP::Tree::Access::Public_>(worldMemberEntityId);
							systemClassChildEntityIds.push_back(worldMemberEntityId);

							//Constructor with parameter.
							//Create(std::shared_ptr<ECS2::World> world)
							//	:
							//	world_{ world }
							//{
							//}
							{
								ECS2::Entity::Id worldParameterEntityId = CPP__TREE__DECL__CREATE_PARAMETER(this,
									CPP__TREE__TYPE__CREATE_TEMPLATE_SPECIALIZATION_TYPE(this,
										CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::shared_ptr"),
										std::vector{ CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,"ECS2::World") }),
									"world");
								std::vector<ECS2::Entity::Id> functionChilds{ worldParameterEntityId, CPP__TREE__STMT__CREATE_EMPTY_COMPOUND_STATEMENT(this) };
								ECS2::Entity::Id constructorWithParametersEntityId = CPP__TREE__DECL__CREATE_FUNCTION(this,
									ECS2::Entity::Id::invalid_,
									GetComponent<File::Table::Name>(systemEntityId)->name_,
									cppSystemClassEntityId,
									functionChilds
								);
								CreateComponent<CPP::Tree::Access::Public_>(constructorWithParametersEntityId);
								//Initialize list.
								CreateComponent<CPP::Tree::Decl::Constructor>(constructorWithParametersEntityId);
								CreateComponent<CPP::Tree::ConstructorInitList>(constructorWithParametersEntityId,
									std::vector<ECS2::Entity::Id>{ CPP__TREE__CREATE_CONSTRUCTOR_INITIALIZER(this, worldMemberEntityId, CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world"))});

								systemClassChildEntityIds.push_back(constructorWithParametersEntityId);
							}
							//GetComponentsFilter
							{
								ECS2::Entity::Id getComponentsFilterFunctionEntityId = CPP__TREE__DECL__CREATE_FUNCTION(this,
									CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::ComponentsFilter"),
									"GetComponentsFilter",
									cppSystemClassEntityId,
									(std::vector<ECS2::Entity::Id>{
									CPP__TREE__DECL__CREATE_PARAMETER(this, CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::Entity::Id"), "entityId"),
										CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
													CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
														CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
															CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world_"),
															"GetEntityComponentsFilter",
															true
														),
														std::vector<ECS2::Entity::Id>{},
														std::vector<ECS2::Entity::Id>{CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "entityId")}
													)
												)
											)
										)
								}));
								systemClassChildEntityIds.push_back(getComponentsFilterFunctionEntityId);
							}

							//Destroy entity.
							//void DestroyEntity(ECS2::Entity::Id entityId)
							//{
							//	world_->DestroyEntity(entityId);
							//};
							{
								ECS2::Entity::Id destroyEntityFunctionEntityId = CPP__TREE__DECL__CREATE_FUNCTION(this,
									CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
									"DestroyEntity",
									cppSystemClassEntityId,
									(std::vector<ECS2::Entity::Id>{
									CPP__TREE__DECL__CREATE_PARAMETER(this, CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::Entity::Id"), "entityId"),
										CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
													CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
														CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
															CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world_"),
															"DestroyEntity",
															true
														),
														std::vector<ECS2::Entity::Id>{},
														std::vector<ECS2::Entity::Id>{CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "entityId")}
													)
												)
											)
										)
								}));
								systemClassChildEntityIds.push_back(destroyEntityFunctionEntityId);
							}

							//CreateComponent
							//template <class Component, class... Args> void CreateComponent(ECS2::Entity::Id entityId, Args &&...args)
							//{
							//	ASSERT_FMSG((world_->IsArchetypeEntity(entityId))
							//		? (world_->GetArchetypeComponents(entityId).IsSet<Component>())
							//		: (true),
							//		"Attempt to create component {} for archetype entity that can't containt this component.",
							//		Component::GetName());
							//	world_->CreateComponent<Component>(entityId, "ECS::File::Load", std::forward<Args>(args)...);
							//};
							//TODO:
							{
								ECS2::Entity::Id componentTemplateTypeEntityId
									= CPP__TREE__DECL__CREATE_TEMPLATE_TYPE_PARAMETER(this,
										"Component",
										ECS2::Entity::Id::invalid_,
										false);
								ECS2::Entity::Id argsTemplateTypeEntityId
									= CPP__TREE__DECL__CREATE_TEMPLATE_TYPE_PARAMETER(this,
										"Args",
										ECS2::Entity::Id::invalid_,
										true);

								ECS2::Entity::Id functionEntityId = CreateEntity();

								CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
									functionEntityId,
									CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
									CPP__TREE__CREATE_ENTITIES_VECTOR(componentTemplateTypeEntityId, argsTemplateTypeEntityId),
									"CreateComponent",
									CPP__TREE__CREATE_ENTITIES_VECTOR(),
									cppSystemClassEntityId,
									CPP__TREE__CREATE_ENTITIES_VECTOR(
										CPP__TREE__DECL__CREATE_PARAMETER(this,
											CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::Entity::Id"),
											"entityId"
										),
										CPP__TREE__DECL__CREATE_PARAMETER_PACK_PARAMETER(this,
											CPP__TREE__TYPE__CREATE_REFERENCE_TYPE(this,
												CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "Args"),
												true
											),
											"args"
										),
										CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
													CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
														CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
															CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world_"),
															"CreateComponent",
															true
														),
														CPP__TREE__CREATE_ENTITIES_VECTOR(
															CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "Component")
														),
														CPP__TREE__CREATE_ENTITIES_VECTOR(
															CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "entityId"),
															CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"" + systemFullName + "\"", ""),
															CPP__TREE__EXPR__CREATE_PACK_EXPANSION(this,
																CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
																	CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "std::forward"),
																	CPP__TREE__CREATE_ENTITIES_VECTOR(
																		CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "Args")
																	),
																	CPP__TREE__CREATE_ENTITIES_VECTOR(
																		CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "args")
																	)
																)
															)
														)
													)
												)
											)
										)
									));
								CreateComponent<CPP::Tree::Access::Public_>(functionEntityId);

								systemClassChildEntityIds.push_back(functionEntityId);
							}

							/*template <class Component, class... Args> void RemoveComponent(ECS2::Entity::Id entityId, Args &&...args)
							{
								STATIC_ASSERT_MSG((std::is_same_v<Component, Async::Manager::Task::State::Pending> ||
									std::is_same_v<Component, Async::Manager::Task::State::InProgress> ||
									std::is_same_v<Component, Async::Manager::Task::State::Finished>),
									"Attempt to remove component that system(ChangeTaskState) can't remove. Added access "
									"entities description to .ecs file that corresponds to system");
								world_->RemoveComponent<Component>(entityId, "Async::Manager::ChangeTaskState", std::forward<Args>(args)...);
							};*/

							//TODO:
							{
								ECS2::Entity::Id componentTemplateTypeEntityId
									= CPP__TREE__DECL__CREATE_TEMPLATE_TYPE_PARAMETER(this,
										"Component",
										ECS2::Entity::Id::invalid_,
										false);
								ECS2::Entity::Id functionEntityId = CreateEntity();

								CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
									functionEntityId,
									CPP__TREE__TYPE__CREATE_VOID_TYPE(this), //Return value
									CPP__TREE__CREATE_ENTITIES_VECTOR(componentTemplateTypeEntityId),
									"RemoveComponent",
									CPP__TREE__CREATE_ENTITIES_VECTOR(),
									cppSystemClassEntityId,
									CPP__TREE__CREATE_ENTITIES_VECTOR(
										//1 method parameter
										CPP__TREE__DECL__CREATE_PARAMETER(this,
											CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::Entity::Id"),
											"entityId"
										),
										//Body
										CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
													CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
														CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
															CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world_"),
															"RemoveComponent",
															true
														),
														//<Component> template parameter
														CPP__TREE__CREATE_ENTITIES_VECTOR(
															CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "Component")
														),

														CPP__TREE__CREATE_ENTITIES_VECTOR(
															CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "entityId")
														)
													)
												)
											)
										)
									));
								CreateComponent<CPP::Tree::Access::Public_>(functionEntityId);

								systemClassChildEntityIds.push_back(functionEntityId);
							}


							//IsComponentExist
							//template <class Component> bool IsComponentExist(ECS2::Entity::Id entityId)
							//{
							//	return world_->IsComponentExist<Component>(entityId);
							//};
							{
								ECS2::Entity::Id componentTemplateTypEntityId = CPP__TREE__DECL__CREATE_TEMPLATE_TYPE_PARAMETER(this, "Component", ECS2::Entity::Id::invalid_, false);
								std::vector<ECS2::Entity::Id> childs;

								childs.push_back(CPP__TREE__DECL__CREATE_PARAMETER(this, CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::Entity::Id"), "entityId"));

								childs.push_back(
									CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
												CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
													CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
														CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world_"),
														"IsComponentExist",
														true
													),
													std::vector<ECS2::Entity::Id>{componentTemplateTypEntityId},
													std::vector<ECS2::Entity::Id>{CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "entityId")}
												)
											)
										)
									));

								ECS2::Entity::Id functionEntityId = CreateEntity();

								CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
									functionEntityId,
									CPP__TREE__TYPE__CREATE_BOOL_TYPE(this),
									std::vector<ECS2::Entity::Id>{ componentTemplateTypEntityId },
									"IsComponentExist",
									std::vector<ECS2::Entity::Id>{ },
									cppSystemClassEntityId,
									childs);

								CreateComponent<CPP::Tree::Access::Public_>(functionEntityId);

								systemClassChildEntityIds.push_back(functionEntityId);
							}

							//CreateEntity
							//ECS2::Entity::Id CreateEntity()
							//{
							//	return world_->CreateEntity();
							//};
							{
								ECS2::Entity::Id createEntityMethodEntityId =
									CPP__TREE__DECL__CREATE_PUBLIC_METHOD(this,
										CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::Entity::Id"),
										"CreateEntity",
										ECS2::Entity::Id::invalid_,
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
												CPP__TREE__CREATE_ENTITIES_VECTOR(
													CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
														CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
															CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
																CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world_"),
																"CreateEntity",
																true
															),
															CPP__TREE__CREATE_ENTITIES_VECTOR(),
															CPP__TREE__CREATE_ENTITIES_VECTOR()
														)
													)
												)
											)
										)
									);
								systemClassChildEntityIds.push_back(createEntityMethodEntityId);
							}

							//template <class... Components> ECS2::Entity::Id CreateEntity()
							//{
							//	return world_->CreateEntity<Components...>();
							//};
							{
								ECS2::Entity::Id componentsTemplateTypEntityId = CPP__TREE__DECL__CREATE_TEMPLATE_TYPE_PARAMETER(this, "Component", ECS2::Entity::Id::invalid_, true);
								std::vector<ECS2::Entity::Id> childs;

								childs.push_back(
									CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
												CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
													CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
														CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world_"),
														"CreateEntity",
														true
													),
													CPP__TREE__CREATE_ENTITIES_VECTOR(componentsTemplateTypEntityId),
													CPP__TREE__CREATE_ENTITIES_VECTOR()
												)
											)
										)
									)
								);

								ECS2::Entity::Id functionEntityId = CreateEntity();

								CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
									functionEntityId,
									CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::Entity::Id"),
									CPP__TREE__CREATE_ENTITIES_VECTOR(componentsTemplateTypEntityId),
									"CreateEntity",
									CPP__TREE__CREATE_ENTITIES_VECTOR(),
									cppSystemClassEntityId,
									childs);

								CreateComponent<CPP::Tree::Access::Public_>(functionEntityId);

								systemClassChildEntityIds.push_back(functionEntityId);
							}


							//IsEntityExist
							//template <class... Components> bool IsEntityExist()
							//{
							//	return world_->IsEntityExist<Components...>();
							//};
							{
								ECS2::Entity::Id componentTemplateTypEntityId = CPP__TREE__DECL__CREATE_TEMPLATE_TYPE_PARAMETER(this, "Components", ECS2::Entity::Id::invalid_, true);

								std::vector<ECS2::Entity::Id> childs;
								childs.push_back(
									CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
												CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
													CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
														CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world_"),
														"IsEntityExist",
														true
													),
													std::vector<ECS2::Entity::Id>{componentTemplateTypEntityId},
													std::vector<ECS2::Entity::Id>{}
												)
											)
										)
									));


								ECS2::Entity::Id functionEntityId = CreateEntity();

								CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
									functionEntityId,
									CPP__TREE__TYPE__CREATE_BOOL_TYPE(this),
									std::vector<ECS2::Entity::Id>{ componentTemplateTypEntityId },
									"IsEntityExist",
									std::vector<ECS2::Entity::Id>{ },
									cppSystemClassEntityId,
									childs);

								CreateComponent<CPP::Tree::Access::Public_>(functionEntityId);

								systemClassChildEntityIds.push_back(functionEntityId);
							}
							//IsEntityExist
							//bool IsEntityExist(ECS2::Entity::Id entityId)
							//{
							//	return world_->IsEntityExist(entityId);
							//};
							{
								std::vector<ECS2::Entity::Id> childs;
								childs.push_back(CPP__TREE__DECL__CREATE_PARAMETER(this, CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::Entity::Id"), "entityId"));
								childs.push_back(
									CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
												CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
													CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
														CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world_"),
														"IsEntityExist",
														true
													),
													std::vector<ECS2::Entity::Id>{},
													std::vector<ECS2::Entity::Id>{CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "entityId")}
												)
											)
										)
									));


								ECS2::Entity::Id functionEntityId = CreateEntity();

								CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
									functionEntityId,
									CPP__TREE__TYPE__CREATE_BOOL_TYPE(this),
									std::vector<ECS2::Entity::Id>{ },
									"IsEntityExist",
									std::vector<ECS2::Entity::Id>{ },
									cppSystemClassEntityId,
									childs);

								CreateComponent<CPP::Tree::Access::Public_>(functionEntityId);

								systemClassChildEntityIds.push_back(functionEntityId);
							}

							//GetComponent
							{
								ECS2::Entity::Id componentTemplateTypEntityId = CPP__TREE__DECL__CREATE_TEMPLATE_TYPE_PARAMETER(this, "Component", ECS2::Entity::Id::invalid_, false);
								std::vector<ECS2::Entity::Id> childs;
								if (IsComponentExist<ECS::File::Table::System::UpdateMethod::EntityId>(systemEntityId)) {

									std::vector<ECS2::Entity::Id> isAnyOfArgs;
									isAnyOfArgs.push_back(componentTemplateTypEntityId);
									ECS2::Entity::Id updateMethodEntityId = GetComponent<ECS::File::Table::System::UpdateMethod::EntityId>(systemEntityId)->id_;
									if (IsComponentExist<ECS::File::Table::System::UpdateMethod::Process::Entity::EntityIds>(updateMethodEntityId)) {

										const auto processEntityIds = GetComponent<ECS::File::Table::System::UpdateMethod::Process::Entity::EntityIds>(updateMethodEntityId)->entityIds_;

										if (!processEntityIds.empty()) {

											if (systemName == "ProcessLoadResourceRequest") {
												Common::BreakPointLine();
											}
											Common::Index entityIndex = 0;
											for (ECS2::Entity::Id processEntityId : processEntityIds) {
												const bool allCombinations = IsComponentExist<ECS::File::Table::System::UpdateMethod::Process::Entity::ProcessAllCombinations>(processEntityId);				\
													if (allCombinations) continue;
												auto processComponentInfos = GetComponent<ECS::File::Table::System::UpdateMethod::Process::Entity::ProcessComponents>(processEntityId)->componentInfos_;

												for (const auto& componentInfo : processComponentInfos) {
													ECS2::Entity::Id componentEntityId = File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<GenerateTrees, ECS::File::Table::Component::Tag>(this, systemEntityId, componentInfo.name_);
													const std::string componentName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, componentEntityId, "::", false);
													isAnyOfArgs.push_back(CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, componentName));
												}
											}

										}
									}

									if (IsComponentExist<ECS::File::Table::System::UpdateMethod::Access::Entity::EntityIds>(updateMethodEntityId)) {

										const auto accessEntityIds = GetComponent<ECS::File::Table::System::UpdateMethod::Access::Entity::EntityIds>(updateMethodEntityId)->entityIds_;

										if (!accessEntityIds.empty()) {

											if (systemName == "ProcessLoadResourceRequest") {
												Common::BreakPointLine();
											}
											Common::Index entityIndex = 0;
											for (ECS2::Entity::Id accessEntityId : accessEntityIds) {
												//components
												if (IsComponentExist<ECS::File::Table::System::UpdateMethod::Access::Entity::AccessComponents>(accessEntityId)) {
													auto accessComponentInfos = GetComponent<ECS::File::Table::System::UpdateMethod::Access::Entity::AccessComponents>(accessEntityId)->components_;

													for (const auto& componentInfo : accessComponentInfos) {
														ECS2::Entity::Id componentEntityId = File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<GenerateTrees, ECS::File::Table::Component::Tag>(this, systemEntityId, componentInfo.name_);
														const std::string componentName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, componentEntityId, "::", false);
														isAnyOfArgs.push_back(CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, componentName));
													}
												}
												//archetype
												if (IsComponentExist<ECS::File::Table::System::UpdateMethod::Access::Entity::Archetype>(accessEntityId)) {
													const std::string archetypeLocalName = GetComponent<ECS::File::Table::System::UpdateMethod::Access::Entity::Archetype>(accessEntityId)->name_;
													ECS2::Entity::Id archetypeEntityId = File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<GenerateTrees, ECS::File::Table::Archetype::Tag>(this, systemEntityId, archetypeLocalName);
													const std::string namespaceStr = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, archetypeEntityId, "__", true);
													isAnyOfArgs.push_back(CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, namespaceStr));

												}
											}

										}
									}




									ECS2::Entity::Id templateIdExprEntityId = CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
										CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "Common::IsAnyOf"),
										isAnyOfArgs,
										CPP__TREE__CREATE_ENTITIES_VECTOR()
									);

									ECS2::Entity::Id parentExprEntityId = CreateEntity();
									CreateComponent<CPP::Tree::Expr::Tag>(parentExprEntityId);
									CreateComponent<CPP::Tree::Expr::ParentExpr>(parentExprEntityId, templateIdExprEntityId);

									std::vector<ECS2::Entity::Id> macroArgEntityId{
										parentExprEntityId,
											CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this,
												"\"Attempt to access component{} that system(GenerateCode) can't access. Added access entities description "
												"to .ecs file that corresponds to system\"", "")
									};

									ECS2::Entity::Id macroCallEntityId = CPP__TREE__EXPR__CREATE_MACRO_INVOCATION(this, "STATIC_ASSERT_MSG",
										macroArgEntityId, ECS2::Entity::Id::invalid_);

									childs.push_back(CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this, std::vector<ECS2::Entity::Id>{CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this, macroCallEntityId)}));

								}

								childs.push_back(
									CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
										CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
											CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world_"),
												"GetComponent",
												true
											),
											std::vector<ECS2::Entity::Id>{componentTemplateTypEntityId},
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "entityId")
											)
										)

									));


								ECS2::Entity::Id functionEntityId = CreateEntity();

								CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
									functionEntityId,
									CPP__TREE__TYPE__CREATE_POINTER_TYPE(this, componentTemplateTypEntityId),
									std::vector<ECS2::Entity::Id>{ componentTemplateTypEntityId },
									"GetComponent",
									CPP__TREE__CREATE_ENTITIES_VECTOR(

									),
									cppSystemClassEntityId,
									CPP__TREE__CREATE_ENTITIES_VECTOR(
										CPP__TREE__DECL__CREATE_PARAMETER(this,
											CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::Entity::Id"),
											"entityId"
										),
										CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
											childs
										)
									)
								);

								CreateComponent<CPP::Tree::Access::Public_>(functionEntityId);

								systemClassChildEntityIds.push_back(functionEntityId);
							}
						/*}*/
						//Update method prototype generation.
						//if(!File::Table::IsSystemDisabled(this, systemEntityId)) {
						std::vector<ECS2::Entity::Id> updateCallParametersEntityIds;
						if (IsComponentExist<ECS::File::Table::System::UpdateMethod::EntityId>(systemEntityId)) {
							ECS2::Entity::Id updateMethodEntityId = GetComponent<ECS::File::Table::System::UpdateMethod::EntityId>(systemEntityId)->id_;
							if (IsComponentExist<ECS::File::Table::System::UpdateMethod::Process::Entity::EntityIds>(updateMethodEntityId)) {
								const auto processEntityIds = GetComponent<ECS::File::Table::System::UpdateMethod::Process::Entity::EntityIds>(updateMethodEntityId)->entityIds_;

								Common::Index entityIndex = 0;
								for (ECS2::Entity::Id processEntityId : processEntityIds) {
									updateCallParametersEntityIds.push_back(CPP__TREE__DECL__CREATE_PARAMETER(this,
										CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::Entity::Id"),
										"entity" + std::to_string(entityIndex) + "Id")
									);
									const bool allCombinations = IsComponentExist<ECS::File::Table::System::UpdateMethod::Process::Entity::ProcessAllCombinations>(processEntityId);				\

										if (allCombinations) continue;

									if (systemName == "CreateLuaContext") {
										Common::BreakPointLine();
									}
									auto processComponentInfos = GetComponent<ECS::File::Table::System::UpdateMethod::Process::Entity::ProcessComponents>(processEntityId)->componentInfos_;
									for (const auto& componentInfo : processComponentInfos) {
										ECS2::Entity::Id componentEntityId = File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<GenerateTrees, ECS::File::Table::Component::Tag>(this, systemEntityId, componentInfo.name_);
										const std::string componentTypeFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, componentEntityId, "::", false);
										std::string componentParameterFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, componentEntityId, "__", false);
										componentParameterFullName = std::string{ (char)std::tolower(componentParameterFullName[0]) } + componentParameterFullName.substr(1);

										ECS2::Entity::Id componentTypeEntityId = CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, componentTypeFullName);
										if (componentInfo.readonly_) {
											CreateComponent<CPP::Tree::Type::ConstType>(componentTypeEntityId);
										}


										updateCallParametersEntityIds.push_back(CPP__TREE__DECL__CREATE_PARAMETER(this,
											CPP__TREE__TYPE__CREATE_POINTER_TYPE(this, componentTypeEntityId),
											componentParameterFullName + std::to_string(entityIndex)
										));
									}
									entityIndex++;
								}

							}
							//}

							systemClassChildEntityIds.push_back(
								CPP__TREE__DECL__CREATE_FUNCTION(this,
									CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
									"Update",
									cppSystemClassEntityId,
									updateCallParametersEntityIds));
						}

						CPP__TREE__DECL__CREATE_CLASS_COMPONENTS(this,
							cppSystemClassEntityId,
							GetComponent<File::Table::Name>(systemEntityId)->name_,
							lastNamespaceEntityId,
							systemClassChildEntityIds
						);

						if (firstNamespaceEntityId.IsInvalid()) {
							hppFileNodeEntityIds.push_back(cppSystemClassEntityId);
						}
					}
					//Create run system prototype for HPP 
					{
						//Parameter: std::shared_ptr<ECS2::World> world
						ECS2::Entity::Id worldParameterEntityId = CPP__TREE__DECL__CREATE_PARAMETER(this,
							CPP__TREE__TYPE__CREATE_TEMPLATE_SPECIALIZATION_TYPE(this,
								CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::shared_ptr"),
								std::vector{ CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,"ECS2::World") }),
							"world");

						CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
							runSystemFunctionPrototypeEntityId,
							CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
							std::vector<ECS2::Entity::Id>{ },
							GetComponent<File::Table::Name>(systemEntityId)->name_ + "System",
							std::vector<ECS2::Entity::Id>{ },
							lastNamespaceEntityId,
							std::vector{ worldParameterEntityId }
						);

						if (firstNamespaceEntityId.IsInvalid()) {
							hppFileNodeEntityIds.push_back(runSystemFunctionPrototypeEntityId);
						}

					}

					if (firstNamespaceEntityId.IsValid()) {
						hppFileNodeEntityIds.push_back(firstNamespaceEntityId);
					}

				}

				//Create run system realization for CPP 
				{
					ECS2::Entity::Id runSystemFunctionRealizationEntityId = CreateEntity();
					const auto ecsParentTables = ECS::File::Table::GET_TABLE_PARENTS(this, systemEntityId);
					std::vector<ECS2::Entity::Id> namespaceChilds{ runSystemFunctionRealizationEntityId };
					const auto [firstNamespaceEntityId, lastNamespaceEntityId] = CPP__TREE__CREATE_CPP_NAMESPACES(this, ecsParentTables, namespaceChilds);


					std::vector<ECS2::Entity::Id> functionChilds;
					//Parameter: std::shared_ptr<ECS2::World> world
					ECS2::Entity::Id worldParameterEntityId = CPP__TREE__DECL__CREATE_PARAMETER(this,
						CPP__TREE__TYPE__CREATE_TEMPLATE_SPECIALIZATION_TYPE(this,
							CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::shared_ptr"),
							std::vector{ CPP__TREE__TYPE__CREATE_NAMED_TYPE(this,"ECS2::World") }),
						"world");
					functionChilds.push_back(worldParameterEntityId);
					//Body.

					if (!File::Table::IsSystemDisabled(this, systemEntityId)) {
						ECS2::Entity::Id bodyCompoundStmtEntityId = CreateEntity();
						std::vector<ECS2::Entity::Id> bodyChilds;
						{//System object variable.
							ECS2::Entity::Id systemObjectVariable = CPP__TREE__DECL__CREATE_VARIABLE(this,
								CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, systemName),
								std::string{ (char)std::tolower(systemName[0]) } + systemName.substr(1),
								CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world"),
								bodyCompoundStmtEntityId
							);
							bodyChilds.push_back(systemObjectVariable);
						}

						//Processing entities
						if (IsComponentExist<ECS::File::Table::System::UpdateMethod::EntityId>(systemEntityId)) {
							ECS2::Entity::Id updateMethodEntityId = GetComponent<ECS::File::Table::System::UpdateMethod::EntityId>(systemEntityId)->id_;
							if (IsComponentExist<ECS::File::Table::System::UpdateMethod::Process::Entity::EntityIds>(updateMethodEntityId)) {
								const auto processEntityIds = GetComponent<ECS::File::Table::System::UpdateMethod::Process::Entity::EntityIds>(updateMethodEntityId)->entityIds_;

								std::vector<ECS2::Entity::Id> updateCallArgumentsIdentifierExprEntityIds;

								std::function<ECS2::Entity::Id(ECS2::Entity::Id, Common::Index)> createForEachCall = [&](ECS2::Entity::Id processEntityId, Common::Index entityIndex) {

									if (entityIndex == processEntityIds.size()) {
										return CPP__TREE__STMT__CREATE_EMPTY_COMPOUND_STATEMENT(this);
									}
									const bool isLastEntity = entityIndex == processEntityIds.size() - 1;

									auto excludeComponents = GetComponent<ECS::File::Table::System::UpdateMethod::Process::Entity::ExcludeComponents>(processEntityId)->components_;
									//Exclude ComponentsFilter
									{
										ECS2::Entity::Id excludeVariable = CPP__TREE__DECL__CREATE_VARIABLE(this,
											CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::ComponentsFilter"),
											"excludesForEntity" + std::to_string(entityIndex),
											ECS2::Entity::Id::invalid_,
											bodyCompoundStmtEntityId
										);
										ECS2::Entity::Id memberAccessExprEntityId = CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "excludesForEntity" + std::to_string(entityIndex)),
											"SetBits",
											false);

										std::vector<ECS2::Entity::Id> templateArgEntityIds;
										for (const auto& excludeComponent : excludeComponents) {
											ECS2::Entity::Id excludeComponentEntityId = File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<GenerateTrees, ECS::File::Table::Component::Tag>(this, systemEntityId, excludeComponent);
											const std::string componentTypeFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, excludeComponentEntityId, "::", false);
											ECS2::Entity::Id componentTypeEntityId = CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, componentTypeFullName);
											templateArgEntityIds.push_back(componentTypeEntityId);
										}

										bodyChilds.push_back(excludeVariable);
										bodyChilds.push_back(CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
											CPP__TREE__EXPR__CREATE_CALL_EXPR(this, memberAccessExprEntityId, templateArgEntityIds, std::vector<ECS2::Entity::Id>{})
										));
									}

									const bool allCombinations = IsComponentExist<ECS::File::Table::System::UpdateMethod::Process::Entity::ProcessAllCombinations>(processEntityId);

									const bool areThereEntityComponents = IsComponentExist<ECS::File::Table::System::UpdateMethod::Process::Entity::ProcessComponents>(processEntityId);
									ECS2::Entity::Id lambdaEntityId = ECS2::Entity::Id::invalid_;
									{
										std::vector<ECS2::Entity::Id> lambdaParameters;
										//ECS2::Entity::Id entity0Id
										lambdaParameters.push_back(CPP__TREE__DECL__CREATE_PARAMETER(this,
											CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "ECS2::Entity::Id"),
											"entity" + std::to_string(entityIndex) + "Id")
										);

										updateCallArgumentsIdentifierExprEntityIds.push_back(CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "entity" + std::to_string(entityIndex) + "Id"));

										if (areThereEntityComponents) {
											auto processComponentInfos = GetComponent<ECS::File::Table::System::UpdateMethod::Process::Entity::ProcessComponents>(processEntityId)->componentInfos_;

											for (const auto& componentInfo : processComponentInfos) {
												ECS2::Entity::Id componentEntityId = File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<GenerateTrees, ECS::File::Table::Component::Tag>(this, systemEntityId, componentInfo.name_);
												const std::string componentTypeFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, componentEntityId, "::", false);
												std::string componentParameterFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, componentEntityId, "__", true);
												componentParameterFullName = std::string{ (char)std::tolower(componentParameterFullName[0]) } + componentParameterFullName.substr(1);
												updateCallArgumentsIdentifierExprEntityIds.push_back(CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, componentParameterFullName));
												ECS2::Entity::Id componentTypeEntityId = CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, componentTypeFullName);
												if (componentInfo.readonly_) {
													CreateComponent<CPP::Tree::Type::ConstType>(componentTypeEntityId);
												}
												lambdaParameters.push_back(CPP__TREE__DECL__CREATE_PARAMETER(this,
													CPP__TREE__TYPE__CREATE_POINTER_TYPE(this, componentTypeEntityId),
													componentParameterFullName
												));
											}
										}
										std::vector<ECS2::Entity::Id> childForEachCallStmt;
										if (!isLastEntity) {
											childForEachCallStmt.push_back(createForEachCall(processEntityIds[entityIndex + 1], entityIndex + 1));
										}
										else {
											ECS2::Entity::Id systemObjectMemberAccessExprEntityId = CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, std::string{ (char)std::tolower(systemName[0]) } + systemName.substr(1)),
												"Update",
												false);

											childForEachCallStmt.push_back(
												CREATE_BEGIN_PROFILE_EXPR_STATEMENT(this,
													("\"" + systemName + "::Update\"")
												)
											);
											childForEachCallStmt.push_back(CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
												CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
													systemObjectMemberAccessExprEntityId,
													CPP__TREE__CREATE_ENTITIES_VECTOR(),
													updateCallArgumentsIdentifierExprEntityIds
												)
											));
											childForEachCallStmt.push_back(CREATE_END_PROFILE_EXPR_STATEMENT(this));

										}
										lambdaEntityId = CPP__TREE__EXPR__CREATE_LAMBDA(this,
											CPP::Tree::Expr::CaptureType::ByReference,
											std::vector<CPP::Tree::Expr::LambdaCapture>{},
											lambdaParameters,
											CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this, childForEachCallStmt));
									}

									ECS2::Entity::Id memberAccessExprEntityId = CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
										CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world"),
										"ForEachEntity",
										true);

									std::vector<ECS2::Entity::Id> templateArgEntityIds;
									if (areThereEntityComponents) {
										auto processComponentInfos = GetComponent<ECS::File::Table::System::UpdateMethod::Process::Entity::ProcessComponents>(processEntityId)->componentInfos_;
										for (const auto& componentInfo : processComponentInfos) {
											ECS2::Entity::Id componentEntityId = File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<GenerateTrees, ECS::File::Table::Component::Tag>(this, systemEntityId, componentInfo.name_);
											const std::string componentTypeFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, componentEntityId, "::", false);
											ECS2::Entity::Id componentTypeEntityId = CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, componentTypeFullName);
											templateArgEntityIds.push_back(componentTypeEntityId);
										}
									}
									std::vector<ECS2::Entity::Id> argEntityIds;
									argEntityIds.push_back(CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "excludesForEntity" + std::to_string(entityIndex)));
									argEntityIds.push_back(lambdaEntityId);

									return CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
										CPP__TREE__EXPR__CREATE_CALL_EXPR(this, memberAccessExprEntityId, templateArgEntityIds, argEntityIds)
									);


									};

								ECS2::Entity::Id forEachExprStmtEntityId = createForEachCall(processEntityIds[0], 0);
								bodyChilds.push_back(forEachExprStmtEntityId);
							}
							else {
								//No process entities need to call update method once.
								ECS2::Entity::Id systemObjectMemberAccessExprEntityId = CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
									CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, std::string{ (char)std::tolower(systemName[0]) } + systemName.substr(1)),
									"Update",
									false);

								bodyChilds.push_back(
									CREATE_BEGIN_PROFILE_EXPR_STATEMENT(this,
										("\"" + systemName + "::Update\"")
									)
								);
								bodyChilds.push_back(CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
									CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
										systemObjectMemberAccessExprEntityId,
										CPP__TREE__CREATE_ENTITIES_VECTOR(),
										CPP__TREE__CREATE_ENTITIES_VECTOR()
									)
								));
								bodyChilds.push_back(CREATE_END_PROFILE_EXPR_STATEMENT(this));
							}
						}
						else {
							// No update method. This system is mark for systems graph.
						}

						bodyCompoundStmtEntityId = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT_COMPONENTS(this,
							bodyCompoundStmtEntityId,
							bodyChilds);

						functionChilds.push_back(bodyCompoundStmtEntityId);
					}

					CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
						runSystemFunctionRealizationEntityId,
						CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
						std::vector<ECS2::Entity::Id>{ },
						GetComponent<File::Table::Name>(systemEntityId)->name_ + "System",
						std::vector<ECS2::Entity::Id>{ },
						lastNamespaceEntityId,
						functionChilds
					);

					if (firstNamespaceEntityId.IsInvalid()) {
						cppFileNodeEntityIds.push_back(runSystemFunctionRealizationEntityId);
					}

					if (firstNamespaceEntityId.IsValid()) {
						cppFileNodeEntityIds.push_back(firstNamespaceEntityId);
					}

				}

			}
			END_PROFILE();


			CreateComponent<CPP::Tree::Node::ChildEntityIds>(hppFileEntityId, hppFileNodeEntityIds);
			CreateComponent<CPP::Tree::Node::ChildEntityIds>(cppFileEntityId, cppFileNodeEntityIds);
		}


		//const auto allComponents = world_->GetComponents<ECS__FILE__TABLE__COMPONENT__COMPONENT>();
		//const Common::Size allComponentsNumber = world_->GetEntitiesNumber<ECS__FILE__TABLE__COMPONENT__COMPONENT>();

		//const auto allSystems = world_->GetComponents<ECS__FILE__TABLE__SYSTEM__SYSTEM>();
		//const Common::Size allSystemsNumber = world_->GetEntitiesNumber<ECS__FILE__TABLE__SYSTEM__SYSTEM>();

	}

	void GenerateComponentUtilsFile::Update(
		ECS2::Entity::Id entity0id,
		const ECS::Project::Tag* eCS__Project__Tag0,
		const ECS::Project::Path* eCS__Project__Path0,
		const Behaviour::LuaScript* luaScript0) {


		const char* fileName = "auto_OksEngine.ECS.Component.Utils";

		//HPP File
		ECS2::Entity::Id hppFileEntityId = CreateEntity<CPP__FILE__FILE>();
		std::vector<ECS2::Entity::Id> hppFileNodeEntityIds;
		{
			CreateComponent<CPP::File::Tag>(hppFileEntityId);
			CreateComponent<CPP::File::Type::Hpp>(hppFileEntityId);
			//File PATH and NAME
			CreateComponent<CPP::File::Path>(hppFileEntityId, std::filesystem::path{ eCS__Project__Path0->path_ }.parent_path().string());
			CreateComponent<CPP::File::Name>(hppFileEntityId, fileName);

			hppFileNodeEntityIds.push_back(CPP__TREE__PREPROCESSOR__CREATE_PRAGMA_ONCE(this));
			hppFileNodeEntityIds.push_back(CPP__TREE__PREPROCESSOR__CREATE_INCLUDE(this, "Common/ECS2/ECS2.hpp"));

		}

		//CPP File
		ECS2::Entity::Id cppFileEntityId = CreateEntity<CPP__FILE__FILE>();
		std::vector<ECS2::Entity::Id> cppFileNodeEntityIds;
		{
			CreateComponent<CPP::File::Tag>(cppFileEntityId);
			CreateComponent<CPP::File::Type::Cpp>(cppFileEntityId);

			//File PATH and NAME
			CreateComponent<CPP::File::Path>(cppFileEntityId, std::filesystem::path{ eCS__Project__Path0->path_ }.parent_path().string());
			CreateComponent<CPP::File::Name>(cppFileEntityId, fileName);

			//Generate .hpp file include.
			{
				cppFileNodeEntityIds.push_back(CPP__TREE__PREPROCESSOR__CREATE_INCLUDE(this, "auto_OksEngine.ECS.Component.Utils.hpp"));
			}


			const auto allFiles = world_->GetComponents<ECS__FILE__FILE>();
			const Common::Size allFilesNumber = world_->GetEntitiesNumber<ECS__FILE__FILE>();

			for (Common::Index fileIndex = 0; fileIndex < allFilesNumber; fileIndex++) {
				ECS2::Entity::Id fileEntityId = *(std::get<ECS2::Entity::Id*>(allFiles) + fileIndex);
				//ECS2::Entity::Id moduleEntityId = GetComponent<ECS::Module::EntityId>(fileEntityId)->id_;
				//std::filesystem::path includePath = GetComponent<ECS::Module::IncludePath>(moduleEntityId)->path_;
				std::filesystem::path includePath = File::Table::ECS__FILE__GET_FILE_INCLUDE_PATH(this,
					fileEntityId,
					std::filesystem::path{ eCS__Project__Path0->path_ }.parent_path(),
					".hpp");
				cppFileNodeEntityIds.push_back(CPP__TREE__PREPROCESSOR__CREATE_INCLUDE(this, includePath.string()));
			}
		}

		//ECS2::Entity::Id cppGetComponentTypeIdByFullName = CreateEntity();
		//ECS2::Entity::Id hppGetComponentTypeIdByFullName = CreateEntity();

		//ECS2::Entity::Id cppGetFullNameByComponentTypeId = CreateEntity();
		//ECS2::Entity::Id hppGetFullNameByComponentTypeId = CreateEntity();

		//ECS2::Entity::Id componentNameParameterEntityId = CPP__TREE__DECL__CREATE_PARAMETER(
		//	CPP__TREE__TYPE__CREATE_CONST_CHAR_POINTER_TYPE(),
		//	"componentName"
		//);

		//ECS2::Entity::Id componentTypeIdParameterEntityId = CPP__TREE__DECL__CREATE_PARAMETER(
		//	CPP__TREE__TYPE__CREATE_NAMED_TYPE("ECS2::ComponentTypeId"),
		//	"componentTypeId"
		//);

		//ECS2::Entity::Id componentTypeIdType = CPP__TREE__TYPE__CREATE_NAMED_TYPE("ECS2::ComponentTypeId");


		//std::vector<ECS2::Entity::Id> fullComponentNameToIdPairs{};
		//std::vector<ECS2::Entity::Id> idToFullComponentNamePairs{};

		//const auto allComponents = world_->GetComponents<ECS__FILE__TABLE__COMPONENT__COMPONENT>();
		//const Common::Size allComponentsNumber = world_->GetEntitiesNumber<ECS__FILE__TABLE__COMPONENT__COMPONENT>();

		//for (Common::Index i = 0; i < allComponentsNumber; i++) {
		//	ECS2::Entity::Id componentEntityId = *(std::get<ECS2::Entity::Id*>(allComponents) + i);
		//	const auto* path = std::get<ECS::File::Table::Name*>(allComponents) + i;
		//	const auto* typeId = std::get<ECS::File::Table::Component::TypeId*>(allComponents) + i;
		//	const std::string componentFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this,componentEntityId, "::", false);
		//	const std::string componentName = ECS__FILE__TABLE__GET_NAME(componentEntityId, false);


		//	fullComponentNameToIdPairs.push_back(
		//		CPP__TREE__EXPR__CREATE_INIT_LIST(
		//			CPP__TREE__CREATE_ENTITIES_VECTOR(
		//				CPP__TREE__EXPR__CREATE_LITERAL_EXPR("\"" + componentFullName + "\"", ""),
		//				CPP__TREE__EXPR__CREATE_CALL_EXPR_NO_ARGS(
		//					CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(componentFullName + "::GetTypeId")
		//				)
		//			)
		//		)
		//	);

		//	idToFullComponentNamePairs.push_back(
		//		CPP__TREE__EXPR__CREATE_INIT_LIST(
		//			CPP__TREE__CREATE_ENTITIES_VECTOR(
		//				CPP__TREE__EXPR__CREATE_CALL_EXPR_NO_ARGS(
		//					CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(componentFullName + "::GetTypeId")
		//				),
		//				CPP__TREE__EXPR__CREATE_LITERAL_EXPR("\"" + componentFullName + "\"", "")
		//			)
		//		)
		//	);

		//}

		//ECS2::Entity::Id fullNameToComponentTypeIdVariable = CPP__TREE__DECL__CREATE_STATIC_VARIABLE(
		//	CPP__TREE__TYPE__CREATE_TEMPLATE_SPECIALIZATION_TYPE(
		//		CPP__TREE__TYPE__CREATE_NAMED_TYPE("std::unordered_map"),
		//		CPP__TREE__TYPE__CREATE_NAMED_TYPE_ENTITIES_VECTOR("std::string", "ECS2::ComponentTypeId")
		//	),
		//	"fullNameToComponentTypeId",
		//	CPP__TREE__EXPR__CREATE_INIT_LIST(
		//		CPP__TREE__CREATE_ENTITIES_VECTOR(fullComponentNameToIdPairs)
		//	),
		//	ECS2::Entity::Id::invalid_
		//);

		//ECS2::Entity::Id componentTypeIdToFullNameToVariable = CPP__TREE__DECL__CREATE_STATIC_VARIABLE(
		//	CPP__TREE__TYPE__CREATE_TEMPLATE_SPECIALIZATION_TYPE(
		//		CPP__TREE__TYPE__CREATE_NAMED_TYPE("std::unordered_map"),
		//		CPP__TREE__TYPE__CREATE_NAMED_TYPE_ENTITIES_VECTOR("ECS2::ComponentTypeId", "std::string")
		//	),
		//	"componentTypeIdToFullName",
		//	CPP__TREE__EXPR__CREATE_INIT_LIST(
		//		CPP__TREE__CREATE_ENTITIES_VECTOR(idToFullComponentNamePairs)
		//	),
		//	ECS2::Entity::Id::invalid_
		//);

		//ECS2::Entity::Id getComponentTypeIdByFullNameBody = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(
		//	CPP__TREE__CREATE_ENTITIES_VECTOR(
		//		fullNameToComponentTypeIdVariable
		//	)
		//);

		//ECS2::Entity::Id getFullNameByComponentTypeIdBody = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(
		//	CPP__TREE__CREATE_ENTITIES_VECTOR(componentTypeIdToFullNameToVariable)
		//);


		//CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(
		//	cppGetComponentTypeIdByFullName,
		//	componentTypeIdType,
		//	CPP__TREE__CREATE_ENTITIES_VECTOR(),
		//	"GetComponentTypeIdByFullName",
		//	CPP__TREE__CREATE_ENTITIES_VECTOR(),
		//	ECS2::Entity::Id::invalid_,
		//	CPP__TREE__CREATE_ENTITIES_VECTOR(
		//		componentNameParameterEntityId,
		//		getComponentTypeIdByFullNameBody,
		//		CPP__TREE__STMT__CREATE_RETURN_STATEMENT(
		//			CPP__TREE__EXPR__CREATE_ARRAY_SUBSCRIPT(
		//				CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR("fullNameToComponentTypeId"),
		//				CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR("componentName")
		//			)
		//		)
		//	)
		//);

		//CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(
		//	hppGetComponentTypeIdByFullName,
		//	componentTypeIdType,
		//	CPP__TREE__CREATE_ENTITIES_VECTOR(),
		//	"GetComponentTypeIdByFullName",
		//	CPP__TREE__CREATE_ENTITIES_VECTOR(),
		//	ECS2::Entity::Id::invalid_,
		//	CPP__TREE__CREATE_ENTITIES_VECTOR(
		//		componentNameParameterEntityId
		//	)
		//);

		//CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(
		//	cppGetFullNameByComponentTypeId,
		//	CPP__TREE__TYPE__CREATE_NAMED_TYPE("std::string"),
		//	CPP__TREE__CREATE_ENTITIES_VECTOR(),
		//	"GetFullNameByComponentTypeId",
		//	CPP__TREE__CREATE_ENTITIES_VECTOR(),
		//	ECS2::Entity::Id::invalid_,
		//	CPP__TREE__CREATE_ENTITIES_VECTOR(
		//		componentTypeIdParameterEntityId,
		//		getFullNameByComponentTypeIdBody,
		//		CPP__TREE__STMT__CREATE_RETURN_STATEMENT(
		//			CPP__TREE__EXPR__CREATE_ARRAY_SUBSCRIPT(
		//				CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR("componentTypeIdToFullName"),
		//				CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR("componentTypeId")
		//			)
		//		)
		//	)
		//);

		//CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(
		//	hppGetFullNameByComponentTypeId,
		//	CPP__TREE__TYPE__CREATE_NAMED_TYPE("std::string"),
		//	CPP__TREE__CREATE_ENTITIES_VECTOR(),
		//	"GetFullNameByComponentTypeId",
		//	CPP__TREE__CREATE_ENTITIES_VECTOR(),
		//	ECS2::Entity::Id::invalid_,
		//	CPP__TREE__CREATE_ENTITIES_VECTOR(
		//		componentTypeIdParameterEntityId
		//	)
		//);


		/*cppFileNodeEntityIds.push_back(cppGetComponentTypeIdByFullName);
		cppFileNodeEntityIds.push_back(cppGetFullNameByComponentTypeId);

		hppFileNodeEntityIds.push_back(hppGetComponentTypeIdByFullName);
		hppFileNodeEntityIds.push_back(hppGetFullNameByComponentTypeId);*/

		//BindComponents
		{
			ECS2::Entity::Id hppBindComponentsEntityId = CreateEntity();
			ECS2::Entity::Id cppBindComponentsEntityId = CreateEntity();

			ECS2::Entity::Id luaStateParameter = CPP__TREE__DECL__CREATE_PARAMETER(this,
				CPP__TREE__TYPE__CREATE_POINTER_TYPE(this,
					CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "lua_State")
				),
				"state"
			);

			std::vector<ECS2::Entity::Id> bodyChilds;

			const auto allComponents = world_->GetComponents<ECS__FILE__TABLE__COMPONENT__COMPONENT>();
			const Common::Size allComponentsNumber = world_->GetEntitiesNumber<ECS__FILE__TABLE__COMPONENT__COMPONENT>();

			for (Common::Index componentIndex = 0; componentIndex < allComponentsNumber; componentIndex++) {
				ECS2::Entity::Id componentEntityId = *(std::get<ECS2::Entity::Id*>(allComponents) + componentIndex);
				if (IsComponentExist<ECS::File::Table::Component::Bindable>(componentEntityId)) {
					bodyChilds.push_back(
						CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
							CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
								CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
									File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, componentEntityId, "::", false) + "Bind"
								),
								CPP__TREE__CREATE_ENTITIES_VECTOR(),
								CPP__TREE__CREATE_ENTITIES_VECTOR(
									CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "state")
								)
							)
						)
					);
				}
			}


			ECS2::Entity::Id bodyEntityId = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
				CPP__TREE__CREATE_ENTITIES_VECTOR(
					bodyChilds
				)
			);

			CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(this,
				hppBindComponentsEntityId,
				CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
				CPP__TREE__CREATE_ENTITIES_VECTOR(),
				"BindComponents",
				CPP__TREE__CREATE_ENTITIES_VECTOR(),
				ECS2::Entity::Id::invalid_,
				CPP__TREE__CREATE_ENTITIES_VECTOR(
					luaStateParameter,
					bodyEntityId
				)
			);
			cppFileNodeEntityIds.push_back(hppBindComponentsEntityId);
		}
		//FOR_EACH_COMPONENTS macros
		{

			hppFileNodeEntityIds.push_back(
				CPP__TREE__COMMENT__CREATE_CLANG_FORMAT_OFF_COMMENT(this)
			);


			const std::string macrosBody = [this]() -> std::string {
				std::string text = "\\\n";

				const auto allComponents = world_->GetComponents<ECS__FILE__TABLE__COMPONENT__COMPONENT>();
				const Common::Size allComponentsNumber = world_->GetEntitiesNumber<ECS__FILE__TABLE__COMPONENT__COMPONENT>();

				for (Common::Index componentIndex = 0; componentIndex < allComponentsNumber - 1; componentIndex++) {
					ECS2::Entity::Id componentEntityId = *(std::get<ECS2::Entity::Id*>(allComponents) + componentIndex);

					text += "DO(" + File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, componentEntityId, "::", false) + ")";

					//if (componentIndex != allComponentsNumber - 1) {
					text += "\\\n";
					//}
				}

				//Process last
				ECS2::Entity::Id componentEntityId = *(std::get<ECS2::Entity::Id*>(allComponents) + allComponentsNumber - 1);
				text += "DO_LAST(" + File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, componentEntityId, "::", false) + ")";
				return text;
				}();

			hppFileNodeEntityIds.push_back(CPP__TREE__PREPROCESSOR__CREATE_DEFINE(
				"FOR_EACH_COMPONENT",
				(std::vector<std::string>{"DO", "DO_LAST"}),
				macrosBody,
				ECS2::Entity::Id::invalid_,
				true));
			hppFileNodeEntityIds.push_back(
				CPP__TREE__COMMENT__CREATE_CLANG_FORMAT_ON_COMMENT(this)
			);

		}


		CreateComponent<CPP::Tree::Node::ChildEntityIds>(hppFileEntityId, hppFileNodeEntityIds);
		CreateComponent<CPP::Tree::Node::ChildEntityIds>(cppFileEntityId, cppFileNodeEntityIds);

	}

	void CreateModuleIncludeFile::Update(

		ECS2::Entity::Id entity0id,
		const ECS::Project::Tag* eCS__Project__Tag0,
		const ECS::Project::Path* eCS__Project__Path0,
		const Behaviour::LuaScript* luaScript0,

		ECS2::Entity::Id entity1id,
		const ECS::Module::Tag* eCS__Module__Tag1,
		const ECS::Module::Path* eCS__Module__Path1,
		const ECS::Module::IncludePath* eCS__Module__IncludePath1,
		const Behaviour::LuaScript* luaScript1) {

		ECS2::Entity::Id hppFileEntityId = CreateEntity<CPP__FILE__FILE>();
		std::vector<ECS2::Entity::Id> hppFileNodeEntityIds;
		{
			CreateComponent<CPP::File::Tag>(hppFileEntityId);
			CreateComponent<CPP::File::Type::Hpp>(hppFileEntityId);
			//File PATH and NAME
			std::string filePath = std::filesystem::path{ eCS__Module__Path1->path_ }.remove_filename().string();
			CreateComponent<CPP::File::Path>(hppFileEntityId, filePath);
			std::string fileName = "auto_" + std::filesystem::path{ eCS__Module__Path1->path_ }.stem().string() + ".Module";
			CreateComponent<CPP::File::Name>(hppFileEntityId, fileName);

			hppFileNodeEntityIds.push_back(CPP__TREE__PREPROCESSOR__CREATE_PRAGMA_ONCE(this));

			const auto moduleIncludePath = std::filesystem::path{ eCS__Module__IncludePath1->path_ };

			//.ecs module path relative to project path.
			const auto modulePath = std::filesystem::path{ eCS__Module__Path1->path_ };

			const auto projectPath = std::filesystem::path{ eCS__Project__Path0->path_ };

			::Lua::Context context;
			::Lua::Script script{ luaScript1->text_ };
			context.LoadScript(script);

			luabridge::LuaRef ecsTable = luabridge::getGlobal(context.state_, "ECS");

			ASSERT_FMSG(ecsTable.isTable() || ecsTable["Files"].isTable(), "");

			luabridge::LuaRef moduleFilesTable = ecsTable["Files"];

			int filesNumber = moduleFilesTable.length();
			for (int i = 1; i <= filesNumber; ++i) {
				luabridge::LuaRef filePathTable = moduleFilesTable[i];

				//.ecs file path relative to module path.
				const auto filePath = std::filesystem::path{ filePathTable.cast<std::string>().value() };

				std::filesystem::path moduleRelativePath = modulePath.parent_path().lexically_relative(projectPath.parent_path());

				std::filesystem::path fileIncludePath = moduleRelativePath.lexically_relative(moduleIncludePath.relative_path());

				fileIncludePath = fileIncludePath / filePath.parent_path() / ("auto_" + filePath.stem().string() + ".hpp");
				fileIncludePath = fileIncludePath.lexically_normal();
				hppFileNodeEntityIds.push_back(CPP__TREE__PREPROCESSOR__CREATE_INCLUDE(this, fileIncludePath.string()));
			}
		}
		CreateComponent<CPP::Tree::Node::ChildEntityIds>(hppFileEntityId, hppFileNodeEntityIds);
	}


	void CreateProjectIncludeFile::Update(
		ECS2::Entity::Id entity0id,
		const ECS::Project::Tag* eCS__Project__Tag0,
		const ECS::Project::Path* eCS__Project__Path0,
		const Behaviour::LuaScript* luaScript0) {

		ECS2::Entity::Id hppProjectFileEntityId = CreateEntity<CPP__FILE__FILE>();
		{
			CreateComponent<CPP::File::Tag>(hppProjectFileEntityId);
			CreateComponent<CPP::File::Type::Hpp>(hppProjectFileEntityId);

			std::string filePath = std::filesystem::path{ eCS__Project__Path0->path_ }.remove_filename().string();
			CreateComponent<CPP::File::Path>(hppProjectFileEntityId, filePath);

			std::string fileName = GET_PROJECT_INCLUDE_FILE_NAME(this, entity0id);
			CreateComponent<CPP::File::Name>(hppProjectFileEntityId, fileName);

			std::vector<ECS2::Entity::Id> hppFileNodeEntityIds;
			hppFileNodeEntityIds.push_back(CPP__TREE__PREPROCESSOR__CREATE_PRAGMA_ONCE(this));

			std::vector<ECS2::Entity::Id> includeEntityIds = [this, entity0id]() {

				std::vector<ECS2::Entity::Id> includeEntityIds;

				const auto allModules = world_->GetComponents<ECS__MODULE__MODULE>();
				const Common::Size allModulesNumber = world_->GetEntitiesNumber<ECS__MODULE__MODULE>();

				for (int i = 0; i < allModulesNumber; ++i) {

					ECS2::Entity::Id moduleEntityId = *(std::get<ECS2::Entity::Id*>(allModules) + i);

					const std::filesystem::path includePath = GET_MODULE_INCLUDE_FILE_INCLUDE_PATH(this, entity0id, moduleEntityId);
					const std::filesystem::path fileName = GET_MODULE_INCLUDE_FILE_NAME(this, moduleEntityId) + ".hpp";

					includeEntityIds.push_back(
						CPP__TREE__PREPROCESSOR__CREATE_INCLUDE(this, (includePath / fileName).string())
					);
				}
				return includeEntityIds;
				}();

			hppFileNodeEntityIds.insert(hppFileNodeEntityIds.end(), includeEntityIds.begin(), includeEntityIds.end());

			CreateComponent<CPP::Tree::Node::ChildEntityIds>(hppProjectFileEntityId, hppFileNodeEntityIds);
		}
	}


	void GenerateCMakeFile::Update(
		ECS2::Entity::Id entity0id,
		const ECS::Project::Tag* eCS__Project__Tag0,
		const ECS::Project::Path* eCS__Project__Path0,
		const Behaviour::LuaScript* luaScript0) {

		using namespace std::string_literals;
		std::filesystem::path cmakeFilePath = eCS__Project__Path0->path_;
		const std::string projectName = cmakeFilePath.filename().stem().string();
		cmakeFilePath.remove_filename();


		//Generate Include directories file.
		const auto includeDirectoriesCMakeFileName = "auto_IncludeDirectories.cmake"s;
		{
			auto file = std::make_shared<OS::TextFile>(cmakeFilePath / (includeDirectoriesCMakeFileName));
			file->Create();
			std::string text;
			{
				std::vector<std::string> allIncludeDirectories;

				auto modules = world_->GetComponents<ECS__MODULE__MODULE>();
				const Common::Size allModulesNumber = world_->GetEntitiesNumber<ECS__MODULE__MODULE>();
				const auto* modulePaths = std::get<ECS::Module::Path*>(modules);
				const auto* entityIds = std::get<ECS2::Entity::Id*>(modules);
				const auto* luaScripts = std::get<Behaviour::LuaScript*>(modules);
				for (Common::Index i = 0; i < allModulesNumber; i++) {

					const std::string luaScriptText = (luaScripts + i)->text_;

					::Lua::Script script{ luaScriptText };

					::Lua::Context context;
					context.LoadScript(script);
					luabridge::LuaRef ecs = luabridge::getGlobal(context.state_, "ECS");
					luabridge::LuaRef includeDirectories = ecs["IncludeDirectories"];
					if (!includeDirectories.isNil()) {
						const std::filesystem::path luaScriptText = (modulePaths + i)->path_;

						//std::string moduleFilePathStr = .string();
						//std::replace(moduleFilePathStr.begin(), moduleFilePathStr.end(), '\\', '/');
						//text += "\t\"" + moduleFilePathStr + "\"\n";
						//text += ")\n";
						std::filesystem::path moduleIncludePath = (modulePaths + i)->path_;
						if (!moduleIncludePath.is_absolute()) {
							moduleIncludePath = cmakeFilePath / moduleIncludePath;
							moduleIncludePath = moduleIncludePath.lexically_normal();
						}

						const int n = includeDirectories.length();
						for (int i = 1; i <= n; ++i) {     // Lua-индексы с 1
							luabridge::LuaRef elem = includeDirectories[i];
							ASSERT(elem.isString());

							std::filesystem::path fullIncludePath = (moduleIncludePath.parent_path() / elem.tostring()).lexically_normal();

							std::string s = fullIncludePath.lexically_normal().generic_string();  // всегда '/'
							std::replace(s.begin(), s.end(), '\\', '/');
							allIncludeDirectories.push_back(s);
						}

					}

				}

				text += "target_include_directories(";
				text += projectName + "\n";
				text += "PUBLIC\n";
				for (const std::string& include : allIncludeDirectories) {

					text += "\"" + include + "\"\n";
				}
				text += ")";
			}
			*file << text;
		}

		//Generate help file.
		const auto helpCMakeFileName = "auto_Help.cmake"s;
		{
			auto file = std::make_shared<OS::TextFile>(cmakeFilePath / (helpCMakeFileName));
			file->Create();
			std::string text;
			{
				text += Generated::cmakeHelpFile;
			}
			*file << text;
		}

		//Generate cmake to find modules sources.
		{
			auto modules = world_->GetComponents<ECS__MODULE__MODULE>();
			const Common::Size allModulesNumber = world_->GetEntitiesNumber<ECS__MODULE__MODULE>();


			const auto* entityIds = std::get<ECS2::Entity::Id*>(modules);
			const auto* modulePaths = std::get<ECS::Module::Path*>(modules);
			const auto* moduleIncludePaths = std::get<ECS::Module::IncludePath*>(modules);

			auto file = std::make_shared<OS::TextFile>(cmakeFilePath / "auto_Modules.cmake");
			file->Create();

			//Generate cmake text.
			std::string text;
			{
				text += "include(" + helpCMakeFileName + ")\n";
				text += "macro(GetModuleSources SOURCES)\n";

				text += " set(ALL_SOURCES \"\")\n";

				for (Common::Index i = 0; i < allModulesNumber; i++) {
					text += "get_source_files(";
					std::filesystem::path moduleFilePath = (modulePaths + i)->path_;

					std::string sourcesVariable = [&]() {
						std::string result = moduleFilePath.filename().stem().string();
						std::transform(result.begin(), result.end(), result.begin(),
							[](unsigned char c) { return std::toupper(c); });
						return result;
						}() + "_SOURCES";

					text += sourcesVariable + "\n";
					std::filesystem::path moduleIncludePath = (moduleIncludePaths + i)->path_;
					if (!moduleIncludePath.is_absolute()) {
						moduleIncludePath = cmakeFilePath / moduleIncludePath;
						moduleIncludePath = moduleIncludePath.lexically_normal();
					}


					std::string moduleFilePathStr = moduleFilePath.parent_path().string();
					std::replace(moduleFilePathStr.begin(), moduleFilePathStr.end(), '\\', '/');
					text += "\t\"" + moduleFilePathStr + "\"\n";
					text += ")\n";

					text += "organize_source_files(\n";
					std::string includePath = [](std::filesystem::path includePath, std::filesystem::path modulePath) {
						// Разбиваем пути на компоненты
						std::vector<std::filesystem::path> marker_parts;
						std::vector<std::filesystem::path> path_parts;

						for (const auto& comp : includePath) {
							if (!comp.empty() && comp != "/") {
								marker_parts.push_back(comp);
							}
						}

						for (const auto& comp : modulePath) {
							if (!comp.empty() && comp != "/") {
								path_parts.push_back(comp);
							}
						}

						// Если маркер пустой или путь короче маркера
						if (marker_parts.empty() || path_parts.size() < marker_parts.size()) {
							ASSERT_FAIL();
						}

						// Ищем последовательность компонентов маркера в полном пути
						for (size_t i = 0; i <= path_parts.size() - marker_parts.size(); ++i) {
							bool found = true;

							// Проверяем все компоненты маркера
							for (size_t j = 0; j < marker_parts.size(); ++j) {
								if (path_parts[i + j] != marker_parts[j]) {
									found = false;
									break;
								}
							}

							if (found) {
								// Строим результат: путь до маркера + сам маркер
								std::filesystem::path result;
								for (size_t j = 0; j < i + marker_parts.size(); ++j) {
									std::string comp_str = path_parts[j].string();

									// Проверяем, является ли компонент буквой диска (например, "D:")
									if (comp_str.length() == 2 && comp_str[1] == ':') {
										// Добавляем как есть, filesystem сам добавит разделитель
										result = std::filesystem::path(comp_str);
									}
									else {
										result /= path_parts[j];
									}
								}
								return result;
							}
						}

						// Маркер не найден
						ASSERT_FAIL();
						}(moduleIncludePath, moduleFilePath).string();
					//HACK: to escape lost /  after D:
					includePath.insert(includePath.begin() + 2, '/');
					//HACK
					std::replace(includePath.begin(), includePath.end(), '\\', '/');
					text += "\"" + includePath + "\"" + "\n";
					text += "${" + sourcesVariable + "}\n";
					text += ")\n";

					text += "list(APPEND ALL_SOURCES ${" + sourcesVariable + "})\n";
					text += "target_include_directories(" + projectName + "  PUBLIC \"" + includePath + "\")\n";
				}

				text += "set(${SOURCES} ${ALL_SOURCES})\n";


				text += "endmacro()\n";

			}

			*file << text;

		}


		//Generate Packages cmake  file.
		const auto packagesCMakeFileName = "auto_Packages.cmake"s;
		{
			auto file = std::make_shared<OS::TextFile>(cmakeFilePath / (packagesCMakeFileName));
			file->Create();

			//Generate cmake text.
			std::string text;
			{
				text += "include(" + helpCMakeFileName + ")\n";

				text += "get_source_files(\"${CMAKE_CURRENT_LIST_DIR} / Packages\" PACKAGES_SOURCES)\n";
				text += "organize_source_files(${CMAKE_CURRENT_LIST_DIR} \"${PACKAGES_SOURCES}\")\n";
			}

			*file << text;
		}

		//Generate CMakeLists.txt
		if (!std::filesystem::exists(cmakeFilePath / "CMakeLists.txt"))
		{
			auto file = std::make_shared<OS::TextFile>(cmakeFilePath / "CMakeLists.txt");
			file->Create();

			//Generate cmake text.
			std::string text;
			{
				text += "cmake_minimum_required(VERSION  3.26.4)\n";
				text += "include(" + packagesCMakeFileName + ")\n";
				text += "get_source_files(\"${CMAKE_CURRENT_LIST_DIR}/Sources\" SOURCES)\n";
				text += "organize_source_files(${CMAKE_CURRENT_LIST_DIR} \"${SOURCES}\")\n";
				text += "add_executable(" + projectName + ")\n";

				//Compile options.
				text += "target_compile_options(" + projectName + " PUBLIC \"/MP\" \"/bigobj\")\n";

				//Include directories.
				text += "target_include_directories(" + projectName + " PUBLIC ";
				if (std::filesystem::exists(cmakeFilePath / "Packages")) {
					text += " ${CMAKE_CURRENT_LIST_DIR}/Packages/";
				}
				text += "${CMAKE_CURRENT_LIST_DIR}/Sources/)\n";

				//Project sources.
				text += "target_sources(" + projectName +
					" PUBLIC "
					"${SOURCES}"
					"${PACKAGES_SOURCES})\n";

			}

			*file << text;
		}


	}

	void GenerateProjectCPPTemplateFiles::Update(
		ECS2::Entity::Id entity0id,
		const ECS::Project::Tag* eCS__Project__Tag0,
		const ECS::Project::Path* eCS__Project__Path0,
		const Behaviour::LuaScript* luaScript0) {

		using namespace std::string_literals;
		std::filesystem::path projectFilePath = eCS__Project__Path0->path_;
		const std::string projectName = projectFilePath.filename().stem().string();
		projectFilePath.remove_filename();

		//Generate project template  files.
		{
			if (!std::filesystem::exists(projectFilePath / (projectName + ".hpp"))) {
				auto fileHpp = std::make_shared<OS::TextFile>(projectFilePath / (projectName + ".hpp"));
				fileHpp->Create();
				std::string text;
				{
					text += Generated::projectHppFile;
				}
				*fileHpp << text;
			}
			if (!std::filesystem::exists(projectFilePath / (projectName + ".cpp"))) {
				auto fileCpp = std::make_shared<OS::TextFile>(projectFilePath / (projectName + ".cpp"));
				fileCpp->Create();
				std::string text;
				{
					text += Generated::projectCppFile;
				}
				*fileCpp << text;
			}
		}

	}

	void GenerateRunSystemsFile::Update(
		ECS2::Entity::Id entity0id,
		const ECS::Project::Tag* eCS__Project__Tag0,
		const ECS::Project::Path* eCS__Project__Path0,
		const Behaviour::LuaScript* luaScript0) {

		//auto_{project_name}.RunSystems.hpp
		ECS2::Entity::Id hppRunSystemsEntityId = CreateEntity<CPP__FILE__FILE>();
		{
			CreateComponent<CPP::File::Tag>(hppRunSystemsEntityId);
			CreateComponent<CPP::File::Type::Hpp>(hppRunSystemsEntityId);

			std::string filePath = std::filesystem::path{ eCS__Project__Path0->path_ }.remove_filename().string();
			CreateComponent<CPP::File::Path>(hppRunSystemsEntityId, filePath);

			std::string fileName = "auto_" +
				std::filesystem::path{ eCS__Project__Path0->path_ }.stem().string() + ".RunSystems";
			CreateComponent<CPP::File::Name>(hppRunSystemsEntityId, fileName);

			std::vector<ECS2::Entity::Id> hppFileNodeEntityIds;
			hppFileNodeEntityIds.push_back(CPP__TREE__PREPROCESSOR__CREATE_PRAGMA_ONCE(this));
			hppFileNodeEntityIds.push_back(CPP__TREE__PREPROCESSOR__CREATE_INCLUDE(this, "Common/ECS2/ECS2.hpp"));
			std::vector<ECS2::Entity::Id> childs = [this, entity0id]() {

				std::vector<ECS2::Entity::Id> prototypeEntityIds;

				prototypeEntityIds.push_back(CPP__TREE__DECL__CREATE_FUNCTION(this,
					CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
					"RunInitializeSystems", ECS2::Entity::Id::invalid_,
					CPP__TREE__CREATE_ENTITIES_VECTOR(
						ECS__GENERATOR__CREATE_WORLD_PARAMETER(this)
					)));

				prototypeEntityIds.push_back(CPP__TREE__DECL__CREATE_FUNCTION(this,
					CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
					"RunSystems", ECS2::Entity::Id::invalid_,
					CPP__TREE__CREATE_ENTITIES_VECTOR(
						ECS__GENERATOR__CREATE_WORLD_PARAMETER(this)
					)));

				return prototypeEntityIds;
				}();

			hppFileNodeEntityIds.insert(hppFileNodeEntityIds.end(), childs.begin(), childs.end());

			CreateComponent<CPP::Tree::Node::ChildEntityIds>(hppRunSystemsEntityId, hppFileNodeEntityIds);
		}

		//auto_{project_name}.RunSystems.cpp
		ECS2::Entity::Id cppRunSystemsEntityId = CreateEntity<CPP__FILE__FILE>();
		{
			CreateComponent<CPP::File::Tag>(cppRunSystemsEntityId);
			CreateComponent<CPP::File::Type::Cpp>(cppRunSystemsEntityId);

			std::string filePath = std::filesystem::path{ eCS__Project__Path0->path_ }.remove_filename().string();
			CreateComponent<CPP::File::Path>(cppRunSystemsEntityId, filePath);

			std::string fileName = "auto_" + \
				std::filesystem::path{ eCS__Project__Path0->path_ }.stem().string() + ".RunSystems"; \
				CreateComponent<CPP::File::Name>(cppRunSystemsEntityId, fileName);

			std::vector<ECS2::Entity::Id> cppFileNodeEntityIds;
			cppFileNodeEntityIds.push_back(CPP__TREE__PREPROCESSOR__CREATE_PRAGMA_ONCE(this));
			cppFileNodeEntityIds.push_back(
				CPP__TREE__PREPROCESSOR__CREATE_INCLUDE(
					this,
					GET_PROJECT_INCLUDE_FILE_INCLUDE_PATH(this, entity0id).string() + ".hpp"
				)
			);
			cppFileNodeEntityIds.push_back(
				CPP__TREE__PREPROCESSOR__CREATE_INCLUDE(this, "taskflow/taskflow.hpp")
			);

			std::vector<ECS2::Entity::Id> childs = [this, entity0id]() {


				std::vector<ECS2::Entity::Id> childs;

				std::vector<ECS2::Entity::Id> synchronizationEntityIds; // Condition variable, mutexes, atomic_flags
				std::vector<ECS2::Entity::Id> realizationEntityIds; // RunSystems function

				realizationEntityIds.push_back(
					[this]() {
						BEGIN_PROFILE("Generate RunInitializeSystems function realization.");
						std::vector<ECS2::Entity::Id> initSystemsOrder = [this]() {

							std::vector<ECS2::Entity::Id> initSystemsOrder;

							std::unordered_map<ECS2::Entity::Id, Common::Size, ECS2::Entity::Id::Hash> inDegree;
							std::queue<ECS2::Entity::Id> zeroInDegreeNodes;

							const auto systemComponents = world_->GetComponents<ECS__FILE__TABLE__SYSTEM__SYSTEM>();
							const Common::Size allSystemsNumber = world_->GetEntitiesNumber<ECS__FILE__TABLE__SYSTEM__SYSTEM>();

							for (Common::Index i = 0; i < allSystemsNumber; i++) {

								const auto systemType = *(std::get<ECS::File::Table::System::Type*>(systemComponents) + i);
								if (systemType.type_ == "Initialize") {

									ECS2::Entity::Id systemEntityId = *(std::get<ECS2::Entity::Id*>(systemComponents) + i);

									std::string fullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, systemEntityId, "::", false);

									if (fullName == "PacketManager::GetPacketsToDownload") {
										Common::BreakPointLine();
									}

									ECS2::Entity::Id callOrderEntityId = (std::get<ECS::File::Table::System::CallOrder::EntityId*>(systemComponents) + i)->id_;
									if (callOrderEntityId.IsValid()) {
										if (IsComponentExist<ECS::File::Table::System::CallOrder::RunAfter>(callOrderEntityId)) {
											ECS::File::Table::System::CallOrder::RunAfter* systemRunAfter = GetComponent<ECS::File::Table::System::CallOrder::RunAfter>(callOrderEntityId);
											inDegree[systemEntityId] = systemRunAfter->systems_.size();
											if (systemRunAfter->systems_.size() == 0) {
												zeroInDegreeNodes.push(systemEntityId);
											}
										}
										else {
											zeroInDegreeNodes.push(systemEntityId);
										}

									}
									else {
										zeroInDegreeNodes.push(systemEntityId);
									}
								}
							}

							// 2. Алгоритм Кана
							while (!zeroInDegreeNodes.empty()) {
								ECS2::Entity::Id currentId = zeroInDegreeNodes.front();
								zeroInDegreeNodes.pop();
								initSystemsOrder.push_back(currentId);
								//
									// Уменьшаем inDegree для всех соседей
								std::string fullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, currentId, "::", false);
								if (IsComponentExist<ECS::File::Table::System::CallOrder::EntityId>(currentId)) {
									ECS2::Entity::Id callOrderEntityId = GetComponent<ECS::File::Table::System::CallOrder::EntityId>(currentId)->id_;
									if (IsComponentExist<ECS::File::Table::System::CallOrder::RunBefore>(callOrderEntityId)) {
										const auto* runBefore = GetComponent<ECS::File::Table::System::CallOrder::RunBefore>(callOrderEntityId);
										for (const auto& runBeforeSystemInfo : runBefore->systems_) {
											const ECS2::Entity::Id runBeforeSystemEntityId
												= File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<GenerateRunSystemsFile, ECS::File::Table::System::Tag>(this, currentId, runBeforeSystemInfo.name_);
											auto it = inDegree.find(runBeforeSystemEntityId);
											if (it != inDegree.end() && it->second > 0) {
												it->second--;
												if (it->second == 0) {
													zeroInDegreeNodes.push(runBeforeSystemEntityId);
												}
											}
										}
									}

								}
							}
							return initSystemsOrder;

							}();

						std::vector<ECS2::Entity::Id> systemUpdateCalls;
						for (auto initSystemEntityId : initSystemsOrder) {
							const std::string systemFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, initSystemEntityId, "::", false);
							systemUpdateCalls.push_back(
								CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
									CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
										CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, systemFullName + "System"),
										CPP__TREE__CREATE_ENTITIES_VECTOR(),
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world")
										)
									)
								)
							);
							// world2->ApplyDelayedRequests();
							systemUpdateCalls.push_back(
								CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
									CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
										CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world"),
											"ApplyDelayedRequests",
											true
										),
										CPP__TREE__CREATE_ENTITIES_VECTOR(),
										CPP__TREE__CREATE_ENTITIES_VECTOR()
									)
								)
							);
						}

						ECS2::Entity::Id body = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this, systemUpdateCalls);
						return CPP__TREE__DECL__CREATE_FUNCTION(this,
							CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
							"RunInitializeSystems", ECS2::Entity::Id::invalid_,
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								ECS__GENERATOR__CREATE_WORLD_PARAMETER(this),
								body));
						END_PROFILE();

					}()
						);

				realizationEntityIds.push_back(
					[this, &synchronizationEntityIds]() {

						auto [allSystems, orderedSystemIdToIndex, mainThreadSystems, childThreadSystems] = [this]() {

							const auto systemComponents = world_->GetComponents<ECS__FILE__TABLE__SYSTEM__SYSTEM>();
							const Common::Size allSystemsNumber = world_->GetEntitiesNumber<ECS__FILE__TABLE__SYSTEM__SYSTEM>();
							for (Common::Index i = 0; i < allSystemsNumber; i++) {
								ECS2::Entity::Id systemEntityId = *(std::get<ECS2::Entity::Id*>(systemComponents) + i);
								if (IsComponentExist<ECS::File::Table::System::Type>(systemEntityId)) {
									const auto systemType = *(std::get<ECS::File::Table::System::Type*>(systemComponents) + i);
									if (systemType.type_ == "Initialize") {
										continue;
									}
								}
							}

							std::vector<ECS2::Entity::Id> systemsOrder;
							std::unordered_map<ECS2::Entity::Id, Common::Index, ECS2::Entity::Id::Hash> orderedSystemIdToIndex;
							std::unordered_map<ECS2::Entity::Id, Common::Size, ECS2::Entity::Id::Hash> inDegree;
							std::queue<ECS2::Entity::Id> zeroInDegreeNodes;

							for (Common::Index i = 0; i < allSystemsNumber; i++) {

								ECS2::Entity::Id systemEntityId = *(std::get<ECS2::Entity::Id*>(systemComponents) + i);
								const std::string systemName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, systemEntityId, "::", false);
								if (IsComponentExist<ECS::File::Table::System::Type>(systemEntityId)) {
									const auto systemType = *(std::get<ECS::File::Table::System::Type*>(systemComponents) + i);
									if (systemType.type_ == "Initialize") {
										continue;
									}
								}
								const auto systemType = *(std::get<ECS::File::Table::System::Type*>(systemComponents) + i);

								ECS2::Entity::Id callOrderEntityId = (std::get<ECS::File::Table::System::CallOrder::EntityId*>(systemComponents) + i)->id_;
								if (callOrderEntityId.IsValid()) {
									if (IsComponentExist<ECS::File::Table::System::CallOrder::RunAfter>(callOrderEntityId)) {
										ECS::File::Table::System::CallOrder::RunAfter* systemRunAfter = GetComponent<ECS::File::Table::System::CallOrder::RunAfter>(callOrderEntityId);
										inDegree[systemEntityId] = systemRunAfter->systems_.size();
										if (systemRunAfter->systems_.size() == 0) {
											zeroInDegreeNodes.push(systemEntityId);
										}
									}
									else {
										zeroInDegreeNodes.push(systemEntityId);
									}

								}
								else {
									zeroInDegreeNodes.push(systemEntityId);
								}

							}

							// 2. Алгоритм Кана
							while (!zeroInDegreeNodes.empty()) {
								ECS2::Entity::Id currentId = zeroInDegreeNodes.front();
								const std::string systemName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, currentId, "::", false);
								zeroInDegreeNodes.pop();
								systemsOrder.push_back(currentId);
								orderedSystemIdToIndex[currentId] = systemsOrder.size() - 1;

								//
									// Уменьшаем inDegree для всех соседей


								ECS2::Entity::Id callOrderEntityId = GetComponent<ECS::File::Table::System::CallOrder::EntityId>(currentId)->id_;

								const auto* runBefore = GetComponent<ECS::File::Table::System::CallOrder::RunBefore>(callOrderEntityId);
								for (const auto& runBeforeSystemInfo : runBefore->systems_) {


									if (runBeforeSystemInfo.id_.invalid_) {
										LOG_INFO("generator/systems", "Skiped UNKNOWN system {} in context {}", runBeforeSystemInfo.name_, systemName);
										const ECS2::Entity::Id runBeforeSystemEntityId
											= File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME_OR_INVALID_ID<GenerateRunSystemsFile, ECS::File::Table::System::Tag>(this, currentId, runBeforeSystemInfo.name_);
										//ASSERT_FMSG(runBeforeSystemEntityId.IsValid(), "Attempt to skip system that is not disabled. Check system validation system");
										continue;
									}

									auto it = inDegree.find(runBeforeSystemInfo.id_);
									if (it != inDegree.end() && it->second > 0) {
										it->second--;
										if (it->second == 0) {
											zeroInDegreeNodes.push(runBeforeSystemInfo.id_);
										}
									}
								}



							}

							std::vector<ECS2::Entity::Id> orderedMainThreadSystems;
							std::vector<ECS2::Entity::Id> orderedChildThreadSystems;

							for (ECS2::Entity::Id systemEntityId : systemsOrder) {
								const std::string systemName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, systemEntityId, "::", false);
								if (IsComponentExist<ECS::File::Table::System::Thread>(systemEntityId)) {
									const std::string thread = GetComponent<ECS::File::Table::System::Thread>(systemEntityId)->thread_;
									if (thread == "Main") {
										orderedMainThreadSystems.push_back(systemEntityId);
									}
									else {
										orderedChildThreadSystems.push_back(systemEntityId);
									}
								}
							}



							return std::tuple{ systemsOrder, orderedSystemIdToIndex, orderedMainThreadSystems,orderedChildThreadSystems };

							}();
						std::vector<ECS2::Entity::Id> runSystemsRealization;

						//Set bool variables to false.
						[&runSystemsRealization, this, &allSystems]() {
							for (auto systemEntityId : allSystems) {
								const std::string systemFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, systemEntityId, "::", false);
								//static std::mutex OksEngine__Test2__TestMain1FinishedMutex;
								if (ECS__FILE__TABLE__SYSTEM__HAS_RUN_BEFORE_SYSTEMS(systemEntityId)) {
									runSystemsRealization.push_back(
										CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
											CPP__TREE__EXPR__CREATE_BINARY_OPERATION(this,
												CPP::Tree::Expr::BinaryOpType::Assign,
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
													File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, systemEntityId, "__", false) + "FinishedFlag"
												),
												CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "false", "")
											)
										)
									);

								}

							}
							}();

						runSystemsRealization.push_back(
							CPP__TREE__COMMENT__CREATE_ONE_LINE_COMMENT(this, "Run main thread systems.")
						);
						// tf::Executor executor;
						runSystemsRealization.push_back(
							CPP__TREE__DECL__CREATE_VARIABLE(this,
								CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "tf::Executor"),
								"executor",
								ECS2::Entity::Id::invalid_,
								ECS2::Entity::Id::invalid_
							)
						);
						//tf::Taskflow taskflow("Demo");
						runSystemsRealization.push_back(
							CPP__TREE__DECL__CREATE_VARIABLE(this,
								CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "tf::Taskflow"),
								"taskFlow",
								CPP__TREE__EXPR__CREATE_INIT_LIST(this,
									CPP__TREE__CREATE_ENTITIES_VECTOR(
										CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"Frame to frame child thread systems\"", "")
									)
								),
								ECS2::Entity::Id::invalid_
							)
						);

						//Generate system Mutexs, atomic_bools, CVs.
						[this, &allSystems, &synchronizationEntityIds]() {

							for (auto systemEntityId : allSystems) {
								ECS2::Entity::Id callOrderEntityId = GetComponent<ECS::File::Table::System::CallOrder::EntityId>(systemEntityId)->id_;
								const auto& beforeSystemInfos = GetComponent<ECS::File::Table::System::CallOrder::RunBefore>(callOrderEntityId)->systems_;
								//Add sync primitives if system has Systems Before in call order.
								if (!beforeSystemInfos.empty()) {
									//static std::mutex OksEngine__Test2__TestChild2FinishedMutex;
									synchronizationEntityIds.push_back(
										CPP__TREE__DECL__CREATE_STATIC_VARIABLE(this,
											CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::mutex"),
											File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, systemEntityId, "__", false) + "FinishedMutex",
											ECS2::Entity::Id::invalid_,
											ECS2::Entity::Id::invalid_
										)
									);
									//static std::atomic_bool OksEngine__Test2__TestMain1FinishedFlag = false;
									synchronizationEntityIds.push_back(
										CPP__TREE__DECL__CREATE_STATIC_VARIABLE(this,
											CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::atomic_bool"),
											File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, systemEntityId, "__", false) + "FinishedFlag",
											CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "false", ""),
											ECS2::Entity::Id::invalid_
										)
									);
									//static std::condition_variable OksEngine__Test2__TestMain1FinishedCV;
									synchronizationEntityIds.push_back(
										CPP__TREE__DECL__CREATE_STATIC_VARIABLE(this,
											CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::condition_variable"),
											File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, systemEntityId, "__", false) + "FinishedCV",
											ECS2::Entity::Id::invalid_,
											ECS2::Entity::Id::invalid_
										)
									);
								}
							}
							}();

						BEGIN_PROFILE("Generate systems creating task and set name code.");
						for (auto childSystemEntityId : childThreadSystems) {

							const std::string systemFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, childSystemEntityId, "::", false);
							BEGIN_PROFILE("Generate %s system creating task and set name code.", systemFullName.c_str());
							std::vector<ECS2::Entity::Id> taskLambdaBodyChilds;


							ECS2::Entity::Id callOrderEntityId = GetComponent<ECS::File::Table::System::CallOrder::EntityId>(childSystemEntityId)->id_;
							auto& afterSystemInfos = GetComponent<ECS::File::Table::System::CallOrder::RunAfter>(callOrderEntityId)->systems_;
							std::sort(afterSystemInfos.begin(), afterSystemInfos.end(),
								[&orderedSystemIdToIndex](const auto& first, const auto& second) {
									return orderedSystemIdToIndex[first.id_] < orderedSystemIdToIndex[second.id_];
								});

							for (auto& afterSystemInfo : afterSystemInfos) {

								if (GetComponent<ECS::File::Table::System::Thread>(afterSystemInfo.id_)->thread_ == "Child") {
									continue;
								}

								ECS2::Entity::Id afterSystemEntityId = File::Table::ECS__FILE__TABLE__GET_TABLE_ENTITY_ID_BY_NAME<GenerateRunSystemsFile, ECS::File::Table::System::Tag>(
									this,
									childSystemEntityId,
									afterSystemInfo.name_
								);

								// if (GetComponent<ECS::File::Table::System::Thread>(afterSystemEntityId)->thread_ == "Main") {
								// Need to wait main thread system.
								taskLambdaBodyChilds.push_back(
									CPP__TREE__DECL__CREATE_VARIABLE(this,
										CPP__TREE__TYPE__CREATE_TEMPLATE_SPECIALIZATION_TYPE(this,
											CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::unique_lock"),
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::mutex")
											)
										),
										"lock" + File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, afterSystemEntityId, "__", false),
										CPP__TREE__EXPR__CREATE_INIT_LIST(this,
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
													File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, afterSystemEntityId, "__", false) + "FinishedMutex"
												)
											)
										),
										ECS2::Entity::Id::invalid_
									)
								);
								taskLambdaBodyChilds.push_back(
									CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
										CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
											CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
													File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, afterSystemEntityId, "__", false) + "FinishedCV"
												),
												"wait",
												false
											),
											CPP__TREE__CREATE_ENTITIES_VECTOR(),
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "lock" + File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, afterSystemEntityId, "__", false)),
												CPP__TREE__EXPR__CREATE_LAMBDA(this,
													CPP::Tree::Expr::CaptureType::ByReference,
													std::vector<CPP::Tree::Expr::LambdaCapture>{},
													CPP__TREE__CREATE_ENTITIES_VECTOR(),
													CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
														CPP__TREE__CREATE_ENTITIES_VECTOR(
															CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
																CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
																	CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
																		CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
																			File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, afterSystemEntityId, "__", false) + "FinishedFlag"
																		),
																		"load",
																		false
																	),
																	CPP__TREE__CREATE_ENTITIES_VECTOR(),
																	CPP__TREE__CREATE_ENTITIES_VECTOR()
																)
															)
														)
													)
												)
											)
										)
									)

								);
								//}
							}


							//BEGIN_PROFILE("Test2::TestChild2");
							taskLambdaBodyChilds.push_back(
								CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
									CPP__TREE__EXPR__CREATE_MACRO_INVOCATION(this,
										"BEGIN_PROFILE",
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"" + systemFullName + "\"", "")
										),
										ECS2::Entity::Id::invalid_
									)
								)
							);


							//If there is system that will wait this system, need to release mutex.
							if (ECS__FILE__TABLE__SYSTEM__HAS_RUN_BEFORE_SYSTEMS(childSystemEntityId)) {
								taskLambdaBodyChilds.push_back(
									CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											//OksEngine::Test2::TestChild2System(world); or comment that system disabled
											[&]() -> ECS2::Entity::Id {

												if (ECS__FILE__TABLE__SYSTEM__IS_DISABLED(childSystemEntityId)) {
													return CPP__TREE__COMMENT__CREATE_ONE_LINE_COMMENT(this, "System " + systemFullName + "disabled.");
												}

												return CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,

													CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
														CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, systemFullName + "System"),
														CPP__TREE__CREATE_ENTITIES_VECTOR(),
														CPP__TREE__CREATE_ENTITIES_VECTOR(
															CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world")
														)
													)

												);
											}(),
												CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
													CPP__TREE__CREATE_ENTITIES_VECTOR(
														//std::unique_lock<std::mutex> lockOksEngine__Test2__TestMain1{OksEngine__Test2__TestMain1FinishedMutex};
														CPP__TREE__DECL__CREATE_VARIABLE(this,
															CPP__TREE__TYPE__CREATE_TEMPLATE_SPECIALIZATION_TYPE(this,
																CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::unique_lock"),
																CPP__TREE__CREATE_ENTITIES_VECTOR(
																	CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::mutex")
																)
															),
															"lock",
															CPP__TREE__EXPR__CREATE_INIT_LIST(this,
																CPP__TREE__CREATE_ENTITIES_VECTOR(
																	CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
																		File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, childSystemEntityId, "__", false) + "FinishedMutex"
																	)
																)
															),
															ECS2::Entity::Id::invalid_
														),

														//OksEngine__Test2__TestChild1FinishedFlag = true;
														CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
															CPP__TREE__EXPR__CREATE_BINARY_OPERATION(this,
																CPP::Tree::Expr::BinaryOpType::Assign,
																CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
																	File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, childSystemEntityId, "__", false) + "FinishedFlag"
																),
																CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "true", "")
															)
														)
													)
												),
												CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
													CPP__TREE__EXPR__CREATE_CALL_EXPR_NO_ARGS(this,
														CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
															CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
																File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, childSystemEntityId, "__", false) + "FinishedCV"
															),
															"notify_all",
															false
														)
													)
												)
												)
									)

								);
							}

							//END_PROFILE();
							taskLambdaBodyChilds.push_back(
								CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
									CPP__TREE__EXPR__CREATE_MACRO_INVOCATION(this,
										"END_PROFILE",
										CPP__TREE__CREATE_ENTITIES_VECTOR(),
										ECS2::Entity::Id::invalid_
									)
								)
							);


							ECS2::Entity::Id taskBody = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
								taskLambdaBodyChilds
							);

							//auto OksEngine__Test2__TestChild1Task = taskFlow.emplace([&]() {
							//	std::unique_lock<std::mutex> lock{ OksEngine__Test2__TestMain1FinishedMutex };
							//	OksEngine__Test2__TestMain1FinishedCV.wait(lock,
							//		[&]() { return OksEngine__Test2__TestMain1FinishedFlag.load(); });
							//	Test2::TestChild1System(world);
							//});
							runSystemsRealization.push_back(
								CPP__TREE__DECL__CREATE_VARIABLE(this,
									CPP__TREE__TYPE__CREATE_AUTO_TYPE(this),
									File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, childSystemEntityId, "__", false) + "Task",
									CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
										CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "taskFlow"),
											"emplace",
											false
										),
										CPP__TREE__CREATE_ENTITIES_VECTOR(),
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											CPP__TREE__EXPR__CREATE_LAMBDA(this,
												CPP::Tree::Expr::CaptureType::ByReference,
												std::vector<CPP::Tree::Expr::LambdaCapture>{},
												CPP__TREE__CREATE_ENTITIES_VECTOR(),
												taskBody
											)
										)
									),
									ECS2::Entity::Id::invalid_
								)
							);

							//OksEngine__Test2__TestChild1Task.name("Test2::TestChild1");
							runSystemsRealization.push_back(
								CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
									CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
										CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
												File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, childSystemEntityId, "__", false) + "Task"
											),
											"name",
											false
										),
										CPP__TREE__CREATE_ENTITIES_VECTOR(),
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"" + File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, childSystemEntityId, "::", false) + "\"", "")
										)
									)
								)
							);
							END_PROFILE();

						}
						END_PROFILE();

						//OksEngine__Test2__TestChild1Task.precede(OksEngine__Test2__TestChild2Task);
						BEGIN_PROFILE("Generate systems set dependence code.");
						for (auto childSystemEntityId : childThreadSystems) {

							const std::string systemFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, childSystemEntityId, "::", false);
							BEGIN_PROFILE("Generate %s system set dependence code.", systemFullName.c_str());
							ECS2::Entity::Id callOrderEntityId = GetComponent<ECS::File::Table::System::CallOrder::EntityId>(childSystemEntityId)->id_;

							const auto& beforeSystemInfos = GetComponent<ECS::File::Table::System::CallOrder::RunBefore>(callOrderEntityId)->systems_;
							for (auto& beforeSystemInfo : beforeSystemInfos) {

								if (beforeSystemInfo.id_.IsInvalid()) {
									continue;
								}

								if (IsComponentExist<ECS::File::Table::System::Thread>(beforeSystemInfo.id_)
									&& GetComponent<ECS::File::Table::System::Thread>(beforeSystemInfo.id_)->thread_ == "Main") {
									continue;
								}

								runSystemsRealization.push_back(
									CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
										CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
											CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
													File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, childSystemEntityId, "__", false) + "Task"
												),
												"precede",
												false
											),
											CPP__TREE__CREATE_ENTITIES_VECTOR(),
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
													File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, beforeSystemInfo.id_, "__", false) + "Task"
												)
											)
										)
									)

								);
							}
							END_PROFILE();
						}
						END_PROFILE();
						//tf::Future<void> future = executor.run(taskFlow);
						runSystemsRealization.push_back(
							CPP__TREE__DECL__CREATE_VARIABLE(this,
								CPP__TREE__TYPE__CREATE_TEMPLATE_SPECIALIZATION_TYPE(this,
									CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "tf::Future"),
									CPP__TREE__CREATE_ENTITIES_VECTOR(
										CPP__TREE__TYPE__CREATE_VOID_TYPE(this)
									)
								),
								"future",
								CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
									CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
										CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "executor"),
										"run",
										false
									),
									CPP__TREE__CREATE_ENTITIES_VECTOR(),
									CPP__TREE__CREATE_ENTITIES_VECTOR(
										CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "taskFlow")
									)
								),
								ECS2::Entity::Id::invalid_
							)

						);


						//Main thread system code generation.
						for (auto mainSystemEntityId : mainThreadSystems) {
							const std::string systemFullName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, mainSystemEntityId, "::", false);

							std::vector<ECS2::Entity::Id> mainThreadRealization;

							////static std::mutex OksEngine__Test2__TestMain1FinishedMutex;
							//synchronizationEntityIds.push_back(
							//	CPP__TREE__DECL__CREATE_STATIC_VARIABLE(
							//		CPP__TREE__TYPE__CREATE_NAMED_TYPE("std::mutex"),
							//		File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this,mainSystemEntityId, "__", false) + "FinishedMutex",
							//		ECS2::Entity::Id::invalid_,
							//		ECS2::Entity::Id::invalid_
							//	)
							//);
							////static std::atomic_bool OksEngine__Test2__TestMain1FinishedFlag = false;
							//synchronizationEntityIds.push_back(
							//	CPP__TREE__DECL__CREATE_STATIC_VARIABLE(
							//		CPP__TREE__TYPE__CREATE_NAMED_TYPE("std::atomic_bool"),
							//		File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this,mainSystemEntityId, "__", false) + "FinishedFlag",
							//		CPP__TREE__EXPR__CREATE_LITERAL_EXPR("false", ""),
							//		ECS2::Entity::Id::invalid_
							//	)
							//);
							////static std::condition_variable OksEngine__Test2__TestMain1FinishedCV;
							//synchronizationEntityIds.push_back(
							//	CPP__TREE__DECL__CREATE_STATIC_VARIABLE(
							//		CPP__TREE__TYPE__CREATE_NAMED_TYPE("std::condition_variable"),
							//		File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this,mainSystemEntityId, "__", false) + "FinishedCV",
							//		ECS2::Entity::Id::invalid_,
							//		ECS2::Entity::Id::invalid_
							//	)
							//);

							ECS2::Entity::Id callOrderEntityId = GetComponent<ECS::File::Table::System::CallOrder::EntityId>(mainSystemEntityId)->id_;
							auto& afterSystemInfos = GetComponent<ECS::File::Table::System::CallOrder::RunAfter>(callOrderEntityId)->systems_;
							std::sort(afterSystemInfos.begin(), afterSystemInfos.end(),
								[&orderedSystemIdToIndex](const auto& first, const auto& second) {
									return orderedSystemIdToIndex[first.id_] < orderedSystemIdToIndex[second.id_];
								});

							//BEGIN_PROFILE("Test2::TestMain2");
							if (!afterSystemInfos.empty()) {
								mainThreadRealization.push_back(
									CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
										CPP__TREE__EXPR__CREATE_MACRO_INVOCATION(this,
											"BEGIN_PROFILE",
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"" + systemFullName + " is waitting for another systems.\"", "")
											),
											ECS2::Entity::Id::invalid_
										)
									)
								);
							}

							for (auto& afterSystemInfo : afterSystemInfos) {

								if (GetComponent<ECS::File::Table::System::Thread>(afterSystemInfo.id_)->thread_ == "Main") {
									continue;
								}

								mainThreadRealization.push_back(
									CPP__TREE__DECL__CREATE_VARIABLE(this,
										CPP__TREE__TYPE__CREATE_TEMPLATE_SPECIALIZATION_TYPE(this,
											CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::unique_lock"),
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::mutex")
											)
										),
										"lock" + File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, afterSystemInfo.id_, "__", false),
										CPP__TREE__EXPR__CREATE_INIT_LIST(this,
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
													File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, afterSystemInfo.id_, "__", false) + "FinishedMutex"
												)
											)
										),
										ECS2::Entity::Id::invalid_
									)
								);
								mainThreadRealization.push_back(
									CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
										CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
											CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
													File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, afterSystemInfo.id_, "__", false) + "FinishedCV"
												),
												"wait",
												false
											),
											CPP__TREE__CREATE_ENTITIES_VECTOR(),
											CPP__TREE__CREATE_ENTITIES_VECTOR(
												CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "lock" + File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, afterSystemInfo.id_, "__", false)),
												CPP__TREE__EXPR__CREATE_LAMBDA(this,
													CPP::Tree::Expr::CaptureType::ByReference,
													std::vector<CPP::Tree::Expr::LambdaCapture>{},
													CPP__TREE__CREATE_ENTITIES_VECTOR(),
													CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
														CPP__TREE__CREATE_ENTITIES_VECTOR(
															CPP__TREE__STMT__CREATE_RETURN_STATEMENT(this,
																CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
																	CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
																		CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
																			File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, afterSystemInfo.id_, "__", false) + "FinishedFlag"
																		),
																		"load",
																		false
																	),
																	CPP__TREE__CREATE_ENTITIES_VECTOR(),
																	CPP__TREE__CREATE_ENTITIES_VECTOR()
																)
															)
														)
													)
												)
											)
										)
									)
								);
							}

							//END_PROFILE();
							if (!afterSystemInfos.empty()) {
								mainThreadRealization.push_back(
									CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
										CPP__TREE__EXPR__CREATE_MACRO_INVOCATION(this,
											"END_PROFILE",
											CPP__TREE__CREATE_ENTITIES_VECTOR(),
											ECS2::Entity::Id::invalid_
										)
									)
								);
							}


							//BEGIN_PROFILE("Test2::TestMain2");
							mainThreadRealization.push_back(
								CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
									CPP__TREE__EXPR__CREATE_MACRO_INVOCATION(this,
										"BEGIN_PROFILE",
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "\"" + systemFullName + "\"", "")
										),
										ECS2::Entity::Id::invalid_
									)
								)
							);


							//Test2::TestMain1System(world);
							mainThreadRealization.push_back(
								CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
									CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
										CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, systemFullName + "System"),
										CPP__TREE__CREATE_ENTITIES_VECTOR(),
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "world")
										)
									)
								)
							);

							//END_PROFILE();
							mainThreadRealization.push_back(
								CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
									CPP__TREE__EXPR__CREATE_MACRO_INVOCATION(this,
										"END_PROFILE",
										CPP__TREE__CREATE_ENTITIES_VECTOR(),
										ECS2::Entity::Id::invalid_
									)
								)
							);
							/*{
								{
									std::unique_lock<std::mutex> lock{ OksEngine__Test2__TestMain1FinishedMutex };
									OksEngine__Test2__TestMain1FinishedFlag = true;
								}
								OksEngine__Test2__TestMain1FinishedCV.notify_all();
							}*/
							if (ECS__FILE__TABLE__SYSTEM__HAS_RUN_BEFORE_SYSTEMS(mainSystemEntityId)) {
								mainThreadRealization.push_back(
									CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
										CPP__TREE__CREATE_ENTITIES_VECTOR(
											CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
												CPP__TREE__CREATE_ENTITIES_VECTOR(
													CPP__TREE__DECL__CREATE_VARIABLE(this,
														CPP__TREE__TYPE__CREATE_TEMPLATE_SPECIALIZATION_TYPE(this,
															CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::unique_lock"),
															CPP__TREE__CREATE_ENTITIES_VECTOR(
																CPP__TREE__TYPE__CREATE_NAMED_TYPE(this, "std::mutex")
															)
														),
														"lock",
														CPP__TREE__EXPR__CREATE_INIT_LIST(this,
															CPP__TREE__CREATE_ENTITIES_VECTOR(
																CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, mainSystemEntityId, "__", false) + "FinishedMutex")
															)
														),
														ECS2::Entity::Id::invalid_
													),
													CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
														CPP__TREE__EXPR__CREATE_BINARY_OPERATION(this,
															CPP::Tree::Expr::BinaryOpType::Assign,
															CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
																File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, mainSystemEntityId, "__", false) + "FinishedFlag"
															),
															CPP__TREE__EXPR__CREATE_LITERAL_EXPR(this, "true", "")
														)
													)
												)
											),
											CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
												CPP__TREE__EXPR__CREATE_CALL_EXPR_NO_ARGS(this,
													CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
														CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this,
															File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, mainSystemEntityId, "__", false) + "FinishedCV"
														),
														"notify_all",
														false
													)
												)
											)
										)
									)

								);
							}

							runSystemsRealization.push_back(
								CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this,
									CPP__TREE__CREATE_ENTITIES_VECTOR(mainThreadRealization)
								)
							);

						}

						//future.wait();
						runSystemsRealization.push_back(
							CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(this,
								CPP__TREE__EXPR__CREATE_CALL_EXPR(this,
									CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(this,
										CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(this, "future"),
										"wait",
										false
									),
									CPP__TREE__CREATE_ENTITIES_VECTOR(),
									CPP__TREE__CREATE_ENTITIES_VECTOR()
								)
							)
						);

						//RunSystems function
						ECS2::Entity::Id body = CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(this, runSystemsRealization);
						return CPP__TREE__DECL__CREATE_FUNCTION(this,
							CPP__TREE__TYPE__CREATE_VOID_TYPE(this),
							"RunSystems", ECS2::Entity::Id::invalid_,
							CPP__TREE__CREATE_ENTITIES_VECTOR(
								ECS__GENERATOR__CREATE_WORLD_PARAMETER(this),
								body));


					}()
						);

				childs.insert(childs.end(), synchronizationEntityIds.begin(), synchronizationEntityIds.end());
				childs.insert(childs.end(), realizationEntityIds.begin(), realizationEntityIds.end());
				return childs;
				}();

			cppFileNodeEntityIds.insert(cppFileNodeEntityIds.end(), childs.begin(), childs.end());

			CreateComponent<CPP::Tree::Node::ChildEntityIds>(cppRunSystemsEntityId, cppFileNodeEntityIds);
		}



	}

	namespace {
		AI_GENERATED
			// Вспомогательная функция для проверки существования альтернативного пути
			bool hasAlternativePath(Agraph_t* graph, Agnode_t* source, Agnode_t* target) {
			// BFS для поиска пути, исключая прямое ребро source->target
			std::unordered_set<Agnode_t*> visited;
			std::queue<Agnode_t*> q;

			visited.insert(source);

			// Начинаем с соседей source (кроме target)
			for (Agedge_t* e = agfstout(graph, source); e; e = agnxtout(graph, e)) {
				Agnode_t* neighbor = aghead(e);
				if (neighbor != target) {
					visited.insert(neighbor);
					q.push(neighbor);
				}
			}

			while (!q.empty()) {
				Agnode_t* current = q.front();
				q.pop();

				if (current == target) {
					return true; // Найден альтернативный путь
				}

				for (Agedge_t* e = agfstout(graph, current); e; e = agnxtout(graph, e)) {
					Agnode_t* neighbor = aghead(e);
					if (visited.find(neighbor) == visited.end()) {
						visited.insert(neighbor);
						q.push(neighbor);
					}
				}
			}

			return false; // Альтернативный путь не найден
		}


		AI_GENERATED
			// Основная функция для удаления транзитивных рёбер
			void removeTransitiveEdges(Agraph_t* graph) {
			if (!graph) return;

			std::set<Agedge_t*> edgesToRemove;

			// Собираем все рёбра для проверки
			for (Agnode_t* source = agfstnode(graph); source; source = agnxtnode(graph, source)) {
				for (Agedge_t* e = agfstout(graph, source); e; e = agnxtout(graph, e)) {
					Agnode_t* target = aghead(e);

					// Проверяем, есть ли альтернативный путь от source к target
					if (hasAlternativePath(graph, source, target)) {
						edgesToRemove.insert(e);
					}
				}
			}

			// Удаляем найденные транзитивные рёбра
			for (Agedge_t* e : edgesToRemove) {
				agdelete(graph, e);
				/*std::cout << "Removed edge: "
					<< agnameof(agtail(e)) << " -> "
					<< agnameof(aghead(e)) << std::endl;*/
			}

			std::cout << "Всего удалено рёбер: " << edgesToRemove.size() << std::endl;
		}

		AI_GENERATED
			// Функция для поиска и раскраски циклов в красный цвет
			void findAndColorCycles(Agraph_t* graph) {
			if (!graph) return;

			// Структуры для хранения состояния узлов
			std::unordered_map<Agnode_t*, int> nodeState; // 0 - не посещен, 1 - в обработке, 2 - обработан
			std::unordered_map<Agnode_t*, Agnode_t*> parent;
			std::vector<std::pair<Agnode_t*, Agnode_t*>> cycleEdges;

			// Рекурсивная функция DFS для поиска циклов
			std::function<void(Agnode_t*)> dfs = [&](Agnode_t* node) {
				nodeState[node] = 1; // Узел в обработке

				// Обход всех исходящих рёбер
				for (Agedge_t* edge = agfstout(graph, node); edge; edge = agnxtout(graph, edge)) {
					Agnode_t* neighbor = aghead(edge);

					if (nodeState[neighbor] == 0) {
						// Соседний узел не посещен
						parent[neighbor] = node;
						dfs(neighbor);
					}
					else if (nodeState[neighbor] == 1) {
						// Найден цикл! Добавляем ребро в список
						cycleEdges.push_back({ node, neighbor });

						// Прокручиваем назад по родителям, чтобы найти все рёбра цикла
						Agnode_t* current = node;
						while (current != neighbor && parent.find(current) != parent.end()) {
							cycleEdges.push_back({ parent[current], current });
							current = parent[current];
						}
					}
				}

				nodeState[node] = 2; // Узел полностью обработан
				};

			// Запускаем DFS для всех узлов
			for (Agnode_t* node = agfstnode(graph); node; node = agnxtnode(graph, node)) {
				if (nodeState[node] == 0) {
					dfs(node);
				}
			}

			// Раскрашиваем рёбра циклов в красный цвет
			for (const auto& edgePair : cycleEdges) {
				// Ищем ребро между узлами
				Agedge_t* edge = agedge(graph, edgePair.first, edgePair.second, nullptr, 0);
				if (edge) {
					// Устанавливаем атрибут цвета
					agsafeset(edge, (char*)"color", (char*)"red", (char*)"");

					// Можно также сделать ребро толще для лучшей видимости
					agsafeset(edge, (char*)"penwidth", (char*)"3.0", (char*)"");
				}
			}

			if (!cycleEdges.empty()) {
				// Выводим информацию о найденных циклах
				OS::LogError("systems_graph", "FOUND LOOP/S IN SYSTEMS GRAPH!!! CHECK .dot graph.");
			}
		}

	}
	void GenerateDotGraph::Update(
		ECS2::Entity::Id entity0id,
		const ECS::Project::Tag* eCS__Project__Tag0,
		const ECS::Project::Path* eCS__Project__Path0,
		const Behaviour::LuaScript* luaScript0) {

		Agraph_t* g = agopen((char*)"G", Agstrictdirected, nullptr);

		const auto systemComponents = world_->GetComponents<ECS__FILE__TABLE__SYSTEM__SYSTEM>();
		const Common::Size allSystemsNumber = world_->GetEntitiesNumber<ECS__FILE__TABLE__SYSTEM__SYSTEM>();

		std::unordered_map<ECS2::Entity::Id, Agnode_t*, ECS2::Entity::Id::Hash> systemToNode;


		std::unordered_set<std::string> names;

		std::unordered_map<std::string, std::string> debug;

		//Create nodes for all systems.
		for (Common::Index i = 0; i < allSystemsNumber; i++) {
			const ECS2::Entity::Id systemEntityId = *(std::get<ECS2::Entity::Id*>(systemComponents) + i);
			const auto systemType = *(std::get<ECS::File::Table::System::Type*>(systemComponents) + i);

			const ECS2::Entity::Id fileEntityId = (std::get<ECS::File::EntityId*>(systemComponents) + i)->id_;

			const std::filesystem::path filePath = GetComponent<ECS::File::Path>(fileEntityId)->path_;

			if (systemType.type_ != "Initialize" /*&& !ECS__FILE__TABLE__SYSTEM__IS_DISABLED(systemEntityId)*/) {

				const ECS2::ComponentsFilter systemCF = GetComponentsFilter(systemEntityId);
				std::string fullSystemName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, systemEntityId, "::", false);

				ASSERT(!names.contains(fullSystemName));
				names.insert(fullSystemName);
				debug.insert({ fullSystemName , filePath.string() });
				Agnode_t* systemNode = agnode(g, (char*)fullSystemName.c_str(), 1);

				if (!systemCF.IsSet<ECS::File::Table::System::UpdateMethod::EntityId>()) {
					//System - mark
					agsafeset(systemNode, (char*)"color", (char*)"green", (char*)"");
				}

				ASSERT(!systemToNode.contains(systemEntityId));
				const auto [it, wasInserted] = systemToNode.insert({ systemEntityId, systemNode });
				ASSERT(wasInserted);

			}
		}

		ASSERT(names.size() == systemToNode.size());

		//Create edges between nodes.
		for (Common::Index i = 0; i < allSystemsNumber; i++) {

			const auto systemType = *(std::get<ECS::File::Table::System::Type*>(systemComponents) + i);
			if (systemType.type_ != "Initialize") {
				const ECS2::Entity::Id systemEntityId = *(std::get<ECS2::Entity::Id*>(systemComponents) + i);
				Agnode_t* systemNode = systemToNode[systemEntityId];
				std::string fullSystemName = File::Table::ECS__FILE__TABLE__GET_FULL_NAME(this, systemEntityId, "::", false);
				const ECS2::Entity::Id callOrderEntityId = (*(std::get<ECS::File::Table::System::CallOrder::EntityId*>(systemComponents) + i)).id_;
				ASSERT(callOrderEntityId.IsValid());
				if (IsComponentExist<ECS::File::Table::System::CallOrder::RunAfter>(callOrderEntityId)) {
					const auto& runAfterSystems = GetComponent<ECS::File::Table::System::CallOrder::RunAfter>(callOrderEntityId)->systems_;
					for (const auto& systemInfo : runAfterSystems) {
						if (systemInfo.id_.IsValid()) {
							Agnode_t* afterSystemNode = systemToNode[systemInfo.id_];
							Agedge_t* gEdge = agedge(g, afterSystemNode, systemNode, nullptr, 1);
						}
						else {
							Agnode_t* disabledSystemNode = agnode(g, (char*)systemInfo.name_.c_str(), 1);
							agsafeset(systemNode, (char*)"color", (char*)"red", (char*)"");
							Agedge_t* gEdge = agedge(g, disabledSystemNode, systemNode, nullptr, 1);
						}

					}
				}

			}
		}
		removeTransitiveEdges(g);
		findAndColorCycles(g);
		{
			GVC_t* gvc = gvContext();
			char* dotData = nullptr;
			unsigned int length = 0;

			gvLayout(gvc, g, "dot");
			gvRenderData(gvc, g, "dot", &dotData, &length);
			std::filesystem::path projectFilePath = eCS__Project__Path0->path_;
			Resource::CreateFileAndSaveContent((projectFilePath.parent_path() / "auto_ECSSystemsCallGraph.dot").string(), dotData, length);
			agclose(g);
			gvFreeContext(gvc);

		}

	}

}; // namespace ECS::Generator