#pragma once
#include "core/sketch/Sketch.h"
#include <cstddef>
namespace microsw::constraints {
enum class SolveStatus { Solved, Unsatisfied, InvalidInput, NumericalFailure };
struct SolverPolicy { math::Scalar stepTolerance{1e-10}, costReductionTolerance{1e-12}, residualSatisfactionTolerance{1e-8}, finiteDifferenceRelativeStep{1e-6}; std::size_t maxIterations{100}; math::Scalar initialDamping{1e-3}, dampingAdjustment{10}, initialStateRegularization{1e-8}; };
struct SolveDiagnostics { std::size_t iterations{}; math::Scalar initialCost{}, finalCost{}, maxLogicalResidual{}; bool likelyUnderConstrained{}; };
struct SolveResult { SolveStatus status{SolveStatus::InvalidInput}; SolveDiagnostics diagnostics; };
class ConstraintSolver { public: explicit ConstraintSolver(SolverPolicy policy = {}); [[nodiscard]] SolveResult solve(sketch::Sketch&) const noexcept; private: SolverPolicy policy_; };
}
