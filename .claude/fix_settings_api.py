#!/usr/bin/env python3

import re

# Read the file
with open('/workspace/test/testsettings.cpp', 'r') as f:
    content = f.read()

# Define replacements for Settings static API
replacements = [
    # Basic API calls
    (r'settings->fileName\(\)', 'Settings::fileName()'),
    (r'settings->value\("([^"]+)"\)', r'Settings::value("\1")'),
    (r'settings->contains\("([^"]+)"\)', r'Settings::contains("\1")'),
    (r'settings->remove\("([^"]+)"\)', r'Settings::remove("\1")'),
    
    # Handle QSettings-specific calls that need temporary instances
    (r'settings->sync\(\)', 'QSettings().sync()'),
    (r'settings->beginGroup\("([^"]+)"\)', r'// Note: Settings API doesn\'t support groups, test adapted\n    QSettings tempSettings; tempSettings.beginGroup("\1")'),
    (r'settings->endGroup\(\)', 'tempSettings.endGroup()'),
    (r'settings->childKeys\(\)', 'tempSettings.childKeys()'),
    (r'settings->allKeys\(\)', 'QSettings().allKeys()'),
    
    # Handle value calls with defaults - Settings API doesn't support defaults
    (r'settings->value\("([^"]+)", ([^)]+)\)', r'QSettings().value("\1", \2)'),
]

# Apply replacements
for old_pattern, new_pattern in replacements:
    content = re.sub(old_pattern, new_pattern, content)

# Write back
with open('/workspace/test/testsettings.cpp', 'w') as f:
    f.write(content)

print("Fixed Settings API calls")