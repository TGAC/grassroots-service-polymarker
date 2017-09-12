
require_relative "SNP"
require 'bio-samtools'
module Bio::PolyploidTools
  class SNPSequenceException < RuntimeError 
  end

  class SNPSequence < SNP

    attr_accessor :sequence_original
    #Format: 
    #snp name,chromsome from contig,microarray sequence
    #BS00068396_51,2AS,CGAAGCGATCCTACTACATTGCGTTCCTTTCCCACTCCCAGGTCCCCCTA[T/C]ATGCAGGATCTTGATTAGTCGTGTGAACAACTGAAATTTGAGCGCCACAA
    def self.parse(reg_str)
      reg_str.chomp!
      snp = SNPSequence.new

      arr = reg_str.split(",")
      
      if arr.size == 3
        snp.gene, snp.chromosome, snp.sequence_original = reg_str.split(",")
      elsif arr.size == 2
       snp.gene, snp.sequence_original = arr
     else
       throw SNPSequenceException.new "Need two or three fields to parse, and got #{arr.size} in #{reg_str}"
      end
      #snp.position = snp.position.to_i
      #snp.original.upcase!
      #snp.snp.upcase!  
      snp.chromosome. strip!
      #snp.parse_sequence_snp
      snp.parse_sequence_multiple_snp
      snp.exon_list = Hash.new()
      snp
    end
    
    def parse_snp
      
    end

		def debug_output (str)
			# comment out the puts call to turn off the debug output
			puts "#{str}"
		end

    def parse_sequence_snp
      pos = 0
			      
			match_data = /(?<pre>\w*)\[(<org>[ACGT])\/(?<snp>[ACGT])\](?<pos>\w*)/.match(sequence_original.strip)
      if match_data
				debug_output "match_data  #{match_data}"
        @position = Regexp.last_match(:pre).size + 1
        @original = Regexp.last_match(:org)
        @snp = Regexp.last_match(:snp)

				debug_output "position #{@position}"
				debug_output "original #{@snp}"
				debug_output "snp #{@snp}"
        
        amb_base = Bio::NucleicAcid.to_IUAPC("#{@original}#{@snp}")
				debug_output "amb_base #{amb_base}"
        
        @template_sequence = "#{Regexp.last_match(:pre)}#{amb_base}#{Regexp.last_match(:pos)}"
				debug_output "template_sequence #{@template_sequence}"
				debug_output "sequence_original #{sequence_original}"

     end 

		end

		#
		# For sequences with multiple SNPs we need to replace each instance
		#
    def parse_sequence_multiple_snp
      pos = 0
			parsed_sequence = ""
			success = 1
			loop = 1

			while loop == 1
				new_pos = sequence_original.index('[', pos)
				debug_output "pos  #{pos}"
				debug_output "new_pos  #{new_pos}"

				if new_pos == nil
					loop = 0
					seq_sub = sequence_original[pos, sequence_original.length - pos]
					debug_output "final seq_sub #{seq_sub}"
					parsed_sequence << seq_sub

					debug_output "exit loop"
				else
					seq_sub = sequence_original[new_pos, 5]
					debug_output "seq_sub #{seq_sub}"

					match_data = /\[([ACGT])\/([ACGT])\]/.match (seq_sub)

					if match_data
						# we specified 2 capture groups for match_data
						debug_output "match_data length #{match_data.string.length}"

				    @position = match_data.begin (0)
				    @original = match_data[1]
				    @snp = match_data[2]

						debug_output "position #{@position}"
						debug_output "original #{@original}"
						debug_output "snp #{@snp}"
				    
				    amb_base = Bio::NucleicAcid.to_IUAPC("#{@original}#{@snp}")
						debug_output "amb_base #{amb_base}"
						    
						debug_output "old parsed_sequence #{parsed_sequence}"

						seq_sub = sequence_original[pos, new_pos - pos]	

						debug_output "appending seq_sub #{seq_sub}"					
						parsed_sequence << seq_sub

						parsed_sequence << amb_base
						debug_output "new parsed_sequence #{parsed_sequence}"

						# move past the matched SNP
						pos = new_pos + match_data.string.length
					else
						puts "no match"
						loop = 0
					end		# if match_data
				
				end		# if new_pos == nil

			end		# while loop == 1
	
			debug_output "parsed_sequence #{parsed_sequence}"
			@template_sequence = parsed_sequence
    end		# def parse_sequence_multiple_snp
    
  end		# class SNPSequence < SNP

end

