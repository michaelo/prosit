cd build
watchexec -c --watch=.. --ignore=test/integration/*  "meson test --suite prosit:unit"
