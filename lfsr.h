#ifndef LFSR_H
#define LFSR_H

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include <map>
#include <vector>

/*
 * This class implements a LFSR (up to 64 bits)
 */
class LFSR {
	private:
	protected:
		uint64_t next_value; // the LFSR value

		typedef std::vector<uint8_t> BITMAP;
		BITMAP bitmap; // the bit pattern  used to compute the next value
                       // of the LFSR

        // advance the LFSR
		void next() {
			uint64_t bit = next_value;
			for (BITMAP::const_iterator i = bitmap.begin(); i != bitmap.end();
                    i++) {
				bit ^= next_value >> *i;
			}
			bit &= 1;
			next_value = (bit << (bits - 1)) | (next_value >> 1);
		}

        // find the size of n in number of bits aka the highest non-zero bit
        // position
		static uint8_t n_bits(const uint64_t & n) {
			uint8_t retval = 0;
			uint64_t nn = n;
			while (nn) {
				nn >>= 1;
				retval++;
			}
			return retval;
		}
	public:
		const uint8_t bits; // the size of the LFSR
		LFSR(const uint64_t & blocks, const uint64_t & seed = getpid()):
            next_value(seed), bits(n_bits(blocks)) {

            // initialize the LFSR
			next_value <<= 64 - n_bits(blocks);
			next_value >>= 64 - n_bits(blocks);
			while (next_value >= blocks) next_value >>= 1;
			if (!next_value) next_value = 1;

            // initialize the bit pattern used to advance the LFSR
			switch (bits) {
				case 3:
				case 4:
				case 6:
				case 7:
				case 15:
				case 22:
				case 60:
				case 63:
					bitmap.push_back(1);
					break;
				case 5:
				case 11:
				case 21:
				case 29:
				case 35:
					bitmap.push_back(2);
					break;
				case 10:
				case 17:
				case 20:
				case 25:
				case 28:
				case 31:
				case 41:
				case 52:
					bitmap.push_back(3);
					break;
				case 9:
				case 39:
					bitmap.push_back(4);
					break;
				case 23:
				case 47:
					bitmap.push_back(5);
					break;
				case 18:
				case 57:
					bitmap.push_back(7);
					break;
				case 49:
					bitmap.push_back(9);
					break;
				case 36:
					bitmap.push_back(11);
					break;
				case 33:
					bitmap.push_back(13);
					break;
				case 58:
					bitmap.push_back(19);
					break;
				case 55:
					bitmap.push_back(24);
					break;
				case 16:
					bitmap.push_back(1);
					bitmap.push_back(3);
					bitmap.push_back(12);
					break;
				case 45:
					bitmap.push_back(1);
					bitmap.push_back(3);
					bitmap.push_back(4);
					break;
				case 24:
					bitmap.push_back(1);
					bitmap.push_back(2);
					bitmap.push_back(7);
					break;
				case 43:
					bitmap.push_back(1);
					bitmap.push_back(5);
					bitmap.push_back(6);
					break;
				case 56:
				case 59:
					bitmap.push_back(1);
					bitmap.push_back(21);
					bitmap.push_back(22);
					break;
				case 51:
				case 53:
				case 61:
					bitmap.push_back(1);
					bitmap.push_back(15);
					bitmap.push_back(16);
					break;
				case 46:
					bitmap.push_back(1);
					bitmap.push_back(20);
					bitmap.push_back(21);
					break;
				case 42:
					bitmap.push_back(1);
					bitmap.push_back(22);
					bitmap.push_back(23);
					break;
				case 44:
				case 50:
					bitmap.push_back(1);
					bitmap.push_back(26);
					bitmap.push_back(27);
					break;
				case 48:
					bitmap.push_back(1);
					bitmap.push_back(27);
					bitmap.push_back(28);
					break;
				case 54:
					bitmap.push_back(1);
					bitmap.push_back(36);
					bitmap.push_back(37);
					break;
				case 62:
					bitmap.push_back(1);
					bitmap.push_back(56);
					bitmap.push_back(57);
					break;
				case 8:
					bitmap.push_back(2);
					bitmap.push_back(3);
					bitmap.push_back(4);
					break;
				case 40:
					bitmap.push_back(2);
					bitmap.push_back(19);
					bitmap.push_back(21);
					break;
				case 12:
					bitmap.push_back(6);
					bitmap.push_back(8);
					bitmap.push_back(11);
					break;
				case 34:
					bitmap.push_back(7);
					bitmap.push_back(32);
					bitmap.push_back(33);
					break;
				case 13:
					bitmap.push_back(9);
					bitmap.push_back(10);
					bitmap.push_back(12);
					break;
				case 14:
					bitmap.push_back(9);
					bitmap.push_back(11);
					bitmap.push_back(13);
					break;
				case 32:
					bitmap.push_back(10);
					bitmap.push_back(30);
					bitmap.push_back(31);
					break;
				case 19:
					bitmap.push_back(13);
					bitmap.push_back(17);
					bitmap.push_back(18);
					break;
				case 26:
					bitmap.push_back(20);
					bitmap.push_back(24);
					bitmap.push_back(25);
					break;
				case 27:
					bitmap.push_back(22);
					bitmap.push_back(25);
					bitmap.push_back(26);
					break;
				case 30:
					bitmap.push_back(24);
					bitmap.push_back(26);
					bitmap.push_back(29);
					break;
				case 38:
					bitmap.push_back(32);
					bitmap.push_back(31);
					bitmap.push_back(37);
					break;
				case 37:
					bitmap.push_back(32);
					bitmap.push_back(33);
					bitmap.push_back(34);
					bitmap.push_back(35);
					bitmap.push_back(36);
					break;
				case 64:
					bitmap.push_back(1);
					bitmap.push_back(3);
					bitmap.push_back(4);
					break;
				default:
					abort();
			}
		}
		virtual ~LFSR() {}

        /*
         * The user API
         */
        // get the current value of the LFSR
		operator uint64_t() const {
			return next_value;
		}
        // the pre and post increment operators for the LFSR
		uint64_t operator++() {
			const uint64_t retval = next_value;
			next();
			return retval;
		}
		uint64_t operator++(int dummy) {
			next();
			return next_value;
		}
};

/*
 * The base class for the logical block offset in a device of size 'blocks'
 */
class Offset {
	protected:
		uint64_t offset; // current logical block offset in the device
		uint64_t blocks; // size of the device
	public:
		Offset(const uint64_t p_blocks): offset(0), blocks(p_blocks) {
		}
		virtual ~Offset() {}
		Offset(const Offset & o): offset(o.offset), blocks(o.blocks) {
		}
		const Offset & operator=(const Offset & o) {
			offset = o.offset;
			blocks = o.blocks;
			return *this;
		}
		operator uint64_t() const {
			return offset;
		}
		virtual uint64_t Next() = 0;
};

/*
 * Class to create sequential logical block offsets in the device
 */
class SequentialOffset: public Offset {
	private:
		SequentialOffset(const SequentialOffset & o): Offset(o) {
		}
		const SequentialOffset & operator=(const SequentialOffset & o) {
			offset = o.offset;
			blocks = o.blocks;
			return *this;
		}
	public:
	       	SequentialOffset(const uint64_t & blocks): Offset(blocks) {
		}
		uint64_t Next() {
			offset += 1;
			if (offset >= blocks) offset = 0;
			return offset;
		}
}; 

/*
 * Class to create random logical block offsets in the device
 */
class RandomOffset: public Offset {
	private:
	       	RandomOffset(const RandomOffset &);
	       	const RandomOffset & operator=(const RandomOffset &);
		LFSR lfsr;
	public:
		RandomOffset(const uint64_t & blocks): Offset(blocks), lfsr(blocks) {
			offset = lfsr - 1;
		}
		uint64_t Next() {
			while (1) {
				offset = (lfsr++) - 1;
			       	if (offset < blocks)
				       	return offset;
			}
			abort();
			return 0;
		}
};

#endif // LFSR_H
