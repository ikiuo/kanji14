"use strict";

// -----------------------------------------------------------------

class Kanji14 {

	static encode(binary) {
		// 引数は Uint8Array オブジェクトのみ.
		if (!(binary instanceof Uint8Array))
			throw TypeError();

		let temp = 0;
		let bits = 0;

		const k14 = [];
		for (const i in binary) {
			temp = (temp << 8) | binary[i];
			bits += 8;
			if (bits >= 14) {
				bits -= 14;
				k14.push(String.fromCharCode(0x4E10 + (0x3FFF & (temp >> bits))));
			}
		}
		if (bits) {
			k14.push(String.fromCharCode(0x4E10 + (0x3FFF & (temp << (14 - bits)))));
			k14.push(String.fromCharCode(0x4E10 - bits));
		}
		return k14.join('')
	}

	static decode(text) {
		// 引数は String オブジェクトのみ.
		if (typeof(text) != 'string')
			throw TypeError();

		const buffer = new ArrayBuffer((text.length * 14 + 7) >> 3);
		const binary = new Uint8Array(buffer);
		let bpos = 0;

		let remain = 0;
		let temp = 0;
		let bits = 0;

		for (const i in text) {
			const code = text.charCodeAt(i);
			if (code < 0x4E10) {
				if (code < 0x4E03)
					continue;
				remain = 0x4E10 - code;
				break;
			}
			if (code > 0x8E0F)
				continue;

			temp = (temp << 14) | (code - 0x4E10);
			bits += 14;
			while (bits >= 8) {
				bits -= 8;
				binary[bpos++] = temp >> bits;
			}
		}
		if (remain)
			bpos -= (14 - remain) >> 3;
		return new Uint8Array(buffer, 0, Math.max(0, bpos));
	}
}

// -----------------------------------------------------------------
// Local Variables:
// tab-width: 4
// End:
