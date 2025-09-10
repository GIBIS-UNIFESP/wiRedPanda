# wiRedPanda Development Container

This directory contains the configuration for a streamlined containerized development environment for wiRedPanda using Ubuntu 22.04 LTS.

## 🚀 Quick Start

### Prerequisites

- [Docker](https://www.docker.com/get-started) installed and running
- [Visual Studio Code](https://code.visualstudio.com/) with the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

### Using the Development Container

1. **Open in VS Code**: Open the wiRedPanda project folder in VS Code
2. **Reopen in Container**: When prompted, click "Reopen in Container" or use the command palette (`Ctrl+Shift+P`) and run "Dev Containers: Reopen in Container"
3. **Wait for Setup**: The container will build and set up automatically (this may take several minutes on first run)
4. **Start Developing**: Once complete, you'll have a fully configured development environment

## 🛠️ What's Included

### Development Tools
- **Build System**: CMake 3.22+ with Ninja generator
- **Compiler**: GCC 11+ with C++20 support
- **Qt Framework**: Qt 5.15 with core, widgets, multimedia, SVG modules
- **Performance**: Mold linker and ccache for faster builds
- **Shell**: Zsh with Oh My Zsh for enhanced terminal experience
- **AI Assistant**: Claude Code CLI for AI-powered development assistance

### VS Code Extensions
- C/C++ extension pack (IntelliSense, debugging, themes)
- CMake Tools for build management and configuration

### Core Utilities
- **Version Control**: Git and GitHub CLI
- **Languages**: Python 3, Node.js 20.x
- **Testing**: lcov for coverage analysis
- **Package Management**: pip3 and npm

## 📁 Container Structure

```
/workspace/          # Your project files (mounted from host)
├── app/            # Application source code
├── test/           # Test suite
├── build/          # Build output directory
└── .devcontainer/  # Container configuration
```

## 🔧 Build Commands

### VS Code Tasks (Recommended)
Use **Ctrl+Shift+P** → "Tasks: Run Task":
- **Build** - Build the project (default: Ctrl+Shift+B)
- **Configure Debug** - Configure CMake for debug build
- **Configure Release** - Configure CMake for release build
- **Clean Build** - Clean and rebuild from scratch
- **Run Tests** - Run the test suite

### Manual Build Process

```bash
# Configure and build with CMake/Ninja
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```


## 🧪 Testing

```bash
# Run all tests (automatic via VS Code task)
# Ctrl+Shift+P → "Tasks: Run Task" → "Run Tests"

# Manual test execution
./build/test/wiredpanda-test

# Run specific test class
./build/test/wiredpanda-test TestBasicGates
```

## 🖥️ Display Mode

The container is configured to run in **offscreen mode** automatically (via testmain.cpp), making it compatible with all host operating systems without requiring X11 setup. This is ideal for:

- Running automated tests
- CI/CD pipelines
- Development on Windows/macOS hosts
- Headless server environments

## 🐛 Troubleshooting

### Container Won't Start
- Ensure Docker is running
- Check Docker has enough resources allocated
- Try rebuilding: "Dev Containers: Rebuild Container"

### Build Failures
- Check Qt installation: `qt5-qmake --version`
- Verify CMake configuration: `cd build && cmake ..`
- Clean build: `rm -rf build && mkdir build`

### Permission Issues
- The container user (developer) should have UID 1000
- If issues persist, rebuild container with correct UID mapping

## 🔄 Updating the Container

When the Dockerfile changes:

1. **Rebuild Container**: Use command palette → "Dev Containers: Rebuild Container"
2. **Clean Rebuild**: Use "Dev Containers: Rebuild Container Without Cache"

## 📝 Customization

### Adding Packages
Edit `.devcontainer/Dockerfile` and add packages to the `apt-get install` command.

### VS Code Settings
Modify `.devcontainer/devcontainer.json` to change VS Code settings and extensions.

### Environment Variables
Add environment variables in the `containerEnv` section of `devcontainer.json`.

## 🤝 Development Workflow

1. **Code**: Write code using VS Code with full IntelliSense support
2. **Build**: Use cmake and ninja commands
3. **Test**: Run test binaries to verify functionality
4. **Debug**: Use VS Code's integrated debugger
5. **AI Assist**: Use Claude Code CLI for code assistance
6. **Commit**: Git is pre-configured for version control

## 🔒 Security Notes

- Container runs as non-root user `developer`
- Minimal package installation for reduced attack surface
- No X11 forwarding required (offscreen mode)

## 📚 Additional Resources

- [Dev Containers Documentation](https://code.visualstudio.com/docs/devcontainers/containers)
- [Qt Documentation](https://doc.qt.io/)
- [CMake Documentation](https://cmake.org/documentation/)
- [Claude Code Documentation](https://docs.anthropic.com/en/docs/claude-code)
- [wiRedPanda Project README](../README.md)

## 🐛 Reporting Issues

If you encounter issues with the development container, please:

1. Check this README for troubleshooting steps
2. Verify your host system requirements
3. Report issues with container logs and system information
