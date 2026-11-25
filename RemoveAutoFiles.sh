#!/bin/bash
set -e
# FIX: -print0 + xargs -0 — безопасно для путей с пробелами
find . -type f -name "auto_*" -print0 | xargs -0 -r rm -f
