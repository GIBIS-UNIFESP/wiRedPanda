# Use Ubuntu 22.04 LTS as base image
FROM ubuntu:22.04

# Avoid warnings by switching to noninteractive
ENV DEBIAN_FRONTEND=noninteractive

# Configure locale
RUN apt-get update && apt-get install -y locales \
    && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8
ENV LANG en_US.utf8

# Install system dependencies
RUN apt-get update && apt-get install -y \
    # Essential build tools
    build-essential \
    cmake \
    make \
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
    libqt5serialport5-dev \
    libqt5printsupport5 \
    # Development tools
    git \
    wget \
    curl \
    vim \
    nano \
    tree \
    htop \
    # X11 and GUI support
    xvfb \
    x11-apps \
    x11-utils \
    x11-xserver-utils \
    # Testing and coverage tools
    lcov \
    gcovr \
    valgrind \
    # Python for build scripts
    python3 \
    python3-pip \
    # Archive tools
    unzip \
    zip \
    tar \
    gzip \
    # Network tools
    net-tools \
    iputils-ping \
    # Documentation tools
    doxygen \
    graphviz \
    # Additional utilities
    software-properties-common \
    apt-transport-https \
    ca-certificates \
    gnupg \
    lsb-release \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js and Claude Code CLI
RUN curl -fsSL https://deb.nodesource.com/setup_20.x | bash - && \
    apt-get install -y nodejs && \
    npm install -g @anthropic-ai/claude-code

# Install additional development tools
RUN pip3 install \
    conan \
    pre-commit \
    cppcheck-junit

# Create a non-root user
ARG USERNAME=developer
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME \
    && rm -rf /var/lib/apt/lists/*

# Set up Qt environment variables
ENV QT_QPA_PLATFORM=xcb
ENV QT_SELECT=qt5

# Create workspace directory
RUN mkdir -p /workspace && chown $USERNAME:$USERNAME /workspace

# Switch to non-root user
USER $USERNAME

# Set up shell environment
RUN echo 'export PATH="/usr/lib/qt5/bin:$PATH"' >> ~/.bashrc \
    && echo 'export QTDIR=/usr/lib/qt5' >> ~/.bashrc \
    && echo 'export QT_PLUGIN_PATH=/usr/lib/x86_64-linux-gnu/qt5/plugins' >> ~/.bashrc \
    && echo 'alias ll="ls -la"' >> ~/.bashrc \
    && echo 'alias la="ls -A"' >> ~/.bashrc \
    && echo 'alias l="ls -CF"' >> ~/.bashrc

# Set working directory
WORKDIR /workspace

# Copy setup script
COPY --chown=$USERNAME:$USERNAME .devcontainer/setup.sh /workspace/.devcontainer/setup.sh
RUN chmod +x /workspace/.devcontainer/setup.sh

# Set the default command
CMD ["/bin/bash"]

# Reset to default for subsequent operations
ENV DEBIAN_FRONTEND=dialog
