/*
 *   Copyright 2025 Team Arboris
 *   Licensed under the Apache License, Version 2.0
 *   http://www.apache.org/licenses/LICENSE-2.0
 */

// Tokenizer benchmarks over two inputs, registered at run time:
//
//   <tokenizer>/corpus/<file>              one document from the corpus (benchmark/corpus.cmake)
//   <tokenizer>/html5lib/<area>[/<feature>] every html5lib-tests case in that area, one pass
//   <tokenizer>/html5lib-case/<case id>    one case at a time, only with --per_case
//
// Only the html5lib cases arboris tokenizes correctly are used, for every tokenizer. A case
// arboris gets wrong is one it does less work on, and timing it would flatter arboris.
//
// Flags besides Google Benchmark's own:
//   --data_dir=<dir>  html5lib_cases (benchmark/export_cases.py) and corpus/ (default: build dir)
//   --per_case        also register one benchmark per html5lib case

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <filesystem>  // NOLINT(build/c++17): the project is C++20
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "benchmark/tokenizers.hpp"

#ifndef ARBORIS_BENCH_DATA_DIR
#define ARBORIS_BENCH_DATA_DIR ""
#endif

namespace arboris::bench {
namespace {

using TokenizeFn = TokenizeResult (*)(std::string_view);

struct Tokenizer {
  std::string name;
  TokenizeFn tokenize;
};

struct Case {
  std::string id;
  std::string area;
  std::string feature;
  std::string input;
};

struct Document {
  std::string name;
  std::string content;
};

std::vector<Tokenizer> Tokenizers() {
  return {
      {"arboris", TokenizeArboris},
#ifdef ARBORIS_BENCH_LEXBOR
      {"lexbor", TokenizeLexbor},
#endif
  };
}

std::string ReadFile(const std::filesystem::path& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    std::cerr << "cannot read " << path << '\n';
    std::exit(EXIT_FAILURE);
  }
  return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

// Keeps the cases arboris passes; see the format in benchmark/export_cases.py.
std::vector<Case> LoadPassingCases(const std::filesystem::path& path) {
  const std::string data = ReadFile(path);
  std::vector<Case> cases;
  std::size_t pos = 0;
  while (pos < data.size()) {
    const std::size_t eol = data.find('\n', pos);
    std::istringstream header(data.substr(pos, eol - pos));
    Case c;
    int passes = 0;
    std::size_t length = 0;
    std::getline(header, c.id, '\t');
    std::getline(header, c.area, '\t');
    std::getline(header, c.feature, '\t');
    header >> passes >> length;
    c.input = data.substr(eol + 1, length);
    pos = eol + 1 + length + 1;
    if (passes != 0) {
      cases.push_back(std::move(c));
    }
  }
  return cases;
}

std::vector<Document> LoadCorpus(const std::filesystem::path& dir) {
  std::vector<Document> corpus;
  for (const auto& entry : std::filesystem::directory_iterator(dir)) {
    if (entry.path().extension() == ".html") {
      corpus.push_back({entry.path().stem().string(), ReadFile(entry.path())});
    }
  }
  std::sort(corpus.begin(), corpus.end(), [](const auto& a, const auto& b) { return a.name < b.name; });
  return corpus;
}

void RegisterDocument(const Tokenizer& tokenizer, const Document& document) {
  benchmark::RegisterBenchmark(
      tokenizer.name + "/corpus/" + document.name, [fn = tokenizer.tokenize, &document](benchmark::State& state) {
        TokenizeResult result;
        for (auto _ : state) {
          result = fn(document.content);
          benchmark::DoNotOptimize(result);
        }
        if (!result.ok) {
          state.SkipWithError("tokenizer reported a failure");
        }
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations() * document.content.size()));
        state.counters["tokens"] = static_cast<double>(result.tokens);
      });
}

void RegisterCases(const Tokenizer& tokenizer, const std::string& name, std::vector<std::string_view> inputs) {
  std::size_t bytes = 0;
  for (std::string_view input : inputs) {
    bytes += input.size();
  }
  benchmark::RegisterBenchmark(
      tokenizer.name + "/" + name,
      [fn = tokenizer.tokenize, inputs = std::move(inputs), bytes](benchmark::State& state) {
        for (auto _ : state) {
          for (std::string_view input : inputs) {
            benchmark::DoNotOptimize(fn(input));
          }
        }
        state.SetBytesProcessed(static_cast<int64_t>(state.iterations() * bytes));
        state.counters["cases"] = static_cast<double>(inputs.size());
      });
}

void RegisterAll(const std::vector<Case>& cases, const std::vector<Document>& corpus, bool per_case) {
  std::map<std::string, std::vector<std::string_view>> groups;
  for (const Case& c : cases) {
    groups["html5lib/" + c.area].push_back(c.input);
    groups["html5lib/" + c.area + "/" + c.feature].push_back(c.input);
  }

  for (const Tokenizer& tokenizer : Tokenizers()) {
    for (const Document& document : corpus) {
      RegisterDocument(tokenizer, document);
    }
    for (const auto& [name, inputs] : groups) {
      RegisterCases(tokenizer, name, inputs);
    }
    if (per_case) {
      for (const Case& c : cases) {
        RegisterCases(tokenizer, "html5lib-case/" + c.id, {c.input});
      }
    }
  }
}

}  // anonymous namespace
}  // namespace arboris::bench

int main(int argc, char** argv) {
  benchmark::Initialize(&argc, argv);

  std::filesystem::path data_dir = ARBORIS_BENCH_DATA_DIR;
  bool per_case = false;
  for (int i = 1; i < argc; ++i) {
    std::string_view arg = argv[i];
    if (arg.starts_with("--data_dir=")) {
      data_dir = arg.substr(std::string_view("--data_dir=").size());
    } else if (arg == "--per_case") {
      per_case = true;
    } else {
      std::cerr << "unknown flag " << arg << '\n';
      return EXIT_FAILURE;
    }
  }

  const auto cases = arboris::bench::LoadPassingCases(data_dir / "html5lib_cases");
  const auto corpus = arboris::bench::LoadCorpus(data_dir / "corpus");
  arboris::bench::RegisterAll(cases, corpus, per_case);

  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return EXIT_SUCCESS;
}
