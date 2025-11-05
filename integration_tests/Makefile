VENV_DIR = .venv
VENV_PYTHON = $(VENV_DIR)/bin/python
VENV_PYTEST = $(VENV_DIR)/bin/pytest
REQUIREMENTS = requirements.txt

# 仮想環境が存在するかを確認
# ディレクトリが存在しない場合、`ifeq` の条件が真になる
ifeq ("$(wildcard $(VENV_DIR))","")
VENV_EXISTS = false
else
VENV_EXISTS = true
endif

.PHONY: all install clean run

all: install run

install:
ifeq ($(VENV_EXISTS), false)
	@echo "Creating virtual environment..."
	python3 -m venv $(VENV_DIR)
	@echo "Installing dependencies..."
	$(VENV_PYTHON) -m pip install -r $(REQUIREMENTS)
else
	@echo "Virtual environment already exists."
	@echo "Updating dependencies..."
	$(VENV_PYTHON) -m pip install -r $(REQUIREMENTS)
endif

run:
	@echo "Running the application..."
	$(VENV_PYTEST) .
#	$(VENV_PYTEST) -v -s .

clean:
	@echo "Cleaning up virtual environment..."
	rm -rf $(VENV_DIR)
	@echo "Cleanup complete."
