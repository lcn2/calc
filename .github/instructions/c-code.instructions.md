---
applyTo: "**/*.c,**/*.h"
---
- Flag any added `goto` statement as a **blocking** issue.
- Suggest structured alternatives: loops, early `return`, a helper function, `break`/`continue`.
- Flag changes lacking test coverage or that could break `make check` / `make chk`.
- Require C code to conform to `.clang-format`; `make clang-format` must not need to modify any files.
