#include "ROM.hpp"

#include <utility>

ROM::ROM(std::vector<uint8_t[BANK_SIZE]> banks) : banks(std::move(banks)) {}
