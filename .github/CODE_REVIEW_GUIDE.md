# Code Review Guide

This guide applies to reviewers and contributors of lcn2/calc.

## 1. No `goto` in C code

**Rule:** pull requests must not add `goto` statements to `.c` or `.h` files.

**Rationale:** `goto` obscures control flow and makes review and maintenance harder.

**Alternatives:** loops, early `return`, a helper function, `break` / `continue`.

**Enforcement:** the `no-goto` workflow (`.github/workflows/no-goto.yml`) scans
only the lines added by the PR (ignoring comments and strings where practical)
and prints the file and line of each offender. Copilot review also flags it as blocking.

## 2. Required tests must pass

Run locally:

```sh
make clobber all
make check    # build and run cal/regress.cal
make chk      # run the regression test and verify with check.awk
```

New or changed behavior should come with test coverage (e.g. in `cal/regress.cal`).
**Enforcement:** the `tests` workflow (`.github/workflows/tests.yml`) installs
build dependencies, builds, and runs `make check` and `make chk`; any failure fails the PR.

## 3. Typos

Misspellings in documentation (README, `help/`, man pages, markdown, text files)
and in C comments must be corrected. Run locally:

```sh
cargo install typos-cli    # or: brew install typos-cli
typos --config _typos.toml <changed files>
```

Project-specific terms and excluded generated/vendored files are configured in `_typos.toml`.
**Enforcement:** the `typos` workflow checks the files changed in the PR.

## 4. C formatting

C code must conform to the options in `.clang-format`. Run `make clang-format`;
the `clang-format(1)` tool must not need to modify any files.
**Enforcement:** the `clang-format` workflow runs this target and fails if it
changes any files.

## Reviewer checklist

- [ ] No added `goto` in `.c` / `.h` files
- [ ] `make check` and `make chk` pass; tests cover the change
- [ ] No typos in docs or code comments
- [ ] `make clang-format` does not modify any files
- [ ] Generated/vendored files not edited by hand

## How the pieces fit together

- `.github/copilot-instructions.md` and `.github/instructions/*.instructions.md`
  (path-specific via `applyTo`) and `.github/copilot/code-review-instructions.md`
  teach Copilot these standards.
- Workflows in `.github/workflows/` (`no-goto`, `tests`, `typos`, and
  `clang-format`) enforce them on pull requests to `master`. Mark the checks
  `no-goto`, `tests`, `typos`, and `clang-format` (workflows "No goto", "Tests",
  "Typos", and "Clang format") as required status checks in branch protection.
