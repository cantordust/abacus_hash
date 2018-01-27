# What?

Abacus hash is a header-only template library for hashing custom structs (see below for limitations).

# Why?

 This is particualrly useful for creating custom hash functions for structs containing integral types, enums (and any other types convertible to integral types) so that they can be used as keys in hashmaps (such as `std::unordered_map`). The resulting map is _flat_ (custom key type: value type), unlike "deep" maps where you have to design a map of maps of maps ... in order to store your custom type.

# How?

**tl;dr** 
You have to specialise `std::hash` for your custom type. Inside the `std::hash` specialisation, return `tuple_hash(Args...)` with your desired types as arguments. An example is included in main.cpp, so you can model your code on that. All types passed to `tuple_hash` must be integral, `enum` or `enum class`.

 Consider the following `struct` which might serve as an ID for some more complex class:
	
	```
	enum class Enum1 : char
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

	struct ID
	{
		int idx;
		Enum1 e1;
		long record;
		Enum2 e2;

		friend bool operator == (const ID& lhs, const ID& rhs) {
			return lhs.idx == rhs.idx && lhs.e1 == rhs.e1 && lhs.record == rhs.record;
		}
	};

	```

You can't store that as a _key_ (not a value!) in `std::unordered_map` because there is no specialisation for `std::hash`. So you have to either (`hmap` means `std::unordered_map`)

- create `hmap<int, hmap<Enum1, hmap<long, hmap<Enum2, value> >> > map` (how unwieldy is that!?); or
- specialise std::hash for your custom type `ID` in order to be able to use ID as a key (in other words, `hmap<ID, value> map`).

The downside of the first approach (besides the horrible type signature) is that the map is not "flat", in other words, you get a hierarchical representation of `ID`. This might be a good thing, but it also might not be what you want. What if you want to iterate over your map and search for a particular ID in one go? You can't do that - you'd need four levels of `for` loops to iterate over a deep map. Not good!

This leaves the second option. Now, this is where abacus hash comes into play. Basically, it exploits the fact that there already exists a specialisation of `std::hash` for `std::bitset`. The idea is to generate a bitset of the right size and store the desired entities from our `struct` in a way that guarantees that the resulting bitset is unique.

Abacus hash stores all provided types in their own "slots" in a long `std::bitset`. Each integral element is converted into a temporary bitset which is shifted to the left by `n` bits (where `n` is the number of bits in all elements stored before the current one), after which the temporary bitset is `or`ed with the main bitset. Finally, the function returns the hash of the main bitset. In the case above, we want to use the combination of all four types (`int`, `Enum1`, `long` and `Enum2`) as a unique key (people familiar with database design would appreciate the analogy). Here, we take the main bitset to be of size `8 * (sizeof(int) + sizeof(Enum1) + sizeof(long) + sizeof(Enum2))`. On my machine this is 168, but it might be different on other machines. The point is that you don't have to think about that. As long as the elements are integral (or can be converted to integral, and all `enum class` types can), you can use abacus hash to compute a unique hash for your type. Here is what you have to do to enable that.

0. `#include "AbacusHash.hpp" (doh!)

1. Add a custom `operator ==` to your `struct`:


	```
	
	struct ID
	{
		int idx;
		Enum1 e1;
		long record;
		Enum2 e2;

		/// Like so
		friend bool operator == (const ID& lhs, const ID& rhs) {
			return lhs.idx == rhs.idx && lhs.e1 == rhs.e1 && lhs.record == rhs.record;
		}
	};

	```

2. Add a specialisation for `std::hash<ID>` (Note: this cannot be inside any other 	`namespace`!):


	```
	/// Like so
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
		
	```

That's it! You can now create hashmaps such as `std::unordered_map<ID, MyAwesomeClass> map;`.

## Features

Besides the obvious convenience, for some reason a flat hashmap using abacus hash is 2~3 times __faster_ than an equivalent "deep" `std::unordered_map`. This was an unexpected but highly welcome side effect. Please test (your mileage may vary).

## Usage

This is a header-only library, so no need to link against it. Just download the AbacusHash.hpp file and `#include` it. If you want to run the example, 

```shell
$ cd ~/abacus_hash
$ mkdir build && cd build && cmake .. && make
$ cd ../bin
$ ./abacus_hash
```
