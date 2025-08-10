#!/usr/bin/env python3

import re

# Read the file
with open('/workspace/test/testcommands_advanced.cpp', 'r') as f:
    content = f.read()

# Fix MoveCommand calls - replace with proper oldPositions
# Pattern: MoveCommand({element}, {QPointF(x, y)}, scene)
# Replace with: MoveCommand({element}, {element->pos()}, scene) - using current position as old position
move_pattern = r'new MoveCommand\(\{([^}]+)\}, \{QPointF\([^)]+\)\}, ([^)]+)\)'
move_replacement = r'new MoveCommand({\1}, {\1->pos()}, \2)'

content = re.sub(move_pattern, move_replacement, content)

# Fix some specific cases where {newPos} is used instead of QPointF
content = re.sub(r'new MoveCommand\(\{([^}]+)\}, \{([^}]+)\}, ([^)]+)\)', r'new MoveCommand({\1}, {\1->pos()}, \3)', content)

# Fix RotateCommand calls - make sure they use proper GraphicElement lists
# Pattern: RotateCommand({element}, angle, scene)
rotate_pattern = r'new RotateCommand\(\{([^}]+)\}, ([^,]+), ([^)]+)\)'
rotate_replacement = r'new RotateCommand({\1}, \2, \3)'

# Fix DeleteItemsCommand calls - ensure they cast GraphicElement* to QGraphicsItem*
delete_pattern = r'new DeleteItemsCommand\(\{([^}]+)\}, ([^)]+)\)'
delete_replacement = r'new DeleteItemsCommand({static_cast<QGraphicsItem*>(\1)}, \2)'

content = re.sub(delete_pattern, delete_replacement, content)

# Write back
with open('/workspace/test/testcommands_advanced.cpp', 'w') as f:
    f.write(content)

print("Fixed Commands API calls")