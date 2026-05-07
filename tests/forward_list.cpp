#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <memory>
#include <random>
#include <stdexcept>
#include <tcl/forward_list.hpp>
#include <vector>

struct CopyOnly
{
	explicit CopyOnly(int v)
	    : value(v)
	{}

	~CopyOnly() noexcept = default;

	CopyOnly(const CopyOnly&)            = default;
	CopyOnly& operator=(const CopyOnly&) = delete;
	CopyOnly(CopyOnly&&)                 = delete;
	CopyOnly& operator=(CopyOnly&&)      = delete;

	int value;
};

struct ThrowOnCopyOnly
{
	explicit ThrowOnCopyOnly(int v)
	    : value(v)
	{}

	~ThrowOnCopyOnly() noexcept = default;

	ThrowOnCopyOnly(const ThrowOnCopyOnly& o)
	    : value(o.value)
	{
		if (countdown > 0 && --countdown == 0) throw std::runtime_error("copy");
	}

	ThrowOnCopyOnly& operator=(const ThrowOnCopyOnly&) = delete;
	ThrowOnCopyOnly(ThrowOnCopyOnly&&)                 = delete;
	ThrowOnCopyOnly& operator=(ThrowOnCopyOnly&&)      = delete;

	static int countdown;

	int value;
};

int ThrowOnCopyOnly::countdown = 0;

struct ThrowOnDefault
{
	ThrowOnDefault()
	{
		if (countdown > 0 && --countdown == 0)
			throw std::runtime_error("default");
	}

	static int countdown;
};

int ThrowOnDefault::countdown = 0;

struct ThrowOnCopy
{
	ThrowOnCopy(int v = 0)
	    : value(v)
	{}

	~ThrowOnCopy() noexcept = default;

	ThrowOnCopy(const ThrowOnCopy& other)
	    : value(other.value)
	{
		if (countdown > 0 && --countdown == 0) throw std::runtime_error("copy");
	}

	ThrowOnCopy& operator=(const ThrowOnCopy& other) = default;
	ThrowOnCopy(ThrowOnCopy&&)                       = delete;
	ThrowOnCopy& operator=(ThrowOnCopy&&)            = delete;

	static int countdown;

	int value;
};

int ThrowOnCopy::countdown = 0;

template class tcl::forward_list<int>;
template class tcl::forward_list<std::unique_ptr<int>>;
template class tcl::forward_list<CopyOnly>;
template class tcl::forward_list<ThrowOnCopyOnly>;
template class tcl::forward_list<ThrowOnDefault>;
template class tcl::forward_list<ThrowOnCopy>;

// --- Construction ------------------------------------------------------------

TEST_CASE("default construction yields empty list",
          "[forward_list][construction]")
{
	tcl::forward_list<int> l;
	REQUIRE(l.size() == 0);
	REQUIRE(l.empty());
	REQUIRE(l.begin() == l.end());
}

TEST_CASE("sized construction default-initialises elements",
          "[forward_list][construction]")
{
	tcl::forward_list<int> l(3);
	REQUIRE(l.size() == 3);
	REQUIRE(!l.empty());
	for (int v : l) REQUIRE(v == 0);
}

TEST_CASE("sized construction of zero is empty", "[forward_list][construction]")
{
	tcl::forward_list<int> l(0);
	REQUIRE(l.empty());
	REQUIRE(l.begin() == l.end());
}

TEST_CASE("sized construction rolls back on allocation failure",
          "[forward_list][construction]")
{
	ThrowOnDefault::countdown = 2;
	REQUIRE_THROWS_AS((tcl::forward_list<ThrowOnDefault>(5)),
	                  std::runtime_error);
	ThrowOnDefault::countdown = 0;
}

// --- Copy & move -------------------------------------------------------------

TEST_CASE("copy construction produces independent duplicate",
          "[forward_list][copy]")
{
	tcl::forward_list<int> a;
	a.push_front(3);
	a.push_front(2);
	a.push_front(1);

	tcl::forward_list<int> b = a;
	REQUIRE(b.size() == 3);
	REQUIRE(b.front() == 1);

	b.front() = 99;
	REQUIRE(a.front() == 1);
}

TEST_CASE("copy assignment over non-empty target", "[forward_list][copy]")
{
	tcl::forward_list<int> a, b;
	a.push_front(2);
	a.push_front(1);
	b.push_front(9);
	b.push_front(8);
	b.push_front(7);

	b = a;
	REQUIRE(b.size() == 2);
	REQUIRE(b.front() == 1);
}

TEST_CASE("self copy assignment is a no-op", "[forward_list][copy]")
{
	tcl::forward_list<int> a;
	a.push_front(42);
	a = a;
	REQUIRE(a.size() == 1);
	REQUIRE(a.front() == 42);
}

TEST_CASE("move construction transfers ownership", "[forward_list][move]")
{
	tcl::forward_list<int> a;
	a.push_front(2);
	a.push_front(1);

	tcl::forward_list<int> b = std::move(a);
	REQUIRE(b.size() == 2);
	REQUIRE(b.front() == 1);
	REQUIRE(a.empty());
}

TEST_CASE("move assignment transfers ownership", "[forward_list][move]")
{
	tcl::forward_list<int> a, b;
	a.push_front(5);
	a.push_front(4);
	b.push_front(9);

	b = std::move(a);
	REQUIRE(b.size() == 2);
	REQUIRE(b.front() == 4);
}

TEST_CASE("self move assignment is safe", "[forward_list][move]")
{
	tcl::forward_list<int> a;
	a.push_front(1);
	a = std::move(a);
}

// --- Equality ----------------------------------------------------------------

TEST_CASE("equality operations are correct", "[forward_list][equality]")
{
	tcl::forward_list<int> a;
	a.push_front(3);
	a.push_front(2);
	a.push_front(1);

	tcl::forward_list<int> b;
	b.push_front(3);
	b.push_front(2);
	b.push_front(1);

	REQUIRE(a == b);
	REQUIRE_FALSE(a != b);
}

TEST_CASE("equality operations are correct for same sized lists",
          "[forward_list][equality]")
{
	tcl::forward_list<int> a;
	a.push_front(3);
	a.push_front(2);
	a.push_front(1);

	tcl::forward_list<int> b;
	b.push_front(1);
	b.push_front(2);
	b.push_front(1);

	REQUIRE(a != b);
	REQUIRE_FALSE(a == b);
}

TEST_CASE("equality operations are correct for different sized lists",
          "[forward_list][equality]")
{
	tcl::forward_list<int> a;
	a.push_front(3);
	a.push_front(2);
	a.push_front(1);

	tcl::forward_list<int> b;
	b.push_front(2);
	b.push_front(1);

	REQUIRE(a != b);
	REQUIRE_FALSE(a == b);
}

TEST_CASE("self equality is correct", "[forward_list][equality]")
{
	tcl::forward_list<int> a;
	a.push_front(1);

	REQUIRE(a == a);
	REQUIRE_FALSE(a != a);
}

// --- push_front / front ------------------------------------------------------

TEST_CASE("push_front prepends and returns reference", "[forward_list][push]")
{
	tcl::forward_list<int> l;
	int& ref = l.push_front(10);
	REQUIRE(ref == 10);
	REQUIRE(l.front() == 10);
	REQUIRE(l.size() == 1);

	l.push_front(20);
	REQUIRE(l.front() == 20);
	REQUIRE(l.size() == 2);
}

TEST_CASE("push_front lvalue overload copies", "[forward_list][push]")
{
	tcl::forward_list<std::string> l;
	std::string s = "hello";
	l.push_front(s);
	REQUIRE(l.front() == "hello");
	REQUIRE(s == "hello");
}

TEST_CASE("emplace_front constructs in-place", "[forward_list][emplace]")
{
	tcl::forward_list<std::pair<int, std::string>> l;
	l.emplace_front(7, "seven");
	REQUIRE(l.front().first == 7);
	REQUIRE(l.front().second == "seven");
}

// --- const accessors ---------------------------------------------------------

TEST_CASE("const front and cend are reachable", "[forward_list][const]")
{
	tcl::forward_list<int> l;
	l.push_front(1);
	l.push_front(2);

	const auto& cl = l;
	REQUIRE(cl.front() == 2);

	auto it = cl.cbegin();
	++it;
	++it;
	REQUIRE(it == cl.cend());
}

// --- pop_front ---------------------------------------------------------------

TEST_CASE("pop_front removes head and returns value", "[forward_list][pop]")
{
	tcl::forward_list<int> l;
	l.push_front(2);
	l.push_front(1);

	int v = l.pop_front();
	REQUIRE(v == 1);
	REQUIRE(l.size() == 1);
	REQUIRE(l.front() == 2);
}

TEST_CASE("pop_front until empty", "[forward_list][pop]")
{
	tcl::forward_list<int> l;
	l.push_front(1);
	l.push_front(2);
	l.pop_front();
	l.pop_front();
	REQUIRE(l.empty());
	REQUIRE(l.begin() == l.end());
}

// --- insert_after / emplace_after --------------------------------------------

TEST_CASE("insert_after inserts at correct position", "[forward_list][insert]")
{
	tcl::forward_list<int> l;
	l.push_front(3);
	l.push_front(1);

	auto it = l.begin();
	l.insert_after(it, 2);
	REQUIRE(l.size() == 3);

	std::vector<int> got(l.begin(), l.end());
	REQUIRE(got == std::vector<int>{1, 2, 3});
}

TEST_CASE("insert_after range inserts in order", "[forward_list][insert]")
{
	tcl::forward_list<int> l;
	l.push_front(0);

	std::vector<int> src{1, 2, 3};
	l.insert_after(l.begin(), src.begin(), src.end());
	REQUIRE(l.size() == 4);

	std::vector<int> got(l.begin(), l.end());
	REQUIRE(got == std::vector<int>{0, 1, 2, 3});
}

TEST_CASE("insert_after empty range is a no-op", "[forward_list][insert]")
{
	tcl::forward_list<int> l;
	l.push_front(1);

	std::vector<int> src;
	l.insert_after(l.begin(), src.begin(), src.end());
	REQUIRE(l.size() == 1);
	REQUIRE(l.front() == 1);
}

TEST_CASE("insert_after range rolls back on exception",
          "[forward_list][insert]")
{
	tcl::forward_list<ThrowOnCopy> l;
	l.emplace_front(0);

	std::vector<ThrowOnCopy> src{
	    ThrowOnCopy{1}, ThrowOnCopy{2}, ThrowOnCopy{3}};

	ThrowOnCopy::countdown = 2;
	REQUIRE_THROWS_AS(l.insert_after(l.cbegin(), src.begin(), src.end()),
	                  std::runtime_error);
	ThrowOnCopy::countdown = 0;

	REQUIRE(l.size() == 1);
	REQUIRE(l.front().value == 0);
}

TEST_CASE("emplace_after returns reference to new element",
          "[forward_list][emplace]")
{
	tcl::forward_list<int> l;
	l.push_front(1);
	int& ref = l.emplace_after(l.begin(), 42);
	REQUIRE(ref == 42);
	REQUIRE(l.size() == 2);
}

// --- erase_after -------------------------------------------------------------

TEST_CASE("erase_after removes single element and returns its value",
          "[forward_list][erase]")
{
	tcl::forward_list<int> l;
	l.push_front(3);
	l.push_front(2);
	l.push_front(1);

	int v = l.erase_after(l.begin());
	REQUIRE(v == 2);
	REQUIRE(l.size() == 2);

	std::vector<int> got(l.begin(), l.end());
	REQUIRE(got == std::vector<int>{1, 3});
}

TEST_CASE("erase_after range removes correct elements", "[forward_list][erase]")
{
	tcl::forward_list<int> l;
	for (int i = 5; i >= 1; --i) l.push_front(i);

	auto first = l.begin(); // points to 1
	auto last  = first;
	++last;
	++last;
	++last;
	l.erase_after(first, last);

	std::vector<int> got(l.begin(), l.end());
	REQUIRE(got == std::vector<int>{1, 4, 5});
}

// --- clear -------------------------------------------------------------------

TEST_CASE("clear empties the list", "[forward_list][clear]")
{
	tcl::forward_list<int> l(5);
	l.clear();
	REQUIRE(l.empty());
	REQUIRE(l.size() == 0);
	REQUIRE(l.begin() == l.end());
}

TEST_CASE("clear on empty list is a no-op", "[forward_list][clear]")
{
	tcl::forward_list<int> l;
	l.clear();
	REQUIRE(l.empty());
}

TEST_CASE("list is usable after clear", "[forward_list][clear]")
{
	tcl::forward_list<int> l(3);
	l.clear();
	l.push_front(99);
	REQUIRE(l.size() == 1);
	REQUIRE(l.front() == 99);
}

// --- assign ------------------------------------------------------------------

TEST_CASE("assign replaces contents from iterator range",
          "[forward_list][assign]")
{
	tcl::forward_list<int> l(3);
	std::vector<int> src{10, 20, 30, 40};
	l.assign(src.begin(), src.end());
	REQUIRE(l.size() == 4);

	std::vector<int> got(l.begin(), l.end());
	REQUIRE(got == src);
}

TEST_CASE("assign to shorter range trims tail", "[forward_list][assign]")
{
	tcl::forward_list<int> l(5);
	std::vector<int> src{1, 2};
	l.assign(src.begin(), src.end());
	REQUIRE(l.size() == 2);
}

TEST_CASE("assign from empty range clears the list", "[forward_list][assign]")
{
	tcl::forward_list<int> l(3);
	std::vector<int> src;
	l.assign(src.begin(), src.end());
	REQUIRE(l.empty());
}

TEST_CASE(
    "assign copy-constructible-only type (non-default-constructible path)",
    "[forward_list][assign]")
{
	tcl::forward_list<CopyOnly> l;
	l.emplace_front(1);

	std::vector<CopyOnly> src{CopyOnly{10}, CopyOnly{20}, CopyOnly{30}};
	l.assign(src.begin(), src.end());
	REQUIRE(l.size() == 3);
	REQUIRE(l.front().value == 10);
}

TEST_CASE("assign from empty range on copy-constructible-only type",
          "[forward_list][assign]")
{
	tcl::forward_list<CopyOnly> l;
	l.emplace_front(1);

	std::vector<CopyOnly> empty;
	l.assign(empty.begin(), empty.end());
	REQUIRE(l.empty());
}

TEST_CASE("assign rolls back on exception for copy-constructible-only type",
          "[forward_list][assign]")
{
	tcl::forward_list<ThrowOnCopyOnly> l;
	l.emplace_front(99);

	std::vector<ThrowOnCopyOnly> src{
	    ThrowOnCopyOnly{1}, ThrowOnCopyOnly{2}, ThrowOnCopyOnly{3}};

	ThrowOnCopyOnly::countdown = 2;
	REQUIRE_THROWS_AS(l.assign(src.begin(), src.end()), std::runtime_error);
	ThrowOnCopyOnly::countdown = 0;

	REQUIRE(l.size() == 1);
	REQUIRE(l.front().value == 99);
}

// --- split_after -------------------------------------------------------------

TEST_CASE("split_after at end returns empty list", "[forward_list][split]")
{
	tcl::forward_list<int> l;
	l.push_front(3);
	l.push_front(2);
	l.push_front(1);

	auto tail = l.split_after(l.cend());
	REQUIRE(tail.empty());
	REQUIRE(l.size() == 3);
}

TEST_CASE("split_after mid-list splits correctly", "[forward_list][split]")
{
	tcl::forward_list<int> l;
	for (int i = 5; i >= 1; --i) l.push_front(i); // [1,2,3,4,5]

	auto it = l.cbegin();
	++it;
	++it; // points to element with value 3

	auto tail = l.split_after(it); // tail should be [4,5]

	REQUIRE(l.size() == 3);
	REQUIRE(tail.size() == 2);

	std::vector<int> head_vals(l.begin(), l.end());
	std::vector<int> tail_vals(tail.begin(), tail.end());
	REQUIRE(head_vals == std::vector<int>{1, 2, 3});
	REQUIRE(tail_vals == std::vector<int>{4, 5});
}

TEST_CASE("split_after at first element yields single-element head",
          "[forward_list][split]")
{
	tcl::forward_list<int> l;
	l.push_front(2);
	l.push_front(1);

	auto tail = l.split_after(l.cbegin()); // tail = [2]
	REQUIRE(l.size() == 1);
	REQUIRE(l.front() == 1);
	REQUIRE(tail.size() == 1);
	REQUIRE(tail.front() == 2);
}

// --- resize ------------------------------------------------------------------

TEST_CASE("forward lists can be sized and resized", "[forward_list]")
{
	tcl::forward_list<int> l(5);
	REQUIRE(l.size() == 5);

	SECTION("resizing bigger changes size")
	{
		l.resize(10);
		REQUIRE(l.size() == 10);
	}
	SECTION("resizing smaller changes size")
	{
		l.resize(3);
		REQUIRE(l.size() == 3);
	}
	SECTION("resizing to zero clears")
	{
		l.resize(0);
		REQUIRE(l.size() == 0);
		REQUIRE(l.empty());
	}
	SECTION("resizing to the same size does nothing")
	{
		l.resize(5);
		REQUIRE(l.size() == 5);
	}
	SECTION("resize from empty appends default elements")
	{
		tcl::forward_list<int> empty;
		empty.resize(3);
		REQUIRE(empty.size() == 3);
		for (int v : empty) REQUIRE(v == 0);
	}
	SECTION("existing values preserved on grow")
	{
		l.front() = 7;
		l.resize(8);
		REQUIRE(l.front() == 7);
	}
}

TEST_CASE("resize rolls back newly appended nodes on allocation failure",
          "[forward_list][resize]")
{
	tcl::forward_list<ThrowOnDefault> l(1);

	ThrowOnDefault::countdown = 2;
	REQUIRE_THROWS_AS(l.resize(4), std::runtime_error);
	ThrowOnDefault::countdown = 0;

	REQUIRE(l.size() == 1);
}

// --- Iterator behaviour ------------------------------------------------------

TEST_CASE("pre- and post-increment traverse in insertion order",
          "[forward_list][iterator]")
{
	tcl::forward_list<int> l;
	l.push_front(3);
	l.push_front(2);
	l.push_front(1);

	auto it = l.begin();
	REQUIRE(*it == 1);
	auto prev = it++;
	REQUIRE(*prev == 1);
	REQUIRE(*it == 2);
	REQUIRE(*++it == 3);
	REQUIRE(++it == l.end());
}

TEST_CASE("const_iterator is obtainable from mutable iterator",
          "[forward_list][iterator]")
{
	tcl::forward_list<int> l;
	l.push_front(42);
	tcl::forward_list<int>::const_iterator cit = l.cbegin();
	REQUIRE(*cit == 42);
}

TEST_CASE("iterators of two empty lists compare equal",
          "[forward_list][iterator]")
{
	tcl::forward_list<int> a, b;
	REQUIRE(a.begin() == a.end());
	REQUIRE(b.begin() == b.end());
}

TEST_CASE("range-for accumulates correct sum", "[forward_list][iterator]")
{
	tcl::forward_list<int> l;
	for (int i = 1; i <= 5; ++i) l.push_front(i);
	int sum = 0;
	for (int v : l) sum += v;
	REQUIRE(sum == 15);
}

// --- Move-only element type --------------------------------------------------

TEST_CASE("move-only type: push, pop, emplace", "[forward_list][move-only]")
{
	tcl::forward_list<std::unique_ptr<int>> l;
	l.push_front(std::make_unique<int>(1));
	l.push_front(std::make_unique<int>(2));
	REQUIRE(l.size() == 2);
	REQUIRE(*l.front() == 2);

	auto p = l.pop_front();
	REQUIRE(*p == 2);
	REQUIRE(l.size() == 1);
}

// --- reverse -----------------------------------------------------------------

TEST_CASE("empty list", "[forward_list][reverse]")
{
	tcl::forward_list<int> l;
	l.reverse();
	REQUIRE(l.empty());
}

TEST_CASE("list with one element", "[forward_list][reverse]")
{
	tcl::forward_list<int> l;
	l.push_front(1);
	l.reverse();

	REQUIRE(l.front() == 1);
	REQUIRE(l.size() == 1);
}

TEST_CASE("list with multiple elements", "[forward_list][reverse]")
{
	tcl::forward_list<int> a;
	a.push_front(1);
	a.push_front(2);
	a.push_front(3);

	tcl::forward_list<int> b;
	b.push_front(3);
	b.push_front(2);
	b.push_front(1);
	b.reverse();

	REQUIRE(a == b);
}

TEST_CASE("before end iterator", "[forward_list][reverse]")
{
	tcl::forward_list<int> a;
	a.push_front(1);
	a.push_front(2);
	a.push_front(3);
	auto it = a.reverse();

	REQUIRE(*it == 3);
	REQUIRE(a.front() == 1);
}

// --- swap --------------------------------------------------------------------

TEST_CASE("empty lists", "[forward_list][swap]")
{
	tcl::forward_list<int> a, b;
	a.swap(b);

	REQUIRE(a.size() == 0);
	REQUIRE(b.size() == 0);
}

TEST_CASE("with empty list", "[forward_list][swap]")
{
	tcl::forward_list<int> a, b;
	a.push_front(1);
	a.push_front(2);
	a.push_front(3);
	a.swap(b);

	REQUIRE(a.size() == 0);
	REQUIRE(b.size() == 3);
}

TEST_CASE("with different lists", "[forward_list][swap]")
{
	tcl::forward_list<int> a;
	a.push_front(1);
	a.push_front(2);
	a.push_front(3);

	tcl::forward_list<int> b;
	b.push_front(4);
	b.push_front(3);
	b.push_front(2);
	b.push_front(1);

	SECTION("idempotent b/a")
	{
		b.swap(a);
	}
	SECTION("idempotent a/b")
	{
		a.swap(b);
	}

	REQUIRE(a.size() == 4);
	REQUIRE(b.size() == 3);
}

// --- sort --------------------------------------------------------------------

TEST_CASE("with empty list", "[forward_list][sort]")
{
	tcl::forward_list<int> a;
	a.sort();
	REQUIRE(a.size() == 0);
}

TEST_CASE("with one element", "[forward_list][sort]")
{
	tcl::forward_list<int> a(1);
	a.sort();
	REQUIRE(a.size() == 1);
}

TEST_CASE("with two elements", "[forward_list][sort]")
{
	tcl::forward_list<int> a;
	a.push_front(1);
	a.push_front(2);
	a.sort();

	REQUIRE(std::vector(a.begin(), a.end()) == std::vector{{1, 2}});
}

TEST_CASE("with three elements", "[forward_list][sort]")
{
	tcl::forward_list<int> a;
	a.push_front(1);
	a.push_front(2);
	a.push_front(3);
	a.sort();

	REQUIRE(std::vector(a.begin(), a.end()) == std::vector{{1, 2, 3}});
}

TEST_CASE("with many elements", "[forward_list][sort]")
{
	const std::size_t size = GENERATE(4, 8, 16, 32, 64, 128, 256, 512, 1024);
	CAPTURE(size);

	tcl::forward_list<int> l;
	std::vector<int> v(size);

	std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> dist(-100000, 100000);

	for (std::size_t i = 0; i < size; ++i)
	{
		v[i] = dist(rng);
		l.push_front(v[i]);
	}

	l.sort();
	std::ranges::sort(v);

	REQUIRE(std::vector(l.begin(), l.end()) == v);
}

// --- unique ------------------------------------------------------------------

TEST_CASE("with empty list", "[forward_list][unique]")
{
	tcl::forward_list<int> a;
	a.unique();
	REQUIRE(a.size() == 0);
}

TEST_CASE("with one element", "[forward_list][unique]")
{
	tcl::forward_list<int> a(1);
	a.unique();
	REQUIRE(a.size() == 1);
}

TEST_CASE("with two different elements", "[forward_list][unique]")
{
	tcl::forward_list<int> a;
	a.push_front(1);
	a.push_front(2);
	a.unique();

	REQUIRE(std::vector(a.begin(), a.end()) == std::vector{{2, 1}});
}

TEST_CASE("with two of the same elements", "[forward_list][unique]")
{
	tcl::forward_list<int> a;
	a.push_front(1);
	a.push_front(1);
	a.unique();

	REQUIRE(a.front() == 1);
	REQUIRE(a.size() == 1);
}

TEST_CASE("with many elements", "[forward_list][unique]")
{
	const std::size_t size = GENERATE(4, 8, 16, 32, 64, 128, 256, 512, 1024);
	CAPTURE(size);

	tcl::forward_list<int> l;
	std::vector<int> v(size);

	std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int> dist(0, 100);

	for (std::size_t i = 0; i < size; ++i)
	{
		v[size - i - 1] = dist(rng);
		l.push_front(v[size - i - 1]);
	}

	l.unique();

	const auto r = std::ranges::unique(v);
	v.erase(r.begin(), r.end());

	REQUIRE(std::vector(l.begin(), l.end()) == v);
}

// --- remove ------------------------------------------------------------------

TEST_CASE("with empty list", "[forward_list][remove]")
{
	tcl::forward_list<int> a;

	SECTION("remove")
	{
		a.remove(0);
	}
	SECTION("remove_of")
	{
		a.remove_if([](int v) { return v == 0; });
	}

	REQUIRE(a.size() == 0);
}

TEST_CASE("with one element", "[forward_list][remove]")
{
	tcl::forward_list<int> a(1);

	SECTION("same element")
	{
		a.remove(0);
		REQUIRE(a.size() == 0);
	}
	SECTION("different element")
	{
		a.remove(1);
		REQUIRE(a.size() == 1);
	}
}

TEST_CASE("with two different elements", "[forward_list][remove]")
{
	tcl::forward_list<int> a;
	a.push_front(1);
	a.push_front(2);

	SECTION("remove")
	{
		a.remove(2);
	}
	SECTION("remove_of")
	{
		a.remove_if([](int v) { return v == 2; });
	}

	REQUIRE(a.front() == 1);
	REQUIRE(a.size() == 1);
}
