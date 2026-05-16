"""Shared validation helpers used by both the domain and protocol layers."""


def require_port_or_label(
    source_port: int | None,
    source_port_label: str | None,
    target_port: int | None,
    target_port_label: str | None,
) -> None:
    """Raise ValueError unless each side of a connection has a port index or label."""
    if source_port is None and source_port_label is None:
        raise ValueError("Either source_port or source_port_label must be provided")
    if target_port is None and target_port_label is None:
        raise ValueError("Either target_port or target_port_label must be provided")
