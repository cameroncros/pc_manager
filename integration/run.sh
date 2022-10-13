#!/bin/bash
set -e

cd "$(dirname "$0")"

pushd ../
cargo build --release
popd

cp ../target/release/pc_manager ./
docker build . -t pc_manager_machine
python -m venv venv

venv/bin/python -m pip install -r requirements.txt
venv/bin/python -m pytest
