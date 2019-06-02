#include "Sema/SemaCommon.h"

namespace ckx {
namespace Sema {

void SemaCommon::PushDeclContext(sona::ref_ptr<AST::DeclContext> context) {
  m_DeclContexts.push_back(context);
}

void SemaCommon::PopDeclContext() {
  m_DeclContexts.pop_back();
}

sona::ref_ptr<AST::DeclContext> SemaCommon::GetCurrentDeclContext() {
  return m_DeclContexts.back();
}

std::shared_ptr<Scope> const& SemaCommon::GetCurrentScope() const noexcept {
  return m_CurrentScope;
}

std::shared_ptr<Scope> const& SemaCommon::GetGlobalScope() const noexcept {
  return m_GlobalScope;
}

void SemaCommon::PushScope(Scope::ScopeFlags flags) {
  std::shared_ptr<Scope> newScope(new Scope(GetCurrentScope(), flags));
  if (GetCurrentScope() == nullptr) {
    m_GlobalScope = newScope;
  }
  m_CurrentScope = newScope;
}

void SemaCommon::PopScope() {
  m_CurrentScope = m_CurrentScope->GetParentScope();
}

sona::ref_ptr<const AST::Type>
SemaCommon::ResolveBuiltinTypeImpl(
    sona::ref_ptr<const Syntax::BuiltinType> basicType) {
  AST::BuiltinType::BuiltinTypeId bid;
  /// @todo consider use tablegen to generate this, or unify the two
  /// "type kinds" enumeration
  switch (basicType->GetTypeKind()) {
  #define BUILTIN_TYPE(name, rep, size, isint, \
                       issigned, signedver, unsignedver) \
    case Syntax::BuiltinType::TypeKind::TK_##name: \
      bid = AST::BuiltinType::BuiltinTypeId::BTI_##name; break;
  #include "Syntax/BuiltinTypes.def"
  }

  return m_ASTContext.GetBuiltinType(bid);
}

sona::ref_ptr<AST::DeclContext const>
SemaCommon::ChooseDeclContext(std::shared_ptr<Scope> scope,
                              std::vector<sona::string_ref> const& nns,
                              bool shouldDiag,
                              std::vector<SingleSourceRange> const& nnsRanges) {
  sona::ref_ptr<AST::Type const> topLevelType = scope->LookupType(nns.front());
  if (topLevelType == nullptr) {
    if (shouldDiag) {
      m_Diag.Diag(Diag::DIR_Error,
                  Diag::Format(Diag::DMT_ErrNotDeclared, {nns.front()}),
                  nnsRanges.front());
    }
    return nullptr;
  }

  if (topLevelType->GetTypeId() != AST::Type::TypeId::TI_UserDefined) {
    m_Diag.Diag(Diag::DIR_Error,
                Diag::Format(Diag::DMT_ErrNotScope, {nns.front()}),
                nnsRanges.front());
    return nullptr;
  }

  sona::ref_ptr<AST::UserDefinedType const> udType =
      topLevelType.cast_unsafe<AST::UserDefinedType const>();
  if (udType->GetUserDefinedTypeId()
      == AST::UserDefinedType::UDTypeId::UTI_Using) {
    m_Diag.Diag(Diag::DIR_Error,
                Diag::Format(Diag::DMT_ErrNotScope, {nns.front()}),
                nnsRanges.front());
    return nullptr;
  }

  sona::ref_ptr<AST::DeclContext const> context
      = udType->GetTypeDecl()->CastAsDeclContext();

  auto r1 = sona::linq::from_container(nns)
                        .zip_with(sona::linq::from_container(nnsRanges));

  for (auto it = r1.begin() + 1; it != r1.end(); ++it) {
    std::vector<sona::ref_ptr<AST::Decl const>> collectedDecls;
    context->LookupDeclContexts((*it).first, collectedDecls);
    if (collectedDecls.size() < 1) {
      m_Diag.Diag(Diag::DIR_Error,
                  Diag::Format(Diag::DMT_ErrNotScope, {(*it).first}),
                  (*it).second);
      return nullptr;
    }
    if (collectedDecls.size() > 1) {
      m_Diag.Diag(Diag::DIR_Error,
                  Diag::Format(Diag::DMT_ErrAmbiguousScope,
                               {(*it).first, (*(it - 1)).first}), (*it).second);
    }
    context = collectedDecls.front()->CastAsDeclContext();
  }

  return context;
}

sona::ref_ptr<AST::Type const>
SemaCommon::LookupType(std::shared_ptr<Scope> scope,
                       const Syntax::Identifier& identifier, bool shouldDiag) {
  if (identifier.GetNestedNameSpecifiers().size() == 0) {
    sona::ref_ptr<AST::Type const> ret
        = scope->LookupType(identifier.GetIdentifier());
    if (ret == nullptr && shouldDiag) {
      m_Diag.Diag(Diag::DIR_Error,
                  Diag::Format(Diag::DMT_ErrNotDeclared,
                               { identifier.GetIdentifier() }),
                  identifier.GetIdSourceRange());
    }
    return ret;
  }

  sona::ref_ptr<AST::DeclContext const> context =
      ChooseDeclContext(scope, identifier.GetNestedNameSpecifiers(),
                        shouldDiag, identifier.GetNNSSourceRanges());
  if (context == nullptr) {
    return nullptr;
  }

  std::vector<sona::ref_ptr<AST::Decl const>> collectedDecls;
  context->LookupTypeDecl(identifier.GetIdentifier(), collectedDecls);

  if (collectedDecls.size() < 1) {
    if (shouldDiag) {
      m_Diag.Diag(Diag::DIR_Error,
                  Diag::Format(Diag::DMT_ErrNotDeclared,
                               { identifier.GetIdentifier() }),
                  identifier.GetIdSourceRange());
    }
    return nullptr;
  }

  if (collectedDecls.size() > 1) {
    if (shouldDiag) {
      m_Diag.Diag(Diag::DIR_Error,
                  Diag::Format(Diag::DMT_ErrAmbiguous,
                               { identifier.GetIdentifier() }),
                  identifier.GetIdSourceRange());
    }
    return nullptr;
  }

  return collectedDecls.front().cast_unsafe<AST::TypeDecl const>()
                       ->GetTypeForDecl();
}

} // namespace Sema
} // namespace ckx
