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

my ($remote_host, $remote_port) = ("192.168.0.10", 4234);

my $NTPB_MAGIC = "\xff\x00NTPB";
my $NTPB_HDR_SIZE = length($NTPB_MAGIC) + 2 + 2;
my ($NTPB_DUMP, $NTPB_HALT, $NTPB_RESUME) = (0x0100, 0x0201, 0x0202);
my ($NTPB_SEND_DUMP, $NTPB_EOT, $NTPB_ACK) = (0x0300, 0xffff, 0x0001);
my $BUFSIZE = 65536;

my $sock;

sub ntpb_connect {
    IO::Socket::INET->new(
        PeerAddr => $remote_host,
        PeerPort => $remote_port,
        Proto => "tcp",
        Timeout => 3
    )
}

sub ntpb_send_cmd {
    my $cmd = shift;
    my $buf = shift || "";
    $sock->send($NTPB_MAGIC . pack("S2", length $buf, $cmd) . $buf) or return undef;
    $sock->recv($buf, $BUFSIZE) // return undef;
}

sub ntpb_recv_data {
    my $file = shift;
    my $eot = 0;

    if ($file) {
        open FILE, ">$file" or die $!;
        binmode FILE;
    }

    do {
        my $buf;
        $sock->recv($buf, $BUFSIZE) // return undef;
        my ($magic, $bytes, $cmd) = unpack("a6 S S", $buf);
        my $size = $bytes + $NTPB_HDR_SIZE;

        while (length($buf) < $size) {
            $sock->recv($_, $BUFSIZE - length($buf)) // return undef;
            $buf .= $_;
        }

        if ($cmd == $NTPB_SEND_DUMP) {
            $buf = substr($buf, $NTPB_HDR_SIZE);
            print FILE $buf if $file;
        } elsif ($cmd == $NTPB_EOT) {
            $eot = 1;
        }

        $sock->send($NTPB_MAGIC . pack("S3", length $NTPB_ACK, $NTPB_EOT, $NTPB_ACK));
    } while (!$eot);

    close(FILE) if $file;
}

my $cmd = shift || die "command missing";

if ($cmd eq "halt") {
    $sock = ntpb_connect($remote_host, $remote_port) or die "$!";
    ntpb_send_cmd($NTPB_HALT);
    ntpb_recv_data();
    close($sock);
} elsif ($cmd eq "resume") {
    $sock = ntpb_connect($remote_host, $remote_port) or die "$!";
    ntpb_send_cmd($NTPB_RESUME);
    ntpb_recv_data();
    close($sock);
} elsif ($cmd eq "dump") {
    die "arguments missing" if (@ARGV != 3);
    my ($start, $end, $file) = @ARGV;
    $sock = ntpb_connect($remote_host, $remote_port) or die "$!";
    ntpb_send_cmd($NTPB_DUMP, pack("L2", hex $start, hex $end));
    ntpb_recv_data($file);
    close($sock);
} else {
    die "invalid command";
}
