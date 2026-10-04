# Copilot code review standards for lcn2/calc

Apply these rules when reviewing pull requests.

1. **No `goto` in C code** (`.c`, `.h`): flag any added `goto` as a blocking
   issue. Suggest loops, early `return`, helper functions, `break`/`continue`.
2. **Required tests must pass**: flag changes that lack test coverage
   (e.g. `cal/regress.cal`) or would break the test suite. The test targets
   are `make check` (build and run `cal/regress.cal`) and `make chk`
   (run the regression test and verify output with `check.awk`).
3. **Typos** in documentation (README, `help/`, man pages, markdown, text
   files) and in C comments must be corrected.

See `.github/CODE_REVIEW_GUIDE.md`.
