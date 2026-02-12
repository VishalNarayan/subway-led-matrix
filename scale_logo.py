#!/usr/bin/env python3
"""
Script to scale train logos from 20x20 to 10x10 pixels
"""

import re

# Read the A train array from the file
with open('src/assets/logos/train_logos20x20.h', 'r') as f:
    content = f.read()
    
# Extract the A train array (between the braces)
start = content.find('a_train [] PROGMEM = {') + len('a_train [] PROGMEM = {')
end = content.find('};', start)
array_str = content[start:end]

# Parse the hex values
values = re.findall(r'0x[0-9a-fA-F]+', array_str)
a_train_20x20 = [int(v, 16) for v in values]

print(f"Loaded {len(a_train_20x20)} pixels (20x20 = 400)")

def scale_20x20_to_10x10(image_20x20):
    """Scale a 20x20 image to 10x10 by sampling every other pixel"""
    image_10x10 = []
    for y in range(0, 20, 2):  # Take every other row (0, 2, 4, ..., 18)
        for x in range(0, 20, 2):  # Take every other column (0, 2, 4, ..., 18)
            index = y * 20 + x
            image_10x10.append(image_20x20[index])
    return image_10x10

# Scale the A train
a_train_10x10 = scale_20x20_to_10x10(a_train_20x20)

print(f"Scaled to {len(a_train_10x10)} pixels (10x10 = 100)")

# Format as C array
print("\n// A Train (10x10 scaled)")
print("const uint16_t a_train_10x10 [] PROGMEM = {")
for i in range(0, len(a_train_10x10), 10):
    row = a_train_10x10[i:i+10]
    line = "\t" + ", ".join(f"0x{val:04x}" for val in row)
    if i+10 < len(a_train_10x10):
        line += ","
    print(line)
print("};")
