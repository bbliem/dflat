#!/usr/bin/env ruby
require 'optparse'

reify = false

optparse = OptionParser.new do |opts|
  opts.banner = "Usage: asp-get.rb [options] width height"

  opts.on('-h', '--help', 'Print usage information') do
    puts opts
    exit
  end

  opts.on('--reify', 'Reify generated program as set of facts') do
    reify = true
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

class Rule
  attr_reader :name, :head, :pos, :neg

  def initialize(name, head, pos, neg)
    @name = name
    @head = head
    @pos = pos
    @neg = neg
  end

  def to_s
    body = pos + neg.map { |a| 'not ' + a}
    str = ''
    if head.empty?
      str << ':-'
      str << ' ' unless body.empty?
    else
      str << head.join(' | ')
      str << ' :- ' unless body.empty?
    end
    str + body.join(', ') + '.'
  end

  def reify
    [:head, :pos, :neg].map do |p|
      method(p).call.map { |a| "#{p}(#{@name},#{a})." }
    end.flatten.join(' ')
  end
end

num_atoms = width * height / 2
num_rules = width * height - num_atoms

define_method :atom_at_grid_position do |x, y|
  (y * width + x) / 2
end

rules = {}

(0..num_rules-1).each do |r|
  # Determine r's position in the grid, starting at (0,0)
  row = 2*r / width
  column = 2*r % width
  column += 1 if row.odd? && width.even?

  occurring_atoms = []
  occurring_atoms << atom_at_grid_position(column, row-1) if row > 0
  occurring_atoms << atom_at_grid_position(column, row+1) if row+1 < height
  occurring_atoms << atom_at_grid_position(column-1, row) if column > 0
  occurring_atoms << atom_at_grid_position(column+1, row) if column+1 < width

  rule_parts = [[],[],[]] # head, positive body, negative body
  occurring_atoms.each { |a| rule_parts[Random.rand(3)] << "a#{a+1}" }

  rule_name = "r#{r+1}"
  rules[rule_name] = Rule.new(rule_name, *rule_parts)
end

if reify
  (0..num_atoms-1).each { |a| puts "atom(a#{a+1})." }
  rules.each { |name, rule| puts "rule(#{name}). #{rule.reify}" }
else
  puts rules.values
end
