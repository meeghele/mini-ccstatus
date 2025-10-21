#!/usr/bin/env node

// Simple script to read JSON from stdin and print it as a single line

let input = '';
process.stdin.on('data', chunk => input += chunk);
process.stdin.on('end', () => {
    try {
        const data = JSON.parse(input);
        console.log(JSON.stringify(data));
    } catch (e) {
        process.stderr.write(`Error parsing JSON: ${e.message}\n`);
        process.exit(1);
    }
});
