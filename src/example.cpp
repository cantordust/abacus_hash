#include "abacus_hash.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <random>
#include <chrono>

using uchar = unsigned char;
using uint = unsigned int;
template<typename T1, typename T2> using hmap = std::unordered_map<T1, T2>;

using namespace AbacusHash;

struct EnumHash
{
	template<typename E, typename std::enable_if<std::is_enum<E>::value, void>::type ... >
	auto operator() (E _e) const
	{
		return utype(_e);
	}
};

template<typename T1, typename T2, typename std::enable_if<std::is_enum<T1>::value, void>::type ...>
using emap = std::unordered_map<T1, T2, EnumHash>;

enum class Enum1 : uchar
{
	Zero,
	One,
	Two,
	Three
};

enum class Enum2 : long
{
	A,
	B,
	C,
	D,
	E
};

std::string to_str(const Enum1 _e)
{
	switch(_e)
	{
	case Enum1::Zero : return "Enum1::Zero";
	case Enum1::One : return "Enum1::One";
	case Enum1::Two : return "Enum1::Two";
	case Enum1::Three : return "Enum1::Three";
	default: return "";
	}
}

std::string to_str(const Enum2 _e)
{
	switch(_e)
	{
	case Enum2::A : return "Enum2::A";
	case Enum2::B : return "Enum2::B";
	case Enum2::C : return "Enum2::C";
	case Enum2::D : return "Enum2::D";
	case Enum2::E : return "Enum2::E";
	default: return "";
	}
}

struct ID
{
	uint idx;
	Enum1 e1;
	uint record;
	Enum2 e2;

	friend bool operator == (const ID& lhs, const ID& rhs)
	{
		return lhs.idx == rhs.idx && lhs.e1 == rhs.e1 && lhs.record == rhs.record;
	}

	friend std::ostream& operator << (std::ostream& _strm, const ID& _id)
	{
		return _strm  << "{" << _id.idx << ", " << to_str(_id.e1) << ", " << _id.record << ", " << to_str(_id.e2) << "}";
	}
};

namespace std
{
	template <>
	struct hash<ID>
	{
		size_t operator()(const ID& _t) const noexcept
		{
			return tuple_hash(std::make_tuple(_t.idx, _t.e1, _t.record, _t.e2));
		}
	};
}

static std::mt19937_64 rng;
static std::uniform_int_distribution<uint> d_idx(0, std::numeric_limits<uint>::max());
static std::uniform_int_distribution<uint> d_enum1(0, 3);
static std::uniform_int_distribution<uint> d_record(0, std::numeric_limits<uint>::max());
static std::uniform_int_distribution<uint> d_enum2(0, 4);

static hmap<ID, uint> flat_map;
static hmap<uint, emap<Enum1, hmap<uint, emap<Enum2, uint>>>> deep_map;

uint get_idx()
{
	return d_idx(rng);
}

Enum1 get_enum1()
{
	switch(d_enum1(rng))
	{
	case 0 : return Enum1::Zero;
	case 1 : return Enum1::One;
	case 2 : return Enum1::Two;
	case 3 : return Enum1::Three;
	default: return Enum1::Zero;
	}
}

uint get_record()
{
	return d_record(rng);
}

Enum2 get_enum2()
{
	switch(d_enum2(rng))
	{
	case 0 : return Enum2::A;
	case 1 : return Enum2::B;
	case 2 : return Enum2::C;
	case 3 : return Enum2::D;
	case 4 : return Enum2::E;
	default: return Enum2::A;
	}
}

static inline auto now()
{
	return std::chrono::steady_clock::now();
//	return std::chrono::high_resolution_clock::now();
}

int main()
{
	rng.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());

	std::cout << "sizeof(ID): " << sizeof(ID) << "\n";
	std::cout << "sizeof(Enum1): " << sizeof(Enum1) << "\n";
	std::cout << "sizeof(uint): " << sizeof(uint) << "\n";
	std::cout << "sizeof(long): " << sizeof(long) << "\n";
	std::cout << "sizeof(hmap<uint,uint>): " << sizeof(hmap<uint, uint>) << "\n";
	std::cout << "sizeof(flat_map): " << sizeof(decltype (flat_map)) << "\n";
	std::cout << "sizeof(deep_map): " << sizeof(decltype (deep_map)) << "\n";

	uint iter(100000);
	std::vector<ID> ids;

	for (uint i	= 0; i < iter; ++i)
	{
		ids.push_back({get_idx(), get_enum1(), get_record(), get_enum2()});
	}

	auto start(now());

	for (const auto& id : ids)
	{
		flat_map[id] = id.idx;
	}

	auto duration(now() - start);

	std::cout << "flat_map size: " << flat_map.size()
			  << "\n\ttook " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms\n";

	start = now();

	for (const auto& id : ids)
	{
		deep_map[id.idx][id.e1][id.record][id.e2] = id.idx;
	}

	duration = now() - start;

	std::cout << "deep_map size: " << deep_map.size()
			  << "\n\ttook " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms\n";

//	std::cout << "Flat map:\n";
//	for (const auto& m : flat_map)
//	{
//		std::cout << m.first << ": " << m.second << "\n";
//	}

//	std::cout << "\nDeep map:\n";
//	for (const auto& idx : deep_map)
//	{
//		std::cout << "{" << idx.first << ", ";
//		for (const auto& e1 : idx.second)
//		{
//			std::cout << to_str(e1.first) << ", ";
//			for (const auto& rec : e1.second)
//			{
//				std::cout << rec.first << ", ";
//				for (const auto& e2 : rec.second)
//				{
//					std::cout << to_str(e2.first) << "}: " << e2.second << "\n";
//				}
//			}
//		}
//	}

	return 0;
}
