# Release Build

Release artifacts are stored under:

- `releases/<version>/`

Each release folder contains:

- `iimperialism-<version>.dsk`
- `iimperialism-manual-<version>.pdf`

## Version Source

The default release version file is:

- `RELEASE_VERSION.txt`

If the release script is run without `-Version`, it reads the version from that
file. If the file is missing or empty, the script prompts for a version number.

## Build A Release

From the repository root on Windows:

```powershell
powershell -ExecutionPolicy Bypass -File tools/build_release.ps1
```

This script:

1. resolves the release version
2. updates `docs/manual/iimperialism-manual.md`
3. builds the disk image
4. builds the manual PDF
5. copies both artifacts into `releases/<version>/`

## Useful Options

Use an explicit version:

```powershell
powershell -ExecutionPolicy Bypass -File tools/build_release.ps1 -Version 0.2.0
```

Force a prompt even if `RELEASE_VERSION.txt` exists:

```powershell
powershell -ExecutionPolicy Bypass -File tools/build_release.ps1 -PromptForVersion
```

Overwrite an existing release directory's artifacts:

```powershell
powershell -ExecutionPolicy Bypass -File tools/build_release.ps1 -Force
```
