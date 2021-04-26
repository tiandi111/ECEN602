set TCP_FLAVOR [lindex $argv 0]
set CASE_NO    [lindex $argv 1]

set tcp_type ""

switch $TCP_FLAVOR {
	"VEGAS" { set tcp_type "Vegas" }
	"SACK"  { set tcp_type "Sack1" }
	default { puts "unsupported tcp flavor" }
}

set latency ""

switch $CASE_NO {
	1 { set latency "12.5ms" }
	2 { set latency "20ms"   }
	3 { set latency "27.5ms" }
	4 { set latency "35ms" }
	5 { set latency "42.5ms" }
	6 { set latency "50ms" }
	7 { set latency "57.5ms" }
	8 { set latency "65ms" }
	9 { set latency "72.5ms" }
	10 { set latency "145.5ms" }
	default { puts "unsupported case number" }
}

puts "TCP flavor: $tcp_type, latency: $latency"

# Create a new Simulator instance
set ns [new Simulator]

$ns color 1 Blue
$ns color 2 Red
$ns color 3 Green
$ns color 4 Black

# Open the trace file
set tracefile1 [open out.tr w]
set winfile [open WinFile w]
$ns trace-all $tracefile1

# Open the Nam trace file
set namfile [open out.nam w]
$ns namtrace-all $namfile

#Open files recording throughput
set f1 [open tp1.tr w]
set f2 [open tp2.tr w]

# Create nodes
set src1 [$ns node]
set src2 [$ns node]
set rcv1 [$ns node]
set rcv2 [$ns node]
set R1   [$ns node]
set R2   [$ns node]

# Routers use shape box
$R1 shape box
$R2 shape box

# Create links
# Arguments description: link type node node bandwidth propagation_delay queue_option
$ns duplex-link $src1 $R1 10Mb 5ms      DropTail
$ns duplex-link $rcv1 $R2 10Mb 5ms      DropTail
$ns duplex-link $src2 $R1 10Mb $latency DropTail
$ns duplex-link $rcv2 $R2 10Mb $latency DropTail
$ns duplex-link $R1   $R2 1Mb  5ms      DropTail

# Set link position and color (nam)
$ns duplex-link-op $src1 $R1 orient right-down 
$ns duplex-link-op $src1 $R1 color "blue"
$ns duplex-link-op $src2 $R1 orient right-up  
$ns duplex-link-op $src2 $R1 color "blue" 
$ns duplex-link-op $rcv1 $R2 orient left-down 
$ns duplex-link-op $rcv1 $R2 color "red" 
$ns duplex-link-op $rcv2 $R2 orient left-up  
$ns duplex-link-op $rcv2 $R2 color "red"
$ns duplex-link-op $R1 $R2   orient left     
$ns duplex-link-op $R1 $R2   color "green"     
$ns duplex-link-op $R2 $R1   orient right     
$ns duplex-link-op $R2 $R1   color "black"    

# set queue size of link (n0-n2) to 20
#$ns queue-limit $n0 $n1 20

# Setup TCP connectionsset tcp_sender11 [new Agent/TCP/$TCP_FLAVOR]
set tcp_sender1 [new Agent/TCP/$tcp_type]
set tcp_sender2 [new Agent/TCP/$tcp_type]
set tcp_sink1   [new Agent/TCPSink]
set tcp_sink2   [new Agent/TCPSink]
$ns attach-agent $src1 $tcp_sender1
$ns attach-agent $src2 $tcp_sender2
$ns attach-agent $rcv1 $tcp_sink1
$ns attach-agent $rcv2 $tcp_sink2

$ns connect $tcp_sender1 $tcp_sink1
$ns connect $tcp_sender2 $tcp_sink2
$tcp_sender1 set fid_ 1
$tcp_sender2 set fid_ 2

# Setup FTP over TCP connection
set ftp1 [new Application/FTP]
set ftp2 [new Application/FTP]
$ftp1 attach-agent $tcp_sender1
$ftp2 attach-agent $tcp_sender2

# Setup scheduler
$ns at 0.0   "$ftp1 start"
$ns at 2.0   "$ftp2 start"
$ns at 400.0 "$ftp1 stop"
$ns at 400.0 "$ftp2 stop"

# Procedure for recording throughput
proc record {} {
	global ns tcp_sink1 tcp_sink2 f1 f2
	set time 1.0
	set bw1 [$tcp_sink1 set bytes_]
	set bw2 [$tcp_sink2 set bytes_]
	set now [$ns now]
	puts $f1 "$now [expr $bw1/$time*8/1000000]"
	puts $f2 "$now [expr $bw2/$time*8/1000000]"
	$tcp_sink1 set bytes_ 0
	$tcp_sink2 set bytes_ 0
	# start the procedure later
	$ns at [expr $now+$time] "record"
}

$ns at 0.0 "record"

# Procedure for plotting window size.
proc plotWindow {tcpSource file} {
	global ns
	set time 0.1 	
	set now [$ns now]
	set cwnd [$tcpSource set cwnd_]
	puts $file "$now $cwnd"
	$ns at [expr $now + $time] "plotWindow $tcpSource $file"
}

$ns at 0.0 "plotWindow $tcp_sender1 $winfile"
$ns at 0.0 "plotWindow $tcp_sender2 $winfile"

# Procedure to finish the simulation
proc finish {} {
	global ns tracefile1 namfile f1 f2
	$ns flush-trace
	close $tracefile1
	close $namfile
	close $f1
	close $f2
       	#exec nam out.nam &
	exec xgraph tp1.tr tp2.tr &
	exit 0
}

$ns at 400.0 "finish"

$ns run



 

