import os

LICENSE_HEADER = '''\
/*
 * Copyright (C) 2025 Nicholas Reimer <nicholas.hans@gmail.com>
 *
 * This file is part of a project licensed under the GNU Affero General Public License v3.0,
 * with an additional non-commercial use restriction.
 *
 * You may redistribute and/or modify this file under the terms of the GNU AGPLv3 as
 * published by the Free Software Foundation, provided that your use is strictly non-commercial.
 *
 * This software is provided "as-is", without any warranty of any kind.
 * See the LICENSE file in the root of the repository for full license terms.
 *
 * SPDX-License-Identifier: AGPL-3.0-only
 */

'''

# Extensions to target
TARGET_EXTENSIONS = {'.cpp', '.hpp'}

def has_license_header(content):
    return "SPDX-License-Identifier: AGPL-3.0-only" in content

def add_license_to_file(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    if has_license_header(content):
        print(f"Already licensed: {file_path}")
        return

    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(LICENSE_HEADER + content)
    print(f"License added: {file_path}")

def walk_and_add_license(root_dir):
    for root, _, files in os.walk(root_dir):
        for file in files:
            if os.path.splitext(file)[1] in TARGET_EXTENSIONS:
                full_path = os.path.join(root, file)
                add_license_to_file(full_path)

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        print("Usage: python add_license_header.py /path/to/your/codebase")
        exit(1)

    target_dir = sys.argv[1]
    if not os.path.isdir(target_dir):
        print(f"Directory does not exist: {target_dir}")
        exit(1)

    walk_and_add_license(target_dir)
