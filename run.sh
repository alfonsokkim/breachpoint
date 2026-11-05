#!/usr/bin/env bash
set -eu
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cmake -S "$project_dir" -B "$project_dir/build"
cmake --build "$project_dir/build" -j 6
exec "$project_dir/build/Craft"
