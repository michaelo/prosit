cd build
watchexec -c --watch=..  "meson test --suite prosit:unit --suite prosit:integration"
