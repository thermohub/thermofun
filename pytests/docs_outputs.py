# SPDX-License-Identifier: LGPL-3.0-or-later
# Copyright (C) 2026 ThermoFun contributors

"""Run the code of a documentation page and check the outputs that the page shows.

- A ```python block followed by a line `Output:` and a ```text block: the text is what the block prints.
- A ```console block: each line `$ python -m thermofun.<tool> ...` runs (with `> file` to write a file), and the lines after
  it, up to the next `$`, are what it prints. `$ cat file`, `$ head -n N file` and `$ ls folder` show files. Only these
  commands run.

Lines are compared without their trailing spaces, and blank lines at the start and the end do not count. Numbers are
compared with a relative tolerance of 1e-6: the last digits of a float differ between platforms (macOS, Windows, Linux).
In a shown line, `...` stands for any text (for a version or a checksum that changes with each release).
"""

import contextlib
import io
import re
import shlex
import subprocess
import sys

NUMBER = re.compile(r"[-+]?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][-+]?\d+)?")
OUTPUT = re.compile(r"\A\n+Output:\n+```text\n(?P<text>.*?)```", re.S)


def _lines(text):
    lines = [line.rstrip() for line in text.splitlines()]
    while lines and not lines[0]:
        lines.pop(0)
    while lines and not lines[-1]:
        lines.pop()
    return lines


def _same(printed, shown):
    """The lines are the same, with numbers equal to a relative tolerance of 1e-6."""
    a, b = _lines(printed), _lines(shown)
    if len(a) != len(b):
        return False
    for x, y in zip(a, b):
        if "..." in y:
            if not re.fullmatch(".*".join(re.escape(part) for part in y.split("...")), x):
                return False
            continue
        if NUMBER.sub("#", x) != NUMBER.sub("#", y):
            return False
        for u, v in zip(NUMBER.findall(x), NUMBER.findall(y)):
            fu, fv = float(u), float(v)
            if abs(fu - fv) > 1e-6 * max(abs(fu), abs(fv)) + 1e-12:
                return False
    return True


def expected_output(page_text, end):
    """The text of the `Output:` block right after a code block that ends at `end`, or None."""
    m = OUTPUT.match(page_text[end:])
    return m["text"] if m else None


def run_python(code, names, name, expected=None):
    printed = io.StringIO()
    with contextlib.redirect_stdout(printed):
        exec(compile(code, name, "exec"), names)
    if expected is not None:
        assert _same(printed.getvalue(), expected), f"{name}: the block prints\n{printed.getvalue()}\nthe page shows\n{expected}"


def run_console(text, name, cwd):
    """Run the commands of a console block and compare what they print with the lines under each command."""
    commands = []
    for line in text.splitlines():
        if line.startswith("$ "):
            commands.append([line[2:], []])
        elif commands:
            commands[-1][1].append(line)
    for command, shown in commands:
        words = shlex.split(command)
        target = None
        if ">" in words:
            target = words[words.index(">") + 1]
            words = words[:words.index(">")]
        if words[0] == "cat":
            printed = (cwd / words[1]).read_text()
        elif words[0] == "head":
            printed = "\n".join((cwd / words[3]).read_text().splitlines()[:int(words[2])])
        elif words[0] == "ls":
            printed = "\n".join(sorted(p.name for p in (cwd / words[1]).iterdir()))
        else:
            assert words[:2] == ["python", "-m"] and words[2].startswith("thermofun."), f"{name}: only python -m thermofun.<tool> runs: {command}"
            done = subprocess.run([sys.executable, "-m"] + words[2:], cwd=cwd, capture_output=True, text=True, timeout=600)
            printed = done.stdout + done.stderr
            if target:
                (cwd / target).write_text(done.stdout)
                printed = done.stderr
        assert _same(printed, "\n".join(shown)), f"{name}: `{command}` prints\n{printed}\nthe page shows\n" + "\n".join(shown)
