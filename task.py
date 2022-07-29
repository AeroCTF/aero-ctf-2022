#!/usr/bin/env python3

import json
from pathlib import Path
from enum import Enum
import re

import typer

app = typer.Typer()

flag_re = re.compile(r"""Aero\{[\w\d_\-+=!@#$%^&*()"';:<>/]+\}""")


class Category(str, Enum):
    crypto = "crypto"
    forensics = "forensics"
    misc = "misc"
    osint = "osint"
    pwn = "pwn"
    reverse = "reverse"
    web = "web"


@app.command(short_help="add new task")
def add(
    task_category: Category,
    task_name: str,
):
    base_path = Path(".") / "tasks" / task_category.value / task_name  # type: Path

    if base_path.exists():
        typer.echo(f"Task {task_category} / {task_name} already exist")
        raise typer.Exit(code=1)

    base_path.mkdir()
    for d in ["deploy", "dev", "public", "solution"]:
        (base_path / d).mkdir()

    (base_path / "desc.md").write_text("Task description")
    (base_path / "task.json").write_text(
        json.dumps(
            {
                "name": task_name,
                "author": "change me",
                "flag": "Aero{change_me}",
            }
        )
    )


@app.command(short_help="validate tasks")
def validate():
    for category in Path("tasks").glob("*"):
        if not category.is_dir():
            continue

        if category.name[0] == ".":
            continue

        for task in category.glob("*"):
            if not task.is_dir():
                continue

            task_dirs = set(i.name for i in task.glob("*") if i.is_dir())
            valid_dirs = set(["deploy", "dev", "public", "solution"])
            if len(task_dirs - valid_dirs) != 0:
                typer.echo(f"[!] [{category.name} / {task.name}] Found extra directory: {task_dirs - valid_dirs}")
                break

            if not (task / "desc.md").exists():
                typer.echo(f"[!] [{category.name} / {task.name}] desc.md not found")
                break

            if not (task / "task.json").exists():
                typer.echo(f"[!] [{category.name} / {task.name}] task.json not found")
                break

            task_info = json.loads((task / "task.json").read_text())
            valid_keys = set(["name", "author", "flag"])
            if set(task_info.keys()) != valid_keys:
                typer.echo(f"[!] [{category.name} / {task.name}] Invalid task.json: {task_info}")
                break

            if not flag_re.match(task_info["flag"]):
                typer.echo(f"[!] [{category.name} / {task.name}] Invalid flag: {task_info['flag']}")
                break

            typer.echo(f"[+] [{category.name} / {task.name}] good!")


if __name__ == "__main__":
    app()
