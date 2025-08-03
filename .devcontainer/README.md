# wiRedPanda Development Container

This directory contains the configuration for a containerized development environment for wiRedPanda using Ubuntu 22.04 LTS.

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
- **Build System**: CMake 3.22+ and make
- **Compiler**: GCC 11+ with C++17 support
- **Qt Framework**: Qt 5.15 with all necessary modules
- **Testing**: Qt Test framework with coverage tools (lcov, gcovr)
- **Debugging**: GDB and Valgrind for memory debugging

### VS Code Extensions
- C/C++ IntelliSense and debugging
- CMake Tools for build management
- Qt syntax highlighting
- Code spell checker
- Better comments
- Hex editor

### Utilities
- Git with configuration template
- Pre-commit hooks support
- Python 3 with development packages
- Network and system utilities

## 📁 Container Structure

```
/workspace/          # Your project files (mounted from host)
├── app/            # Application source code
├── test/           # Test suite
├── build/          # Build output directory
└── .devcontainer/  # Container configuration
```

## 🔧 Build Commands

The setup script creates several useful aliases:

- `wpbuild` - Build the project using make
- `wptest` - Run the test suite
- `wprun` - Run the wiRedPanda application
- `wpcoverage` - Generate code coverage reports
- `wpclean` - Clean and recreate build directory

### Manual Build Process

```bash
# CMake build (recommended)
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# qmake build (legacy)
qmake WPanda.pro
make -j$(nproc)
```

## 🧪 Testing

```bash
# Run all tests
wptest

# Run specific test class
./build/test/WPanda-test TestBasicGates

# Generate coverage report
wpcoverage
```

## 🎨 GUI Applications

The container supports GUI applications through X11 forwarding:

- **Linux Host**: GUI applications should work automatically
- **Windows Host**: Requires X server (e.g., VcXsrv, Xming)
- **macOS Host**: Requires XQuartz

For headless testing, applications can run with `-platform offscreen`.

## 🐛 Troubleshooting

### Container Won't Start
- Ensure Docker is running
- Check Docker has enough resources allocated
- Try rebuilding: "Dev Containers: Rebuild Container"

### GUI Applications Don't Display
- **Linux**: Run `xhost +local:docker` on host
- **Windows**: Start X server and set DISPLAY environment variable
- **macOS**: Install and configure XQuartz

### Build Failures
- Check Qt installation: `qmake --version`
- Verify CMake configuration: `cd build && cmake ..`
- Clean build: `wpclean` then rebuild

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
2. **Build**: Use `wpbuild` or CMake commands
3. **Test**: Run `wptest` to verify functionality
4. **Debug**: Use VS Code's integrated debugger
5. **Coverage**: Generate reports with `wpcoverage`
6. **Commit**: Git is pre-configured for version control

## 🔒 Security Notes

- Container runs as non-root user `developer`
- SYS_PTRACE capability enabled for debugging
- X11 forwarding requires trust considerations on multi-user systems

## 📚 Additional Resources

- [Dev Containers Documentation](https://code.visualstudio.com/docs/devcontainers/containers)
- [Qt Documentation](https://doc.qt.io/)
- [CMake Documentation](https://cmake.org/documentation/)
- [wiRedPanda Project README](../README.md)

## 🐛 Reporting Issues

If you encounter issues with the development container, please:

1. Check this README for troubleshooting steps
2. Verify your host system requirements
3. Report issues with container logs and system information
