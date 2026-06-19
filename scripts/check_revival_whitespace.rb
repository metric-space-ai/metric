#!/usr/bin/env ruby
# frozen_string_literal: true

globs = [
  'README.md',
  'BUILD.md',
  'CHANGELOG.md',
  'CMakeLists.txt',
  'CMakePresets.json',
  'cmake/**/*.cmake.in',
  '.github/workflows/core-cpp.yml',
  '.github/workflows/docs-and-format.yml',
  '.github/workflows/pages.yml',
  '.github/workflows/python-core.yml',
  '.github/workflows/release-artifacts.yml',
  'docs/**/*.md',
  'examples/README.md',
  'examples/core/**/*',
  'metric/{concepts,metric,operators,space}.hpp',
  'python/CMakeLists.txt',
  'python/MANIFEST.in',
  'python/README.md',
  'python/cmake_ext.py',
  'python/examples/metric_space/**/*',
  'python/pkg/metric/{__init__,mappings,metrics,operators,spaces,transforms}.py',
  'python/pkg/metric/distance/__init__.py',
  'python/pkg/metric/space/{__init__,matrix}.py',
  'python/pyproject.toml',
  'python/setup.py',
  'python/src/CMakeLists.txt',
  'python/src/distance/**/*',
  'scripts/check_markdown_links.rb',
  'scripts/check_revival_format.rb',
  'scripts/check_revival_whitespace.rb',
  'python/tests/core/**/*',
  'tests/README.md',
  'tests/core_smoke/**/*',
  'tests/downstream_consumer/**/*'
]

files = globs.flat_map { |pattern| Dir[pattern] }.uniq.select do |path|
  File.file?(path) && !path.include?('/__pycache__/') && File.extname(path) != '.pyc'
end
failures = []

files.each do |file|
  contents = File.binread(file)
  failures << "#{file}: missing final newline" unless contents.empty? || contents.end_with?("\n")

  contents.each_line.with_index(1) do |line, line_number|
    failures << "#{file}:#{line_number}: trailing whitespace" if line.match?(/[ \t]\r?\n?\z/)
  end
end

if failures.empty?
  puts 'revival whitespace ok'
else
  warn failures.join("\n")
  exit 1
end
