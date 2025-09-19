# Multi-stage build for wiRedPanda development environment
# Stage 1: Builder - Full environment for Qt installation
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    curl wget ca-certificates python3 python3-pip \
    && pip3 install aqtinstall \
    && mkdir -p /opt/Qt \
    && cd /opt/Qt \
    && aqt install-qt linux desktop 6.9.2 linux_gcc_64 \
        -m qtmultimedia \
    && rm -rf /opt/Qt/6.9.2/gcc_64/doc \
    && rm -rf /opt/Qt/6.9.2/gcc_64/examples \
    && rm -rf /opt/Qt/6.9.2/gcc_64/qml \
    && find /opt/Qt -name "*.debug" -delete \
    && find /opt/Qt -name "*.pdb" -delete \
    && find /opt/Qt -name "*_debug*" -delete

# Stage 2: Development runtime - Essential development environment
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
# Set UTF-8 locale to prevent Qt encoding warnings
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# Install essential development packages (keeping current tools for now)
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Essential build tools
    build-essential cmake ninja-build ccache pkg-config mold \
    # System dependencies for Qt
    libfontconfig1 libgl1-mesa-dev libglu1-mesa-dev libpulse0 \
    libxcb-cursor0 libxcb-xinerama0 libxkbcommon0 \
    # Development tools
    git gdb nano \
    # Shell and utilities
    zsh curl wget sudo \
    # Coverage tools
    lcov \
    # GitHub CLI and Node.js dependencies
    gnupg ca-certificates \
    && curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | gpg --dearmor -o /usr/share/keyrings/githubcli-archive-keyring.gpg \
    && chmod go+r /usr/share/keyrings/githubcli-archive-keyring.gpg \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | tee /etc/apt/sources.list.d/github-cli.list > /dev/null \
    && curl -fsSL https://deb.nodesource.com/setup_20.x | bash - \
    && apt-get update && apt-get install -y --no-install-recommends gh nodejs \
    # Basic cleanup
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install Claude Code CLI
RUN npm install -g @anthropic-ai/claude-code

# Copy Qt installation from builder stage
COPY --from=builder /opt/Qt /opt/Qt

# Set Qt environment variables
ENV QT_ROOT="/opt/Qt/6.9.2/gcc_64"
ENV PATH="${QT_ROOT}/bin:${PATH}"
ENV LD_LIBRARY_PATH="${QT_ROOT}/lib"
ENV PKG_CONFIG_PATH="${QT_ROOT}/lib/pkgconfig"
ENV CMAKE_PREFIX_PATH="${QT_ROOT}"

# Set up ccache
RUN /usr/sbin/update-ccache-symlinks 2>/dev/null || true \
    && ccache --set-config=max_size=2G \
    && mkdir -p /home/vscode/.ccache \
    && chown -R 1000:1000 /home/vscode/.ccache

# Configure container user with zsh and passwordless sudo
RUN groupadd --gid 1000 developer \
    && useradd --uid 1000 --gid developer --shell /bin/zsh --create-home developer \
    && usermod -aG sudo developer \
    && echo "developer:developer" | chpasswd \
    && echo "developer ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Install Oh My Zsh for developer user
USER developer
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended \
    && echo 'export PATH="/usr/lib/ccache:$PATH"' >> ~/.zshrc \
    && echo 'export CCACHE_DIR="/home/vscode/.ccache"' >> ~/.zshrc
USER root

# Add ccache to PATH
ENV PATH="/usr/lib/ccache:${PATH}"
ENV CCACHE_DIR="/home/vscode/.ccache"

# Create workspace directory
RUN mkdir -p /workspace
WORKDIR /workspace

# Set default command
CMD ["/bin/bash"]

# Reset for subsequent operations
ENV DEBIAN_FRONTEND=dialog
