import os
import re
import argparse
from pathlib import Path

def extract_namespace_blocks(content):
    """Return list of (start_line, end_line, namespace_stack) for each block."""
    lines = content.splitlines()
    namespace_stack = []
    namespace_blocks = []
    open_brace_stack = []

    for idx, line in enumerate(lines):
        ns_match = re.match(r'\s*namespace\s+([\w:]+)\s*{', line)
        if ns_match:
            namespace_stack.append(ns_match.group(1))
            open_brace_stack.append(idx)
        elif '{' in line:
            open_brace_stack.append(idx)
        elif '}' in line:
            if open_brace_stack:
                start_idx = open_brace_stack.pop()
                if namespace_stack and start_idx < idx:
                    ns = namespace_stack.pop()
                    namespace_blocks.append((start_idx, idx, ns))

    return namespace_blocks

def is_template_class(header_content):
    return re.search(r'template\s*<.*>\s*class', header_content)

def extract_class_methods_with_bodies(header_content, class_name):
    pattern = re.compile(
        rf'(?P<ret_type>[\w:&<>\[\]\s\*]+?)\s+{class_name}::(?P<func>\w+)\s*\((?P<args>[^\)]*)\)\s*(?P<const>const)?\s*{{(?P<body>.*?)^}}',
        re.DOTALL | re.MULTILINE
    )
    matches = pattern.finditer(header_content)
    stubs = []
    for match in matches:
        ret_type = match.group("ret_type").strip()
        func = match.group("func").strip()
        args = match.group("args").strip()
        const_qual = " const" if match.group("const") else ""
        body = match.group("body").strip()
        stub = f"{ret_type} {class_name}::{func}({args}){const_qual} {{\n    {body}\n}}\n"
        stubs.append(stub)
    return stubs

def extract_classes_with_methods(header_content):
    classes = {}
    lines = header_content.splitlines()
    current_class = None
    in_class = False

    for line in lines:
        class_match = re.match(r'\s*class\s+(\w+)', line)
        if class_match:
            current_class = class_match.group(1)
            in_class = True
            classes[current_class] = []
            continue
        if in_class and '{' in line:
            continue
        if in_class and '};' in line:
            in_class = False
    return list(classes.keys())

def generate_cpp_file(header_path: Path, recursive: bool):
    with open(header_path, 'r') as f:
        content = f.read()

    if is_template_class(content):
        return  # skip template classes

    class_names = extract_classes_with_methods(content)
    if not class_names:
        return

    rel_path = header_path.parent
    cpp_path = rel_path / (header_path.stem + ".cpp")

    output_lines = [f'#include "{header_path.name}"\n']

    ns_blocks = extract_namespace_blocks(content)
    ns_open = ""
    ns_close = ""
    for _, _, ns in ns_blocks:
        ns_chain = ns.split("::")
        ns_open += "\n".join([f"namespace {n} {{" for n in ns_chain]) + "\n"
        ns_close = "\n".join([f"}} // namespace {n}" for n in reversed(ns_chain)])

    output_lines.append(ns_open)

    for cls in class_names:
        stubs = extract_class_methods_with_bodies(content, cls)
        output_lines.extend(stubs)

    if ns_close:
        output_lines.append(ns_close)

    with open(cpp_path, 'w') as f:
        f.write("\n".join(output_lines))

    print(f"[Generated] {cpp_path}")

def scan_directory(path: Path, recursive: bool):
    for root, dirs, files in os.walk(path):
        for file in files:
            if file.endswith(".hpp") or file.endswith(".h"):
                full_path = Path(root) / file
                generate_cpp_file(full_path, recursive)
        if not recursive:
            break

def main():
    parser = argparse.ArgumentParser(description="Generate .cpp files from .hpp files.")
    parser.add_argument("directory", nargs="?", default=".", help="Root directory to scan")
    parser.add_argument("--recursive", action="store_true", help="Enable recursive directory scan")
    args = parser.parse_args()

    scan_directory(Path(args.directory), args.recursive)

if __name__ == "__main__":
    main()
