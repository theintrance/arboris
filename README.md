<p align="center">
    <img src="assets/arboris.png" width="400"/>
    <br>
    <img src='assets/description.svg'>
</p>

---

<p align="center">
    <img src="https://img.shields.io/github/stars/theintrance/arboris?style=social" alt="GitHub stars"/>
    <img src="https://img.shields.io/github/last-commit/theintrance/arboris"/>
    <img src="https://img.shields.io/github/license/theintrance/arboris"/>
</p>

### Overview

* *TODO: Add description*

### Compatibility

* `arboris` targets well-formed HTML. Recovering from broken markup the way a browser does is
  out of scope, and the conformance suite skips those cases.

### Installation

* *TODO: Add installation*

### Usage

* *TODO: Add usage*

### Benchmark

* *TODO: Add benchmark*

### Testing

Build with tests enabled, then run them through CTest:

```bash
cmake -S . -B build
cmake --build build
cd build && ctest
```

#### Tokenizer conformance

The tokenizer is checked against [html5lib-tests](https://github.com/html5lib/html5lib-tests).
Enable it with `ARBORIS_HTML5LIB_TESTS`, which fetches the suite at a pinned commit and registers
a CTest entry:

```bash
cmake -S . -B build -DARBORIS_HTML5LIB_TESTS=ON
cmake --build build
uv run --group test pytest tests/html5lib                    # cases in scope
uv run --group test pytest tests/html5lib --include-broken   # everything, including out of scope
uv run --group test pytest tests/html5lib -k "test1.test:9]" # a single case
```

Every well-formed case that runs in the Data state is in scope. Cases html5lib-tests marks with
parse `errors` are broken HTML and are skipped, as are cases that need tokenizer state switching.
`tests/html5lib/tokenizer_excludes.txt` drops individual cases the project decided not to support.

The suite is red while the tokenizer is being built out: a failure there is remaining work, not a
regression. On a pull request against `develop`, CI runs the same cases against both branches and
reports which ones started and stopped passing, and fails only on the latter.

### Documentation

* *TODO: Add documentation*

### Contributing

* *TODO: Add contributing*

### License

* `arboris` is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for details.