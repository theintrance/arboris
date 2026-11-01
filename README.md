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

`arboris` is an HTML parser written in C++20 with a Python package on top. It reads a page
once into a compact DOM and answers queries against it:

* **CSS selectors** (Selectors Level 3: combinators, attribute operators, structural
  pseudo-classes) and condition queries by tag, class, attribute and text
* **element data** as views into the document, no copies: tag, id, classes, attributes, text with
  character references decoded, and the element's own HTML as written
* **navigation** by parent, children, siblings and ancestors
* **input encodings**: UTF-8 first; UTF-16 by byte order mark, and legacy encodings named by the
  caller or a `<meta charset>` through iconv when it is available

The tokenizer passes every in-scope case of the html5lib conformance suite (table below) and
runs at several hundred MiB/s; see [Benchmark](#benchmark).

### Compatibility

* `arboris` targets well-formed HTML. Recovering from broken markup the way a browser does is
  out of scope, and the conformance suite skips those cases.

### Installation

**Python** (the distribution is `pyarboris`; the import name is `arboris`):

```bash
pip install pyarboris
```

Wheels are published for CPython 3.10 to 3.13 on Linux x86_64, macOS (arm64 and x86_64) and
Windows. Anywhere else `pip` builds from the sdist, which needs CMake and a C++20 compiler.

**C++** (CMake 3.14+, a C++20 compiler):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DNO_TESTS=ON
cmake --build build
cmake --install build --prefix /where/you/want/it
```

Then, in your project:

```cmake
find_package(arboris REQUIRED)
target_link_libraries(your_target PRIVATE arboris::arboris)
```

or pull the sources in with `FetchContent` and link `arboris` the same way. The one header to
include is `<arboris/arboris.hpp>`. `ARBORIS_ICONV` (default `ON`) links iconv for legacy
encodings when CMake finds it.

### Usage

**Python**

```python
import arboris

doc = arboris.parse(open("page.html", "rb").read())   # bytes: encoding sniffed; str: UTF-8

for link in doc.select("nav a[href^='/docs/']"):       # CSS selectors, in document order
    print(link.text, link.attrs["href"])

title = doc.select_one("title")                        # the first match, or None
cards = doc.find_all(tag="div", classes="card")        # conditions: tag, classes, attrs, text
first = doc.find(attrs={"data-id": "42"})

node = doc.get_by_id("main")
node.tag, node.id, node.classes, node.attrs            # "div", "main", [...], {...}
node.text                                              # text with references decoded
node.outer_html, node.inner_html                       # the source as written
node.parent, node.children, node.next_sibling, node.ancestors
node.select("p")                                       # queries from a node search below it

doc.encoding, doc.parse_error_offset                   # "euc-kr"; None when all was read
```

A `Node` keeps its document alive, so results can outlive the `Document` they came from. A
selector that does not parse raises `ValueError`.

**C++**

```cpp
#include <arboris/arboris.hpp>

arboris::DOMManager dom(html);                          // std::string_view, copied once

for (const arboris::DOMQuery& hit : dom.SelectAll("nav a[href^='/docs/']")) {
  const arboris::NodeRef link = hit.Get();
  std::cout << link.text_content() << ' ' << link.attributes()[0].value << '\n';
}

if (const auto main = dom.Find(std::string("main"))) {    // by id
  for (const arboris::NodeRef child : main->Get().children()) { /* ... */ }
  for (const arboris::DOMQuery& p : main->FindAll({.tag = arboris::Tag::kP})) { /* ... */ }
  main->Get().outer_html();                               // the source as written
}
```

Queries hand out `NodeRef`s: 16-byte handles into the `DOMManager`, valid for as long as it
lives. `DOMManager` is neither copyable nor movable; keep it in place or hold it through
`std::unique_ptr`. `include/arboris/arboris.hpp` lists the public types; everything under
`dom/`, `string/`, `utils/` and `encoding/` is implementation detail.

### Benchmark

Tokenizer throughput on the CI runner (GitHub `ubuntu-latest`, 4 cores, Release build), from the
`benchmark` workflow that runs on every pull request and compares against `develop`:

| Input | Size | Throughput |
|---|---:|---:|
| Wikipedia "United States" | 2.8 MiB | 500 MiB/s |
| Wikipedia "HTML" | 0.75 MiB | 435 MiB/s |
| Wikipedia "Web browser" | 0.39 MiB | 440 MiB/s |
| generated, 1M tokens deeply nested | 24 MiB | 287 MiB/s |

On an Apple M-series laptop the same "United States" page tokenizes at about 670 MiB/s, and
building the DOM on top of the tokens brings it to about 430 MiB/s.

To run it yourself (downloads the corpus on first configure; `-DARBORIS_BENCH_LEXBOR=ON` adds lexbor
for comparison):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DNO_TESTS=ON -DARBORIS_BENCHMARK=ON
cmake --build build --target arboris_bench
build/benchmark/arboris_bench
```

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
uv run --no-project --with pytest pytest tests/html5lib                    # cases in scope
uv run --no-project --with pytest pytest tests/html5lib --include-broken   # everything, including out of scope
uv run --no-project --with pytest pytest tests/html5lib -k "test1.test:9]" # a single case
```

Every well-formed case that runs in the Data state is in scope. Cases html5lib-tests marks with
parse `errors` are broken HTML and are skipped, as are cases that need tokenizer state switching.
`tests/html5lib/tokenizer_excludes.txt` drops individual cases the project decided not to support.

The suite is red while the tokenizer is being built out: a failure there is remaining work, not a
regression. On a pull request against `develop`, CI runs the same cases against both branches and
reports which ones started and stopped passing, and fails only on the latter.

Each case is filed under the tokenizer feature it needs, worked out from its input and expected
tokens in `tests/html5lib/categories.py`. Regenerate the table below after changing the tokenizer;
CI fails on a pull request that leaves it stale:

```bash
uv run --no-project --with pytest pytest tests/html5lib -q --junitxml=html5lib.xml
uv run --no-project --with pytest python scripts/html5lib_status.py --junit html5lib.xml --write
```

<!-- html5lib:start -->

**100% conformant** — the unweighted mean of the six area rates below.

✅ complete · ⚠️ in progress · ❌ nothing passes yet

| Area | Passing | Rate | |
|---|---:|---:|:-:|
| **Text** | **2272 / 2272** | **100%** | ✅ |
| ↳ reference left as-is | 1981 / 1981 | 100% | ✅ |
| ↳ plain | 148 / 148 | 100% | ✅ |
| ↳ unicode | 137 / 137 | 100% | ✅ |
| ↳ newline normalisation | 6 / 6 | 100% | ✅ |
| **Tags** | **68 / 68** | **100%** | ✅ |
| ↳ start tag | 51 / 51 | 100% | ✅ |
| ↳ end tag | 9 / 9 | 100% | ✅ |
| ↳ self-closing | 7 / 7 | 100% | ✅ |
| ↳ start + end | 1 / 1 | 100% | ✅ |
| **Attributes** | **230 / 230** | **100%** | ✅ |
| ↳ valueless | 80 / 80 | 100% | ✅ |
| ↳ unquoted value | 49 / 49 | 100% | ✅ |
| ↳ single-quoted value | 44 / 44 | 100% | ✅ |
| ↳ double-quoted value | 32 / 32 | 100% | ✅ |
| ↳ reference in value | 25 / 25 | 100% | ✅ |
| **Character references** | **2367 / 2367** | **100%** | ✅ |
| ↳ named table | 2125 / 2125 | 100% | ✅ |
| ↳ numeric hex | 236 / 236 | 100% | ✅ |
| ↳ named in context | 4 / 4 | 100% | ✅ |
| ↳ numeric decimal | 2 / 2 | 100% | ✅ |
| **Comments** | **15 / 15** | **100%** | ✅ |
| **DOCTYPE** | **14 / 14** | **100%** | ✅ |
| ↳ public/system id | 8 / 8 | 100% | ✅ |
| ↳ name only | 6 / 6 | 100% | ✅ |

<sub>4966 of 4966 in-scope cases pass. Areas differ by orders of magnitude in case count, so that ratio is not the figure above. Out of scope: 1724 broken-HTML cases and 116 that need tokenizer state switching. Generated by <code>scripts/html5lib_status.py</code>.</sub>

<!-- html5lib:end -->

### Documentation

* The public C++ API is the comment at the top of
  [`include/arboris/arboris.hpp`](include/arboris/arboris.hpp) and the comments on the types it
  names; `src/dom/node_ref.hpp` is the one to read for what a node gives you.
* In Python, `help(arboris)` and `help(arboris.Node)` carry the same information as docstrings.
* Design decisions are recorded in the pull requests; the
  [issues](https://github.com/theintrance/arboris/issues) are grouped by milestone.

### Contributing

Pull requests go to `develop`. Set up once:

```bash
uv sync --group test              # Python environment, builds the extension
git config core.hooksPath hooks   # cpplint, clang-tidy and ruff before every commit
cmake -S . -B build               # the hook's clang-tidy reads build/compile_commands.json
```

CI builds and tests on Linux and macOS, under ASan and UBSan, runs clang-tidy, cpplint and
ruff, checks that an installed copy can be found with `find_package`, runs the Python tests,
and compares html5lib conformance and tokenizer throughput against `develop`. Keep one topic
per commit; the pull request description carries the reasoning.

### License

* `arboris` is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for details.