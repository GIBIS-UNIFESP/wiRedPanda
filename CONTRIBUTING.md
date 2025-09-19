# Contributing to wiRedPanda

Thank you for your interest in contributing to wiRedPanda! 🐼

wiRedPanda is a free, open-source digital logic circuit simulator designed to help students learn about logic circuits in an intuitive and friendly way. We welcome contributions from developers, translators, educators, and users of all skill levels.

## Table of Contents

- [Ways to Contribute](#ways-to-contribute)
- [Development Setup](#development-setup)
- [Code Style Guidelines](#code-style-guidelines)
- [Testing](#testing)
- [Pull Request Process](#pull-request-process)
- [Issue Reporting](#issue-reporting)
- [Translation](#translation)
- [Documentation](#documentation)
- [Project Management](#project-management)
- [License](#license)

## Ways to Contribute

### 🛠️ Code Contributions
- Fix bugs and implement new features
- Improve performance and optimization
- Add new circuit elements and components
- Enhance user interface and experience

### 🌍 Translation
- Help translate wiRedPanda to new languages
- Improve existing translations
- Review and validate translations

### 📚 Documentation
- Improve README files and build instructions
- Write tutorials and educational content
- Update API documentation
- Create examples and sample circuits

### 🐛 Bug Reports & Feature Requests
- Report bugs with detailed reproduction steps
- Suggest new features and improvements
- Test beta releases and provide feedback

### 🎓 Educational Content
- Create learning materials and tutorials
- Share classroom experiences and feedback
- Suggest educational improvements

## Development Setup

### Using Development Container (Recommended)

The easiest way to get started is using our development container:

1. **Prerequisites**: Install [Docker](https://www.docker.com/) and [VS Code](https://code.visualstudio.com/) with the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

2. **Setup**:
   ```bash
   git clone https://github.com/GIBIS-UNIFESP/wiRedPanda.git
   cd wiRedPanda
   code .
   ```

3. **Open in Container**: When prompted, select "Reopen in Container" or use `Ctrl+Shift+P` → "Dev Containers: Reopen in Container"

4. **Build and Test**:
   ```bash
   cmake --preset release
   cmake --build --preset release --target wiredpanda
   ctest --test-dir build
   ```

### Local Development Setup

If you prefer local development, see our detailed [BUILD.md](BUILD.md) instructions for your platform.

**Quick Start (Linux/macOS)**:
```bash
# Install dependencies (example for Ubuntu)
sudo apt install build-essential qtbase5-dev qtmultimedia5-dev libqt5svg5-dev

# Clone and build
git clone https://github.com/GIBIS-UNIFESP/wiRedPanda.git
cd wiRedPanda
cmake --preset release
cmake --build --preset release
```

## Code Style Guidelines

### General Requirements
- **C++ Standard**: C++20
- **Build System**: CMake 3.16+ with Ninja generator (auto-selected)
- **Trailing Newlines**: All source files must end with a trailing newline
- **Line Trimming**: Remove trailing whitespace from all lines
- **File Types**: Apply style to `.cpp`, `.h`, `.cmake`, `CMakeLists.txt`, etc.

### Code Organization
- **Build Directory**: Always use `build/` directory to prevent accidental commits
- **Header Guards**: Use pragma once or include guards
- **Naming**: Follow Qt naming conventions for consistency

### Copyright Headers
Add copyright headers to new files:
```cpp
/*
 * Copyright (C) 2025 - [Your Name]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
```

## Testing

wiRedPanda uses Qt Test framework with CTest integration.

### Running Tests
```bash
# Run all tests
ctest --test-dir build

# Run with verbose output
ctest --test-dir build --verbose

# Run specific test pattern
ctest --test-dir build -R testcommands

# Run individual test executable
./build/testelements
./build/testfiles
```

### Test Coverage
Enable coverage analysis during development:
```bash
cmake --preset coverage
cmake --build build
ctest --test-dir build
```

### Sanitizers
Use sanitizers to catch bugs early:
```bash
# Address Sanitizer
cmake --preset asan

# Thread Sanitizer
cmake --preset tsan

# UB Sanitizer
cmake --preset ubsan
```

### Writing Tests
- Create test files in the `test/` directory
- Use Qt Test framework (`QTEST_MAIN`, `QVERIFY`, etc.)
- Test both positive and negative cases
- Include edge cases and error conditions

## Pull Request Process

### Before Submitting
1. **Check Issues**: Look for existing issues or create one to discuss your changes
2. **Follow Style**: Ensure code follows our style guidelines
3. **Test Thoroughly**: Run all tests and add new tests for your changes
4. **Update Documentation**: Update relevant documentation and comments

### Creating a Pull Request
1. **Fork and Branch**: Create a feature branch from `master`
2. **Commit Messages**: Use descriptive commit messages following conventional commits
3. **Small Changes**: Keep PRs focused and reasonably sized
4. **Description**: Provide clear description of changes and motivation

### Example Commit Message
```
feat: add 16-segment display component

- Implement new display element with 16 segments
- Add corresponding tests and documentation
- Update element palette and resources

Fixes #123
```

### Review Process
- PRs require review from maintainers
- Address feedback promptly and professionally
- Automated checks (build, tests, coverage) must pass
- Maintain clean commit history

## Issue Reporting

### Bug Reports
When reporting bugs, please include:
- **Environment**: OS, Qt version, wiRedPanda version
- **Steps to Reproduce**: Clear, step-by-step instructions
- **Expected vs Actual**: What should happen vs what actually happens
- **Screenshots/Files**: Visual aids and sample circuit files if applicable
- **Logs**: Any error messages or crash logs

### Feature Requests
For new features:
- **Use Case**: Describe the problem you're trying to solve
- **Proposed Solution**: Your idea for implementation
- **Alternatives**: Other approaches you've considered
- **Educational Value**: How it benefits learning/teaching

## Translation

wiRedPanda supports multiple languages through our translation system.

### Using Weblate (Recommended)
1. Visit our [Weblate project](https://hosted.weblate.org/projects/wiredpanda/wiredpanda)
2. Select your language or request a new one
3. Translate strings using the web interface
4. Translations are automatically submitted as pull requests

### Manual Translation
1. Find translation files in `app/resources/translations/`
2. Copy `wpanda_en.ts` to `wpanda_[language_code].ts`
3. Use Qt Linguist or edit XML directly
4. Test your translation in the application
5. Submit a pull request

### Translation Guidelines
- **Context**: Understand the UI context before translating
- **Consistency**: Use consistent terminology throughout
- **Length**: Consider UI space constraints
- **Cultural**: Adapt to local conventions where appropriate

## Documentation

### Types of Documentation
- **User Documentation**: README files, tutorials, help content
- **Developer Documentation**: Code comments, API docs, contributing guides
- **Educational Materials**: Learning resources, examples, lesson plans

### Documentation Standards
- **Clarity**: Write clearly and concisely
- **Accuracy**: Keep information up-to-date
- **Multilingual**: Consider translation for user-facing docs
- **Examples**: Include practical examples and code snippets

### API Documentation
We use Doxygen for API documentation:
```cpp
/**
 * @brief Brief description of the function
 * @param parameter Description of parameter
 * @return Description of return value
 */
```

## Project Management

### GitHub Project
We use our [GitHub Project](https://github.com/orgs/GIBIS-UNIFESP/projects/1) to track:
- Current development status
- Planned features and improvements
- Bug fixes and issues
- Community contributions

### Communication
- **GitHub Issues**: For bug reports and feature requests
- **GitHub Discussions**: For questions and general discussion
- **Pull Request Comments**: For code review and specific implementation details

### Getting Involved
1. Check our [project roadmap](https://github.com/orgs/GIBIS-UNIFESP/projects/1)
2. Look for issues labeled `good first issue` or `help wanted`
3. Join discussions and provide feedback
4. Share your use cases and educational experiences

## License

wiRedPanda is licensed under the [GNU General Public License v3.0](LICENSE).

By contributing to wiRedPanda, you agree that your contributions will be licensed under the same GPL v3.0 license. This ensures that wiRedPanda remains free and open-source software that benefits the entire educational community.

### Key Points
- Your contributions become part of the GPL v3.0 codebase
- You retain copyright to your contributions
- You grant others the right to use your contributions under GPL v3.0
- Commercial use is allowed under GPL v3.0 terms

---

## Getting Help

- **Build Issues**: Check [BUILD.md](BUILD.md) and existing issues
- **Development Questions**: Create a GitHub Discussion
- **Bug Reports**: Create a GitHub Issue with details
- **Feature Ideas**: Check roadmap and create issue for discussion

Thank you for contributing to wiRedPanda! Together, we're making digital logic education more accessible and engaging for students worldwide. 🚀
