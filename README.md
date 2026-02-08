# Project Vega v0.02

Short description: a work-in-progress Unreal Engine project (Project Vega).

## Getting started

Prerequisites
- Unreal Engine (use the version you developed with)
- Git
- Git LFS (recommended for `.uasset` and `.umap` files)

Clone and fetch LFS files:

```bash
git clone https://github.com/Archangel2303/ProjectVegav0.02.git
cd "MyProject"
git lfs install
git lfs pull
```

Or if you've already cloned: `git lfs install` and `git lfs pull` in the repo root.

## Repo layout (important folders)
- `Content/` - project assets and Blueprints
- `Config/` - project INI files
- `Saved/`, `Intermediate/`, `DerivedDataCache/` - generated/ignored folders

## Git LFS
Large binary Unreal assets (`*.uasset`, `*.umap`, etc.) are tracked with Git LFS. Do not commit large binary files without LFS enabled.

## Contributing
- Create a feature branch: `git checkout -b feat/your-change`
- Commit logically and open a Pull Request on GitHub

## License
Add a license file (e.g., `LICENSE`) or update this section with the chosen license.
