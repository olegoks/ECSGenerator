#pragma once 

#include <string>
#include <CPP\auto_OksEngine.CPP.ClangTree.hpp>

using namespace std::string_literals;

#define CPP_CONSTEXPR "constexpr"
#define CPP_CONSTEXPR_STR "constexpr"s

#define CPP_CONST "const"
#define CPP_CONST_STR "const"s

#define CPP_VOID "void"
#define CPP_VOID_STR "void"s

#define CPP_CHAR "char"
#define CPP_CHAR_STR "char"s

#define CPP_BOOL "bool"
#define CPP_BOOL_STR "bool"s

#define CPP_FLOAT "float"
#define CPP_FLOAT_STR "float"s

#define CPP_ENUM "enum"
#define CPP_ENUM_STR "enum"s

#define CPP_RETURN "return"
#define CPP_RETURN_STR "return"s

#define CPP_CONSTEXPR "constexpr"
#define CPP_CONSTEXPR_STR "constexpr"s

#define CPP_STATIC "static"
#define CPP_STATIC_STR "static"s

#define CPP_INLINE "inline"
#define CPP_INLINE_STR "inline"s

#define CPP_SPACE " "
#define CPP_SPACE_STR " "s
#define CPP_COMMA ","
#define CPP_COMMA_STR ","s

#define CPP_NEWLINE "\n"
#define CPP_NEWLINE_STR "\n"s

#define CPP_CLASS "class"
#define CPP_CLASS_STR "class"s

#define CPP_PUBLIC "public:"
#define CPP_PUBLIC_STR "public:"s

#define CPP_PRIVATE "private:"
#define CPP_PRIVATE_STR "private:"s

#define CPP_DEFINE "define"
#define CPP_DEFINE_STR "define"s


//#define CPP__TREE__CREATE_ENTITIES_VECTOR(...)std::vector<ECS2::Entity::Id>{__VA_ARGS__}

namespace CPP__TREE__DETAIL {

    // Одиночный ECS2::Entity::Id — кладём как один элемент.
    inline void AppendEntity(std::vector<ECS2::Entity::Id>& out, ECS2::Entity::Id id) {
        out.push_back(id);
    }

    // lvalue-вектор — вставляем копированием.
    inline void AppendEntity(std::vector<ECS2::Entity::Id>& out,
        const std::vector<ECS2::Entity::Id>& ids) {
        out.insert(out.end(), ids.begin(), ids.end());
    }

    // rvalue-вектор — вставляем перемещением; для пустого out можно украсть буфер.
    inline void AppendEntity(std::vector<ECS2::Entity::Id>& out,
        std::vector<ECS2::Entity::Id>&& ids) {
        if (out.empty()) {
            out = std::move(ids);
        }
        else {
            out.insert(out.end(),
                std::make_move_iterator(ids.begin()),
                std::make_move_iterator(ids.end()));
        }
    }

    template<class T>
    inline constexpr bool is_valid_entities_vector_arg_v =
        std::is_same_v<std::decay_t<T>, ECS2::Entity::Id> ||
        std::is_same_v<std::decay_t<T>, std::vector<ECS2::Entity::Id>>;

} // namespace CPP__TREE__DETAIL


/**
 * Собирает плоский std::vector<ECS2::Entity::Id> из аргументов.
 *
 * Допустимые аргументы:
 *   - ECS2::Entity::Id         (кладётся как один элемент)
 *   - std::vector<ECS2::Entity::Id>   (все элементы конкатенируются в результат)
 *
 * Примеры:
 *   CPP__TREE__CREATE_ENTITIES_VECTOR()                                  // {}
 *   CPP__TREE__CREATE_ENTITIES_VECTOR(a, b, c)                           // {a, b, c}
 *   CPP__TREE__CREATE_ENTITIES_VECTOR(a, vec1, b, std::move(vec2))       // a + vec1 + b + vec2
 */
template<class... Args>
std::vector<ECS2::Entity::Id> CPP__TREE__CREATE_ENTITIES_VECTOR(Args&&... args) {
    static_assert(
        (CPP__TREE__DETAIL::is_valid_entities_vector_arg_v<Args> && ...),
        "CPP__TREE__CREATE_ENTITIES_VECTOR accepts only ECS2::Entity::Id "
        "and std::vector<ECS2::Entity::Id>"
        );

    // 1. Вычисляем точный размер, чтобы сделать один reserve.
    auto countHint = [](auto&& x) -> std::size_t {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, std::vector<ECS2::Entity::Id>>) {
            return x.size();
        }
        else {
            return 1u;
        }
        };

    std::size_t total = 0;
    ((total += countHint(args)), ...);   // args как lvalue: только читаем

    std::vector<ECS2::Entity::Id> result;
    result.reserve(total);

    // 2. Заполняем: std::forward даёт move для rvalue-векторов/ID.
    (CPP__TREE__DETAIL::AppendEntity(result, std::forward<Args>(args)), ...);

    return result;
}

template<class System>
ECS2::Entity::Id CPP__TREE__COMMENT__CREATE_ONE_LINE_COMMENT(System* system, const std::string& str) {
	ECS2::Entity::Id comment = system->CreateEntity();
	system->CreateComponent<CPP::Tree::Comment>(comment, str, CPP::Tree::CommentType::OneLine);
	return comment;
}

template<class System>
ECS2::Entity::Id CPP__TREE__COMMENT__CREATE_CLANG_FORMAT_OFF_COMMENT(System* system) {
	ECS2::Entity::Id comment = system->CreateEntity();
	system->CreateComponent<CPP::Tree::Comment>(comment, ::std::string{ " clang-format off" }, CPP::Tree::CommentType::OneLine);
	return comment;
}

template<class System>
ECS2::Entity::Id CPP__TREE__COMMENT__CREATE_CLANG_FORMAT_ON_COMMENT(System* system) {
	ECS2::Entity::Id comment = system->CreateEntity();
	system->CreateComponent<CPP::Tree::Comment>(comment, ::std::string{ " clang-format on\n" }, CPP::Tree::CommentType::OneLine);
	return comment;
}


/*					PREPROCESSOR					*/
#define CPP__TREE__PREPROCESSOR__CREATE_PRAGMA(str)\
	[this](const std::string& text){\
		ECS2::Entity::Id pragma = CreateEntity();\
		CreateComponent<CPP::Tree::Preprocessor::Tag>(pragma);\
		CreateComponent<CPP::Tree::Preprocessor::Pragma_>(pragma, text);\
		return pragma;\
	}(str)

#define CPP__TREE__PREPROCESSOR__CREATE_PRAGMA_OPTIMIZE()\
	[this](){\
		ECS2::Entity::Id pragmaOptimize = CreateEntity();\
		CreateComponent<CPP::Tree::Preprocessor::Tag>(pragmaOptimize);\
		CreateComponent<CPP::Tree::Preprocessor::Pragma_>(pragmaOptimize, "optimize");\
		return pragmaOptimize;\
	}()

template<class System>
ECS2::Entity::Id CPP__TREE__PREPROCESSOR__CREATE_PRAGMA_ONCE(System* system) {
	ECS2::Entity::Id pragmaOptimize = system->CreateEntity();
	system->template CreateComponent<CPP::Tree::Preprocessor::Tag>(pragmaOptimize);
	system->template CreateComponent<CPP::Tree::Preprocessor::Pragma_>(pragmaOptimize, "once");
	return pragmaOptimize;
}

template<class System>
ECS2::Entity::Id CPP__TREE__PREPROCESSOR__CREATE_INCLUDE(System* system, const std::string& text) {
	ECS2::Entity::Id includeEntityId = system->CreateEntity();
	system->template CreateComponent<CPP::Tree::Preprocessor::Tag>(includeEntityId);
	system->template CreateComponent<CPP::Tree::Preprocessor::Include_>(includeEntityId, text, false);
	return includeEntityId;
}

#define CPP__TREE__PREPROCESSOR__CREATE_DEFINE(name, params, bodyStr, bodyEntity, isFunction)\
	[this](const std::string& nameStr, const std::vector<std::string>& parameters, const std::string& bodyString, ECS2::Entity::Id bodyEntityId, bool isFunctionLike){\
		ECS2::Entity::Id macrosEntityId = CreateEntity();\
		CreateComponent<CPP::Tree::Preprocessor::Tag>(macrosEntityId);\
		CreateComponent<CPP::Tree::Preprocessor::Define_>(macrosEntityId,nameStr, parameters, bodyString, bodyEntityId, isFunctionLike);\
		return macrosEntityId;\
	}(name, params, bodyStr, bodyEntity, isFunction)


template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_VARIABLE_COMPONENTS(System* system, ECS2::Entity::Id variableEntityId, ECS2::Entity::Id typeEntityId, const std::string& variableName, ECS2::Entity::Id initializeEntityId, ECS2::Entity::Id parentEntityId) {
	system->template CreateComponent<CPP::Tree::Node::Tag>(variableEntityId);
	system->template CreateComponent<CPP::Tree::Decl::Tag>(variableEntityId);
	system->template CreateComponent<CPP::Tree::Decl::Variable>(variableEntityId, typeEntityId, initializeEntityId);
	system->template CreateComponent<CPP::Tree::Node::Name>(variableEntityId, variableName);
	if (parentEntityId.IsValid()) {
		system->template CreateComponent<CPP::Tree::Node::ParentEntityId>(variableEntityId, parentEntityId);
	}
	return variableEntityId;
}

/* ======================== VARIABLES ======================== */

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_VARIABLE(
    System* system,
    ECS2::Entity::Id typeEntityId,
    const std::string& variableName,
    ECS2::Entity::Id initializeEntityId,
    ECS2::Entity::Id parentEntityId) {
    ECS2::Entity::Id variableEntityId = system->CreateEntity();
    CPP__TREE__DECL__CREATE_VARIABLE_COMPONENTS(
        system,
        variableEntityId,
        typeEntityId,
        variableName,
        initializeEntityId,
        parentEntityId);
    return variableEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_STATIC_VARIABLE(
    System* system,
    ECS2::Entity::Id typeEntityId,
    const std::string& variableName,
    ECS2::Entity::Id initializeEntityId,
    ECS2::Entity::Id parentEntityId) {
    ECS2::Entity::Id variableEntityId = CPP__TREE__DECL__CREATE_VARIABLE(
        system,
        typeEntityId,
        variableName,
        initializeEntityId,
        parentEntityId);
    system->template CreateComponent<CPP::Tree::StorageClass::Static_>(variableEntityId);
    return variableEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_STATIC_CONSTEXPR_VARIABLE(
    System* system,
    ECS2::Entity::Id typeEntityId,
    const std::string& variableName,
    ECS2::Entity::Id initializeEntityId,
    ECS2::Entity::Id parentEntityId) {
    const ECS2::Entity::Id variableEntityId = CPP__TREE__DECL__CREATE_VARIABLE(
        system,
        typeEntityId,
        variableName,
        initializeEntityId,
        parentEntityId);
    system->template CreateComponent<CPP::Tree::StorageClass::Static_>(variableEntityId);
    system->template CreateComponent<CPP::Tree::Constexpr_>(variableEntityId);
    return variableEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_NAMESPACE(System* system, const std::string& name) {
	const ECS2::Entity::Id namespaceEntityId = system->CreateEntity();
	system->template CreateComponent<CPP::Tree::Node::Tag>(namespaceEntityId);
	system->template CreateComponent<CPP::Tree::Decl::Tag>(namespaceEntityId);
	system->template CreateComponent<CPP::Tree::Decl::Namespace_>(namespaceEntityId);
	system->template CreateComponent<CPP::Tree::Node::Name>(namespaceEntityId, name);
	return namespaceEntityId;
}

template<class System>
std::pair<ECS2::Entity::Id, ECS2::Entity::Id> CPP__TREE__CREATE_CPP_NAMESPACES(System* system, const std::vector<ECS2::Entity::Id>& parentTableEntityIds, std::vector<ECS2::Entity::Id> cppTableEntityId) {
	ECS2::Entity::Id firstNamespaceEntityId = ECS2::Entity::Id::invalid_;
	ECS2::Entity::Id lastNamespaceEntityId = ECS2::Entity::Id::invalid_;
	std::vector<ECS2::Entity::Id> cppNamespaceEntityIds;
	for (Common::Index i = 0; i < parentTableEntityIds.size(); i++) {
		ECS2::Entity::Id parentTableEntityId = parentTableEntityIds[i];
		const ECS2::Entity::Id namespaceEntityId = CPP__TREE__DECL__CREATE_NAMESPACE(system, system->template GetComponent<ECS::File::Table::Name>(parentTableEntityId)->name_);
		cppNamespaceEntityIds.push_back(namespaceEntityId);
	}
	for (Common::Index i = 0; i < parentTableEntityIds.size(); i++) {
		ECS2::Entity::Id parentTableEntityId = parentTableEntityIds[i];
		const ECS2::ComponentsFilter tableCF = system->GetComponentsFilter(parentTableEntityId);
		ASSERT(tableCF.IsSet<ECS::File::Table::Namespace::Tag>());
		ASSERT(tableCF.IsSet<ECS::File::Table::ChildTablesEntityIds>());
		const ECS2::Entity::Id namespaceEntityId = cppNamespaceEntityIds[i];
		if (i != parentTableEntityIds.size() - 1) {
			system->template CreateComponent<CPP::Tree::Node::ChildEntityIds>(namespaceEntityId, std::vector{ cppNamespaceEntityIds[i + 1] });
		}
		else {
			if (!cppTableEntityId.empty()) {
				system->template CreateComponent<CPP::Tree::Node::ChildEntityIds>(namespaceEntityId, cppTableEntityId);
			}
			lastNamespaceEntityId = namespaceEntityId;
		}
		if (i != 0) {
			system->template CreateComponent<CPP::Tree::Node::ParentEntityId>(namespaceEntityId, cppNamespaceEntityIds[i - 1]);
		}
		else {
			firstNamespaceEntityId = namespaceEntityId;
		}
	}
	return { firstNamespaceEntityId, lastNamespaceEntityId };
}

template<class System>
ECS2::Entity::Id CPP__TREE__TYPE__CREATE_NAMED_TYPE(System* system, const std::string& name) {
	ECS2::Entity::Id typeEntityId = system->CreateEntity();
	system->template CreateComponent<CPP::Tree::Type::Tag>(typeEntityId);
	system->template CreateComponent<CPP::Tree::Type::Name>(typeEntityId, name);
	return typeEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__TYPE__CREATE_AUTO_TYPE(System* system) {
	ECS2::Entity::Id typeEntityId = system->CreateEntity();
	system->template CreateComponent<CPP::Tree::Type::Tag>(typeEntityId);
	system->template CreateComponent<CPP::Tree::Type::Name>(typeEntityId, "auto");
	return typeEntityId;
}

template<class System>
std::vector<ECS2::Entity::Id> CPP__TREE__TYPE__CREATE_NAMED_TYPE_ENTITIES_VECTOR(System* system, const std::vector<std::string> names) {
	std::vector<ECS2::Entity::Id> typeEntityIds;
	for (std::string name : names) {
		typeEntityIds.push_back(CPP__TREE__TYPE__CREATE_NAMED_TYPE(system, name));
	}
	return typeEntityIds;
}

template<class System>
ECS2::Entity::Id CPP__TREE__TYPE__CREATE_VOID_TYPE(System* system) {
    ECS2::Entity::Id typeEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Type::Tag>(typeEntityId);
    system->template CreateComponent<CPP::Tree::Type::BuiltinType>(
        typeEntityId, CPP::Tree::Type::BuiltinTypes::Void);
    return typeEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__TYPE__CREATE_BOOL_TYPE(System* system) {
    ECS2::Entity::Id typeEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Type::Tag>(typeEntityId);
    system->template CreateComponent<CPP::Tree::Type::BuiltinType>(
        typeEntityId, CPP::Tree::Type::BuiltinTypes::Bool);
    return typeEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__TYPE__CREATE_FLOAT_TYPE(System* system) {
    ECS2::Entity::Id typeEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Type::Tag>(typeEntityId);
    system->template CreateComponent<CPP::Tree::Type::BuiltinType>(
        typeEntityId, CPP::Tree::Type::BuiltinTypes::Float);
    return typeEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__TYPE__CREATE_CHAR_TYPE(System* system) {
    ECS2::Entity::Id typeEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Type::Tag>(typeEntityId);
    system->template CreateComponent<CPP::Tree::Type::BuiltinType>(
        typeEntityId, CPP::Tree::Type::BuiltinTypes::Char);
    return typeEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__TYPE__CREATE_CONST_CHAR_TYPE(System* system) {
    ECS2::Entity::Id typeEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Type::Tag>(typeEntityId);
    system->template CreateComponent<CPP::Tree::Type::BuiltinType>(
        typeEntityId, CPP::Tree::Type::BuiltinTypes::Char);
    system->template CreateComponent<CPP::Tree::Type::ConstType>(typeEntityId);
    return typeEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__TYPE__CREATE_POINTER_TYPE(
    System* system,
    ECS2::Entity::Id pointerToTypeEntityId) {
    ECS2::Entity::Id typeEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Type::Tag>(typeEntityId);
    system->template CreateComponent<CPP::Tree::Type::PointerType>(
        typeEntityId, pointerToTypeEntityId);
    return typeEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__TYPE__CREATE_CONST_CHAR_POINTER_TYPE(System* system) {
    return CPP__TREE__TYPE__CREATE_POINTER_TYPE(
        system,
        CPP__TREE__TYPE__CREATE_CONST_CHAR_TYPE(system));
}

template<class System>
ECS2::Entity::Id CPP__TREE__TYPE__CREATE_REFERENCE_TYPE(
    System* system,
    ECS2::Entity::Id referenceToTypeEntityId,
    bool isRvalue) {
    ECS2::Entity::Id typeEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Type::Tag>(typeEntityId);
    system->template CreateComponent<CPP::Tree::Type::ReferenceType>(
        typeEntityId, referenceToTypeEntityId, isRvalue);
    return typeEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__TYPE__CREATE_TEMPLATE_SPECIALIZATION_TYPE(
    System* system,
    ECS2::Entity::Id baseType,
    std::vector<ECS2::Entity::Id> templateArgs) {
    ECS2::Entity::Id typeEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Type::Tag>(typeEntityId);
    system->template CreateComponent<CPP::Tree::Type::TemplateSpecializationType>(
        typeEntityId, baseType, templateArgs);
    return typeEntityId;
}

/* ======================== STATEMENTS ======================== */

template<class System>
ECS2::Entity::Id CPP__TREE__STMT__CREATE_EXPRESSION_STATEMENT(
    System* system,
    ECS2::Entity::Id exprEntityId) {
    ECS2::Entity::Id stmtEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Stmt::Tag>(stmtEntityId);
    system->template CreateComponent<CPP::Tree::Stmt::ExpressionStmt>(
        stmtEntityId, exprEntityId);
    return stmtEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT_COMPONENTS(
    System* system,
    ECS2::Entity::Id entityId,
    std::vector<ECS2::Entity::Id> childEntityIds) {
    system->template CreateComponent<CPP::Tree::Node::Tag>(entityId);
    system->template CreateComponent<CPP::Tree::Stmt::Tag>(entityId);
    system->template CreateComponent<CPP::Tree::Stmt::CompoundStmt>(entityId);
    system->template CreateComponent<CPP::Tree::Node::ChildEntityIds>(
        entityId, childEntityIds);
    return entityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__STMT__CREATE_EMPTY_COMPOUND_STATEMENT(System* system) {
    ECS2::Entity::Id emptyStatementEntityId = system->CreateEntity();
    return CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT_COMPONENTS(
        system, emptyStatementEntityId, {});
}

template<class System>
ECS2::Entity::Id CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT(
    System* system,
    std::vector<ECS2::Entity::Id> childEntityIds) {
    ECS2::Entity::Id compoundStatementEntityId = system->CreateEntity();
    return CPP__TREE__STMT__CREATE_COMPOUND_STATEMENT_COMPONENTS(
        system, compoundStatementEntityId, childEntityIds);
}

template<class System>
ECS2::Entity::Id CPP__TREE__STMT__CREATE_RETURN_STATEMENT(
    System* system,
    ECS2::Entity::Id returnExprEntityId) {
    ECS2::Entity::Id returnStmtEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Stmt::Tag>(returnStmtEntityId);
    system->template CreateComponent<CPP::Tree::Stmt::ReturnStmt>(
        returnStmtEntityId, returnExprEntityId);
    return returnStmtEntityId;
}

/* ======================== PARAMETER DECLARATION ======================== */

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_PARAMETER(
    System* system,
    ECS2::Entity::Id typeEntityId,
    std::string parameterName) {
    ECS2::Entity::Id parameterEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Node::Tag>(parameterEntityId);
    system->template CreateComponent<CPP::Tree::Decl::Tag>(parameterEntityId);
    system->template CreateComponent<CPP::Tree::Decl::Parameter>(
        parameterEntityId,
        parameterName,
        typeEntityId,
        ECS2::Entity::Id::invalid_,
        false);
    return parameterEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_PARAMETER_PACK_PARAMETER(
    System* system,
    ECS2::Entity::Id typeEntityId,
    std::string parameterName) {
    ECS2::Entity::Id parameterEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Node::Tag>(parameterEntityId);
    system->template CreateComponent<CPP::Tree::Decl::Tag>(parameterEntityId);
    system->template CreateComponent<CPP::Tree::Decl::Parameter>(
        parameterEntityId,
        parameterName,
        typeEntityId,
        ECS2::Entity::Id::invalid_,
        true);
    return parameterEntityId;
}

/* ======================== EXPRESSIONS ======================== */

template<class System>
ECS2::Entity::Id CPP__TREE__EXPR__CREATE_PACK_EXPANSION(
    System* system,
    ECS2::Entity::Id patternEntityId) {
    ECS2::Entity::Id packExpansionEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Expr::Tag>(packExpansionEntityId);
    system->template CreateComponent<CPP::Tree::Expr::PackExpansion>(
        packExpansionEntityId, patternEntityId);
    return packExpansionEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__EXPR__CREATE_MACRO_INVOCATION(
    System* system,
    const std::string& macroName,
    std::vector<ECS2::Entity::Id> arguments,
    ECS2::Entity::Id macroEntityId) {
    ECS2::Entity::Id macroInvocationEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Expr::Tag>(macroInvocationEntityId);
    system->template CreateComponent<CPP::Tree::Expr::MacroInvocation>(
        macroInvocationEntityId, macroName, arguments, macroEntityId);
    return macroInvocationEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__EXPR__CREATE_LITERAL_EXPR(
    System* system,
    const std::string& literal,
    const std::string& suffixStr) {
    ECS2::Entity::Id literalEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Expr::Tag>(literalEntityId);
    system->template CreateComponent<CPP::Tree::Expr::Literal>(
        literalEntityId,
        Common::Limits<decltype(CPP::Tree::Expr::Literal::kind_)>::Max(),
        literal,
        suffixStr);
    return literalEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__EXPR__CREATE_MEMBER_PTR_EXPR(
    System* system,
    ECS2::Entity::Id typeEntityId,
    const std::string& memberName) {
    ECS2::Entity::Id memberPtrExprEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Expr::Tag>(memberPtrExprEntityId);
    system->template CreateComponent<CPP::Tree::Expr::MemberPtrExpr>(
        memberPtrExprEntityId, typeEntityId, memberName);
    return memberPtrExprEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(
    System* system,
    const std::string& identifierName) {
    ECS2::Entity::Id identifierExprEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Expr::Tag>(identifierExprEntityId);
    system->template CreateComponent<CPP::Tree::Expr::Identifier>(
        identifierExprEntityId, identifierName);
    return identifierExprEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__EXPR__CREATE_UNARY_OPERATION(
    System* system,
    CPP::Tree::Expr::UnaryOpType operationType,
    ECS2::Entity::Id operantEntityId) {
    ECS2::Entity::Id unaryOpExprEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Expr::Tag>(unaryOpExprEntityId);
    system->template CreateComponent<CPP::Tree::Expr::UnaryOp>(
        unaryOpExprEntityId, operationType, operantEntityId);
    return unaryOpExprEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__EXPR__CREATE_BINARY_OPERATION(
    System* system,
    CPP::Tree::Expr::BinaryOpType operationType,
    ECS2::Entity::Id leftEntityId,
    ECS2::Entity::Id rightEntityId) {
    ECS2::Entity::Id binaryOpExprEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Expr::Tag>(binaryOpExprEntityId);
    system->template CreateComponent<CPP::Tree::Expr::BinaryOp>(
        binaryOpExprEntityId, operationType, leftEntityId, rightEntityId);
    return binaryOpExprEntityId;
}

template<class System>
void CPP__TREE__EXPR__CREATE_LAMBDA_COMPONENTS(
    System* system,
    ECS2::Entity::Id lambdaEntityId,
    CPP::Tree::Expr::CaptureType captureType,
    std::vector<CPP::Tree::Expr::LambdaCapture> captures,
    std::vector<ECS2::Entity::Id> parameterEntityIds,
    ECS2::Entity::Id bodyEntityId) {
    system->template CreateComponent<CPP::Tree::Expr::Tag>(lambdaEntityId);
    system->template CreateComponent<CPP::Tree::Expr::Lambda>(
        lambdaEntityId,
        captureType,
        captures,
        parameterEntityIds,
        bodyEntityId);
}

template<class System>
ECS2::Entity::Id CPP__TREE__EXPR__CREATE_LAMBDA(
    System* system,
    CPP::Tree::Expr::CaptureType captureType,
    std::vector<CPP::Tree::Expr::LambdaCapture> captures,
    std::vector<ECS2::Entity::Id> parameterEntityIds,
    ECS2::Entity::Id bodyEntityId) {
    ECS2::Entity::Id lambdaEntityId = system->CreateEntity();
    CPP__TREE__EXPR__CREATE_LAMBDA_COMPONENTS(
        system, lambdaEntityId, captureType, captures, parameterEntityIds, bodyEntityId);
    return lambdaEntityId;
}

template<class System>
void CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR_COMPONENTS(
    System* system,
    ECS2::Entity::Id exprEntityId,
    ECS2::Entity::Id objectEntityId,
    const std::string& memberName,
    bool isArrow) {
    system->template CreateComponent<CPP::Tree::Expr::Tag>(exprEntityId);
    system->template CreateComponent<CPP::Tree::Expr::MemberAccess>(
        exprEntityId, objectEntityId, memberName, isArrow);
}

template<class System>
ECS2::Entity::Id CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR(
    System* system,
    ECS2::Entity::Id objectEntityId,
    const std::string& memberName,
    bool isArrow) {
    ECS2::Entity::Id exprEntityId = system->CreateEntity();
    CPP__TREE__EXPR__CREATE_MEMBER_ACCESS_EXPR_COMPONENTS(
        system, exprEntityId, objectEntityId, memberName, isArrow);
    return exprEntityId;
}

template<class System>
void CPP__TREE__EXPR__CREATE_CALL_EXPR_COMPONENTS(
    System* system,
    ECS2::Entity::Id callExprEntityId,
    ECS2::Entity::Id calleeEntityId,
    std::vector<ECS2::Entity::Id> templateArgumentEntityIds,
    std::vector<ECS2::Entity::Id> argumentEntityIds) {
    system->template CreateComponent<CPP::Tree::Expr::Tag>(callExprEntityId);
    system->template CreateComponent<CPP::Tree::Expr::CallExpr>(
        callExprEntityId,
        calleeEntityId,
        templateArgumentEntityIds,
        argumentEntityIds);
}

template<class System>
ECS2::Entity::Id CPP__TREE__EXPR__CREATE_CALL_EXPR(
    System* system,
    ECS2::Entity::Id calleeEntityId,
    std::vector<ECS2::Entity::Id> templateArgumentEntityIds,
    std::vector<ECS2::Entity::Id> argumentEntityIds) {
    ECS2::Entity::Id callExprEntityId = system->CreateEntity();
    CPP__TREE__EXPR__CREATE_CALL_EXPR_COMPONENTS(
        system,
        callExprEntityId,
        calleeEntityId,
        templateArgumentEntityIds,
        argumentEntityIds);
    return callExprEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__EXPR__CREATE_CALL_EXPR_NO_ARGS(
    System* system,
    ECS2::Entity::Id calleeEntityId) {
    ECS2::Entity::Id callExprEntityId = system->CreateEntity();
    CPP__TREE__EXPR__CREATE_CALL_EXPR_COMPONENTS(
        system, callExprEntityId, calleeEntityId, {}, {});
    return callExprEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__EXPR__CREATE_ARRAY_SUBSCRIPT(
    System* system,
    ECS2::Entity::Id arrayEntityId,
    ECS2::Entity::Id indexEntityId) {
    ECS2::Entity::Id arraySubscriptEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Expr::Tag>(arraySubscriptEntityId);
    system->template CreateComponent<CPP::Tree::Expr::ArraySubscript>(
        arraySubscriptEntityId, arrayEntityId, indexEntityId);
    return arraySubscriptEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__EXPR__CREATE_INIT_LIST(
    System* system,
    std::vector<ECS2::Entity::Id> initEntityIds) {
    ECS2::Entity::Id initListEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Expr::Tag>(initListEntityId);
    system->template CreateComponent<CPP::Tree::Expr::InitList>(
        initListEntityId, initEntityIds);
    return initListEntityId;
}

/* ======================== FUNCTIONS ======================== */

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(
    System* system,
    ECS2::Entity::Id functionEntityId,
    ECS2::Entity::Id returnValueEntityId,
    std::vector<ECS2::Entity::Id> templateParameters,
    const std::string& funcName,
    std::vector<ECS2::Entity::Id> templateExplicitParameters,
    ECS2::Entity::Id parentEntityId,
    std::vector<ECS2::Entity::Id> childEntityIds) {
    system->template CreateComponent<CPP::Tree::Node::Tag>(functionEntityId);
    system->template CreateComponent<CPP::Tree::Decl::Tag>(functionEntityId);
    system->template CreateComponent<CPP::Tree::Decl::Function>(
        functionEntityId, returnValueEntityId);
    system->template CreateComponent<CPP::Tree::Node::Name>(
        functionEntityId, funcName);

    if (!templateParameters.empty()) {
        system->template CreateComponent<CPP::Tree::Decl::TemplateDecl>(
            functionEntityId, templateParameters);
    }
    if (!templateExplicitParameters.empty()) {
        system->template CreateComponent<CPP::Tree::Decl::ExplicitTemplateArgs>(
            functionEntityId, templateExplicitParameters);
    }
    if (parentEntityId.IsValid()) {
        system->template CreateComponent<CPP::Tree::Node::ParentEntityId>(
            functionEntityId, parentEntityId);
    }
    system->template CreateComponent<CPP::Tree::Node::ChildEntityIds>(
        functionEntityId, childEntityIds);
    return functionEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_CONSTRUCTOR(
    System* system,
    const std::string& constructorName,
    std::vector<ECS2::Entity::Id> initializerEntityIds,
    ECS2::Entity::Id parentEntityId,
    ECS2::Entity::Id parentDeclEntityId,
    std::vector<ECS2::Entity::Id> childEntityId) {
    ECS2::Entity::Id constructorEntityId = system->CreateEntity();
    CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(
        system,
        constructorEntityId,
        ECS2::Entity::Id::invalid_,
        {},
        constructorName,
        {},
        parentEntityId,
        childEntityId);

    system->template CreateComponent<CPP::Tree::Decl::Constructor>(
        constructorEntityId, parentDeclEntityId);

    if (!initializerEntityIds.empty()) {
        system->template CreateComponent<CPP::Tree::ConstructorInitList>(
            constructorEntityId, initializerEntityIds);
    }
    return constructorEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_EMPTY_CONSTRUCTOR(
    System* system,
    const std::string& constructorName,
    ECS2::Entity::Id parentEntityId,
    ECS2::Entity::Id parentDeclEntityId) {
    return CPP__TREE__DECL__CREATE_CONSTRUCTOR(
        system,
        constructorName,
        {},
        parentEntityId,
        parentDeclEntityId,
        { CPP__TREE__STMT__CREATE_EMPTY_COMPOUND_STATEMENT(system) });
}

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_PUBLIC_EMPTY_CONSTRUCTOR(
    System* system,
    const std::string& constructorName,
    ECS2::Entity::Id parentEntityId,
    ECS2::Entity::Id parentDeclEntityId) {
    ECS2::Entity::Id constructorEntityId = CPP__TREE__DECL__CREATE_CONSTRUCTOR(
        system,
        constructorName,
        {},
        parentEntityId,
        parentDeclEntityId,
        { CPP__TREE__STMT__CREATE_EMPTY_COMPOUND_STATEMENT(system) });

    system->template CreateComponent<CPP::Tree::Access::Public_>(constructorEntityId);
    return constructorEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__CREATE_CONSTRUCTOR_INITIALIZER(
    System* system,
    ECS2::Entity::Id memberEntityId,
    ECS2::Entity::Id exprEntityId) {
    ECS2::Entity::Id initializer = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::ConstructorInitializer>(
        initializer, memberEntityId, exprEntityId);
    return initializer;
}

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_FUNCTION(
    System* system,
    ECS2::Entity::Id returnValueEntityId,
    const std::string& funcName,
    ECS2::Entity::Id parentEntityId,
    std::vector<ECS2::Entity::Id> childEntityIds) {
    ECS2::Entity::Id functionEntityId = system->CreateEntity();
    CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(
        system,
        functionEntityId,
        returnValueEntityId,
        {},
        funcName,
        {},
        parentEntityId,
        childEntityIds);
    return functionEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_TEMPLATE_FUNCTION(
    System* system,
    ECS2::Entity::Id returnValueEntityId,
    std::vector<ECS2::Entity::Id> templateParams,
    const std::string& funcName,
    std::vector<ECS2::Entity::Id> templateExplicitParameters,
    ECS2::Entity::Id parentEntityId,
    std::vector<ECS2::Entity::Id> childEntityIds) {
    ECS2::Entity::Id functionEntityId = system->CreateEntity();
    CPP__TREE__DECL__CREATE_FUNCTION_COMPONENTS(
        system,
        functionEntityId,
        returnValueEntityId,
        templateParams,
        funcName,
        templateExplicitParameters,
        parentEntityId,
        childEntityIds);
    return functionEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_PUBLIC_METHOD(
    System* system,
    ECS2::Entity::Id returnValueEntityId,
    const std::string& funcName,
    ECS2::Entity::Id parentEntityId,
    std::vector<ECS2::Entity::Id> childEntityIds) {
    ECS2::Entity::Id functionEntityId = CPP__TREE__DECL__CREATE_FUNCTION(
        system, returnValueEntityId, funcName, parentEntityId, childEntityIds);

    system->template CreateComponent<CPP::Tree::Access::Public_>(functionEntityId);
    return functionEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_PUBLIC_STATIC_METHOD(
    System* system,
    ECS2::Entity::Id returnValueEntityId,
    const std::string& funcName,
    ECS2::Entity::Id parentEntityId,
    std::vector<ECS2::Entity::Id> childEntityIds) {
    ECS2::Entity::Id functionEntityId = CPP__TREE__DECL__CREATE_PUBLIC_METHOD(
        system, returnValueEntityId, funcName, parentEntityId, childEntityIds);

    system->template CreateComponent<CPP::Tree::StorageClass::Static_>(functionEntityId);
    return functionEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_PUBLIC_STATIC_INLINE_METHOD(
    System* system,
    ECS2::Entity::Id returnValueEntityId,
    const std::string& funcName,
    ECS2::Entity::Id parentEntityId,
    std::vector<ECS2::Entity::Id> childEntityIds) {
    ECS2::Entity::Id functionEntityId = CPP__TREE__DECL__CREATE_PUBLIC_STATIC_METHOD(
        system, returnValueEntityId, funcName, parentEntityId, childEntityIds);

    system->template CreateComponent<CPP::Tree::Inline_>(functionEntityId);
    return functionEntityId;
}

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_USING_DIRECTIVE(
    System* system,
    std::string namespaceNameStr) {
    ECS2::Entity::Id usingDirectiveEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Decl::Tag>(usingDirectiveEntityId);
    system->template CreateComponent<CPP::Tree::Decl::UsingDirective>(
        usingDirectiveEntityId, namespaceNameStr);
    return usingDirectiveEntityId;
}

/* ======================== TEMPLATE TYPE PARAMETER ======================== */

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_TEMPLATE_TYPE_PARAMETER(
    System* system,
    const std::string& name,
    ECS2::Entity::Id defaultTypeEntityId,
    bool isPack) {
    ECS2::Entity::Id templateTypeParameterEntityId = system->CreateEntity();
    system->template CreateComponent<CPP::Tree::Decl::Tag>(templateTypeParameterEntityId);
    system->template CreateComponent<CPP::Tree::Decl::TemplateTypeParam>(
        templateTypeParameterEntityId, name, defaultTypeEntityId, isPack);
    return templateTypeParameterEntityId;
}

/* ======================== CLASS DECLARATION ======================== */

template<class System>
void CPP__TREE__DECL__CREATE_CLASS_COMPONENTS(
    System* system,
    ECS2::Entity::Id classEntityId,
    const std::string& className,
    ECS2::Entity::Id parentEntityId,
    std::vector<ECS2::Entity::Id> childEntityIds) {
    system->template CreateComponent<CPP::Tree::Node::Tag>(classEntityId);
    system->template CreateComponent<CPP::Tree::Decl::Tag>(classEntityId);
    system->template CreateComponent<CPP::Tree::Decl::Class_>(classEntityId);
    system->template CreateComponent<CPP::Tree::Node::Name>(classEntityId, className);

    if (parentEntityId.IsValid()) {
        system->template CreateComponent<CPP::Tree::Node::ParentEntityId>(
            classEntityId, parentEntityId);
    }
    if (!childEntityIds.empty()) {
        system->template CreateComponent<CPP::Tree::Node::ChildEntityIds>(
            classEntityId, childEntityIds);
    }
}

template<class System>
ECS2::Entity::Id CPP__TREE__DECL__CREATE_CLASS(
    System* system,
    const std::string& className) {
    ECS2::Entity::Id classEntityId = system->CreateEntity();
    CPP__TREE__DECL__CREATE_CLASS_COMPONENTS(
        system, classEntityId, className, ECS2::Entity::Id::invalid_, {});
    return classEntityId;
}

/* ======================== HELPERS ======================== */

template<class System>
ECS2::Entity::Id CPP__TREE__EXPR__CREATE_CALL_EXPR_NO_PARAMETERS(
    System* system,
    const std::string& identifier) {
    return CPP__TREE__EXPR__CREATE_CALL_EXPR(
        system,
        CPP__TREE__EXPR__CREATE_IDENTIFIER_EXPR(system, identifier),
        {},
        {});
}

template<class System>
std::vector<ECS2::Entity::Id> CPP__TREE__TYPE__CREATE_NAMED_TYPE_ENTITIES_VECTOR(
    System* system,
    const std::vector<std::string>& names) {
    std::vector<ECS2::Entity::Id> typeEntityIds;
    for (const std::string& name : names) {
        typeEntityIds.push_back(CPP__TREE__TYPE__CREATE_NAMED_TYPE(system, name));
    }
    return typeEntityIds;
}