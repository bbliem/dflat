#!/usr/bin/env ruby
require 'optparse'

sphere = false

optparse = OptionParser.new do |opts|
  opts.banner = "Usage: asp-get.rb [options] width height"

  opts.on('-h', '--help', 'Print usage information') do
    puts opts
    exit
  end

  opts.on('--sphere', 'Draw grid on a sphere (every vertex has degree 4)') do
    sphere = true
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
    edge_right = width > 1 && (sphere || x+1 < width)
    edge_down = height > 1 && (sphere || y+1 < height)

    here = "x#{x}y#{y}"
    puts "vertex(#{here})."
    puts "edge(#{here},x#{(x+1) % width}y#{y})." if edge_right
    puts "edge(#{here},x#{x}y#{(y+1) % height})." if edge_down
  end
end
