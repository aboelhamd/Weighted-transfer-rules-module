if (ARGV.length < 2)
  puts "\nUsage : ruby2.3 spcCharsRem.rb oldFilePath newFilePath"
  exit
end

file = File.open(ARGV[1], "w")

File.open(ARGV[0]).each do |line1|
	#line1.delete! ('\\\(\)\[\]\{\}\<\>\|\$\/\'\"')
  if ((line1 =~ /\s*\n/) == 0)  
    next
  end

  file.puts line1
end

file.close

