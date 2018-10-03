#include "abacus_hash.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <random>
#include <chrono>
#include <functional>
#include <memory>

using uchar = unsigned char;
using uint = unsigned int;
template<typename T1, typename T2> using hmap = std::unordered_map<T1, T2>;
template<typename T> using sptr = std::shared_ptr<T>;

struct EnumHash
{
	template<typename E, typename std::enable_if<std::is_enum<E>::value, void>::type ... >
	auto operator() (E _e) const
	{
		return AbacusHash::Internal::utype(_e);
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

	ID(const uint _idx,
	   const Enum1 _e1,
	   const uint _record,
	   const Enum2 _e2)
		:
		  idx(_idx),
		  e1(_e1),
		  record(_record),
		  e2(_e2)
	{}

	friend bool operator == (const ID& lhs, const ID& rhs)
	{
		return lhs.idx == rhs.idx && lhs.e1 == rhs.e1 && lhs.record == rhs.record;
	}

	friend std::ostream& operator << (std::ostream& _strm, const ID& _id)
	{
		return _strm  << "{" << _id.idx << ", " << to_str(_id.e1) << ", " << _id.record << ", " << to_str(_id.e2) << "}";
	}
};

using IDRef = std::reference_wrapper<ID>;
using IDPtr = std::shared_ptr<ID>;

namespace std
{
	template <>
	struct hash<ID>
	{
		size_t operator()(const ID& _id) const noexcept
		{
			return AbacusHash::hash(_id.idx, _id.e1, _id.record, _id.e2);
		}
	};

	template <>
	struct hash<IDRef>
	{
		size_t operator()(const IDRef& _idref) const noexcept
		{
			return AbacusHash::hash(_idref.get().idx, _idref.get().e1, _idref.get().record, _idref.get().e2);
		}
	};
}

std::mt19937_64 rng;
std::uniform_int_distribution<uint> d_idx(0, std::numeric_limits<uint>::max());
std::uniform_int_distribution<uint> d_enum1(0, 3);
std::uniform_int_distribution<uint> d_record(0, std::numeric_limits<uint>::max());
std::uniform_int_distribution<uint> d_enum2(0, 4);

hmap<IDPtr, uint> sptr_map;
hmap<IDRef, uint> flat_map;
hmap<uint, emap<Enum1, hmap<uint, emap<Enum2, uint>>>> deep_map;

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

inline auto now()
{
	return std::chrono::steady_clock::now();
//	return std::chrono::high_resolution_clock::now();
}

int main()
{
	rng.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());

	uint elements(1000000);
	std::vector<IDPtr> ids;

	for (uint i	= 0; i < elements; ++i)
	{
		ids.push_back(std::make_shared<ID>(get_idx(), get_enum1(), get_record(), get_enum2()));
	}

	auto start(now());

	for (auto& id : ids)
	{
		flat_map[std::ref(*id)] = id->idx;
	}

	auto duration(now() - start);

	std::cout << "flat_map size: " << flat_map.size()
			  << "\n\ttook " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms\n";

	start = now();

	for (const auto& id : ids)
	{
		deep_map[id->idx][id->e1][id->record][id->e2] = id->idx;
	}

	duration = now() - start;

	std::cout << "deep_map size: " << deep_map.size()
			  << "\n\ttook " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms\n";


	start = now();

	for (const auto& id : ids)
	{
		sptr_map[id] = id->idx;
	}

	duration = now() - start;

	std::cout << "sptr_map size: " << sptr_map.size()
			  << "\n\ttook " << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() << " ms\n";

//	std::cout << "Flat map:\n";
//	for (const auto& m : flat_map)
//	{
//		std::cout << m.first << ": " << m.second << "\n";
//	}

//	std::cout << "\nSptr map:\n";
//	for (const auto& s : sptr_map)
//	{
//		std::cout << *s.first << ": " << s.second << "\n";
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

	std::cout << "\nChanging ID at ids.back() to 5...\n";
	ids.back()->idx = 5;

	/// Important: rehash to avoid dangling references
	flat_map.rehash(0);

//	std::cout << "\nFlat map:\n";
//	for (const auto& m : flat_map)
//	{
//		std::cout << m.first << ": " << m.second << "\n";
//	}

//	std::cout << "\nID with idx 5:\n";
//	for (const auto& m : flat_map)
//	{
//		if (m.first.get().idx == 5)
//		{
//			std::cout << m.first << ": " << m.second << "\n";
//		}
//	}

	std::cout << "\nErasing ref to last ID from flat map...\n";
	flat_map.erase(std::ref(*ids.back()));

//	std::cout << "\nFlat map:\n";
//	for (const auto& m : flat_map)
//	{
//		std::cout << m.first << ": " << m.second << "\n";
//	}

//	std::cout << "\nID vector size: " << ids.size() << "\n"
//			  << "\nFlat map size: " << flat_map.size() << "\n"
//			  << "\nDeep map size: " << deep_map.size() << "\n"
//			  << "\nSptr map size: " << sptr_map.size() << "\n";

//	std::cout << "\nSptr map:\n";
//	for (const auto& id : sptr_map)
//	{
//		std::cout << "(use count: " << id.first.use_count() << ") " << *id.first << ": " << id.second << "\n";
//	}

	std::cout << "\nErasing sptr to last ID from sptr map...\n";

	sptr_map.erase(ids.back());

//	std::cout << "\nSptr map:\n";
//	for (const auto& id : sptr_map)
//	{
//		std::cout << *id.first << ": " << id.second << "\n";
//	}

	return 0;
}
