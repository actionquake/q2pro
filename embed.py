#!/usr/bin/python3

# loosely based on the following answer on SO: https://stackoverflow.com/a/61128721

import os, sys, shutil

# get absolute input and output paths
input_path = sys.argv[1]
output_path = sys.argv[2]

# make sure destination directory exists
dest_dir = os.path.dirname(output_path)
if len(dest_dir) > 0:
    os.makedirs(dest_dir, exist_ok = True)

# read in the file
in_file = open(input_path, "rb")
in_contents = in_file.read()
in_file.close()

# convert
size = len(in_contents)

escaped_contents = ""
current_line = ""
def flush_line():
    global current_line
    global escaped_contents
    if len(current_line) > 0:
        escaped_contents += f'\t"{current_line}"\n'
    current_line = ""

for c in in_contents:
    if c == ord(b'\\'):
        current_line += '\\\\'
    elif c == ord(b'\n'):
        current_line += '\\n'
        flush_line()
    elif c == ord(b'\r'):
        current_line += '\\r'
    elif c == ord(b'"'):
        current_line += '\\"'
    elif c >= 32 and c < 128:
        current_line += chr(c)
    else:
        current_line += f'\\x{c:02x}" "'
    if len(current_line) >= 132:
        flush_line()

ident = sys.argv[3]
in_contents = '\n'.join(['#include <stddef.h>',
    f'const char {ident}[] = {{',
    escaped_contents.rstrip(),
    '};',
    f'const size_t {ident}_size = {size};'
])

# write
out_file = open(output_path, "w", encoding='ascii')
out_contents = out_file.write(in_contents)
out_file.close()
