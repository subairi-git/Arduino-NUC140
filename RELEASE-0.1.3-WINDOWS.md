# Update Arduino NUC140 to v0.1.3 (Windows CMD)

## 1. Apply patch files

Extract `Arduino-NUC140-Fix-v0.1.3.zip` **into the existing local Git repository folder**, preserving folder structure and replacing corresponding files. The patch deliberately excludes generated `dist/` and `package_nuc140_index.json`: GitHub Actions generates the correct checksum and index after publishing the Release.

## 2. Check, commit, and synchronize

```cmd
cd /d C:\Users\USER\Downloads\Arduino-NUC140
git status
git add .
git commit -m "NUC140 v0.1.3: Nu-Link hotfix and memory reporting"
git fetch origin
git rebase origin/main
git push origin main
```

If `git commit` prints `nothing to commit`, only continue if you have verified the v0.1.3 files were actually copied. If `git rebase` reports `CONFLICT`, stop and resolve it before pushing. **Never use `git push --force` on main**. GitHub Actions may automatically update the package index, creating remote commits since your last pull; this fetch+rebase step handles that situation.

## 3. Tag the exact commit and publish

Check the tag first:

```cmd
git tag -l v0.1.3
```

If no tag exists, and `git push origin main` succeeded:

```cmd
git tag v0.1.3
git push origin v0.1.3
```

Check `https://github.com/subairi-git/Arduino-NUC140/actions` for a green release workflow. When GitHub Actions updates `package_nuc140_index.json`, your local branch may become behind again; run `git pull --rebase origin main` **after** ensuring your working tree is clean.

## 4. Update Arduino Board Manager

Keep the same Additional Boards Manager URL:

`https://raw.githubusercontent.com/subairi-git/Arduino-NUC140/main/package_nuc140_index.json`

Arduino IDE → Boards Manager → **NUC140 → 0.1.3 → Install / Update**. Compile Blink; size output should now display program and static RAM use. Only if Blink compiles successfully and you are ready to replace the APROM application, use **Upload** via Nu-Link (SWD, not COM). The uploader rechecks `NUC140VE3CN` before flash operations.

**Do not rename files, paste the entire ZIP as one file, or upload a prebuilt index from an earlier release.**