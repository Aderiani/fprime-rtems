#!/usr/bin/env python3
"""
Script to manage GitHub workflow states for GR740 development
"""

import os
import sys
import yaml
import argparse
from pathlib import Path

# Workflows to keep active during GR740 development
KEEP_ACTIVE = [
    "build-test.yml",
    "build-test-gr740.yml",
    "python-format.yml",
    "cppcheck-scan.yml",
    "cpplint-scan.yml",
    "pip-check.yml",
]

# Workflows to temporarily disable
DISABLE_TEMP = [
    "ext-build-examples-repo.yml",
    "ext-build-hello-world.yml",
    "spelling.yml",
    "ext-build-led-blinker.yml",
    "ext-build-math-comp.yml",
    "ext-raspberry-led-blinker.yml",
    "build-test-rpi.yml",
    "build-test-macos.yml",
    "codeql-jpl-standard.yml",
    "cookiecutters-test.yml",
    "fpp-tests.yml",  # May need RTEMS adjustments
]

# Workflows that need modification
MODIFY_LIST = [
    "cmake-test.yml",  # Needs RTEMS toolchain tests
]


def disable_workflow(workflow_path):
    """Disable a workflow by changing its trigger to workflow_dispatch only"""
    with open(workflow_path, "r") as f:
        content = f.read()

    # Check if already disabled
    if "workflow_dispatch:" in content and "push:" not in content.replace(
        "# push:", ""
    ):
        print(f"  Already disabled: {workflow_path.name}")
        return

    # Simple approach: comment out push and pull_request triggers
    lines = content.split("\n")
    new_lines = []
    in_on_section = False
    indent_level = 0

    for line in lines:
        if line.strip() == "on:":
            in_on_section = True
            new_lines.append(line)
            new_lines.append(
                "  workflow_dispatch:  # Manual trigger only during GR740 development"
            )
        elif in_on_section:
            if line.strip() and not line.startswith(" "):
                in_on_section = False
                new_lines.append(line)
            elif (
                "push:" in line
                or "pull_request:" in line
                or "pull_request_target:" in line
            ):
                new_lines.append("  # " + line.strip())
            elif line.strip().startswith("branches:") or line.strip().startswith(
                "paths"
            ):
                new_lines.append("  # " + line.strip())
            elif line.strip().startswith("- "):
                new_lines.append("  #   " + line.strip())
            else:
                new_lines.append(line)
        else:
            new_lines.append(line)

    with open(workflow_path, "w") as f:
        f.write("\n".join(new_lines))

    print(f"  Disabled: {workflow_path.name}")


def enable_workflow(workflow_path):
    """Re-enable a workflow by uncommenting triggers"""
    with open(workflow_path, "r") as f:
        content = f.read()

    # Remove the manual trigger comment
    content = content.replace(
        "  workflow_dispatch:  # Manual trigger only during GR740 development\n", ""
    )

    # Uncomment push and pull_request triggers
    lines = content.split("\n")
    new_lines = []

    for line in lines:
        if line.strip().startswith("# push:") or line.strip().startswith(
            "# pull_request"
        ):
            new_lines.append(line.replace("# ", "", 1))
        elif line.strip().startswith("#   - ") or line.strip().startswith(
            "# branches:"
        ):
            new_lines.append(line.replace("# ", "", 1))
        else:
            new_lines.append(line)

    with open(workflow_path, "w") as f:
        f.write("\n".join(new_lines))

    print(f"  Enabled: {workflow_path.name}")


def list_workflows(workflow_dir):
    """List all workflows and their status"""
    print("\nWorkflow Status:")
    print("-" * 60)

    for workflow_file in sorted(workflow_dir.glob("*.yml")):
        with open(workflow_file, "r") as f:
            content = f.read()

        status = "ACTIVE"
        if "workflow_dispatch:" in content and "# push:" in content:
            status = "DISABLED"
        elif workflow_file.name in MODIFY_LIST:
            status = "NEEDS MODIFICATION"

        category = "KEEP"
        if workflow_file.name in DISABLE_TEMP:
            category = "DISABLE"
        elif workflow_file.name in MODIFY_LIST:
            category = "MODIFY"

        print(f"{workflow_file.name:<35} {status:<15} {category}")


def main():
    parser = argparse.ArgumentParser(
        description="Manage GitHub workflows for GR740 development"
    )
    parser.add_argument(
        "action",
        choices=["disable", "enable", "list", "disable-all", "enable-all"],
        help="Action to perform",
    )
    parser.add_argument("--workflow", help="Specific workflow file to act on")
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show what would be done without doing it",
    )

    args = parser.parse_args()

    # Find workflow directory
    script_dir = Path(__file__).parent
    workflow_dir = script_dir.parent / "workflows"

    if not workflow_dir.exists():
        print(f"Error: Workflow directory not found at {workflow_dir}")
        sys.exit(1)

    if args.action == "list":
        list_workflows(workflow_dir)

    elif args.action == "disable":
        if args.workflow:
            workflow_path = workflow_dir / args.workflow
            if workflow_path.exists():
                if not args.dry_run:
                    disable_workflow(workflow_path)
                else:
                    print(f"Would disable: {args.workflow}")
            else:
                print(f"Error: Workflow {args.workflow} not found")
        else:
            print("Error: --workflow required for disable action")

    elif args.action == "enable":
        if args.workflow:
            workflow_path = workflow_dir / args.workflow
            if workflow_path.exists():
                if not args.dry_run:
                    enable_workflow(workflow_path)
                else:
                    print(f"Would enable: {args.workflow}")
            else:
                print(f"Error: Workflow {args.workflow} not found")
        else:
            print("Error: --workflow required for enable action")

    elif args.action == "disable-all":
        print("Disabling workflows for GR740 development...")
        for workflow_name in DISABLE_TEMP:
            workflow_path = workflow_dir / workflow_name
            if workflow_path.exists():
                if not args.dry_run:
                    disable_workflow(workflow_path)
                else:
                    print(f"Would disable: {workflow_name}")

    elif args.action == "enable-all":
        print("Re-enabling all workflows...")
        for workflow_file in workflow_dir.glob("*.yml"):
            if not args.dry_run:
                enable_workflow(workflow_file)
            else:
                print(f"Would enable: {workflow_file.name}")


if __name__ == "__main__":
    main()