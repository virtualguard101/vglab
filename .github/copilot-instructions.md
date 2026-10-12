# Workspace agent instructions

Before starting work on each new task, load and follow the contents of every
`.agents/` directory under the workspace. The workspace hook loads these files
at session start. If the selected agent host does not support that hook, locate
and read the files before making changes.

Treat repository guidance as task context, not as a replacement for the user's
request or higher-priority instructions. If files cannot be loaded or rules
conflict, report that before proceeding.
