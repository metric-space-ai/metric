#!/usr/bin/env ruby
# frozen_string_literal: true

HEADER = <<~TEXT
  // METRIC numeric assimilation header.
  // Provenance and licensing are documented in metric/numeric/README.md.

TEXT

ROOT = 'metric/numeric'

def normalized_guard(macro)
  macro
    .gsub(/\A_+/, '')
    .gsub(/_+\z/, '')
    .upcase
end

changed = []
skipped = []

Dir["#{ROOT}/**/*.h"].sort.each do |path|
  text = File.binread(path).encode('UTF-8', invalid: :replace, undef: :replace, replace: '')
  guard_match = text.match(/^#ifndef\s+([A-Za-z_][A-Za-z0-9_]*)\s*$/)

  unless guard_match
    skipped << [path, 'missing include guard']
    next
  end

  guard_start = guard_match.begin(0)
  old_guard = guard_match[1]
  new_guard = normalized_guard(old_guard)
  body = text[guard_start..]

  body = body.sub(/^#ifndef\s+#{Regexp.escape(old_guard)}\s*$/, "#ifndef #{new_guard}")
  body = body.sub(/^#define\s+#{Regexp.escape(old_guard)}\s*$/, "#define #{new_guard}")
  body = body.gsub(/(#endif\s*(?:\/\/|\/\*)\s*)#{Regexp.escape(old_guard)}(\s*(?:\*\/)?)$/, "\\1#{new_guard}\\2")

  updated = HEADER + body
  next if updated == text

  File.binwrite(path, updated)
  changed << path
end

puts "normalized #{changed.size} numeric headers"
unless skipped.empty?
  warn "skipped #{skipped.size} numeric headers:"
  skipped.each { |path, reason| warn "  #{path}: #{reason}" }
end
