#!/usr/bin/env ruby
require 'optparse'

optparse = OptionParser.new do |opts|
  opts.banner = "Usage: asp-get.rb [options] width height"

  opts.on('-h', '--help', 'Print usage information') do
    puts opts
    exit
  end

  opts.on('--seed N', 'Use N as random seed') do |n|
    srand(Integer(n))
  end
end

optparse.parse!

if ARGV.length != 2
  puts optparse
  exit
end

width = Integer(ARGV[0])
height = Integer(ARGV[1])

height.times do |y|
  width.times do |x|
    v = y * width + x
    puts "vertex(#{v})."
    puts "edge(#{v},#{v+1})." if x+1 < width
    puts "edge(#{v},#{v+width})." if y+1 < height
  end
end
