#pragma once

#include <cstdint>

#include <GameDefinitions/BaseUtilities.h>
#include <GameDefinitions/BaseMemory.h>

namespace bg3se
{
	unsigned int GetNearestLowerPrime(unsigned int num);

	template <class T>
	struct ContiguousIterator
	{
		T* Ptr;

		ContiguousIterator(T* p) : Ptr(p) {}

		ContiguousIterator operator ++ ()
		{
			ContiguousIterator<T> it(Ptr);
			Ptr++;
			return it;
		}

		ContiguousIterator& operator ++ (int)
		{
			Ptr++;
			return *this;
		}

		bool operator == (ContiguousIterator const& it)
		{
			return it.Ptr == Ptr;
		}

		bool operator != (ContiguousIterator const& it)
		{
			return it.Ptr != Ptr;
		}

		T& operator * ()
		{
			return *Ptr;
		}

		T* operator -> ()
		{
			return Ptr;
		}
	};


	template <class T>
	struct ContiguousConstIterator
	{
		T const* Ptr;

		ContiguousConstIterator(T const* p) : Ptr(p) {}

		ContiguousConstIterator operator ++ ()
		{
			ContiguousConstIterator<T> it(Ptr);
			Ptr++;
			return it;
		}

		ContiguousConstIterator& operator ++ (int)
		{
			Ptr++;
			return *this;
		}

		bool operator == (ContiguousConstIterator const& it)
		{
			return it.Ptr == Ptr;
		}

		bool operator != (ContiguousConstIterator const& it)
		{
			return it.Ptr != Ptr;
		}

		T const& operator * ()
		{
			return *Ptr;
		}

		T const* operator -> ()
		{
			return Ptr;
		}
	};


	template <class TKey, class TValue>
	class Map : public Noncopyable<Map<TKey, TValue>>
	{
	public:
		struct Node
		{
			Node* Next{ nullptr };
			TKey Key;
			TValue Value;
		};

		class Iterator
		{
		public:
			Iterator(Map& map) 
				: Node(map.HashTable), NodeListEnd(map.HashTable + map.HashSize), Element(nullptr)
			{
				while (Node < NodeListEnd && *Node == nullptr) {
					Node++;
				}

				if (Node < NodeListEnd && *Node) {
					Element = *Node;
				}
			}
			
			Iterator(Map& map, Node** node, Node* element)
				: Node(node), NodeListEnd(map.HashTable + map.HashSize), Element(element)
			{}

			Iterator operator ++ ()
			{
				Iterator it(*this);

				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return it;
			}

			Iterator& operator ++ (int)
			{
				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return *this;
			}

			bool operator == (Iterator const& it)
			{
				return it.Node == Node && it.Element == Element;
			}

			bool operator != (Iterator const& it)
			{
				return it.Node != Node || it.Element != Element;
			}

			TKey & Key () const
			{
				return Element->Key;
			}

			TKey & Value () const
			{
				return Element->Value;
			}

			Node& operator * () const
			{
				return *Element;
			}

			Node& operator -> () const
			{
				return *Element;
			}

		private:
			Node** Node, ** NodeListEnd;
			Map<TKey, TValue>::Node* Element;
		};

		class ConstIterator
		{
		public:
			ConstIterator(Map const& map)
				: Node(map.HashTable), NodeListEnd(map.HashTable + map.HashSize), Element(nullptr)
			{
				while (Node < NodeListEnd && *Node == nullptr) {
					Node++;
				}

				if (Node < NodeListEnd && *Node) {
					Element = *Node;
				}
			}

			ConstIterator(Map const& map, Node* const* node, Node const* element)
				: Node(node), NodeListEnd(map.HashTable + map.HashSize), Element(element)
			{}

			ConstIterator operator ++ ()
			{
				Iterator it(*this);

				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return it;
			}

			ConstIterator& operator ++ (int)
			{
				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return *this;
			}

			bool operator == (Iterator const& it)
			{
				return it.Node == Node && it.Element == Element;
			}

			bool operator != (Iterator const& it)
			{
				return it.Node != Node || it.Element != Element;
			}

			TKey const& Key() const
			{
				return Element->Key;
			}

			TKey const& Value() const
			{
				return Element->Value;
			}

			Node const& operator * () const
			{
				return *Element;
			}

			Node const& operator -> () const
			{
				return *Element;
			}

		private:
			Node* const * Node, * const * NodeListEnd;
			Map<TKey, TValue>::Node const* Element;
		};

		Map() {}

		Map(uint32_t hashSize)
		{
			Init(hashSize);
		}

		~Map()
		{
			Clear();
		}

		void Init(uint32_t hashSize)
		{
			HashSize = hashSize;
			HashTable = GameAllocArray<Node*>(hashSize);
			ItemCount = 0;
			memset(HashTable, 0, sizeof(Node*) * hashSize);
		}

		void Clear()
		{
			ItemCount = 0;
			for (uint32_t i = 0; i < HashSize; i++) {
				auto item = HashTable[i];
				if (item != nullptr) {
					FreeHashChain(item);
					HashTable[i] = nullptr;
				}
			}
		}

		void FreeHashChain(Node* node)
		{
			do {
				auto next = node->Next;
				GameDelete(node);
				node = next;
			} while (node != nullptr);
		}

		TValue* Insert(TKey const& key, TValue const& value)
		{
			auto nodeValue = Insert(key);
			*nodeValue = value;
			return nodeValue;
		}

		TValue* Insert(TKey const& key)
		{
			auto item = HashTable[Hash(key) % HashSize];
			auto last = item;
			while (item != nullptr) {
				if (key == item->Key) {
					return &item->Value;
				}

				last = item;
				item = item->Next;
			}

			auto node = GameAlloc<Node>();
			node->Next = nullptr;
			node->Key = key;

			if (last == nullptr) {
				HashTable[Hash(key) % HashSize] = node;
			}
			else {
				last->Next = node;
			}

			ItemCount++;
			return &node->Value;
		}

		TValue* Find(TKey const& key) const
		{
			auto item = HashTable[Hash(key) % HashSize];
			while (item != nullptr) {
				if (key == item->Key) {
					return &item->Value;
				}

				item = item->Next;
			}

			return nullptr;
		}

		TKey* FindByValue(TValue const& value) const
		{
			for (uint32_t bucket = 0; bucket < HashSize; bucket++) {
				Node* item = HashTable[bucket];
				while (item != nullptr) {
					if (value == item->Value) {
						return &item->Key;
					}

					item = item->Next;
				}
			}

			return nullptr;
		}

		template <class Visitor>
		void Iterate(Visitor visitor)
		{
			for (uint32_t bucket = 0; bucket < HashSize; bucket++) {
				Node* item = HashTable[bucket];
				while (item != nullptr) {
					visitor(item->Key, item->Value);
					item = item->Next;
				}
			}
		}

		template <class Visitor>
		void Iterate(Visitor visitor) const
		{
			for (uint32_t bucket = 0; bucket < HashSize; bucket++) {
				Node* item = HashTable[bucket];
				while (item != nullptr) {
					visitor(item->Key, item->Value);
					item = item->Next;
				}
			}
		}

		Iterator begin()
		{
			return Iterator(*this);
		}

		Iterator end()
		{
			return Iterator(*this, HashTable + HashSize, nullptr);
		}

		Iterator begin() const
		{
			return ConstIterator(*this);
		}

		Iterator end() const
		{
			return ConstIterator(*this, HashTable + HashSize, nullptr);
		}

		inline uint32_t Count() const
		{
			return ItemCount;
		}

	private:
		uint32_t HashSize{ 0 };
		Node** HashTable{ nullptr };
		uint32_t ItemCount{ 0 };
	};

	template <class TKey, class TValue>
	class RefMap : public Noncopyable<RefMap<TKey, TValue>>
	{
	public:
		struct Node
		{
			Node* Next{ nullptr };
			TKey Key;
			TValue Value;
		};

		class Iterator
		{
		public:
			Iterator(RefMap& map) 
				: Node(map.HashTable), NodeListEnd(map.HashTable + map.HashSize), Element(nullptr)
			{
				while (Node < NodeListEnd && *Node == nullptr) {
					Node++;
				}

				if (Node < NodeListEnd && *Node) {
					Element = *Node;
				}
			}
			
			Iterator(RefMap& map, Node** node, Node* element)
				: Node(node), NodeListEnd(map.HashTable + map.HashSize), Element(element)
			{}

			Iterator operator ++ ()
			{
				Iterator it(*this);

				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return it;
			}

			Iterator& operator ++ (int)
			{
				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return *this;
			}

			bool operator == (Iterator const& it)
			{
				return it.Node == Node && it.Element == Element;
			}

			bool operator != (Iterator const& it)
			{
				return it.Node != Node || it.Element != Element;
			}

			TKey & Key () const
			{
				return Element->Key;
			}

			TKey & Value () const
			{
				return Element->Value;
			}

			Node& operator * () const
			{
				return *Element;
			}

			Node& operator -> () const
			{
				return *Element;
			}

		private:
			Node** Node, ** NodeListEnd;
			RefMap<TKey, TValue>::Node* Element;
		};

		class ConstIterator
		{
		public:
			ConstIterator(RefMap const& map)
				: Node(map.HashTable), NodeListEnd(map.HashTable + map.HashSize), Element(nullptr)
			{
				while (Node < NodeListEnd && *Node == nullptr) {
					Node++;
				}

				if (Node < NodeListEnd && *Node) {
					Element = *Node;
				}
			}

			ConstIterator(RefMap const& map, Node* const* node, Node const* element)
				: Node(node), NodeListEnd(map.HashTable + map.HashSize), Element(element)
			{}

			ConstIterator operator ++ ()
			{
				Iterator it(*this);

				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return it;
			}

			ConstIterator& operator ++ (int)
			{
				Element = Element->Next;
				if (Element == nullptr) {
					do {
						Node++;
					} while (Node < NodeListEnd && *Node == nullptr);

					if (Node < NodeListEnd && *Node) {
						Element = *Node;
					}
				}

				return *this;
			}

			bool operator == (Iterator const& it)
			{
				return it.Node == Node && it.Element == Element;
			}

			bool operator != (Iterator const& it)
			{
				return it.Node != Node || it.Element != Element;
			}

			TKey const& Key() const
			{
				return Element->Key;
			}

			TKey const& Value() const
			{
				return Element->Value;
			}

			Node const& operator * () const
			{
				return *Element;
			}

			Node const& operator -> () const
			{
				return *Element;
			}

		private:
			Node* const * Node, * const * NodeListEnd;
			RefMap<TKey, TValue>::Node const* Element;
		};

		RefMap(uint32_t hashSize = 31)
			: ItemCount(0), HashSize(hashSize)
		{
			HashTable = GameAllocArray<Node*>(hashSize);
			memset(HashTable, 0, sizeof(Node*) * hashSize);
		}

		~RefMap()
		{
			if (HashTable != nullptr) {
				GameFree(HashTable);
			}
		}

		Iterator begin()
		{
			return Iterator(*this);
		}

		Iterator end()
		{
			return Iterator(*this, HashTable + HashSize, nullptr);
		}

		Iterator begin() const
		{
			return ConstIterator(*this);
		}

		Iterator end() const
		{
			return ConstIterator(*this, HashTable + HashSize, nullptr);
		}

		inline uint32_t Count() const
		{
			return ItemCount;
		}

		void Clear()
		{
			ItemCount = 0;
			for (uint32_t i = 0; i < HashSize; i++) {
				auto item = HashTable[i];
				if (item != nullptr) {
					FreeHashChain(item);
					HashTable[i] = nullptr;
				}
			}
		}

		void FreeHashChain(Node* node)
		{
			do {
				auto next = node->Next;
				GameDelete(node);
				node = next;
			} while (node != nullptr);
		}

		TValue* Find(TKey const& key) const
		{
			auto item = HashTable[Hash(key) % HashSize];
			while (item != nullptr) {
				if (key == item->Key) {
					return &item->Value;
				}

				item = item->Next;
			}

			return nullptr;
		}

		TValue* Insert(TKey const& key, TValue const& value)
		{
			auto nodeValue = Insert(key);
			*nodeValue = value;
			return nodeValue;
		}

		TValue* Insert(TKey const& key)
		{
			auto item = HashTable[Hash(key) % HashSize];
			auto last = item;
			while (item != nullptr) {
				if (key == item->Key) {
					return &item->Value;
				}

				last = item;
				item = item->Next;
			}

			auto node = GameAlloc<Node>();
			node->Next = nullptr;
			node->Key = key;

			if (last == nullptr) {
				HashTable[Hash(key) % HashSize] = node;
			}
			else {
				last->Next = node;
			}

			ItemCount++;
			return &node->Value;
		}

		template <class Visitor>
		void Iterate(Visitor visitor)
		{
			for (uint32_t bucket = 0; bucket < HashSize; bucket++) {
				Node* item = HashTable[bucket];
				while (item != nullptr) {
					visitor(item->Key, item->Value);
					item = item->Next;
				}
			}
		}

	private:
		uint32_t ItemCount{ 0 };
		uint32_t HashSize{ 0 };
		Node** HashTable{ nullptr };
	};


	template <class T, class Allocator = GameMemoryAllocator, bool StoreSize = false>
	struct CompactSet
	{
		T* Buf{ nullptr };
		uint32_t Capacity{ 0 };
		uint32_t Size{ 0 };

		inline CompactSet() {}

		CompactSet(CompactSet const& other)
		{
			Reallocate(other.Size);
			Size = other.Size;
			for (uint32_t i = 0; i < other.Size; i++) {
				Buf[i] = other.Buf[i];
			}
		}

		~CompactSet()
		{
			if (Buf) {
				for (uint32_t i = 0; i < Size; i++) {
					Buf[i].~T();
				}

				FreeBuffer(Buf);
			}
		}

		CompactSet& operator = (CompactSet const& other)
		{
			Reallocate(other.Size);
			Size = other.Size;
			for (uint32_t i = 0; i < other.Size; i++) {
				Buf[i] = other.Buf[i];
			}
			return *this;
		}

		inline T const& operator [] (uint32_t index) const
		{
			return Buf[index];
		}

		inline T& operator [] (uint32_t index)
		{
			return Buf[index];
		}

		void FreeBuffer(void* buf)
		{
			if (StoreSize) {
				if (buf != nullptr) {
					Allocator::Free((void*)((std::ptrdiff_t)buf - 8));
				}
			}
			else {
				if (buf != nullptr) {
					Allocator::Free(buf);
				}
			}
		}

		void RawReallocate(uint32_t newCapacity)
		{
			if (newCapacity > 0) {
				if (StoreSize) {
					auto newBuf = Allocator::Alloc(newCapacity * sizeof(T) + 8);
					*(uint64_t*)newBuf = newCapacity;

					Buf = (T*)((std::ptrdiff_t)newBuf + 8);
					for (uint32_t i = 0; i < newCapacity; i++) {
						new (Buf + i) T();
					}
				}
				else {
					Buf = Allocator::New<T>(newCapacity);
				}
			}
			else {
				Buf = nullptr;
			}

			Capacity = newCapacity;
		}

		void Reallocate(uint32_t newCapacity)
		{
			auto oldBuf = Buf;
			RawReallocate(newCapacity);
			for (uint32_t i = 0; i < std::min(Size, newCapacity); i++) {
				Buf[i] = oldBuf[i];
			}
			FreeBuffer(oldBuf);
		}

		void Remove(uint32_t index)
		{
			if (index >= Size) {
				ERR("Tried to remove out-of-bounds index %d!", index);
				return;
			}

			for (auto i = index; i < Size - 1; i++) {
				Buf[i] = Buf[i + 1];
			}

			Size--;
		}

		void Clear()
		{
			Size = 0;
		}

		ContiguousIterator<T> begin()
		{
			return ContiguousIterator<T>(Buf);
		}

		ContiguousConstIterator<T> begin() const
		{
			return ContiguousConstIterator<T>(Buf);
		}

		ContiguousIterator<T> end()
		{
			return ContiguousIterator<T>(Buf + Size);
		}

		ContiguousConstIterator<T> end() const
		{
			return ContiguousConstIterator<T>(Buf + Size);
		}
	};

	template <class T, class Allocator = GameMemoryAllocator, bool StoreSize = false>
	struct Set : public CompactSet<T, Allocator, StoreSize>
	{
		uint64_t CapacityIncrementSize{ 0 };

		uint32_t CapacityIncrement() const
		{
			if (CapacityIncrementSize != 0) {
				return Capacity + (uint32_t)CapacityIncrementSize;
			}
			else if (Capacity > 0) {
				return 2 * Capacity;
			}
			else {
				return 1;
			}
		}

		void Add(T const& value)
		{
			if (Capacity <= Size) {
				Reallocate(CapacityIncrement());
			}

			Buf[Size++] = value;
		}

		void InsertAt(uint32_t index, T const& value)
		{
			if (Capacity <= Size) {
				Reallocate(CapacityIncrement());
			}

			for (auto i = Size; i > index; i--) {
				Buf[i] = Buf[i - 1];
			}

			Buf[index] = value;
			Size++;
		}
	};

	template <class T, class Allocator = GameMemoryAllocator>
	struct PrimitiveSmallSet : public CompactSet<T, Allocator, false>
	{
		virtual ~PrimitiveSmallSet() {}

		uint32_t CapacityIncrement() const
		{
			if (Capacity > 0) {
				return 2 * Capacity;
			}
			else {
				return 1;
			}
		}

		void Add(T const& value)
		{
			if (Capacity <= Size) {
				Reallocate(CapacityIncrement());
			}

			Buf[Size++] = value;
		}
	};

	template <class T, class Allocator = GameMemoryAllocator, bool StoreSize = false>
	struct ObjectSet : public Set<T, Allocator, StoreSize>
	{
	};

	template <class T, class Allocator = GameMemoryAllocator>
	struct PrimitiveSet : public ObjectSet<T, Allocator, false>
	{
	};

	template <unsigned TDWords>
	struct BitArray
	{
		uint32_t Bits[TDWords];

		inline bool Set(uint32_t index)
		{
			if (index <= 0 || index > (TDWords * 32)) {
				return false;
			}

			Bits[(index - 1) >> 5] |= (1 << ((index - 1) & 0x1f));
			return true;
		}

		inline bool Clear(uint32_t index)
		{
			if (index <= 0 || index > (TDWords * 32)) {
				return false;
			}

			Bits[(index - 1) >> 5] &= ~(1 << ((index - 1) & 0x1f));
			return true;
		}

		inline bool IsSet(uint32_t index) const
		{
			if (index <= 0 || index > (TDWords * 32)) {
				return false;
			}

			return (Bits[(index - 1) >> 5] & (1 << ((index - 1) & 0x1f))) != 0;
		}
	};

	template <class T>
	struct Array
	{
		T* Buf{ nullptr };
		unsigned int Capacity{ 0 };
		unsigned int Unknown{ 0 };
		unsigned int Size{ 0 };
		unsigned int Unknown2{ 0 };

		inline Array() {}

		Array(Array const& a)
		{
			CopyFrom(a);
		}

		Array& operator =(Array const& a)
		{
			CopyFrom(a);
			return *this;
		}

		void CopyFrom(Array const& a)
		{
			Unknown = a.Unknown;
			Unknown2 = a.Unknown2;
			Clear();

			if (a.Size > 0) {
				Reallocate(a.Size);
				Size = a.Size;
				for (uint32_t i = 0; i < Size; i++) {
					Buf[i] = a[i];
				}
			}
		}

		inline T const& operator [] (uint32_t index) const
		{
			return Buf[index];
		}

		inline T& operator [] (uint32_t index)
		{
			return Buf[index];
		}

		uint32_t CapacityIncrement() const
		{
			if (Capacity > 0) {
				return 2 * Capacity;
			}
			else {
				return 1;
			}
		}

		void Clear()
		{
			Size = 0;
		}

		void Reallocate(uint32_t newCapacity)
		{
			auto newBuf = GameAllocArray<T>(newCapacity);
			for (uint32_t i = 0; i < std::min(Size, newCapacity); i++) {
				newBuf[i] = Buf[i];
			}

			if (Buf != nullptr) {
				GameFree(Buf);
			}

			Buf = newBuf;
			Capacity = newCapacity;
		}

		void Add(T const& value)
		{
			if (Capacity <= Size) {
				Reallocate(CapacityIncrement());
			}

			Buf[Size++] = value;
		}

		bool SafeAdd(T const& val)
		{
			if (Capacity <= Size) {
				Reallocate(CapacityIncrement());
			}

			if (Size < Capacity) {
				Buf[Size++] = val;
				return true;
			}
			else {
				return false;
			}
		}

		void Remove(uint32_t index)
		{
			if (index >= Size) {
				ERR("Tried to remove out-of-bounds index %d!", index);
				return;
			}

			for (auto i = index; i < Size - 1; i++) {
				Buf[i] = Buf[i + 1];
			}

			Size--;
		}

		ContiguousIterator<T> begin()
		{
			return ContiguousIterator<T>(Buf);
		}

		ContiguousConstIterator<T> begin() const
		{
			return ContiguousConstIterator<T>(Buf);
		}

		ContiguousIterator<T> end()
		{
			return ContiguousIterator<T>(Buf + Size);
		}

		ContiguousConstIterator<T> end() const
		{
			return ContiguousConstIterator<T>(Buf + Size);
		}
	};

	template <class T>
	struct VirtualArray : public Array<T>
	{
		inline virtual ~VirtualArray() {};
	};

	// Special hashing needed for FixedStrings in the new hash table

	template <class T>
	uint64_t MultiHashMapHash(T const& v)
	{
		return Hash<T>(v);
	}

	template <>
	inline uint64_t MultiHashMapHash<FixedString>(FixedString const& v)
	{
		return v.GetHash();
	}

	template <class TKey, class TValue>
	struct MultiHashMap : public ProtectedGameObject<MultiHashMap<TKey, TValue>>
	{
		int32_t* HashKeys;
		int32_t NumHashKeys;
		Array<int32_t> NextIds;
		Array<TKey> Keys;
		TValue* Values;
		int32_t NumValues;

		int FindIndex(TKey const& key) const
		{
			if (NumHashKeys <= 0) return -1;

			auto keyIndex = HashKeys[MultiHashMapHash(key) % NumHashKeys];
			while (keyIndex >= 0) {
				if (Keys[keyIndex] == key) return keyIndex;
				keyIndex = NextIds[keyIndex];
			}

			return -1;
		}

		std::optional<TValue const*> Find(TKey const& key) const
		{
			auto index = FindIndex(key);
			if (index == -1) {
				return {};
			} else {
				return Values + index;
			}
		}

		std::optional<TValue*> Find(TKey const& key)
		{
			auto index = FindIndex(key);
			if (index == -1) {
				return {};
			} else {
				return Values + index;
			}
		}
	};

	template <class TKey, class TValue>
	struct VirtualMultiHashMap : public MultiHashMap<TKey, TValue>
	{
		virtual inline void Dummy() {}
	};
}
