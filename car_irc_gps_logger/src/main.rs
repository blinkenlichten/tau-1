use std::io::{self, Read, Write/*, ErrorKind*/};
use std::net::TcpStream;
//use std::io::prelude::*;

pub struct Context
{
    pub stream: io::Result<TcpStream>,
}

impl Context
{
    pub fn new(host_and_port: String) -> Self
    {
        return Context { stream: TcpStream::connect(host_and_port) };
    } 
}

fn main() {
    println!("Hello, world!");
    let ctx = Context::new(String::from("10.0.0.1:6667"));
    let mut stream = match ctx.stream
    {
        Ok(_stream) => _stream,
        Err(_e) => panic!("network failure")
    };

    let snd_commands = [String::from("NICK t4-test\r\n"),
        String::from("USER t4-test 0 * :botenko\r\n"),
        String::from("PASS qwerty\r\n:t4-test!botenko@localhost\r\n"),
        String::from("WHO *\r\n"), String::from("UPTIME\r\n")];

    for cmd in snd_commands.iter() {
        println!("Sending cmd: {}", cmd);
        stream.write(cmd.as_bytes()).unwrap();
    }
    let mut rdbuffer = [0 as u8; 1024];
    let endbuf = [10,13,10,13 as u8];// \r\n\r\n

    println!("read from server: ");
    loop {
        let mut rd_len = 0 as usize;
        match stream.read(&mut rdbuffer) {
            Ok(_len) => {
                for idx in 0 .. _len {
                    let ch = rdbuffer[idx] as char;
                    print!("{}", ch);
                }
                io::stdout().flush().unwrap();
                rd_len = _len;
            },
            Err(_e) => break 
        };

        if rd_len > 4 as usize &&  rdbuffer[rd_len - 5..rd_len-1] == endbuf {
            println!("done reading");
            break;
        }
    }

    match stream.write(b"QUIT\r\n")
    {
        Ok(_wlen) => println!("quit send."),
        Err(_e) => println!("E: quit, stale connection")
    }
}
