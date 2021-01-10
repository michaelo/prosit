cd build
watchexec -c --watch=.. --ignore=test/unit/*  "meson test --suite prosit:integration"
