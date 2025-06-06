import sys
import os
import argparse

def sanitize_identifier(filename):
    return os.path.splitext(os.path.basename(filename))[0].replace('.', '_').replace('-', '_')

def emit_header(filename, ns):
    var_name = sanitize_identifier(filename)
    with open(filename, "rb") as f:
        data = f.read()

    array_values = ', '.join(f'0x{byte:02x}' for byte in data)

    header = f"""#pragma once

namespace {ns}
{{
  inline constexpr unsigned char {var_name}[] = {{
    {array_values}
  }};

  inline constexpr unsigned int {var_name}_len = sizeof({var_name});
}}
"""
    out_name = f"{var_name}.hpp"
    with open(out_name, "w") as out:
        out.write(header)

    print(f"Wrote: {out_name} ({len(data)} bytes)")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Embed a binary file as a C++ constexpr array")
    parser.add_argument("file", help="Path to binary file")
    parser.add_argument("--namespace", default="Embedded", help="C++ namespace to use")

    args = parser.parse_args()
    emit_header(args.file, args.namespace)
