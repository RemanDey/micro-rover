const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

const repoRoot = process.cwd();
const outDir = path.join(repoRoot, 'docs', 'diagrams');
const tmpDir = path.join(repoRoot, '.github', '.mermaid_tmp');

function ensureDir(d) {
  if (!fs.existsSync(d)) fs.mkdirSync(d, { recursive: true });
}

ensureDir(outDir);
ensureDir(tmpDir);

function walk(dir) {
  let results = [];
  const list = fs.readdirSync(dir);
  list.forEach(function(file) {
    const full = path.join(dir, file);
    const stat = fs.statSync(full);
    if (stat && stat.isDirectory()) {
      results = results.concat(walk(full));
    } else if (file.endsWith('.md')) {
      results.push(full);
    }
  });
  return results;
}

const mdFiles = walk(repoRoot);
let generated = 0;

mdFiles.forEach(mdPath => {
  const rel = path.relative(repoRoot, mdPath);
  const content = fs.readFileSync(mdPath, 'utf8');
  const re = /```(?:mermaid(?:[\s\S]*?)?)\n([\s\S]*?)\n```/g;
  let m;
  let idx = 0;
  while ((m = re.exec(content)) !== null) {
    const code = m[1].trim();
    if (!code) continue;
    idx += 1;
    const safeName = (rel.replace(/[\\/\\\\]/g, '_') + `_diagram_${idx}`).replace(/[^a-zA-Z0-9_\-\.]/g, '_');
    const tmpFile = path.join(tmpDir, `${safeName}.mmd`);
    const outFile = path.join(outDir, `${safeName}.svg`);
    fs.writeFileSync(tmpFile, code, 'utf8');
    try {
      console.log(`Rendering ${rel} #${idx} -> ${path.relative(repoRoot, outFile)}`);
      execSync(`mmdc -i "${tmpFile}" -o "${outFile}" --backgroundColor transparent`, { stdio: 'inherit' });
      generated += 1;
    } catch (err) {
      console.error('Failed rendering:', err.message || err);
    }
  }
});

// cleanup
try {
  fs.rmSync(tmpDir, { recursive: true, force: true });
} catch (e) {}

console.log(`Rendered ${generated} diagrams.`);
if (generated === 0) process.exit(0);
