# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: kamitsui <kamitsui@student.42tokyo.jp>     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/03/15 19:38:29 by kamitsui          #+#    #+#              #
#    Updated: 2025/10/12 22:08:18 by kamitsui         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# @file example_cpp09/Makefile

# Directories
PRJ_ROOT = ../../ft_irc
# Default for the build rule (re, fclean, clean)
OBJ_DIR = obj
DEP_DIR = .dep

# Compiler
CXX = c++
CF_DEP = -MMD -MP -MF $(@:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d)

# Google Test Path without .env file.
# 42 tokyo campas PC : no define this variable in .env file.
CXXFLAGS ?= -Wall -Wextra -Werror -std=c++17 -I$(HOME)/goinfre/.brew/include -pthread
LIBS ?= -L$(HOME)/goinfre/.brew/lib -lgtest -lgtest_main

# Google Test Path with .env. ( My environment is Ubuntu22.04 )
#CXXFLAGS = -Wall -Wextra -Werror -std=c++14 -I/usr/include -pthread
#LIBS = -L/usr/lib -lgtest -lgtest_main

# if existing .env file, read variable.
# '-' initial hyphen is no error if '.env' is no existing.
-include .env

# ALL Targets
TARGET_EX00 = ex00_app
TARGET_EX01 = ex01_app
TARGET_EX02 = ex02_app
ALL = $(TARGET_EX02)# $(TARGET_EX00) $(TARGET_EX01)

# Definitions for building ex00 test program.
ifeq ($(MAKECMDGOALS),ex00)
EX_NUM = ex00
SRCS = test_bitcoin_exchange.cpp main.cpp BitcoinExchange.cpp
NAME = $(TARGET_EX00)
endif

# Definitions for building ex01 test program.
ifeq ($(MAKECMDGOALS),ex01)
EX_NUM = ex01
SRCS = test_rpn.cpp main.cpp RPN.cpp
NAME = $(TARGET_EX01)
endif

# Definitions for building ex02 test program.
ifeq ($(MAKECMDGOALS),ex02)
EX_NUM = src_cpp09_ex02
SRCS = test_pmergeme.cpp main.cpp PmergeMe.cpp
NAME = $(TARGET_EX02)
endif

# Selected Target Project Directory
PRJ_DIR = $(PRJ_ROOT)/$(EX_NUM)
SRCS_DIR = ./ ./ex02 $(PRJ_DIR)
CF_INC = -I$(PRJ_DIR) -I$(EX_NUM)
OBJ_DIR = obj/$(EX_NUM)
DEP_DIR = .dep/$(EX_NUM)

# vpath for serching source files in multiple directories
vpath %.cpp $(SRCS_DIR)

# Object files and dependency files
APP_OBJ = $(addprefix $(OBJ_DIR)/, $(APP_SRCS:.cpp=.o))
TEST_OBJ = $(addprefix $(OBJ_DIR)/, $(TEST_SRCS:.cpp=.o))
OBJ = $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))
DEP = $(addprefix $(DEP_DIR)/, $(SRCS:.cpp=.d))

# Rules for building object files
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(DEP_DIR)
	$(CXX) $(CXXFLAGS) $(CF_INC) $(CF_DEP) -c $< -o $@

# Rules for building dependency files
$(DEP_DIR)/%.d: %.cpp
	@mkdir -p $(DEP_DIR)

# Enable dependency file
-include $(DEP)

# Default target
all:
	make ex02
#	make ex00
#	make ex01
.PHONY: all

# Build Target
$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(LIBS) -o $(NAME)

# Rule for removing object & dependency files
clean:
	rm -rf $(OBJ_DIR) $(DEP_DIR)
.PHONY: clean

# Rule for removing Target & others
fclean: clean
	rm -f $(ALL)
.PHONY: fclean

# Rule for Clean & Build Target
re: fclean all
.PHONY: re

# Rule for ex00 target
ex00: $(NAME)
	@echo "Build" "'$(TARGET_EX00)'" "Complete!"
	$(call ASCII_ART,$(NAME))
	$(call ASK_AND_EXECUTE_ON_YES, ./$(NAME))

	$(call CONTINUE_NEXT, --- Running Integration Tests ---)
	@cd ${CURDIR}/ex00_integration && ./test_runner.sh

	$(call CONTINUE_NEXT, --- Running Performance Tests ---)
	@cd ${CURDIR}/ex00_performance && ./performance_test.sh
.PHONY: ex00

# Rule for ex01 target
ex01: $(NAME)
	@echo "Build" "'$(TARGET_EX01)'" "Complete!"
	$(call ASCII_ART,$(NAME))
	$(call ASK_AND_EXECUTE_ON_YES, ./$(NAME))

	$(call CONTINUE_NEXT, --- Running Integration Tests ---)
	@cd ${CURDIR}/ex01_integration && ./test_runner.sh

	$(call CONTINUE_NEXT, --- Running Performance Tests ---)
	@cd ${CURDIR}/ex01_performance && ./performance_test.sh
.PHONY: ex01

# Rule for ex02 target
ex02: $(NAME)
	@echo "Build" "'$(TARGET_EX02)'" "Complete!"
	$(call ASCII_ART,$(NAME))
	$(call ASK_AND_EXECUTE_ON_YES, ./$(NAME))

	@echo "--- Build in Project directory ---"
	$(call ASK_AND_EXECUTE_ON_YES, "make -C ${PRJ_DIR}")

	@echo "--- Running Integration Tests ---"
	@cd ${CURDIR}/ex02_integration && \
	$(call ASK_AND_EXECUTE_AND_EXIT, "integration tests", ./test_runner.sh)

	@echo "--- Running Performance Tests ---"
	@cd ${CURDIR}/ex02_performance && \
	$(call ASK_AND_EXECUTE_AND_EXIT, "performance tests", ./performance_test.sh)
.PHONY: ex02

# ASCII Art : Display Tips the way to use.
define ASCII_ART
	@echo " _____________________________________________"
	@echo "< Usage :                                     >"
	@echo "<  $$ ./$(1)                               >"
	@echo "----------------------------------------------"
	@echo "       \   ^__^"
	@echo "        \  (oo)\_______"
	@echo "           (__)\       )\\/\\"
	@echo "               ||----w |"
	@echo "               ||     ||\n"
endef

# Ask and Exit on No
define ASK_AND_EXECUTE_ON_YES
	@echo "Execute? $(1) (y/n)"
	@read -r answer; \
	case "$$answer" in \
		[nN]) \
			echo "Skip ..."; \
			;; \
		*) \
			echo $(1); \
			eval "$(1)"; \
			;; \
	esac
endef

# Ask and Execute ... if input other than y , Exit Makefile
define ASK_AND_EXECUTE_AND_EXIT
	echo "Execute $(1) (y/n)? " ; \
	read -r answer ; \
	if [ "$$answer" = "n" ] || [ "$$answer" = "N" ]; then \
		echo "Exiting." ; \
		exit 1 ; \
	else \
		echo "Running tests..." ; \
		$(2) ; \
	fi
endef

define CONTINUE_NEXT
	@echo "Continue Enter $(1)"
	@read -r answer
endef

# Makefile Option : Disable '--print-directory'
MAKEFLAGS += --no-print-directory
