# Overview

## What?

Abacus hash is a header-only template library for hashing composite types such as `struct`s, `class`es, `tuple`s and so on (see below for limitations).

## Why?

Being able to hash composite types allows storing such types as keys in hashmaps (such as `std::unordered_map`). The resulting map is _flat_ (meaning there is only one level of hierarchy `custom key type`: `value type`), unlike "deep" maps where you would have to use a map of maps of maps ...

## How?

### tl;dr 
You have to specialise `std::hash` for your composite type. Inside the `std::hash` specialisation, return `tuple_hash(Args...)` with your desired types as arguments. An example is included in main.cpp, so you can model your code on that. All types passed to `tuple_hash` must be integral or `enum class`.

### Details

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
};

```

You can't store that as a _key_ in `std::unordered_map` because there is no specialisation for `std::hash<ID>`. So you have to either (`hmap` means `std::unordered_map`)

- create `hmap<int, hmap<Enum1, hmap<long, hmap<Enum2, value>>>> map` (how unwieldy is that!?); or
- specialise std::hash for your custom type `ID` in order to be able to use `ID` as a key (in other words, `hmap<ID, value> map`).

The downside of the first approach (besides the horrible type signature) is that the map is not "flat", in other words, you get a hierarchical representation of `ID`. This might be useful in some cases, but it defeats the purpose of using `ID` as a unique key. What if you want to iterate over your map and search for a particular ID in one go? To do that, you need four levels of `for` loops to iterate over a deep map. Not good!

This leaves the second option. Now, this is where abacus hash comes into play. Basically, it exploits the fact that there already exists a specialisation of `std::hash` for `std::bitset`. The idea is to generate a bitset of the right size and store the desired entities from our `struct` in a way that guarantees that the resulting bitset is unique.

Abacus hash stores the elements (integral and `enum class` types) of the custom type in their own "slots" in a `std::bitset`. Each element is converted into a temporary bitset which is shifted to the left by `n` bits, where `n` is the number of bits in all elements stored before the current one (like an abacus!), after which the temporary bitset is `or`ed with the main bitset. Finally, the hash of the main bitset is computed using `std::hash<std::bitset>`. 

In the case of `ID` above, we want to use the combination of all four members (of types `int`, `Enum1`, `long` and `Enum2`) as a unique key (people familiar with database design would appreciate the analogy). So the hashed bitset should be of size `8 * (sizeof(int) + sizeof(Enum1) + sizeof(long) + sizeof(Enum2))`.

# Features

Besides the obvious convenience, for some reason a flat hashmap using abacus hash is about 2 times _faster_ than an equivalent "deep" `std::unordered_map`. This was an unexpected but highly welcome side effect. Please test (your mileage may vary).

# Usage

This is a header-only library, just download abacus_ash.hpp and `#include` it. You have to add the `std::hash` specialisation:

0. `#include "abacus_hash.hpp"` (doh!)

1. Add a custom `operator ==` to your `struct` or `class`:

```
struct ID
{
	int idx;
	Enum1 e1;
	long record;
	Enum2 e2;

	/// Like so
	friend bool operator == (const ID& lhs, const ID& rhs) 
	{
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

## Running the example

```shell
$ cd ~/abacus_hash
$ mkdir build && cd build && cmake .. && make
$ cd ../bin
$ ./abacus_hash
```
