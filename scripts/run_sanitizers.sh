#!/bin/bash
# Run all sanitizer builds and tests
#
# Usage:
#   ./scripts/run_sanitizers.sh [asan|tsan|ubsan|all]

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

SANITIZER="${1:-all}"

run_asan() {
    echo -e "${BLUE}=== AddressSanitizer (Memory Errors) ===${NC}"

    # Build
    echo -e "${GREEN}Building with ASan...${NC}"
    cmake --preset asan-linux
    cmake --build build-asan

    # Run tests
    echo -e "${GREEN}Running tests with ASan...${NC}"
    export ASAN_OPTIONS="detect_leaks=1:check_initialization_order=1:strict_init_order=1:halt_on_error=0"

    if ctest --test-dir build-asan --output-on-failure; then
        echo -e "${GREEN}✅ ASan: No memory errors detected${NC}"
        return 0
    else
        echo -e "${RED}❌ ASan: Memory errors found${NC}"
        return 1
    fi
}

run_tsan() {
    echo -e "${BLUE}=== ThreadSanitizer (Race Conditions) ===${NC}"

    # Build
    echo -e "${GREEN}Building with TSan...${NC}"
    cmake --preset tsan-linux
    cmake --build build-tsan

    # Run tests
    echo -e "${GREEN}Running tests with TSan...${NC}"
    export TSAN_OPTIONS="second_deadlock_stack=1:halt_on_error=0"

    if ctest --test-dir build-tsan --output-on-failure; then
        echo -e "${GREEN}✅ TSan: No race conditions detected${NC}"
        return 0
    else
        echo -e "${RED}❌ TSan: Race conditions found${NC}"
        return 1
    fi
}

run_ubsan() {
    echo -e "${BLUE}=== UndefinedBehaviorSanitizer (UB Detection) ===${NC}"

    # Build
    echo -e "${GREEN}Building with UBSan...${NC}"
    cmake --preset ubsan-linux
    cmake --build build-ubsan

    # Run tests
    echo -e "${GREEN}Running tests with UBSan...${NC}"
    export UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=0"

    if ctest --test-dir build-ubsan --output-on-failure; then
        echo -e "${GREEN}✅ UBSan: No undefined behavior detected${NC}"
        return 0
    else
        echo -e "${RED}❌ UBSan: Undefined behavior found${NC}"
        return 1
    fi
}

# Track results
ASAN_RESULT=0
TSAN_RESULT=0
UBSAN_RESULT=0

# Run requested sanitizers
case "${SANITIZER}" in
    asan)
        run_asan || ASAN_RESULT=$?
        ;;
    tsan)
        run_tsan || TSAN_RESULT=$?
        ;;
    ubsan)
        run_ubsan || UBSAN_RESULT=$?
        ;;
    all)
        echo -e "${BLUE}Running all sanitizers...${NC}\n"

        run_asan || ASAN_RESULT=$?
        echo ""

        run_tsan || TSAN_RESULT=$?
        echo ""

        run_ubsan || UBSAN_RESULT=$?
        echo ""
        ;;
    *)
        echo -e "${RED}Unknown sanitizer: ${SANITIZER}${NC}"
        echo "Usage: $0 [asan|tsan|ubsan|all]"
        exit 1
        ;;
esac

# Print summary
echo -e "${BLUE}=== Summary ===${NC}"
if [ "${SANITIZER}" = "all" ] || [ "${SANITIZER}" = "asan" ]; then
    if [ ${ASAN_RESULT} -eq 0 ]; then
        echo -e "  ASan:  ${GREEN}✅ PASS${NC}"
    else
        echo -e "  ASan:  ${RED}❌ FAIL${NC}"
    fi
fi

if [ "${SANITIZER}" = "all" ] || [ "${SANITIZER}" = "tsan" ]; then
    if [ ${TSAN_RESULT} -eq 0 ]; then
        echo -e "  TSan:  ${GREEN}✅ PASS${NC}"
    else
        echo -e "  TSan:  ${RED}❌ FAIL${NC}"
    fi
fi

if [ "${SANITIZER}" = "all" ] || [ "${SANITIZER}" = "ubsan" ]; then
    if [ ${UBSAN_RESULT} -eq 0 ]; then
        echo -e "  UBSan: ${GREEN}✅ PASS${NC}"
    else
        echo -e "  UBSan: ${RED}❌ FAIL${NC}"
    fi
fi

# Exit with error if any sanitizer failed
TOTAL_ERRORS=$((ASAN_RESULT + TSAN_RESULT + UBSAN_RESULT))
if [ ${TOTAL_ERRORS} -ne 0 ]; then
    echo -e "\n${RED}Sanitizer tests failed${NC}"
    exit 1
else
    echo -e "\n${GREEN}All sanitizer tests passed${NC}"
    exit 0
fi
