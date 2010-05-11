#!/usr/bin/perl
#
# Perl rewrite of ntpbclient
#
# Copyright (C) 2010 Mathias Lafeldt <misfire@debugon.org>
#

use warnings;
use strict;
use IO::Socket;

my $remote_host = "192.168.0.1";
my $remote_port = 4234;

my $REMOTE_CMD_HALT = 0x201;
my $REMOTE_CMD_RESUME = 0x202;

print "target: $remote_host:$remote_port\n";

my $sock = IO::Socket::INET->new(
	PeerAddr => $remote_host,
	PeerPort => $remote_port,
	Proto => "tcp"
) or die "Could not create socket: $!\n";

my @magic = (0xff, 0x00, "NTPB");
my $size = 0;

my $cmd;
if ($ARGV[0] eq "halt") {
	$cmd = $REMOTE_CMD_HALT;
} else {
	$cmd = $REMOTE_CMD_RESUME;
}

my $buf = pack("C2 a4 S2", @magic, $size, $cmd);

$sock->send($buf) or die "Send error: $!\n";

close($sock);
