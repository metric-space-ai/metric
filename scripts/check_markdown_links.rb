#!/usr/bin/env ruby
# frozen_string_literal: true

require 'pathname'

files = Dir[
  'README.md',
  'BUILD.md',
  'CHANGELOG.md',
  'docs/**/*.md',
  'examples/README.md',
  'python/README.md',
  'tests/README.md'
]
failures = []

files.each do |file|
  text = File.read(file)
  text.scan(/\[[^\]]+\]\(([^)]+)\)/).flatten.each do |target|
    next if target.start_with?('http://', 'https://', 'mailto:')

    path = target.split('#', 2).first
    next if path.empty?

    full = (Pathname.new(file).dirname + path).cleanpath
    failures << "#{file}: #{target}" unless full.exist?
  end
end

if failures.empty?
  puts 'markdown local links ok'
else
  warn failures.join("\n")
  exit 1
end
