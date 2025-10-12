#!/usr/bin/bash

SCRIPT_PATH="$(readlink -f "$0")"
SCRIPT_DIR="$(dirname "${SCRIPT_PATH}")"
ROOT_DIR="$(readlink -f ${SCRIPT_DIR})"

FILES_FOR_FORMATTING="$(find "${ROOT_DIR}" |
 grep -vP "build"|
 grep -P "\.[c|h|t](pp)?$")"

clang-format --style=file -i ${FILES_FOR_FORMATTING}
