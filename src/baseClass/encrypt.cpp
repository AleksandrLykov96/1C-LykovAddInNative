
#include "baseClass/encrypt.hpp"

using namespace kuz;
using namespace std;

Kuznechik::Kuznechik(const wstring& key) {
	key_t masterKey{};

	const auto add = wstring(skCrypt_key(L"Зачем ты это сделала, надела платье белое", L'6', L'9').decrypt());
	const auto sizeStr = key.size();
	for (unsigned char i = 0; i < 32; i++) {
		uint32_t code = i + add[i];
		if (i < sizeStr)
			code += key[i];

		masterKey[i] = static_cast<BYTE>(code);
	}

	p_Key_Pairs = getRoundKeys(masterKey);
	size_t j = 0;
	for (size_t i = 0; i < p_Key_Pairs.size(); i++, j += 2) {
		p_Keys[j] = p_Key_Pairs[i][0];
		p_Keys[j + 1] = p_Key_Pairs[i][1];
	}

	p_First_Gamma.fill(0);
	for (unsigned char i = 0; i < block_size / 2; i++)
		p_First_Gamma[i] = static_cast<BYTE>(i * key.size());
}

string Kuznechik::encrypt(const wstring& data) const {
	const auto dataStr = gl_conv_wstring_to_string(data);
	size_t blocksNum = dataStr.length() / block_size;

	if (const unsigned long bytesLeft = dataStr.length() % block_size; bytesLeft != 0)
		blocksNum++;

	vector<block_t> dataBlocks;

	dataBlocks.assign(blocksNum, block_t());
	for (size_t i = 0; i < dataStr.length(); i += block_size) {
		const size_t bytesLeft = dataStr.length() - i;
		const size_t blockIndex = i / block_size;

		if (bytesLeft > block_size) {
			for (unsigned char j = 0; j < block_size; j++)
				dataBlocks[blockIndex][j] = static_cast<unsigned char>(dataStr[i + j]);
		} else {
			dataBlocks[blockIndex].fill(0);
			for (size_t j = 0; j < bytesLeft; j++)
				dataBlocks[blockIndex][j] = static_cast<unsigned char>(dataStr[i + j]);
		}
	}

	block_t gammaBlock = p_First_Gamma;
	vector<block_t> p_Gamma_Blocks;

	for (size_t i = 0; i < blocksNum; i++) {
		p_Gamma_Blocks.push_back(gammaBlock);
		gammaBlock = increment(gammaBlock);
	}

	for (size_t i = 0; i < blocksNum; i++) {
		for (unsigned char j = 0; j < block_size; j++)
			dataBlocks[i][j] ^= p_Gamma_Blocks[i][j];
	}
	
	vector<BYTE> encryptedData;
	encryptedData.assign(block_size * blocksNum, 0);

	for (size_t i = 0; i < blocksNum; i++) {
		block_t plsx = lsx(p_Keys[0], dataBlocks[i]);
		for (unsigned char k = 1; k < num_keys - 1; k++)
			plsx = lsx(p_Keys[k], plsx);

		block_t encBlock = X(p_Keys[num_keys - 1], plsx);
		for (unsigned char j = 0; j < block_size; j++)
			encryptedData[i * block_size + j] = encBlock[j];
	}

	string result;
	result.reserve(encryptedData.size());

	for (const unsigned char i : encryptedData)
		result.push_back(static_cast<char>(i));

	while (!result.empty() && result.ends_with('\0'))
		result.pop_back();

	// Возвращаем в формате Base64
	using it = boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<string::const_iterator, 6, 8>>;
	auto tmp = string(it(result.begin()), it(result.end()));

	return tmp.append((3 - result.size() % 3) % 3, '=');
}

wstring Kuznechik::decrypt(const wstring& dataSrc) const {
	return decrypt(gl_conv_wstring_to_string(dataSrc));
}

wstring Kuznechik::decrypt(const string& dataSrc) const {
	using it = boost::archive::iterators::transform_width<boost::archive::iterators::binary_from_base64<string::const_iterator>, 8, 6>;
	auto dataString = string(it(begin(dataSrc)), it(end(dataSrc)));

	while (!dataString.empty() && dataString.ends_with('\0'))
		dataString.pop_back();

	vector<BYTE> data{};
	data.reserve(dataString.size());

	for (const char i : dataString)
		data.push_back(static_cast<BYTE>(i));

	size_t blocksNum = data.size() / block_size;
	if (const size_t bytesLeft = data.size() % block_size; bytesLeft != 0)
		blocksNum++;
	
	vector<block_t> dataBlocks;
	dataBlocks.assign(blocksNum, block_t());

	for (size_t i = 0; i < data.size(); i += block_size) {
		const size_t blockIndex = i / block_size;
		for (size_t j = 0; j < block_size; j++)
			dataBlocks[blockIndex][j] = data[i + j];
	}

	vector<block_t> decryptedBlocks;
	for (size_t i = 0; i < blocksNum; i++) {
		const block_t block = dataBlocks[i];

		block_t x = X(p_Keys[num_keys - 1], block);
		block_t li = l_inv(x);
		block_t si = S_inv(li);
		
		for (unsigned char j = num_keys - 2; j > 0; j--) {
			x = X(p_Keys[j], si);
			li = l_inv(x);
			si = S_inv(li);
		}

		block_t decBlock = X(p_Keys[0], si);
		decryptedBlocks.push_back(decBlock);
	}

	block_t gammaBlock = p_First_Gamma;
	vector<block_t> p_Gamma_Blocks;

	for (size_t i = 0; i < blocksNum; i++) {
		p_Gamma_Blocks.push_back(gammaBlock);
		gammaBlock = increment(gammaBlock);
	}

	for (size_t i = 0; i < blocksNum; i++)
		decryptedBlocks[i] = X(decryptedBlocks[i], p_Gamma_Blocks[i]);

	vector<char> decData;
	for (size_t i = 0; i < blocksNum; i++) {
		for (unsigned char j = 0; j < block_size; j++)
			decData.push_back(static_cast<char>(decryptedBlocks[i][j]));
	}

	auto forReturn = string(decData.begin(), decData.end());
	while (!forReturn.empty() && forReturn.ends_with('\0'))
		forReturn.pop_back();

	return gl_conv_string_to_wstring(forReturn);
}

block_t Kuznechik::increment(const block_t& block) {
	auto result = block;
	auto remain = false;

	do {
		for (size_t i = 0; i < block.size(); i++) {
			if (const BYTE b = result[i]; b < 255) {
				result[i]++;
				remain = false;
			} else {
				result[i] = 0;
				remain = true;
			}
		}
	} while (remain);

	return result;
}

block_t Kuznechik::lsx(const block_t& k, const block_t& a) const {
	const auto x = X(k, a);
	const auto sx = S(x);
	const auto plsx = L(sx);

	return plsx;
}

vector<key_pair> Kuznechik::getRoundKeys(const key_t& k) const {
	block_t c[key_size];
	constexpr unsigned char numKeyPairs = num_keys / 2;

	key_pair keys[numKeyPairs];
	for (unsigned char i = 1; i <= key_size; i++) {
		block_t vi;
		ranges::fill(vi, i);

		c[i - 1] = L(vi);
	}
	
	block_t k1;
	for (unsigned char i = key_size / 2, j = 0; i < key_size; i++, j++)
		k1[j] = k[i];

	block_t k2;
	for (unsigned char j = 0; j < key_size / 2; j++)
		k2[j] = k[j];

	keys[0][0] = k1;
	keys[0][1] = k2;

	for (unsigned char i = 0; i < numKeyPairs - 1; i++) {
		for (unsigned char j = 0; j < 8; j++) {
			const auto x = f(c[i + j], k1, k2);

			k1 = x[0];
			k2 = x[1];
		}

		keys[i + 1][0] = k1;
		keys[i + 1][1] = k2;
	}

	vector<key_pair> kps;
	for (const auto& key : keys) {
		kps.push_back(key);
	}

	return kps;
}

BYTE Kuznechik::gf_Mul(BYTE a, BYTE b) {
	uint8_t c = 0;

	for (unsigned char i = 0; i < 8; i++) {
		if (b & 1)
			c ^= a;

		const uint8_t hiBit = a & 0x80;
		a <<= 1;
		if (hiBit)
			a ^= 0xc3;

		b >>= 1;
	}
	return c;
}

block_t Kuznechik::X(const block_t& k, const block_t& a) {
	block_t x;
	for (unsigned char i = 0; i < block_size; i++)
		x[i] = k[i] ^ a[i];

	return x;
}

array<block_t, 2> Kuznechik::f(const block_t& k, const block_t& a_1, const block_t& a_0) const {
	block_t lsx = L(S(X(k, a_1)));
	for (unsigned char i = 0; i < block_size; i++)
		lsx[i] ^= a_0[i];

	return { lsx, a_1 };
}

block_t Kuznechik::S(const block_t& a) const {
	block_t s;
	for (unsigned char i = 0; i < block_size; i++)
		s[i] = pi[a[i]];
	
	return s;
}

block_t Kuznechik::S_inv(const block_t &a) const {
	block_t s_inv;
	for (unsigned char i = 0; i < block_size; i++)
		s_inv[i] = pi_inv[a[i]];
	
	return s_inv;
}

block_t Kuznechik::R(const block_t& a) const {
	block_t r;
	BYTE a_15 = 0;

	for (unsigned char i = 0; i < 15; i++)
		r[i] = a[i + 1];
	
	for (char i = 15, j = 0; i >= 0; i--, j++)
		a_15 ^= gf_Mul(a[i], l_vec[static_cast<unsigned char>(j)]);

	r[block_size - 1] = a_15;
	return r;
}

block_t Kuznechik::r_inv(const block_t& a) const {
	block_t r_Inv;

	for (unsigned char i = 0; i < 15; i++)
		r_Inv[i + 1] = a[i];

	BYTE a_0 = a[15];
	for (char i = 14, j = 0; i >= 0; i--, j++)
		a_0 ^= gf_Mul(a[i], l_vec[j]);
	
	r_Inv[0] = a_0;
	return r_Inv;
}

block_t Kuznechik::L(const block_t& a) const {
	block_t l = a;
	for (unsigned char i = 0; i < 16; i++)
		l = R(l);
	
	return l;
}

block_t Kuznechik::l_inv(const block_t& a) const {
	block_t l_Inv = a;
	for (unsigned char i = 0; i < 16; i++)
		l_Inv = r_inv(l_Inv);
	
	return l_Inv;
}
