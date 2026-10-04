---
applyTo: "**/*.c,**/*.h"
---
- Flag any added `goto` statement as a **blocking** issue.
- Suggest structured alternatives: loops, early `return`, a helper function, `break`/`continue`.
- Flag changes lacking test coverage or that could break `make check` / `make chk`.
