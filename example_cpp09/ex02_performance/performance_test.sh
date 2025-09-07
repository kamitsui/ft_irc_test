#!/bin/bash

# This script runs the PmergeMe program with different large datasets
# to measure and compare the performance of std::vector and std::deque.

# --- Color Definitions ---
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# --- Variables ---
PRJ_DIR="../../../ft_irc/src_cpp09_ex02"
EXECUTABLE="./$PRJ_DIR/PmergeMe"
NUM_GENERATOR="./generate_numbers.sh"

# Array of dataset sizes to test
TEST_SIZES=(500 3000 5000 10000)

# --- Pre-flight Checks ---
if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${RED}Error: Executable '$EXECUTABLE' not found.${NC}"
    echo -e "${YELLOW}Please compile the program first with 'make'.${NC}"
    exit 1
fi
if [ ! -x "$NUM_GENERATOR" ]; then
    echo -e "${RED}Error: Number generator '$NUM_GENERATOR' is not executable.${NC}"
    echo -e "${YELLOW}Please run 'chmod +x $NUM_GENERATOR'.${NC}"
    exit 1
fi

# --- Test Execution ---
echo -e "--- Running Performance Tests for PmergeMe ---"
echo "This will run the sort on datasets of various sizes."
echo "Observe the time reported for std::vector vs std::deque."

for N in "${TEST_SIZES[@]}"; do
    echo ""
    echo -e "${YELLOW}=====================================================${NC}"
    echo -e "${YELLOW}  Testing with $N elements...${NC}"
    echo -e "${YELLOW}=====================================================${NC}"

    # 1. Generate the large sequence of numbers
    echo "Generating $N random numbers..."
    INPUT_SEQUENCE=$("$NUM_GENERATOR" "$N")

    # 2. Run the PmergeMe program using xargs to avoid "Argument list too long"
    # The PmergeMe program will print its own timing results.
    echo "$INPUT_SEQUENCE" | xargs "$EXECUTABLE"

    if [ $? -ne 0 ]; then
        echo -e "${RED}Error: PmergeMe failed for $N elements.${NC}"
    fi
done

# --- Summary ---
echo ""
echo -e "${GREEN}=====================================================${NC}"
echo -e "${GREEN}  Performance Test Finished${NC}"
echo -e "${GREEN}=====================================================${NC}"
echo "Review the output above to compare the performance."
echo "Does std::vector or std::deque perform better as N increases?"
echo "This demonstrates the trade-off between random access speed and insertion cost."
