#ifndef SEMA_PHASE1_H
#define SEMA_PHASE1_H

#include "Sema/SemaCommon.h"
#include "sona/either.h"

#include "AST/Expr.h"

namespace ckx {
namespace Sema {

class SemaPhase1 : public SemaCommon {
public:
  SemaPhase1(AST::ASTContext &astContext,
             std::vector<sona::ref_ptr<AST::DeclContext>> &declContexts,
             Diag::DiagnosticEngine &diag);

  void PostTranslateIncompletes(
      std::vector<sona::ref_ptr<Sema::IncompleteDecl>> incompletes);

  void TranslateFunctions(std::vector<IncompleteFuncDecl> &funcs);

protected:
  AST::QualType ResolveType(std::shared_ptr<Scope> scope,
                            sona::ref_ptr<Syntax::Type const> type);
  sona::owner<AST::Expr> ActOnExpr(std::shared_ptr<Scope> scope,
                                   sona::ref_ptr<Syntax::Expr const> expr);

#define CST_TYPE(name) \
  AST::QualType \
  Resolve##name(std::shared_ptr<Scope> scope, \
                sona::ref_ptr<Syntax::name const> type);

#define CST_EXPR(name) \
  sona::owner<AST::Expr> \
  ActOn##name(std::shared_ptr<Scope> scope, \
              sona::ref_ptr<Syntax::name const> type);

protected:
  sona::owner<AST::Expr>
  ActOnStaticCast(sona::ref_ptr<const Syntax::CastExpr> concrete,
                  sona::owner<AST::Expr> &&castedExpr,
                  AST::QualType destType);

  /// @note this function does not always "move away" or "consume" the input
  /// `castedExpr`, it only consumes its input when the cast is valid, and
  /// its return value is not nullptr. This'll get refactored by sometime, but
  /// let us keep it until we can make changes.
  sona::owner<AST::Expr>
  TryImplicitCast(sona::ref_ptr<Syntax::Expr const> concrete,
                  sona::owner<AST::Expr> &&castedExpr,
                  AST::QualType destType, bool shouldDiag = false);

  bool
  TryNumericPromotion(AST::Expr::ValueCat castedExprValueCat,
                      AST::QualType fromType, AST::QualType destType,
                      sona::ref_ptr<AST::BuiltinType const> fromBtin,
                      sona::ref_ptr<AST::BuiltinType const> destBtin,
                      std::vector<AST::CastStep> &outputVec);

  void DoNumericCast(AST::QualType fromType, AST::QualType destType,
                     sona::ref_ptr<AST::BuiltinType const> fromBtin,
                     sona::ref_ptr<AST::BuiltinType const> destBtin,
                     std::vector<AST::CastStep> &outputVec);

  bool
  TryPointerCast(AST::QualType formType, AST::QualType destType,
                 sona::ref_ptr<AST::BuiltinType const> fromBtin,
                 sona::ref_ptr<AST::BuiltinType const> destBtin,
                 std::vector<AST::CastStep> &outputVec);

  sona::owner<AST::Expr> LValueToRValueDecay(sona::owner<AST::Expr> &&expr);

protected:
  sona::owner<AST::Expr>
  TryFindUnaryOperatorOverload(sona::ref_ptr<AST::Expr const> baseExpr,
                               Syntax::UnaryOperator uop);

  sona::owner<AST::Expr>
  CreateOrAddImplicitCast(sona::owner<AST::Expr> &&expr,
                          AST::CastStep::CastStepKind castStepKind,
                          AST::QualType destType,
                          AST::Expr::ValueCat destValueCat);

  std::int8_t SIntRank(AST::BuiltinType::BuiltinTypeId btid);
  std::int8_t UIntRank(AST::BuiltinType::BuiltinTypeId btid);
  std::int8_t FloatRank(AST::BuiltinType::BuiltinTypeId btid);

  void PostTranslateIncompleteVar(sona::ref_ptr<Sema::IncompleteVarDecl> iVar);
  void PostTranslateIncompleteTag(sona::ref_ptr<Sema::IncompleteTagDecl> iTag);
  void PostTranslateIncompleteValueCtor(
      sona::ref_ptr<Sema::IncompleteValueCtorDecl> iValueCtor);
  void PostTranslateIncompleteUsing(
      sona::ref_ptr<Sema::IncompleteUsingDecl> iusing);

#include "Syntax/Nodes.def"
};

} // namespace Sema
} // namespace ckx

#endif // SEMA_PHASE1_H
