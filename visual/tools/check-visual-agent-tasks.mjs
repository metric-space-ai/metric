#!/usr/bin/env node
/*
 * Agent-task registry gate.
 *
 * The visual engine work is coordinated through docs/visual/agent-tasks.
 * This check prevents drift between the registry table and the actual task
 * files. It does not judge whether a task is complete; it only checks that the
 * executable work orders remain addressable and mechanically consistent.
 */

import { readdir, readFile } from "node:fs/promises";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, "..", "..");
const TASK_DIR = resolve(ROOT, "docs", "visual", "agent-tasks");
const README = resolve(TASK_DIR, "README.md");
const AGENT_ID_PATTERN = /^019[a-f0-9-]{32,}$/;

async function main() {
  const readme = await readFile(README, "utf8");
  const registered = parseTaskRegistry(readme);
  const files = (await readdir(TASK_DIR))
    .filter((name) => name.endsWith(".md") && name !== "README.md")
    .sort();
  const registeredNames = new Set(registered.map((entry) => entry.file));
  const fileNames = new Set(files);
  const issues = [];

  for (const file of registeredNames) {
    if (!fileNames.has(file)) issues.push({ code: "registered-task-file-missing", file });
  }
  for (const file of fileNames) {
    if (!registeredNames.has(file)) issues.push({ code: "task-file-not-in-registry", file });
  }
  for (const entry of registered) {
    if (!entry.status) issues.push({ code: "task-status-missing", file: entry.file });
    if (/assigned to subagent/.test(entry.status)) {
      const match = /`([^`]+)`/.exec(entry.status);
      if (!match || !AGENT_ID_PATTERN.test(match[1])) {
        issues.push({ code: "assigned-task-missing-agent-id", file: entry.file, status: entry.status });
      }
    }
    if (/accepted/i.test(entry.status) && /review-pending/i.test(entry.status)) {
      issues.push({ code: "ambiguous-accepted-and-review-pending-status", file: entry.file, status: entry.status });
    }
  }

  const report = {
    ok: issues.length === 0,
    registry: README,
    taskDir: TASK_DIR,
    registeredTaskCount: registered.length,
    taskFileCount: files.length,
    assignedTaskCount: registered.filter((entry) => /assigned to subagent/.test(entry.status)).length,
    reviewPendingTaskCount: registered.filter((entry) => /review-pending/.test(entry.status)).length,
    issues,
  };
  console.log(JSON.stringify(report, null, 2));
  if (!report.ok) process.exitCode = 1;
}

function parseTaskRegistry(readme) {
  const rows = [];
  for (const line of readme.split(/\r?\n/)) {
    const match = /^\|\s*`([^`]+\.md)`\s*\|\s*([^|]+?)\s*\|\s*([^|]+?)\s*\|/.exec(line);
    if (!match) continue;
    rows.push({
      file: match[1].trim(),
      ownerScope: match[2].trim(),
      status: match[3].trim(),
    });
  }
  return rows;
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
