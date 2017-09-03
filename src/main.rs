use std::io::prelude::*;
use std::net::TcpStream;

fn main() {
    let mut stream = TcpStream::connect("192.168.0.10:4234").unwrap();

    stream.write_all(b"\xff\x00NTPB\x00\x00\x01\x02").unwrap();

    let mut buf = [0; 65536];
    stream.read(&mut buf).unwrap();
    println!("{:?}", buf.len());

    stream.write_all(b"\xff\x00NTPB\x00\x00\xff\xff").unwrap();
}
