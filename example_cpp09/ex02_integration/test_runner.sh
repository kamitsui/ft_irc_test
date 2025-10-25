#!/bin/bash

# --- Color Definitions ---
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# --- Variables ---
PRJ_DIR="../../../reference/src_cpp09_ex02"
EXECUTABLE="./$PRJ_DIR/PmergeMe"
TMP_OUTPUT="tmp_output.log"
TMP_ERROR="tmp_error.log"
TEST_COUNT=0
PASS_COUNT=0

# --- Pre-flight Check ---
if [ ! -f "$EXECUTABLE" ]; then
    echo -e "${RED}Error: Executable '$EXECUTABLE' not found.${NC}"
    echo -e "${YELLOW}Please compile the program first with 'make'.${NC}"
    exit 1
fi

# --- Test Function ---
# $1: Test case description
# $2: Command to execute
# $3: Expected stdout content (use "CHECK_SORT" for dynamic check)
# $4: Expected stderr content
run_test() {
    ((TEST_COUNT++))
    DESCRIPTION=$1
    COMMAND=$2
    EXPECTED_STDOUT_CONTENT=$3
    EXPECTED_STDERR_CONTENT=$4

    # Execute the command and capture output/error
    eval "$COMMAND" > "$TMP_OUTPUT" 2> "$TMP_ERROR"

    local PASS=true

    # Check stdout
    if [ "$EXPECTED_STDOUT_CONTENT" == "CHECK_SORT" ]; then
        # Dynamic check for correctness
        BEFORE_LINE=$(grep "Before:" "$TMP_OUTPUT")
        AFTER_LINE=$(grep "After:" "$TMP_OUTPUT")
        TIME_VEC_LINE=$(grep "Time to process.*std::vector" "$TMP_OUTPUT")
        TIME_DEQ_LINE=$(grep "Time to process.*std::deque" "$TMP_OUTPUT")

        # Check if all required lines exist
        if [ -z "$BEFORE_LINE" ] || [ -z "$AFTER_LINE" ] || [ -z "$TIME_VEC_LINE" ] || [ -z "$TIME_DEQ_LINE" ]; then
            PASS=false
        else
            # Extract numbers and check if 'After' is sorted
            AFTER_NUMS=$(echo "$AFTER_LINE" | sed 's/After: *//')
            SORTED_NUMS=$(echo "$AFTER_NUMS" | tr ' ' '\n' | sort -n | tr '\n' ' ')
            # Trim trailing space
            AFTER_NUMS_TRIMMED=$(echo "$AFTER_NUMS" | sed 's/ *$//')
            SORTED_NUMS_TRIMMED=$(echo "$SORTED_NUMS" | sed 's/ *$//')

            if [ "$AFTER_NUMS_TRIMMED" != "$SORTED_NUMS_TRIMMED" ]; then
                PASS=false
            fi
        fi
    else
        # Static check against expected content
        if ! echo -n "$EXPECTED_STDOUT_CONTENT" | diff -q - "$TMP_OUTPUT" > /dev/null; then
            PASS=false
        fi
    fi

    #### debug ####
    #echo "--- $PASS ---"
    #echo "ex:"
    #echo -n "$EXPECTED_STDOUT_CONTENT"
    #echo "ac:"
    #cat "$TMP_OUTPUT"
    #echo "-------------"

    # Check stderr
    #if ! echo -n "$EXPECTED_STDERR_CONTENT" | diff -q - "$TMP_ERROR" > /dev/null; then
    if ! printf "%b" "$EXPECTED_STDERR_CONTENT" | diff -q - "$TMP_ERROR" > /dev/null; then
         PASS=false
    fi

    #### debug ####
    #echo "--- $PASS ---"
    #echo "ex:"
    #printf "%b" "$EXPECTED_STDERR_CONTENT"
    #echo "ac:"
    #cat "$TMP_ERROR"
    #echo "-------------"
    #return 1

    # Print result
    if [ "$PASS" = true ]; then
        echo -e "[ ${GREEN}OK${NC} ] $DESCRIPTION"
        ((PASS_COUNT++))
    else
        echo -e "[ ${RED}KO${NC} ] $DESCRIPTION"
        if [ "$EXPECTED_STDOUT_CONTENT" == "CHECK_SORT" ]; then
            echo "--- Validation Failed for Sorted Output ---"
            echo "Sorted numbers should be: $SORTED_NUMS_TRIMMED"
            echo "Actual 'After:' line was: $AFTER_NUMS_TRIMMED"
        else
            echo "--- Expected STDOUT ---"
            echo "$EXPECTED_STDOUT_CONTENT"
        fi
        echo "--- Actual STDOUT ---"
        cat "$TMP_OUTPUT"
        echo "--------------------"
        echo "--- Expected STDERR ---"
        echo "$EXPECTED_STDERR_CONTENT"
        echo "--- Actual STDERR ---"
        cat "$TMP_ERROR"
        echo "--------------------"
    fi
}

# --- Test Execution ---
echo "--- Running Integration Tests for PmergeMe ---"

# 1. Normal Case from subject
run_test "Normal Case: Subject example" \
         "$EXECUTABLE 3 5 9 7 4" \
         "CHECK_SORT" \
         ""

# 2. Argument Errors
run_test "Argument Error: No arguments" \
         "$EXECUTABLE" \
         "" \
         "Error: No input sequence provided.\n"

# 3. Input Validation Errors
run_test "Input Error: Negative number" \
         "$EXECUTABLE 5 -1 2" \
         "" \
         "Error: Invalid input.\n"

run_test "Input Error: Non-numeric input" \
         "$EXECUTABLE 5 three 2" \
         "" \
         "Error: Invalid input.\n"

run_test "Input Error: Zero (not a positive integer)" \
         "$EXECUTABLE 5 0 2" \
         "" \
         "Error: Invalid input.\n"

# 4. Edge Cases
run_test "Edge Case: Already sorted" \
         "$EXECUTABLE 1 2 3 4 5" \
         "CHECK_SORT" \
         ""

run_test "Edge Case: Reverse sorted" \
         "$EXECUTABLE 5 4 3 2 1" \
         "CHECK_SORT" \
         ""

run_test "Edge Case: With duplicates" \
         "$EXECUTABLE 5 8 2 5 2" \
         "CHECK_SORT" \
         ""

# 5. Large Data Test (3000 elements)
echo -e "${YELLOW}Running large data test (3000 elements)...${NC}"
# Use jot on macOS or shuf on Linux
if command -v jot &> /dev/null; then
    LARGE_INPUT=$(jot -r 3000 1 100000 | tr '\n' ' ')
else
    LARGE_INPUT=$(shuf -i 1-100000 -n 3000 | tr '\n' ' ')
fi
run_test "Large Data: 3000 random elements" \
         "$EXECUTABLE $LARGE_INPUT" \
         "CHECK_SORT" \
         ""


# --- Cleanup ---
rm -f "$TMP_OUTPUT" "$TMP_ERROR"

# --- Summary ---
echo "---------------------------------------------------"
if [ "$PASS_COUNT" -eq "$TEST_COUNT" ]; then
    echo -e "${GREEN}All $TEST_COUNT tests passed successfully!${NC}"
else
    echo -e "${RED}$((TEST_COUNT - PASS_COUNT)) out of $TEST_COUNT tests failed.${NC}"
fi
echo "---------------------------------------------------"
