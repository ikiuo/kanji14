#!/usr/bin/env python3

def encode(binary: bytes) -> str:
    if isinstance(binary, str):
        binary = binary.encode()
    elif not isinstance(binary, (bytes, bytearray)):
        binary = bytes(binary)

    mask = (1 << 14) - 1

    bits = len(binary) << 3
    count = bits // 14
    remain = bits % 14

    out = ''
    for opos in range(count):
        bbit = opos * 14
        bidx = bbit >> 3
        brsh = 10 - (bbit & 7)

        bd0 = binary[bidx]
        bd1 = binary[bidx + 1]
        bd2 = binary[bidx + 2] if brsh < 8 else 0
        val = (((bd0 << 16) | (bd1 << 8) | bd2) >> brsh) & mask

        out += chr(0x4E10 + val)

    if remain:
        bbit = count * 14
        ebit = bbit + remain - 1
        bidx = bbit >> 3
        eidx = ebit >> 3
        shft = 10 - (bbit & 7)

        bd0 = binary[bidx]
        bd1 = binary[eidx] if bidx != eidx else 0
        val = (((bd0 << 16) | (bd1 << 8)) >> shft) & mask

        out += chr(0x4E10 + val)
        out += chr(0x4E10 - remain)

    return out


def decode(ucsdata: str) -> bytes:
    code_min = 0x4E10 - (14 - 1)
    code_max = 0x8E10 - 1

    remain = 0
    temp = 0
    pbits = 0

    binary = []
    for wchr in ucsdata:
        code = ord(wchr)
        if code < 0x4E10:
            if code < code_min:
                continue
            remain = 0x4E10 - code
            break
        if code > code_max:
            continue

        val = code - 0x4E10
        temp = (temp << 14) | val
        pbits += 14
        while pbits >= 8:
            binary.append((temp >> (pbits - 8)) & 0xff)
            pbits -= 8
        temp &= (1 << pbits) - 1

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
        for cnt in range(1):
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
