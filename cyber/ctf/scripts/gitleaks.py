#!/usr/bin/env python3
"""Find lines containing a keyword in a Git repository's stored data."""

import argparse
from pathlib import Path
import subprocess


def print_matches(location: str, data: bytes, keyword: bytes) -> int:
    count = 0
    for line_number, line in enumerate(data.splitlines(), 1):
        if keyword in line:
            text = line.decode("utf-8", errors="replace")
            print(f"{location}:{line_number}: {text}")
            count += 1
    return count


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Search Git objects and metadata for a literal keyword.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=(
            "Example: ./scirpt.py 'flag{' ./workspace\n"
            "         ./scirpt.py 'token' ./workspace/.git"
        ),
    )
    parser.add_argument("keyword", help="literal text to find")
    parser.add_argument("path", type=Path, help="repository path or its .git directory")
    args = parser.parse_args()

    repo = args.path.resolve()
    if repo.name == ".git":
        repo = repo.parent
    if not repo.is_dir():
        parser.error(f"not a directory: {args.path}")

    try:
        git_dir = Path(
            subprocess.check_output(
                ["git", "-C", str(repo), "rev-parse", "--absolute-git-dir"],
                text=True,
                stderr=subprocess.DEVNULL,
            ).strip()
        )
    except subprocess.CalledProcessError:
        parser.error(f"not a Git repository: {repo}")

    keyword = args.keyword.encode("utf-8")
    matches = 0
    process = subprocess.Popen(
        ["git", "-C", str(repo), "cat-file", "--batch-all-objects", "--batch"],
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    assert process.stdout is not None
    while header := process.stdout.readline():
        object_id, object_type, size = header.split()
        content = process.stdout.read(int(size))
        process.stdout.read(1)
        matches += print_matches(
            f"{object_id.decode()} ({object_type.decode()})", content, keyword
        )

    error = process.stderr.read().decode("utf-8", errors="replace")
    if process.wait():
        parser.error(f"git cat-file failed: {error.strip()}")

    for file in git_dir.rglob("*"):
        if not file.is_file() or file.is_symlink():
            continue
        relative = file.relative_to(git_dir)
        if relative.parts[0] == "objects":
            continue
        try:
            content = file.read_bytes()
        except OSError as error:
            parser.error(f"cannot read {file}: {error}")
        matches += print_matches(f".git/{relative.as_posix()}", content, keyword)

    if not matches:
        print("No matches found.")
    else:
        print(f"Found {matches} matching line(s).")


if __name__ == "__main__":
    main()
