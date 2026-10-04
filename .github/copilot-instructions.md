# Repository-wide Copilot instructions

When reviewing or writing code for calc:

- Never add `goto` statements to C code (`.c`, `.h`); use structured alternatives.
- Changes must keep `make check` / `make chk` passing; add tests to `cal/regress.cal` when behavior changes.
- Correct typos in documentation and code comments.
- C code must conform to `.clang-format`; `make clang-format` must not need to modify any files.
- Do not modify generated or vendored files.

Details: `.github/CODE_REVIEW_GUIDE.md` and `.github/instructions/`.
