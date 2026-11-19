# .ReaperPkg packaging

A `.ReaperPkg` is a zip archive containing `reaper-package-info.txt` (generated
from `reaper-package-info.txt.in`) plus payload files under a supported REAPER
install directory. REAPER only extracts files whose OS/arch prefix matches the
host, so the same format serves every platform.

Build a package with:

```sh
mise run package          # macOS/Linux
mise run package-windows  # Windows
```

Both run the deployment build and then the `ReaperPkg` CMake target. Deployment
builds output to the repo-root `dist/` (not `ReacomaExtension/build/`), which
ends up holding the plugin binary and `dist/ReaCoMa-<platform>.ReaperPkg`:

```
reaper-package-info.txt
os_mac_64/UserPlugins/reaper_ReacomaExtension.dylib   # macOS universal build
os_win_x64/UserPlugins/reaper_ReacomaExtension.dll    # Windows x64 build
```

The prefix is chosen automatically: a universal macOS build maps to
`os_mac_64`, a single-architecture build to `os_mac_arm64` or
`os_mac_x86_64`; Windows uses `os_win_x64`/`os_win_32`; Linux maps the processor
to `os_linux_x86_64`, `os_linux_aarch64`, `os_linux_armv7l`, or
`os_linux_i686`. To ship additional content (for example `Scripts/`), add it to
`REAPERPKG_STAGE_DIR` in `ReacomaExtension/CMakeLists.txt` before the archive
command.

`mise run package-windows` additionally writes the raw plugin dll zip
`dist/ReaCoMa-Windows.zip`, which CI keeps publishing
for users installing manually.

CI builds the package after macOS signing/notarization so the packaged dylib is
signed, and publishes `ReaCoMa-MacOS.dmg`, `ReaCoMa-MacOS.ReaperPkg`, and on
Windows both `ReaCoMa-Windows.ReaperPkg` and `ReaCoMa-Windows.zip`.
