#!/usr/bin/env ruby
# frozen_string_literal: true

require 'optparse'

roots = ['metric/numeric', 'python/pkg']
output = nil

OptionParser.new do |opts|
  opts.on('--output PATH', 'Write Markdown report to PATH') { |value| output = value }
end.parse!

patterns = {
  'visible_blaze_name' => /\bBlaze\b|\bblaze\b|\bBLAZE\b/,
  'upstream_author_notice' => /Klaus Iglberger|Metric numeric development group/,
  'upstream_license_block' => /New \(Revised\) BSD License|This Source Code Form is subject to the terms of the Mozilla Public/,
  'upstream_disclaimer_block' => /THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS/,
  'legacy_guard_style' => /#ifndef _METRIC_NUMERIC_|#define _METRIC_NUMERIC_/,
  'stale_public_aggregator' => %r{metric/numeric/math/(BLAS|LAPACK|Serialization)\.h$},
  'stale_parallel_backend' => %r{metric/numeric/math/smp/(openmp|threads|hpx|ParallelSection|ThreadMapping)},
  'stale_system_stub' => %r{metric/numeric/(config/MPI|system/(MPI|Deprecated|System|WarningDisable|Signature))\.h$},
  'stale_python_blaze_source' => %r{src/blaze\.cpp}
}.freeze

ignored_files = [
  'metric/numeric/README.md',
  'metric/numeric/LICENSE.BSD-3-Clause'
].freeze

files = roots.flat_map { |root| Dir["#{root}/**/*"] }.select { |path| File.file?(path) && !ignored_files.include?(path) }.sort

rows = []
totals = Hash.new(0)

files.each do |path|
  text = File.binread(path).encode('UTF-8', invalid: :replace, undef: :replace, replace: '')
  hits = {}

  patterns.each do |label, pattern|
    count = text.scan(pattern).size
    if %w[stale_public_aggregator stale_parallel_backend stale_system_stub].include?(label)
      count = path.match?(pattern) ? 1 : 0
    end
    next if count.zero?

    hits[label] = count
    totals[label] += count
  end

  next if hits.empty?

  rows << [path, hits]
end

markdown = +"# Native Numeric Assimilation Inventory\n\n"
markdown << "`metric/numeric` is METRIC's native numerical core, derived from the Blaze C++ math project; provenance and license are documented centrally; there is no external Blaze dependency/public boundary.\n\n"
markdown << "This generated report lists concrete visible seams that would make the numeric core look like an external or merely renamed library.\n\n"
markdown << "Assimilation does not mean deleting useful mathematical primitive implementations or calling this \"renamed Blaze.\" It means narrowing the surface to METRIC contracts and making the code read as one coherent codebase.\n\n"
markdown << "## Totals\n\n"
markdown << "| Marker | Count |\n"
markdown << "| --- | ---: |\n"
patterns.each_key do |label|
  markdown << "| `#{label}` | #{totals[label]} |\n"
end

markdown << "\n## Files With Markers\n\n"
markdown << "| File | Markers |\n"
markdown << "| --- | --- |\n"
rows.each do |path, hits|
  marker_text = hits.map { |label, count| "`#{label}`: #{count}" }.join('<br>')
  markdown << "| `#{path}` | #{marker_text} |\n"
end

if output
  File.write(output, markdown)
else
  puts markdown
end
