# Base Image
FROM ubuntu:22.04

# Set non-interactive mode for APT
ENV DEBIAN_FRONTEND=noninteractive \
	LANG=ja_JP.UTF-8 \
	LANGUAGE=ja_JP:ja \
	LC_ALL=ja_JP.UTF-8 \
	TZ=Asia/Tokyo

#RUN DEBIAN_PRIORITY=critical apt-get update && apt-get install -y \
#    keyboard-configuration \
#    console-setup \
#	&& apt-get clean

# Install essential packages and Japanese support
RUN DEBIAN_PRIORITY=critical apt-get update && apt-get install -y \
	# time synchronization \
	locales \
	tzdata \
	ibus \
	ibus-mozc \
	fonts-noto-cjk \
	&& apt-get clean

# Configure Japanese locale
RUN echo "ja_JP.UTF-8 UTF-8" > /etc/locale.gen && \
    locale-gen ja_JP.UTF-8 && \
    update-locale LANG=ja_JP.UTF-8

# Set timezone to Japan
RUN ln -sf /usr/share/zoneinfo/Asia/Tokyo /etc/localtime && \
    dpkg-reconfigure -f noninteractive tzdata

# Install necessary dependencies
RUN DEBIAN_PRIORITY=critical apt-get update && apt-get install -y \
	keyboard-configuration \
	console-setup \
	libx11-dev \
	libxext-dev \
	libxrandr-dev \
	libxinerama-dev \
	libxcursor-dev \
	libxi-dev \
	x11-apps \
	mesa-utils \
	xorg \
	openbox \
	libglu1-mesa-dev freeglut3-dev mesa-common-dev \
	libgl1-mesa-glx \
	libgl1-mesa-dri \
	mesa-utils \
	build-essential \
	zlib1g-dev \
	libbsd-dev \
	clang \
	cmake \
	git \
	git-lfs \
	valgrind \
	wget \
	curl \
	vim fzf \
	silversearcher-ag \
	tmux \
	stow \
	# google test\
	libgtest-dev \
	# documantation \
	doxygen \
	graphviz \
	inotify-tools \
	python3.10-venv \
	&& apt-get clean

# Git LFS を初期化（ユーザーごとに実施）：git pull, checkout, restore 操作を実行した際に、Git LFSが正しく動作します。
RUN git lfs install

# 基本ツール
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates curl gnupg \
 && rm -rf /var/lib/apt/lists/*

# NodeSource レポジトリ（LTS固定の例）
RUN mkdir -p /etc/apt/keyrings \
 && curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key \
    | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg \
 && echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_24.x nodistro main" \
    > /etc/apt/sources.list.d/nodesource.list

# Node.js インストール
RUN apt-get update && apt-get install -y --no-install-recommends nodejs \
 && rm -rf /var/lib/apt/lists/*

# バージョン確認（ビルドログ用）
RUN node -v && npm -v

# Gemini CLI をインストール
RUN npm install -g @google/gemini-cli \
 && echo "GEMINI_API_KEYを設定して下さい。"


# Create working directory inside the container
WORKDIR /app

# Expose the live-server port
EXPOSE 8080

# Set entry point to bash
CMD ["bash"]
