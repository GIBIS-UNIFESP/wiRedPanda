# Logic Element Coverage Improvement Plan

## Current Coverage Analysis

### Overall Statistics
- **Current Line Coverage**: 232/270 lines (85.9%)
- **Current Function Coverage**: 37/41 functions (90.2%)
- **Missing Coverage**: 38 lines and 4 functions need testing

### Coverage by File

#### 🔴 **Critical (0% Coverage) - HIGH PRIORITY**
1. **logicsrlatch.cpp**: 0/23 lines (0.0%), 0/2 functions (0.0%)
2. **logicnone.h**: 0/2 lines (0.0%), 0/2 functions (0.0%)

#### 🟡 **Medium Coverage (85-90%) - MEDIUM PRIORITY** 
3. **logicinput.cpp**: 7/8 lines (87.5%), 2/2 functions (100%)
4. **logicnand.cpp**: 7/8 lines (87.5%), 2/2 functions (100%)
5. **logicnode.cpp**: 6/7 lines (85.7%), 2/2 functions (100%)
6. **logicnor.cpp**: 7/8 lines (87.5%), 2/2 functions (100%)
7. **logicoutput.cpp**: 7/8 lines (87.5%), 2/2 functions (100%)
8. **logicxnor.cpp**: 7/8 lines (87.5%), 2/2 functions (100%)

#### 🟢 **Good Coverage (90-95%) - LOW PRIORITY**
9. **logicmux.cpp**: 9/10 lines (90.0%), 2/2 functions (100%)
10. **logicdemux.cpp**: 14/15 lines (93.3%), 2/2 functions (100%)
11. **logicdlatch.cpp**: 16/17 lines (94.1%), 2/2 functions (100%)
12. **logictruthtable.cpp**: 15/16 lines (93.8%), 3/3 functions (100%)

#### ✅ **Complete Coverage (95%+) - MAINTAIN**
13. **logicjkflipflop.cpp**: 31/32 lines (96.9%), 2/2 functions (100%)
14. **logicsrflipflop.cpp**: 27/28 lines (96.4%), 2/2 functions (100%)
15. **logictflipflop.cpp**: 24/25 lines (96.0%), 2/2 functions (100%)
16. **logicand.cpp**: 8/8 lines (100%), 2/2 functions (100%)
17. **logicdflipflop.cpp**: 24/24 lines (100%), 2/2 functions (100%)
18. **logicnot.cpp**: 7/7 lines (100%), 2/2 functions (100%)
19. **logicor.cpp**: 8/8 lines (100%), 2/2 functions (100%)
20. **logicxor.cpp**: 8/8 lines (100%), 2/2 functions (100%)

## Root Cause Analysis

### Missing Test Cases
1. **LogicSRLatch**: No test method exists in TestLogicElements
2. **LogicNone**: Header-only class, likely needs separate test approach
3. **Edge Cases**: Many elements missing edge case testing (invalid inputs, boundary conditions)
4. **Error Conditions**: Logic elements with single uncovered lines likely missing error handling tests

### Current Test Gaps
- **Constructor edge cases** not tested
- **Invalid input handling** not covered
- **State transitions** for latches/flip-flops incomplete
- **Multi-input scenarios** for complex elements missing

## Implementation Plan

### Phase 1: Critical Coverage (Target: 100% for 0% files)
**Priority: HIGH - Missing fundamental logic elements**

#### Task 1.1: LogicSRLatch Complete Coverage
- **File**: `logicsrlatch.cpp` (0% → 100%)
- **Lines to Cover**: 23 lines, 2 functions
- **Implementation**: Add `testLogicSRLatch()` method to TestLogicElements
- **Test Cases**:
  - Basic S-R latch truth table (S=0,R=0 / S=0,R=1 / S=1,R=0)
  - Forbidden state (S=1,R=1) behavior
  - State persistence when both inputs are 0
  - Output validation (Q and !Q complementary)
- **Estimated Impact**: +23 lines coverage

#### Task 1.2: LogicNone Coverage Strategy
- **File**: `logicnone.h` (0% → 100%)
- **Lines to Cover**: 2 lines, 2 functions
- **Analysis Needed**: Determine if LogicNone is actually used in production code
- **Options**:
  - Create dedicated test if used
  - Document as dead code if unused
  - Mark as excluded from coverage if intentionally empty
- **Estimated Impact**: +2 lines coverage

### Phase 2: Medium Coverage Enhancement (Target: 95%+ for 85-90% files)
**Priority: MEDIUM - Improve existing partial coverage**

#### Task 2.1-2.6: Single Line Coverage Gaps
**Pattern**: Most medium coverage files missing exactly 1 line (8/8 files)
- **Files**: logicinput, logicnand, logicnor, logicoutput, logicxnor, logicnode
- **Typical Issue**: Constructor edge cases or error handling paths
- **Strategy**: Analyze uncovered lines and add specific edge case tests
- **Implementation**: Extend existing test methods with edge cases
- **Estimated Impact**: +6 lines coverage (1 per file)

### Phase 3: Coverage Completion (Target: 100% for 90-95% files)  
**Priority: LOW - Final coverage optimization**

#### Task 3.1-3.4: Final Line Coverage
- **Files**: logicmux (1 line), logicdemux (1 line), logicdlatch (1 line), logictruthtable (1 line)
- **Strategy**: Identify specific uncovered edge cases
- **Estimated Impact**: +4 lines coverage

#### Task 3.5-3.7: Perfect Coverage Maintenance  
- **Files**: logicjkflipflop (1 line), logicsrflipflop (1 line), logictflipflop (1 line)
- **Strategy**: Complete the last uncovered lines in high-coverage files
- **Estimated Impact**: +3 lines coverage

## Detailed Task Breakdown

### 🔥 **CRITICAL TASKS (Must Complete)**

#### Task 1: Implement LogicSRLatch Testing
**Status**: Missing entirely
**Priority**: CRITICAL
**Implementation**:
```cpp
void TestLogicElements::testLogicSRLatch()
{
    LogicSRLatch elm;
    elm.connectPredecessor(0, switches.at(0), 0); // S input
    elm.connectPredecessor(1, switches.at(1), 0); // R input

    // Test truth table for S-R Latch
    const QVector<QVector<bool>> truthTable{
        // S, R, expectedQ, expectedQBar
        {0, 0, elm.outputValue(0), elm.outputValue(1)}, // Hold state
        {0, 1, false, true},  // Reset
        {1, 0, true, false},  // Set
        {1, 1, false, false}, // Forbidden (or specific behavior)
    };

    for (const auto &test : truthTable) {
        switches.at(0)->setOutputValue(test.at(0)); // S
        switches.at(1)->setOutputValue(test.at(1)); // R
        
        elm.updateLogic();
        
        QCOMPARE(elm.outputValue(0), test.at(2)); // Q output
        QCOMPARE(elm.outputValue(1), test.at(3)); // Q̄ output
    }
}
```

#### Task 2: Analyze LogicNone Usage
**Status**: Unknown purpose
**Priority**: CRITICAL
**Actions**:
1. Search codebase for LogicNone usage
2. Determine if it's dead code or serves specific purpose
3. Create appropriate test or mark as excluded

### ⚡ **HIGH IMPACT TASKS**

#### Task 3-8: Edge Case Coverage for 87.5% Files
**Files**: logicinput, logicnand, logicnor, logicoutput, logicxnor, logicnode
**Strategy**: Each file missing exactly 1 line - likely edge cases
**Implementation**: Extend existing tests with:
- Invalid input scenarios
- Boundary conditions
- Error handling paths
- Constructor parameter validation

#### Task 9-12: Complete 90%+ Files
**Files**: logicmux, logicdemux, logicdlatch, logictruthtable
**Strategy**: Final edge case completion
**Estimated Effort**: Low (1 line each)

## Expected Results

### Coverage Targets
- **Post-Phase 1**: 85.9% → 95.9% (+25 lines)
- **Post-Phase 2**: 95.9% → 98.1% (+6 lines)  
- **Post-Phase 3**: 98.1% → 100% (+7 lines)
- **Final Target**: 270/270 lines (100%), 41/41 functions (100%)

### Quality Improvements
- **Complete logic element coverage** ensuring all digital logic components tested
- **Edge case robustness** with comprehensive error handling validation
- **Behavioral verification** for all S-R latch state transitions
- **Regression prevention** for critical simulation logic

### Risk Assessment
- **Low Risk**: Most tasks involve extending existing proven test patterns
- **Medium Risk**: LogicSRLatch implementation (new test method)
- **Unknown Risk**: LogicNone purpose determination

## Success Metrics
1. **Coverage**: Achieve 100% line and function coverage for app/logicelement/
2. **Quality**: All new tests follow existing patterns and maintain test reliability
3. **Completeness**: No logic element left untested
4. **Maintainability**: Tests are clear, documented, and follow project conventions

## Timeline Estimate
- **Phase 1**: 2-3 hours (LogicSRLatch + LogicNone analysis)
- **Phase 2**: 1-2 hours (6 single-line fixes)
- **Phase 3**: 1 hour (final cleanup)
- **Total**: 4-6 hours for complete coverage improvement

This plan provides a systematic approach to achieve 100% coverage for the logic element directory while maintaining code quality and test reliability.