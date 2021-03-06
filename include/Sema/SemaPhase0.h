#ifndef SEMA_PHASE0_H
#define SEMA_PHASE0_H

#include "Sema/SemaCommon.h"
#include "sona/either.h"

namespace ckx {
namespace Sema {

class SemaPhase0 : public SemaCommon {
public:
  SemaPhase0(AST::ASTContext &astContext,
             std::vector<sona::ref_ptr<AST::DeclContext>> &declContexts,
             Diag::DiagnosticEngine &diag);

  sona::owner<AST::TransUnitDecl>
  ActOnTransUnit(sona::ref_ptr<Syntax::TransUnit> transUnit);

  void PostSubstituteDepends();

  std::vector<sona::ref_ptr<Sema::IncompleteDecl>> FindTranslationOrder();
  std::vector<Sema::IncompleteFuncDecl> &GetIncompleteFuncs();

protected:
  sona::either<AST::QualType, std::vector<Dependency>>
  ResolveType(sona::ref_ptr<Syntax::Type const> type);

  std::pair<sona::owner<AST::Decl>, bool>
  ActOnDecl(sona::ref_ptr<Syntax::Decl const> decl);

#define CST_TYPE(name) \
  sona::either<AST::QualType, std::vector<Dependency>> \
  Resolve##name(sona::ref_ptr<Syntax::name const> type);

#define CST_DECL(name) \
  std::pair<sona::owner<AST::Decl>, bool> \
  ActOn##name(sona::ref_ptr<Syntax::name const> decl);

#include "Syntax/Nodes.def"

  std::pair<sona::owner<AST::Decl>, bool>
  ActOnValueConstructor(
      sona::ref_ptr<Syntax::ADTDecl::ValueConstructor const> dc);

  sona::ref_ptr<IncompleteDecl>
  SearchInUnfinished(sona::ref_ptr<AST::Decl const> decl);

protected:
  bool CheckTypeComplete(sona::ref_ptr<AST::Type const> type);
  bool CheckUserDefinedTypeComplete(
      sona::ref_ptr<AST::UserDefinedType const> type);

  std::vector<Syntax::Export> m_Exports;

  std::unordered_map<sona::ref_ptr<AST::VarDecl const>,
                     Sema::IncompleteVarDecl>
    m_IncompleteVars;
  std::unordered_map<sona::ref_ptr<AST::Decl const>,
                     Sema::IncompleteTagDecl>
    m_IncompleteTags;
  std::unordered_map<sona::ref_ptr<AST::UsingDecl const>,
                     Sema::IncompleteUsingDecl>
    m_IncompleteUsings;
  std::unordered_map<sona::ref_ptr<AST::ValueCtorDecl const>,
                     Sema::IncompleteValueCtorDecl>
    m_IncompleteValueCtors;
  std::vector<Sema::IncompleteFuncDecl> m_IncompleteFuncs;
};

} // namespace Sema
} // namespace ckx

#endif // SEMA_PHASE0_H
