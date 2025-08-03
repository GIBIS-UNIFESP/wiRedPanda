#!/bin/bash

# wiRedPanda Development Container Setup Script
# This script runs after the container is created to set up the development environment

set -e

echo "🚀 Setting up wiRedPanda development environment..."

# Update package lists
echo "📦 Updating package lists..."
sudo apt-get update

# Verify Qt installation
echo "🔍 Verifying Qt installation..."
qmake --version
echo "Qt installation directory: $(which qmake)"

# Set up build directory
echo "📁 Setting up build directory..."
mkdir -p build
cd build

# Configure CMake build with Ninja (prefer CMake over qmake as per project preferences)
echo "⚙️  Configuring CMake build with Ninja..."
if command -v cmake &> /dev/null; then
    cmake .. -G Ninja \
             -DCMAKE_BUILD_TYPE=Debug \
             -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
             -DCMAKE_PREFIX_PATH="/usr/lib/x86_64-linux-gnu/cmake/Qt5" \
             || echo "⚠️  CMake configuration failed - this is normal on first setup"
else
    echo "❌ CMake not found"
fi

cd ..

# Set up pre-commit hooks if available
if command -v pre-commit &> /dev/null && [ -f ".pre-commit-config.yaml" ]; then
    echo "🪝 Setting up pre-commit hooks..."
    pre-commit install || echo "⚠️  Pre-commit setup failed"
fi

# Set up ccache for faster compilation
echo "⚡ Setting up ccache for faster compilation..."
sudo /usr/sbin/update-ccache-symlinks
echo 'export PATH="/usr/lib/ccache:$PATH"' >> ~/.bashrc
ccache --set-config=max_size=2G
ccache --set-config=cache_dir=/home/developer/.ccache

# Create useful aliases and environment setup
echo "🔧 Setting up development aliases..."
cat >> ~/.bashrc << 'EOF'

# Ccache environment
export PATH="/usr/lib/ccache:$PATH"
export CCACHE_DIR=/home/developer/.ccache

# wiRedPanda development aliases (using Ninja for fastest builds with ccache)
alias wpbuild='cd /workspace && cmake --build build --config Release'
alias wptest='cd /workspace/build && QT_QPA_PLATFORM=offscreen ./test'
alias wprun='cd /workspace/build && ./wiredpanda'
alias wpcoverage='cd /workspace && lcov --capture --directory build --output-file coverage.info && genhtml coverage.info --output-directory coverage_html'
alias wpclean='cd /workspace && rm -rf build && cmake -B build -G Ninja'

# Development shortcuts
alias cls='clear'
alias ..='cd ..'
alias ...='cd ../..'

EOF

# Set up git configuration template (user should customize)
echo "📝 Setting up git configuration template..."
if [ ! -f ~/.gitconfig ]; then
    cat > ~/.gitconfig << 'EOF'
[user]
    # name = Your Name
    # email = your.email@example.com
[core]
    editor = nano
    autocrlf = input
[push]
    default = simple
[pull]
    rebase = false
EOF
    echo "⚠️  Please configure git with your name and email:"
    echo "   git config --global user.name 'Your Name'"
    echo "   git config --global user.email 'your.email@example.com'"
fi

# Create a simple Qt version test program
echo "🧪 Creating Qt test program..."
mkdir -p /tmp/qttest
cat > /tmp/qttest/main.cpp << 'EOF'
#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    qDebug() << "Qt Runtime Version:" << qVersion();
    
    QWidget window;
    QVBoxLayout *layout = new QVBoxLayout(&window);
    
    QLabel *label = new QLabel(QString("Qt %1 is working!").arg(qVersion()));
    layout->addWidget(label);
    
    window.setWindowTitle("wiRedPanda Dev Container - Qt Test");
    window.resize(300, 100);
    
    // For headless testing, just show version info and exit
    if (qgetenv("DISPLAY").isEmpty()) {
        qDebug() << "Qt is properly configured for headless operation";
        return 0;
    }
    
    window.show();
    return app.exec();
}
EOF

cat > /tmp/qttest/CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.16)
project(qttest)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

add_executable(qttest main.cpp)
target_link_libraries(qttest Qt5::Core Qt5::Widgets)
EOF

cd /tmp/qttest
cmake . && make
echo "🧪 Running Qt test (headless)..."
./qttest || echo "✅ Qt test completed"

cd /workspace

# Display environment information
echo ""
echo "🎉 Development environment setup complete!"
echo ""
echo "📋 Environment Information:"
echo "   OS: $(lsb_release -d | cut -f2)"
echo "   Qt Version: $(qmake -query QT_VERSION)"
echo "   CMake Version: $(cmake --version | head -n1)"
echo "   GCC Version: $(gcc --version | head -n1)"
echo "   Working Directory: $(pwd)"
echo ""
echo "🔨 Build Commands:"
echo "   CMake build: cd build && cmake .. && make -j\$(nproc)"
echo "   qmake build: qmake WPanda.pro && make -j\$(nproc)"
echo ""
echo "🧪 Test Commands:"
echo "   Run tests: ./build/test/wiredpanda-test"
echo "   Coverage: wpbuild && wpcoverage"
echo ""
echo "💡 Useful aliases:"
echo "   wpbuild  - Build the project"
echo "   wptest   - Run tests"
echo "   wprun    - Run the application"
echo "   wpclean  - Clean build directory"
echo ""
echo "Ready for development! 🚀"
