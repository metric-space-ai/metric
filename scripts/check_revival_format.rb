#!/usr/bin/env ruby
# frozen_string_literal: true

tracked_files = `git ls-files`.lines.map(&:chomp)

text_extensions = %w[
  .cpp .hpp .in .json .md .py .rb .toml .txt .yml .yaml
]

format_scope = tracked_files.select do |path|
  path == 'CMakeLists.txt' ||
    path == 'CMakePresets.json' ||
    path.start_with?('.github/workflows/') ||
    path.start_with?('docs/') ||
    path.start_with?('examples/core/') ||
    path.start_with?('python/examples/metric_space/') ||
    path.start_with?('python/pkg/metric/') ||
    path.start_with?('python/tests/core/') ||
    path.start_with?('scripts/') ||
    path.start_with?('tests/core_smoke/') ||
    path.start_with?('tests/downstream_consumer/')
end

text_files = format_scope.select do |path|
  text_extensions.include?(File.extname(path)) || File.basename(path) == 'CMakeLists.txt'
end

space_indented_extensions = %w[.md .py .rb .toml .yml .yaml]
failures = []

tracked_files.each do |path|
  if path.include?('/__pycache__/') || File.extname(path) == '.pyc'
    failures << "#{path}: tracked Python cache artifact"
  end
end

text_files.each do |path|
  contents = File.binread(path)
  failures << "#{path}: CRLF line endings" if contents.include?("\r\n")

  next unless space_indented_extensions.include?(File.extname(path))

  contents.each_line.with_index(1) do |line, line_number|
    failures << "#{path}:#{line_number}: leading tab indentation" if line.start_with?("\t")
  end
end

if failures.empty?
  puts 'revival format ok'
else
  warn failures.join("\n")
  exit 1
end
