# Contributing to wiRedPanda

Thank you for your interest in contributing to wiRedPanda! Whether you're here to report issues, suggest new features, or submit your code, your contributions are highly appreciated. This guide will help you get started.

---

## Table of Contents
- [Contributing to wiRedPanda](#contributing-to-wiredpanda)
  - [Table of Contents](#table-of-contents)
  - [How to Contribute](#how-to-contribute)
    - [Reporting Issues](#reporting-issues)
    - [Suggesting Features](#suggesting-features)
    - [Submitting Code](#submitting-code)
  - [Development Workflow](#development-workflow)
    - [Prerequisites](#prerequisites)
    - [Setting Up](#setting-up)
    - [Building and Running](#building-and-running)
  - [Style Guide](#style-guide)
    - [Example Commit Message Format:](#example-commit-message-format)
  - [Resources](#resources)

---

## How to Contribute

### Reporting Issues
If you encounter any bugs, crashes, or unexpected behaviors:
1. Search the issue tracker to see if the issue has already been reported.
2. If not, open a new issue and include:
   - A clear title and description.
   - Steps to reproduce the issue.
   - Screenshots or logs, if applicable.
   - Your system details (OS, Qt version, etc.).

### Suggesting Features
Have a cool idea for wiRedPanda? We'd love to hear it!  
1. Check the Ideas page to ensure it hasn’t already been suggested.
2. Start a new discussion in the "Ideas" category with:
   - A detailed explanation of your idea.
   - How it could improve wiRedPanda.
   - Examples, if applicable.

### Submitting Code
If you're familiar with C++ and Qt development:
1. Fork the repository.
2. Create a feature branch:
   git checkout -b feature/your-feature-name
3. Make your changes and commit them with clear, descriptive messages.
4. Push your changes to your fork and open a pull request (PR).
5. Clearly explain your PR:
   - What it does.
   - Why it's needed.
   - Any related issues or discussions.

---

## Development Workflow

### Prerequisites
- C++ Compiler (supporting C++17 or later).
- Qt Development Environment (preferably Qt 6.x).
- Git for version control.

### Setting Up
1. Clone the repository:
   git clone https://github.com/GIBIS-UNIFESP/wiRedPanda.git
2. Navigate to the project directory:
   cd wiRedPanda
3. Open the project in your preferred IDE or Qt Creator.

### Building and Running
1. Configure the project using Qt Creator or qmake.
2. Build the project using your IDE or:
   make
3. Run the application to test your changes.

---

## Style Guide

To maintain consistency, follow these guidelines:
- **C++**: Use consistent indentation (4 spaces) and adhere to modern C++ practices.
- **Qt**: Prefer Qt's APIs and conventions over standard C++ when applicable.
- **Git Commits**: Write clear commit messages, e.g., `Fix: Resolved crash on loading large circuits`.

### Example Commit Message Format:
Fix: Corrected circuit rendering bug

The rendering pipeline for large circuits caused a crash due to unhandled edge cases. This fix ensures proper handling.

---

## Resources

- **Documentation**: Qt Documentation  
- **Bug Tracker**: GitHub Issues  
- **Discussions**: GitHub Discussions  

---

Thank you for contributing to wiRedPanda and helping us build a better tool for exploring and learning about logic circuits!
