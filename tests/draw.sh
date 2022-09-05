#!/bin/bash

if [[ $# -ne 1 ]]; then
  echo "usage: draw.sh {C_SOURCE_CODE}"
  exit 1
fi

path="$(pwd)/$1"

cd "$(dirname "$0")" || exit 1
cd ../build/ || exit 1
./informater -t "$path"
dot -Tsvg AST-graph.dot -o AST-graph.svg

