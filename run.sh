#!/usr/bin/env bash
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-RelWithDebInfo}"
BIN_REL_PATH="src/TwentyAndFiveEyeRestApp"
RELEASE_DIR="${ROOT_DIR}/release_versions"
VERSION_FILE="${ROOT_DIR}/version.txt"

if [ ! -f "$VERSION_FILE" ]; then echo "0.1.0" > "$VERSION_FILE"; fi
read_version(){ cat "$VERSION_FILE"; }
increment_version_patch(){ ver=$(read_version); IFS='.' read -r -a parts <<< "$ver"; major=${parts[0]:-0}; minor=${parts[1]:-0}; patch=${parts[2]:-0}; patch=$((patch+1)); echo "${major}.${minor}.${patch}"; }

menu(){ cat <<EOF
TwentyAndFiveEyeRest helper menu:
1) Build
2) Clean & Build
3) Run tests
4) Run program
5) Generate installer (cpack) and increment version
6) Exit
7) Show current version
8) Check memory leaks (valgrind)
Choose an option:
EOF
}

do_build(){
  mkdir -p "$BUILD_DIR"
  pushd "$BUILD_DIR" >/dev/null
  cmake -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" ..
  cmake --build . -- -j$(nproc)
  popd >/dev/null
  echo "Build finished."

  exe_build_path="${BUILD_DIR}/${BIN_REL_PATH}"
  if [ -x "$exe_build_path" ]; then
    ver="$(read_version)"
    dest="${RELEASE_DIR}/v${ver}"
    mkdir -p "$dest"
    cp -f "$exe_build_path" "${dest}/TwentyAndFiveEyeRest"
    chmod +x "${dest}/TwentyAndFiveEyeRest"
    echo "Saved working executable to ${dest}/TwentyAndFiveEyeRest"
  else
    echo "Warning: expected executable not found at ${exe_build_path}"
  fi
}

do_clean_build(){ rm -rf "$BUILD_DIR"; do_build; }

run_tests(){
  pushd "$BUILD_DIR" >/dev/null
  if [ -f "CTestTestfile.cmake" ]; then
    ctest --output-on-failure
  else
    echo "No tests built. Building tests..."
    do_build
    ctest --output-on-failure
  fi
  popd >/dev/null
}

run_program(){
  exe="${BUILD_DIR}/${BIN_REL_PATH}"
  if [ ! -x "$exe" ]; then
    echo "Program not built. Building..."
    do_build
  fi
  "$exe"
}

generate_installer(){
  newver=$(increment_version_patch)
  echo "$newver" > "$VERSION_FILE"
  echo "Version incremented to $newver"
  do_build
  pushd "$BUILD_DIR" >/dev/null
  cpack -G TGZ || echo "CPack failed"
  popd >/dev/null
}

show_version(){ echo "Current version: $(cat $VERSION_FILE)"; }

check_memory_leaks(){
  exe="${BUILD_DIR}/${BIN_REL_PATH}"
  if [ ! -x "$exe" ]; then
    echo "Program not built. Building..."
    do_build
  fi
  if ! command -v valgrind >/dev/null 2>&1; then
    echo "Valgrind not installed. Install with: sudo apt install valgrind"
    return 1
  fi
  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes "$exe"
}

while true; do
  menu
  read -r choice
  case "$choice" in
    1) do_build ;;
    2) do_clean_build ;;
    3) run_tests ;;
    4) run_program ;;
    5) generate_installer ;;
    6) echo "Bye."; exit 0 ;;
    7) show_version ;;
    8) check_memory_leaks ;;
    *) echo "Unknown option: $choice" ;;
  esac
done
