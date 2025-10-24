# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: kamitsui <kamitsui@student.42tokyo.jp>     +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/03/15 19:38:29 by kamitsui          #+#    #+#              #
#    Updated: 2025/10/25 00:52:40 by kamitsui         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# @file unit_test/Makefile

# Target Name
NAME = ft_irc_unittest
#
# Directories
#PRJ_DIR = ../..
SRC_DIR = ./ ../../src
OBJ_DIR = obj
DEP_DIR = .dep

# Source
SRCS = \
      Channel.cpp \
      Client.cpp \
      CommandManager.cpp \
      JoinCommand.cpp \
      NickCommand.cpp \
      PartCommand.cpp \
      PassCommand.cpp \
      PrivmsgCommand.cpp \
      Replies.cpp \
      Server.cpp \
      UserCommand.cpp \
	  \
	  ClientTest.cpp \
      PassCommandTest.cpp \
      NickCommandTest.cpp \
      UserCommandTest.cpp \
	  ChannelTest.cpp \
	  JoinPrivmsgPartTest.cpp \
      main.cpp \
	  \
	  #\
	  #ChannelTest.cpp \
	  #ClientTest.cpp \
	  #CommandManagerTest.cpp \
	  #JoinCommandTest.cpp \
      #NickCommandTest.cpp \
      #PartCommandTest.cpp \
      #PassCommandTest.cpp \
      #PrivmsgCommandTest.cpp \
      #RepliesTest.cpp \
      #ServerTest.cpp \
      #UserCommandTest.cpp \
      #main.cpp

# Compiler
CXX = c++
CF_INC = -I../../inc
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

# vpath for serching source files in multiple directories
vpath %.cpp $(SRC_DIR)

# Object files and dependency files
OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))
DEPS = $(addprefix $(DEP_DIR)/, $(SRCS:.cpp=.d))

# Rules for building object files
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CF_INC) $(CF_DEP) -c $< -o $@

# Rules for building dependency files
#$(DEP_DIR)/%.d: %.cpp
#	@mkdir -p $(DEP_DIR)

# Default target
all: directories $(NAME)
	@echo "Build" "'$(NAME)'" "Complete!"
	$(call ASCII_ART,$(NAME))
	$(call ASK_AND_EXECUTE_ON_YES, ./$(NAME))
.PHONY: all

# Make Directories
directories:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(DEP_DIR)
.PHONY: directories

# Build Target
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBS) -o $(NAME)

# Rule for removing object & dependency files
clean:
	rm -rf $(OBJ_DIR) $(DEP_DIR)
.PHONY: clean

# Rule for removing Target & others
fclean: clean
	rm -f $(NAME)
.PHONY: fclean

# Rule for Clean & Build Target
re: fclean all
.PHONY: re

# Enable dependency file
-include $(DEPS)

# ASCII Art : Display Tips the way to use.
define ASCII_ART
	@echo " _____________________________________________"
	@echo "< Usage :                                     >"
	@echo "<  $$ ./$(1)                        >"
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

# Makefile Option : Disable '--print-directory'
MAKEFLAGS += --no-print-directory
