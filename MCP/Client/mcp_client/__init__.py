"""MCP client core modules.

Houses the infrastructure, test orchestration, validation, and protocol
helpers used by the MCP test suite and example clients. Keeping these
together as a real package (rather than the previous flat layout) lets
import-linter walk their import graph and enforce layering contracts —
see pyproject.toml for the contracts in force.
"""
