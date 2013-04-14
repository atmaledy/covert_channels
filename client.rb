require "rubygems"
require 'packetfu'
require 'optparse'
#a comment
#-----------------------   Variable Declaration   -------------------------------

source_ip = nil
sport=nil

dport=nil
dest_ip = nil
filename=nil
iface = "en1"

#----------------------------   Functions   -------------------------------------

#
# print_usage()
# Displays help for the application
#
def print_usage_and_quit()
  puts "client.rb -d <Destination IP> -p <Destination port> -f <Filename>"
  exit
end

#
# get_src_ip(iface)
# Gets the source IP from an interface
#
def get_src_ip(iface)  
  ifc =  PacketFu::Utils::ifconfig("en1")
  return ifc[:ip_saddr ]
end

#------------------------- Application Entry Point  -----------------------------
#
# Process command line arguments and if we didn't get them, then set defaults...
#
options = {}
OptionParser.new do|opts|
opts.banner = "Usage: example.rb [options]"
opts.on( '-h:', '--help', 'Display this screen' ) do 
  print_usage();
  end
opts.on( '-s:', '--source-ip', 'Source IP to send packets to.' ) do |source_ip|
  end
opts.on( '-d:', '--dest-ip', 'Destination IP to send packets to.' ) do |dest_ip|
  end  
opts.on( '-q:', '--source-port', 'Source port to send packets to.' ) do |sport|
  end  
opts.on( '-p:', '--dest-port', 'Destination port to send packets to.' ) do |dport|
  end    
opts.on( '-f:', '--filename', 'File to send.' ) do |filename|
  end    
end.parse!

if dest_ip.nil? | dport.nil? | filename.nil?
  print_usage_and_quit()
end

if source_ip.nil?
  source_ip = get_src_ip(iface)
end

 #
 # Generate a new TCP packet and set syn flag 
 #
tcp_pkt = PacketFu::TCPPacket.new()

# set flags
flags = PacketFu::TcpFlags.new(:syn => 1)
tcp_pkt.tcp_flags = flags;

# set IP addresses
s_ip = PacketFu::Octets.new
tcp_pkt.ip_src = s_ip.read_quad(source_ip);

# set dest port
d_ip = PacketFu::Octets.new
tcp_pkt.ip_dst = d_ip.read_quad(dest_ip);

tcp_pkt.tcp_dst = dport.to_i
# puts tcp_pkt.inspect
#read the file character by character. x is in ascii because we are reading byte by byte =)
File.open(filename) do |file|
  file.each_byte {|x| 
    tcp_pkt.ip_id = x
    tcp_pkt.recalc #recalcualte checksum
    tcp_pkt.to_w("en1")

  
  }
end


#show the packet (for testing)
# puts tcp_pkt.ip_id

#tcp_pkt.to_w('eth0')