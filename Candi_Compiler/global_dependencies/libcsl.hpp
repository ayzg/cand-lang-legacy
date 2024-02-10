#pragma once
#include "libstd_types.hpp"


namespace caoco {

	namespace sl {
		// Converts a string_t utf 8 string to a std::string char string
		SL_CXIN sl_string to_str(const sl_u8string& str) {
			return sl_string(str.begin(), str.end());
		}

		// Converts a string_t utf 8 string to a vector of chars
		SL_CXIN sl_char8_vector to_char8_vector(const sl_u8string& str) {
			return sl_char8_vector(str.begin(), str.end());
		}

		// Converts a regular string literal to a utf 8 char vector
		SL_CXIN sl_char8_vector to_char8_vector(const char* str) {
			return sl_char8_vector(str, str + sl_char_traits<char>::length(str));
		}

		// Convert u8 string to a vector of char_8t
		sl_vector<char8_t> to_u8vec(const char8_t* str) {
			sl_vector<char8_t> vec;
			for (int i = 0; str[i] != '\0'; i++) {
				vec.push_back(str[i]);
			}
			return vec;
		}


		// Loads a file into a vector of chars
		sl_char8_vector load_file_to_char8_vector(sl_string name) {


			std::ifstream ifs(name, std::ios::binary | std::ios::ate);

			if (!ifs)
				throw std::runtime_error(name + ": " + std::strerror(errno));

			auto end = ifs.tellg();
			ifs.seekg(0, std::ios::beg);

			auto size = std::size_t(end - ifs.tellg());

			if (size == 0) // avoid undefined behavior 
				return {};

			std::vector<std::byte> buffer(size);

			if (!ifs.read((char*)buffer.data(), buffer.size()))
				throw std::runtime_error(name + ": " + std::strerror(errno));

			// Convert the vector of chars to a vector of char_t
			sl_char8_vector chars;
			chars.reserve(buffer.size());
			for (auto& c : buffer) {
				chars.push_back(static_cast<char8_t>(c));
			}

			// if the last char is not a null terminator, add one
			if (chars.back() != '\0')
				chars.push_back('\0');

			return chars;
		}

	}
}