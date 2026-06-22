#!/usr/bin/env ruby
# frozen_string_literal: true

source_files = `git ls-files --cached --others --exclude-standard`.lines.map(&:chomp).select do |path|
  File.file?(path)
end

failures = []

vendor_path_patterns = [
  %r{(^|/)(blaze|blazemark|blazetest)(/|$)}i,
  %r{(^|/)_deps/(blaze|blazemark|blazetest)(-|/|$)}i,
  %r{(^|/)(third_party|vendor|external)/(blaze|blazemark|blazetest)(/|$)}i
]

source_files.each do |path|
  vendor_path_patterns.each do |pattern|
    failures << "#{path}: numeric core must not reintroduce an upstream/vendor source path" if path.match?(pattern)
  end
end

facade_include_files = source_files.reject do |path|
  path == 'metric/numeric.hpp' || path.start_with?('metric/numeric/')
end

facade_include_files.each do |path|
  contents = File.binread(path).encode('UTF-8', invalid: :replace, undef: :replace, replace: '')
  contents.each_line.with_index(1) do |line, line_number|
    next unless line.match?(/^\s*#\s*include\s*[<"]metric\/numeric\/Numeric\.h[>"]/)

    failures << "#{path}:#{line_number}: include metric/numeric.hpp instead of the numeric core root header"
  end
end

numeric_files = source_files.select { |path| path.start_with?('metric/numeric/') }
license_path = 'metric/numeric/LICENSE.BSD-3-Clause'
# Provenance/documentation files that are intentionally not owned headers.
provenance_files = [license_path, 'metric/numeric/README.md']

# Curated METRIC-owned Level-2 facade headers. These deliberately use the .hpp
# extension to distinguish the small promoted-contract entry points from the
# inherited .h implementation headers. Any other non-.h file is still rejected.
facade_files = %w[
  metric/numeric/scalar.hpp
  metric/numeric/vector.hpp
  metric/numeric/matrix.hpp
  metric/numeric/sparse.hpp
  metric/numeric/linear_algebra.hpp
  metric/numeric/graph.hpp
  metric/numeric/random.hpp
  metric/numeric/parallel.hpp
  metric/numeric/io.hpp
].freeze

unless numeric_files.include?(license_path)
  failures << "#{license_path}: missing Metric numeric-core license/provenance file"
end

numeric_files.each do |path|
  next if provenance_files.include?(path)

  unless File.extname(path) == '.h' || facade_files.include?(path)
    failures << "#{path}: Metric numeric core should expose source as owned headers only"
  end

  if File.basename(path).match?(/\.(new|orig|rej|bak|tmp|old)\z/i)
    failures << "#{path}: temporary rewrite artifact inside Metric numeric core"
  end

  contents = File.binread(path).encode('UTF-8', invalid: :replace, undef: :replace, replace: '')

  contents.each_line.with_index(1) do |line, line_number|
    if (match = line.match(/^\s*#\s*include\s*([<"])([^>"]+)[>"]/))
      delimiter = match[1]
      include_path = match[2]
      if include_path.start_with?('blaze/')
        failures << "#{path}:#{line_number}: upstream numeric include path"
      elsif include_path.start_with?('../') || include_path.start_with?('./')
        failures << "#{path}:#{line_number}: relative include inside Metric numeric core"
      elsif include_path.start_with?('metric/numeric/') && delimiter != '<'
        failures << "#{path}:#{line_number}: Metric numeric include should use angle brackets"
      end
    end

    if line.match?(/\bBLAZE(_|[A-Z])|\bblaze::|#\s*(define|ifdef|ifndef)\s+_?BLAZE/i)
      failures << "#{path}:#{line_number}: upstream numeric marker inside Metric numeric core"
    end
  end
end

if failures.empty?
  puts 'metric numeric seam guard ok'
else
  warn failures.join("\n")
  exit 1
end
