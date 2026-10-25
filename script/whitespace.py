#!/usr/bin/env python3
"""Checks or removes trailing whitespace in the source files; C/C++ files must end with exactly one newline.

usage: script/whitespace.py [--apply] [repository]
  without --apply: lists the files that need fixing, exit code 1 when there are any
  --apply:         fixes them (line endings, CRLF or LF, are kept)
  repository:      defaults to the repository this script is in

Covers the files git knows (without git: all files except build trees): C/C++, assembly, Python, CMake and shell/batch
scripts. The build runs it (the whitespacecheck target in CMakeLists.txt). The third-party paths listed in
.clang-format-ignore are left as they were imported; Markdown is skipped (two trailing spaces are a line break).
"""
import fnmatch
import pathlib
import re
import subprocess
import sys

CPP = (".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".inc")
OTHER_SOURCE = (".asm", ".py", ".cmake", ".sh", ".bat", ".s")
NAMES = ("CMakeLists.txt",)
TRAILING = re.compile(rb"[ \t]+(?=\r?\n|$)")


def third_party_patterns(repository):
    patterns = []
    for line in (repository / ".clang-format-ignore").read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if line and not line.startswith("#"):
            patterns.append(line)
    return patterns


def all_files(repository):
    # the files git knows; a copy without .git (a source archive): all files, except build trees
    try:
        listing = subprocess.run(["git", "-C", str(repository), "ls-files"], capture_output=True, text=True,
                                 check=True)
        return listing.stdout.splitlines()
    except (OSError, subprocess.CalledProcessError):
        files = []
        for file in repository.rglob("*"):
            relative = file.relative_to(repository)
            if file.is_file() and not any(part.startswith(("build", ".")) for part in relative.parts[:-1]):
                files.append(relative.as_posix())
        return files


def source_files(repository):
    ignore = third_party_patterns(repository)
    for path in all_files(repository):
        lower = path.lower()
        if not (lower.endswith(CPP) or lower.endswith(OTHER_SOURCE) or path.rsplit("/", 1)[-1] in NAMES):
            continue
        if any(fnmatch.fnmatch(path, pattern) for pattern in ignore):
            continue
        yield path, lower.endswith(CPP)


def cleaned(data, is_cpp):
    result = TRAILING.sub(b"", data)
    if is_cpp and result:
        newline = b"\r\n" if b"\r\n" in result else b"\n"
        result = result.rstrip(b"\r\n") + newline  # exactly one newline at the end
    return result


def main():
    arguments = [argument for argument in sys.argv[1:] if argument != "--apply"]
    apply = "--apply" in sys.argv[1:]
    repository = pathlib.Path(arguments[0]) if arguments else pathlib.Path(__file__).resolve().parent.parent

    needs_fixing = []
    for path, is_cpp in source_files(repository):
        file = repository / path
        data = file.read_bytes()
        result = cleaned(data, is_cpp)
        if result == data:
            continue
        needs_fixing.append(path)
        lines = len(TRAILING.findall(data))
        no_newline = is_cpp and not data.endswith(b"\n")
        print(f"{lines:5} trailing{'  no final newline' if no_newline else ''}  {path}")
        if apply:
            file.write_bytes(result)

    if not needs_fixing:
        print("no trailing whitespace, all C/C++ files end with a newline")
        return 0
    print(f"{len(needs_fixing)} files {'fixed' if apply else 'need fixing (run with --apply)'}")
    return 0 if apply else 1


if __name__ == "__main__":
    sys.exit(main())
