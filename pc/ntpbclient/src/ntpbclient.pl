#!/usr/bin/perl
#
# $Id$
#
# Copyright (C) 2010 Mathias Lafeldt <misfire@debugon.org>
#
# This file is part of PS2rd, the PS2 remote debugger.
#
# PS2rd is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# PS2rd is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with PS2rd.  If not, see <http://www.gnu.org/licenses/>.
#

use warnings;
use strict;
use IO::Socket;

my $remote_host = "192.168.0.10";
my $remote_port = 4234;

my $REMOTE_CMD_HALT = 0x201;
my $REMOTE_CMD_RESUME = 0x202;

my $EOT = 0xffff; 
my $ACK = 0x0001;

print "target: $remote_host:$remote_port\n";

my $sock = IO::Socket::INET->new(
	PeerAddr => $remote_host,
	PeerPort => $remote_port,
	Proto => "tcp",
	Type => SOCK_STREAM,
	Timeout => 3
) or die "Could not create socket: $!\n";

my @magic = (0xff, 0x00, "NTPB");
my $size = 0;
my $cmd = $REMOTE_CMD_HALT;

my $buf = pack("C2 a4 S S", @magic, $size, $cmd);

$sock->send($buf) or die "Send error: $!\n";

my $ret = $sock->recv($buf, 65536);
unless(defined $ret) { die "Recv error"; }

$ret = $sock->recv($buf, 65536);
unless(defined $ret) { die "Recv error"; }

$buf = pack("C2 a4 S S S", @magic, $size, $EOT, $ACK);

$sock->send($buf) or die "Send error: $!\n";

print "PS2 halted!\n";

close($sock);
