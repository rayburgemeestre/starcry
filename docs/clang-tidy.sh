find src -type f | xargs -n 1 clang-tidy-15 --config-file .clang-tidy  -p . --fix
