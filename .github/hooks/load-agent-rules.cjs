'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { TextDecoder } = require('node:util');

const ignoredDirectories = new Set(['.git', 'node_modules']);
let input = '';

process.stdin.setEncoding('utf8');
process.stdin.on('data', (chunk) => {
  input += chunk;
});

process.stdin.on('end', () => {
  try {
    const event = JSON.parse(input || '{}');
    const workspaceRoot = path.resolve(event.cwd || process.cwd());
    const agentDirectories = [];

    function findAgentDirectories(directory) {
      const entries = fs.readdirSync(directory, { withFileTypes: true });
      entries.sort((a, b) => a.name.localeCompare(b.name));

      for (const entry of entries) {
        if (entry.isSymbolicLink() || !entry.isDirectory()) {
          continue;
        }

        const fullPath = path.join(directory, entry.name);
        if (entry.name === '.agents') {
          agentDirectories.push(fullPath);
        }

        if (!ignoredDirectories.has(entry.name)) {
          findAgentDirectories(fullPath);
        }
      }
    }

    findAgentDirectories(workspaceRoot);

    const files = new Map();
    function collectFiles(directory) {
      const entries = fs.readdirSync(directory, { withFileTypes: true });
      entries.sort((a, b) => a.name.localeCompare(b.name));

      for (const entry of entries) {
        if (entry.isSymbolicLink()) {
          continue;
        }

        const fullPath = path.join(directory, entry.name);
        if (entry.isDirectory()) {
          collectFiles(fullPath);
        } else if (entry.isFile()) {
          files.set(fullPath, fs.readFileSync(fullPath));
        }
      }
    }

    for (const directory of agentDirectories) {
      collectFiles(directory);
    }

    const decoder = new TextDecoder('utf-8');
    const sections = [...files.entries()]
      .sort(([a], [b]) => a.localeCompare(b))
      .map(([filePath, contents]) => {
        const relativePath = path.relative(workspaceRoot, filePath);
        return `### ${relativePath}\n\n${decoder.decode(contents)}`;
      });

    const skipped = 'Symlinks, .git, and node_modules are not followed.';
    const context = sections.length
      ? `Repository guidance from all discovered .agents directories follows. Apply relevant rules to this task.\n\n${sections.join('\n\n---\n\n')}\n\n${skipped}`
      : `No .agents directories were found under ${workspaceRoot}. ${skipped}`;

    const output = event.hook_event_name === 'SessionStart'
      ? {
          hookSpecificOutput: {
            hookEventName: 'SessionStart',
            additionalContext: context
          }
        }
      : { additionalContext: context };

    process.stdout.write(JSON.stringify(output));
  } catch (error) {
    process.stderr.write(`Unable to load workspace .agents files: ${error.message}\n`);
    process.exitCode = 2;
  }
});
