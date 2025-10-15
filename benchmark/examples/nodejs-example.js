#!/usr/bin/env node
// Source: https://docs.claude.com/en/docs/claude-code/statusline

const fs = require('fs');
const path = require('path');

// Read JSON from stdin
let input = '';
process.stdin.on('data', chunk => input += chunk);
process.stdin.on('end', () => {
    const data = JSON.parse(input);

    // Extract values
    const model = data.model.display_name;
    const currentDir = path.basename(data.workspace.current_dir);

    // Check for git branch
    let gitBranch = '';
    try {
        const headContent = fs.readFileSync('.git/HEAD', 'utf8').trim();
        if (headContent.startsWith('ref: refs/heads/')) {
            gitBranch = ` | 🌿 ${headContent.replace('ref: refs/heads/', '')}`;
        }
    } catch (e) {
        // Not a git repo or can't read HEAD
    }

    console.log(`[${model}] 📁 ${currentDir}${gitBranch}`);
});
