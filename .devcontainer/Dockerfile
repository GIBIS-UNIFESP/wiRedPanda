# Use Ubuntu 22.04 LTS as base image
FROM ubuntu:22.04

# Avoid warnings by switching to noninteractive
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies with proper ordering
RUN apt-get update && apt-get install -y --no-install-recommends --no-install-suggests \
    curl \
    wget \
    ca-certificates \
    gnupg \
    lsb-release \
    apt-transport-https \
    && curl -fsSL https://deb.nodesource.com/setup_20.x | bash - \
    && curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | dd of=/usr/share/keyrings/githubcli-archive-keyring.gpg \
    && chmod go+r /usr/share/keyrings/githubcli-archive-keyring.gpg \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | tee /etc/apt/sources.list.d/github-cli.list > /dev/null \
    && apt-get update && apt-get install -y --no-install-recommends --no-install-suggests \
    # Essential build tools
    build-essential \
    clang \
    clang-tools \
    lld \
    cmake \
    ninja-build \
    mold \
    ccache \
    pkg-config \
    # Qt5 development packages
    qtbase5-dev \
    qttools5-dev \
    qttools5-dev-tools \
    qtmultimedia5-dev \
    qt5-qmake \
    libqt5svg5-dev \
    libqt5printsupport5 \
    # Development tools
    git \
    gh \
    # Debugging tools
    gdb \
    gdbserver \
    lldb \
    strace \
    ltrace \
    # Performance analysis tools
    valgrind \
    perf-tools-unstable \
    linux-tools-generic \
    # Testing and coverage tools
    lcov \
    # Python for build scripts
    python3 \
    python3-pip \
    # Node.js
    nodejs \
    # Additional development utilities
    htop \
    iotop \
    sysstat \
    lsof \
    net-tools \
    tcpdump \
    vim \
    nano \
    tmux \
    screen

# Install Claude Code CLI
RUN npm install -g @anthropic-ai/claude-code

# Set up ccache for faster compilation
RUN /usr/sbin/update-ccache-symlinks 2>/dev/null || true \
    && ccache --set-config=max_size=2G \
    && mkdir -p /home/vscode/.ccache \
    && chown -R 1000:1000 /home/vscode/.ccache

# Add ccache to PATH for all users
ENV PATH="/usr/lib/ccache:${PATH}"
ENV CCACHE_DIR="/home/vscode/.ccache"

# Create workspace directory
RUN mkdir -p /workspace

# Set working directory
WORKDIR /workspace

# Set the default command
CMD ["/bin/bash"]

# Reset to default for subsequent operations
ENV DEBIAN_FRONTEND=dialog
