#!/usr/bin/env python3
"""Build both documentation languages with strict MkDocs validation."""
import argparse
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path, default=root.parent / 'build-docs')
args = parser.parse_args()
output = args.output.resolve()
if output == root.parent or output in root.parents or output == root or root in output.parents:
    parser.error('Output must be a build directory outside the documentation sources')
for language, prefix in [('en', 'doc'), ('ru', 'doc_ru')]:
    subprocess.run([sys.executable, '-m', 'mkdocs', 'build', '--strict', '--clean',
                    '-f', str(root / language / 'mkdocs.yml'),
                    '--site-dir', str(output / prefix)], check=True)
print(f'Built documentation: {output}/doc and {output}/doc_ru')
