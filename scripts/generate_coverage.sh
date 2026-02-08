#!/bin/bash
# Generate code coverage report from instrumented build
#
# Prerequisites:
#   - lcov and genhtml installed
#   - Tests must have been run with coverage preset
#
# Usage:
#   ./scripts/generate_coverage.sh [build_dir]

set -e

# Default to build-coverage directory
BUILD_DIR="${1:-build-coverage}"
COVERAGE_DIR="${BUILD_DIR}/coverage"
COVERAGE_INFO="${BUILD_DIR}/coverage.info"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== YAPL Code Coverage Report Generator ===${NC}"

# Check if lcov is installed
if ! command -v lcov &> /dev/null; then
    echo -e "${RED}Error: lcov is not installed${NC}"
    echo "Install with: sudo apt-get install lcov"
    exit 1
fi

# Check if build directory exists
if [ ! -d "${BUILD_DIR}" ]; then
    echo -e "${RED}Error: Build directory '${BUILD_DIR}' not found${NC}"
    echo "Run: cmake --preset coverage-linux && cmake --build ${BUILD_DIR}"
    exit 1
fi

# Check if coverage data exists
GCDA_COUNT=$(find "${BUILD_DIR}" -name "*.gcda" 2>/dev/null | wc -l)
if [ "${GCDA_COUNT}" -eq 0 ]; then
    echo -e "${YELLOW}Warning: No coverage data found (.gcda files)${NC}"
    echo "Run tests first: ctest --test-dir ${BUILD_DIR}"
    exit 1
fi

echo -e "${GREEN}Found ${GCDA_COUNT} coverage data files${NC}"

# Create coverage directory
mkdir -p "${COVERAGE_DIR}"

# Capture coverage data
echo -e "${GREEN}Capturing coverage data...${NC}"
lcov --capture \
     --directory "${BUILD_DIR}" \
     --output-file "${COVERAGE_INFO}" \
     --rc lcov_branch_coverage=1 \
     2>&1 | grep -v "ignoring data for external file" || true

# Remove unwanted coverage (system headers, tests, third-party)
echo -e "${GREEN}Filtering coverage data...${NC}"
lcov --remove "${COVERAGE_INFO}" \
     '/usr/*' \
     '*/tests/*' \
     '*/build*/_deps/*' \
     '*/examples/*' \
     '*/benchmarks/*' \
     --output-file "${COVERAGE_INFO}" \
     --rc lcov_branch_coverage=1 \
     2>&1 | grep -v "ignoring data for external file" || true

# Generate HTML report
echo -e "${GREEN}Generating HTML report...${NC}"
genhtml "${COVERAGE_INFO}" \
        --output-directory "${COVERAGE_DIR}" \
        --title "YAPL Code Coverage" \
        --legend \
        --show-details \
        --branch-coverage \
        --demangle-cpp \
        2>&1 | tail -n 20

# Display summary
echo ""
echo -e "${GREEN}=== Coverage Summary ===${NC}"
lcov --list "${COVERAGE_INFO}" --rc lcov_branch_coverage=1 | tail -n 20

# Calculate overall coverage percentage
COVERAGE_PCT=$(lcov --summary "${COVERAGE_INFO}" 2>&1 | grep "lines......:" | awk '{print $2}' | sed 's/%//')

echo ""
if (( $(echo "${COVERAGE_PCT} >= 80.0" | bc -l) )); then
    echo -e "${GREEN}✅ Coverage: ${COVERAGE_PCT}% (>= 80% target)${NC}"
elif (( $(echo "${COVERAGE_PCT} >= 60.0" | bc -l) )); then
    echo -e "${YELLOW}⚠️  Coverage: ${COVERAGE_PCT}% (60-80%, needs improvement)${NC}"
else
    echo -e "${RED}❌ Coverage: ${COVERAGE_PCT}% (< 60%, critical)${NC}"
fi

echo ""
echo -e "${GREEN}HTML report generated at:${NC}"
echo "  file://$(realpath ${COVERAGE_DIR}/index.html)"
echo ""
echo "Open with:"
echo "  xdg-open ${COVERAGE_DIR}/index.html"
echo ""
