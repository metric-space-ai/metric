#!/usr/bin/env ruby
# frozen_string_literal: true

require 'set'

promoted_globs = [
  '.github/workflows/**/*.yml',
  '.github/workflows/**/*.yaml',
  'CMakeLists.txt',
  'python/CMakeLists.txt',
  'cmake/**/*.cmake.in',
  'metric/**/*',
  'examples/**/*',
  'tests/**/*',
  'benchmarks/**/*',
  'python/src/**/*'
]

patterns = {
  'upstream numeric namespace' => /blaze::/,
  'upstream numeric include' => /#\s*include\s*[<"]blaze\//,
  'upstream numeric package lookup' => /find_package\s*\(\s*blaze\b/i,
  'upstream numeric fetch declaration' => /FetchContent_Declare\s*\(\s*blaze\b/im,
  'upstream numeric fetch properties' => /FetchContent_GetProperties\s*\(\s*blaze\b/i,
  'upstream numeric fetch populate' => /FetchContent_Populate\s*\(\s*blaze\b/i,
  'upstream numeric package dependency' => /find_dependency\s*\(\s*blaze\b/i,
  'upstream numeric target' => /metric::numeric::blaze/,
  'upstream numeric name' => /\bblaze\b/i
}.freeze

# Zero tolerance: Metric owns the numeric core. Old upstream lookup, include,
# namespace, or target markers must not reappear in project source paths.
allowed_marker_counts = {}.freeze

# The numeric-core provenance/license docs are the only place the historical source project may be
# named; everything else in the promoted source tree is held to zero tolerance.
excluded_paths = Set['metric/numeric/LICENSE.BSD-3-Clause', 'metric/numeric/README.md']
files = promoted_globs.flat_map { |pattern| Dir[pattern] }.to_set.to_a.sort.select do |path|
  File.file?(path) && !excluded_paths.include?(path)
end
failures = []

files.each do |path|
  matches = []
  text = File.binread(path).encode('UTF-8', invalid: :replace, undef: :replace, replace: '')
  patterns.each do |label, pattern|
    text.to_enum(:scan, pattern).each do
      match = Regexp.last_match
      line_number = text[0...match.begin(0)].count("\n") + 1
      snippet = match[0].lines.first.strip
      matches << "#{path}:#{line_number}: #{label}: #{snippet}"
    end
  end

  allowed_count = allowed_marker_counts[path]
  if matches.empty?
    failures << "#{path}: stale Metric numeric-core marker allowance, expected #{allowed_count} markers" if allowed_count
  elsif allowed_count.nil?
    failures.concat(matches)
  elsif matches.size != allowed_count
    failures << "#{path}: #{matches.size} Metric numeric-core upstream markers, expected exactly #{allowed_count}"
    failures.concat(matches)
  end
end

if failures.empty?
  puts 'metric numeric guard ok'
else
  warn failures.join("\n")
  exit 1
end
