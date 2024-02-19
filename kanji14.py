#!/usr/bin/env python3

def encode(binary: bytes) -> str:
    if isinstance(binary, str):
        binary = binary.encode()
    elif not isinstance(binary, (bytes, bytearray)):
        binary = bytes(binary)

    temp = 0
    bits = 0

    text = ''
    for val in binary:
        temp = (temp << 8) | val
        bits += 8
        if bits >= 14:
            bits -= 14
            text += chr(0x4E10 + (0x3FFF & (temp >> bits)))
    if bits:
        text += chr(0x4E10 + (0x3FFF & (temp << (14 - bits))))
        text += chr(0x4E10 - bits)
    return text


def decode(ucsdata: str) -> bytes:
    remain = 0
    temp = 0
    bits = 0

    binary = []
    for wchr in ucsdata:
        code = ord(wchr)
        if code < 0x4E10:
            if code < 0x4E03:
                continue
            remain = 0x4E10 - code
            break
        if code > 0x8E0F:
            continue

        temp = (temp << 14) | (code - 0x4E10)
        bits += 14
        while bits >= 8:
            bits -= 8
            binary.append((temp >> bits) & 0xff)

    if remain:
        padding = 14 - remain
        if padding >= 8 and binary:
            del binary[-1]

    return bytes(binary)


if __name__ == '__main__':
    import argparse
    import random
    import sys

    def test():
        for cnt in range(20):
            for size in range(50):
                data = [int(random.random() * 256) for _ in range(size)]
                denc = encode(data)
                ddec = decode(denc)

                istr = ''.join(f'{v:02x}' for v in data)
                ostr = ''.join(f'{v:02x}' for v in ddec)
                if istr != ostr:
                    print('Error!!')
                    print('Data  :', istr)
                    print('Decode:', ostr)
                    print('Encode:', denc)
                    raise Exception()

    def read_data(path, mode):
        if path == '-':
            if 'b' in mode:
                sys.stdin.reconfigure(encoding='iso8859-1')
                return map(ord, sys.stdin.read())
            return sys.stdin.read()
        with open(path, mode) as ifp:
            return ifp.read()

    def write_data(path, mode, data):
        if path == '-':
            if 'b' in mode:
                sys.stdout.reconfigure(encoding='iso8859-1')
                data = ''.join(map(chr, data))
            sys.stdout.write(data)
        else:
            with open(path, mode) as ofp:
                ofp.write(data)

    def main():
        parser = argparse.ArgumentParser()
        parser.add_argument('-d', '--decode', action='store_true', default=False)
        parser.add_argument('-e', '--encode', action='store_true', default=False)
        parser.add_argument('-w', '--width', type=int, default=0)
        parser.add_argument('-t', '--test', action='store_true', default=False)
        parser.add_argument('inpfile', metavar='INP', nargs='?', default='-')
        parser.add_argument('outfile', metavar='OUT', nargs='?', default='-')

        args = parser.parse_args()

        if args.test:
            return test()
        if args.decode == args.encode:
            parser.print_help()
            return sys.exit(1)
        width = args.width

        if args.decode:
            write_data(args.outfile, 'wb', decode(read_data(args.inpfile, 'rt')))
        elif args.encode:
            enc = encode(read_data(args.inpfile, 'rb'))
            if width > 0:
                nenc = ''
                for pos in range(0, len(enc), width):
                    nenc += enc[pos:pos+width] + '\n'
                enc = nenc
            write_data(args.outfile, 'wt', enc)

    main()
