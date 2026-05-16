"""Shared validation helpers used by both the domain and protocol layers."""

from typing import Optional


def require_port_or_label(
    source_port: Optional[int],
    source_port_label: Optional[str],
    target_port: Optional[int],
    target_port_label: Optional[str],
) -> None:
    """Raise ValueError unless each side of a connection has a port index or label."""
    if source_port is None and source_port_label is None:
        raise ValueError("Either source_port or source_port_label must be provided")
    if target_port is None and target_port_label is None:
        raise ValueError("Either target_port or target_port_label must be provided")
