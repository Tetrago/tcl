#include <catch2/catch_test_macros.hpp>
#include <tcl/deque.hpp>

struct NoMove
{
	int v;

	NoMove(int v)
	    : v(v)
	{}

	~NoMove() noexcept = default;

	NoMove(const NoMove&)            = default;
	NoMove& operator=(const NoMove&) = default;
	NoMove(NoMove&&)                 = delete;
	NoMove& operator=(NoMove&&)      = delete;
};

template class tcl::deque<int>;
template class tcl::deque<NoMove>;

// --- reserve -----------------------------------------------------------------

TEST_CASE("reserve smaller size", "[deque][resserve]")
{
	tcl::deque<int> q;
	auto c = q.capacity();
	REQUIRE(q.size() == 0);
	q.reserve(c / 2);
	REQUIRE(q.size() == 0);
	REQUIRE(q.capacity() == c);
}

TEST_CASE("reserve same size", "[deque][resserve]")
{
	tcl::deque<int> q;
	auto c = q.capacity();
	REQUIRE(q.size() == 0);
	q.reserve(c);
	REQUIRE(q.size() == 0);
	REQUIRE(q.capacity() == c);
}

TEST_CASE("reserve larger size", "[deque][resserve]")
{
	tcl::deque<int> q;
	auto c = q.capacity();
	REQUIRE(q.size() == 0);
	q.reserve(c * 2);
	REQUIRE(q.size() == 0);
	REQUIRE(q.capacity() == c * 2);
}

TEST_CASE("reserve with contiguous data copies elements", "[deque][reserve]")
{
	tcl::deque<int> q;
	q.push_back(1);
	q.push_back(2);
	q.push_back(3);
	auto c = q.capacity();
	q.reserve(c * 2);
	REQUIRE(q.size() == 3);
	REQUIRE(q[0] == 1);
	REQUIRE(q[1] == 2);
	REQUIRE(q[2] == 3);
}

TEST_CASE("reserve with wrapped data copies elements", "[deque][reserve]")
{
	tcl::deque<int> q;
	auto c = q.capacity();
	for (std::size_t i = 0; i < c; ++i) q.push_back(static_cast<int>(i));
	for (std::size_t i = 0; i < 3; ++i) q.pop_front();
	q.push_back(100);
	q.push_back(101);
	q.push_back(102);
	auto sz = q.size();
	q.reserve(c * 4);
	REQUIRE(q.size() == sz);
	REQUIRE(q.front() == 3);
	REQUIRE(q.back() == 102);
}

TEST_CASE("reserve with non-trivially-copyable type", "[deque][reserve]")
{
	tcl::deque<std::string> q;
	auto c = q.capacity();
	q.push_back("a");
	q.push_back("b");
	q.push_back("c");
	q.reserve(c * 2);
	REQUIRE(q.size() == 3);
	REQUIRE(q[0] == "a");
	REQUIRE(q[1] == "b");
	REQUIRE(q[2] == "c");
}

// --- push --------------------------------------------------------------------

TEST_CASE("push_front prepends and returns reference", "[deque][push]")
{
	tcl::deque<int> d;
	int& ref = d.push_front(10);
	REQUIRE(ref == 10);
	REQUIRE(d.front() == 10);
	REQUIRE(d.size() == 1);

	d.push_front(20);
	REQUIRE(d.front() == 20);
	REQUIRE(d.size() == 2);
}

TEST_CASE("push_front lvalue overload copies", "[deque][push]")
{
	tcl::deque<std::string> d;
	std::string s = "hello";
	d.push_front(s);
	REQUIRE(d.front() == "hello");
	REQUIRE(s == "hello");
}

TEST_CASE("push_back apapends and returns reference", "[deque][push]")
{
	tcl::deque<int> d;
	int& ref = d.push_back(10);
	REQUIRE(ref == 10);
	REQUIRE(d.back() == 10);
	REQUIRE(d.size() == 1);

	d.push_back(20);
	REQUIRE(d.back() == 20);
	REQUIRE(d.size() == 2);
}

TEST_CASE("push_back lvalue overload copies", "[deque][push]")
{
	tcl::deque<std::string> d;
	std::string s = "hello";
	d.push_back(s);
	REQUIRE(d.back() == "hello");
	REQUIRE(s == "hello");
}

TEST_CASE("push_front triggers reallocation when full", "[deque][push]")
{
	tcl::deque<int> d;
	auto c = d.capacity();
	for (std::size_t i = 0; i < c; ++i) d.push_front(static_cast<int>(i));
	REQUIRE(d.size() == c);
	d.push_front(999);
	REQUIRE(d.size() == c + 1);
	REQUIRE(d.front() == 999);
	REQUIRE(d.capacity() > c);
}

TEST_CASE("push_back triggers reallocation when full", "[deque][push]")
{
	tcl::deque<int> d;
	auto c = d.capacity();
	for (std::size_t i = 0; i < c; ++i) d.push_back(static_cast<int>(i));
	REQUIRE(d.size() == c);
	d.push_back(999);
	REQUIRE(d.size() == c + 1);
	REQUIRE(d.back() == 999);
	REQUIRE(d.capacity() > c);
}

// --- pop ---------------------------------------------------------------------

TEST_CASE("pop_front returns value and shrinks", "[deque][pop]")
{
	tcl::deque<int> d;
	d.push_back(1);
	d.push_back(2);
	d.push_back(3);
	REQUIRE(d.pop_front() == 1);
	REQUIRE(d.size() == 2);
	REQUIRE(d.front() == 2);
}

TEST_CASE("pop_back returns value and shrinks", "[deque][pop]")
{
	tcl::deque<int> d;
	d.push_back(1);
	d.push_back(2);
	d.push_back(3);
	REQUIRE(d.pop_back() == 3);
	REQUIRE(d.size() == 2);
	REQUIRE(d.back() == 2);
}

TEST_CASE("pop_front on non-movable type shrinks without return",
          "[deque][pop]")
{
	tcl::deque<NoMove> d;
	d.push_back(NoMove{42});
	d.push_back(NoMove{43});
	d.pop_front();
	REQUIRE(d.size() == 1);
	REQUIRE(d.front().v == 43);
}

TEST_CASE("pop_back on non-movable type shrinks without return", "[deque][pop]")
{
	tcl::deque<NoMove> d;
	d.push_back(NoMove{42});
	d.push_back(NoMove{43});
	d.pop_back();
	REQUIRE(d.size() == 1);
	REQUIRE(d.back().v == 42);
}

// --- clear -------------------------------------------------------------------

TEST_CASE("clear on empty deque", "[deque][clear]")
{
	tcl::deque<int> d;
	d.clear();
	REQUIRE(d.size() == 0);
	REQUIRE(d.empty());
}

TEST_CASE("clear destroys all elements", "[deque][clear]")
{
	tcl::deque<std::string> d;
	d.push_back("a");
	d.push_back("b");
	d.push_back("c");
	d.clear();
	REQUIRE(d.size() == 0);
	REQUIRE(d.empty());
}

TEST_CASE("clear allows reuse", "[deque][clear]")
{
	tcl::deque<int> d;
	d.push_back(1);
	d.push_back(2);
	d.clear();
	d.push_back(99);
	REQUIRE(d.size() == 1);
	REQUIRE(d.front() == 99);
}

// --- element access ----------------------------------------------------------

TEST_CASE("operator[] reads elements by index", "[deque][access]")
{
	tcl::deque<int> d;
	d.push_back(10);
	d.push_back(20);
	d.push_back(30);
	REQUIRE(d[0] == 10);
	REQUIRE(d[1] == 20);
	REQUIRE(d[2] == 30);
}

TEST_CASE("operator[] const overload", "[deque][access]")
{
	tcl::deque<int> d;
	d.push_back(10);
	d.push_back(20);
	const tcl::deque<int>& cd = d;
	REQUIRE(cd[0] == 10);
	REQUIRE(cd[1] == 20);
}

TEST_CASE("at returns element within bounds", "[deque][access]")
{
	tcl::deque<int> d;
	d.push_back(5);
	d.push_back(6);
	REQUIRE(d.at(0) == 5);
	REQUIRE(d.at(1) == 6);
}

TEST_CASE("at throws on out-of-range index", "[deque][access]")
{
	tcl::deque<int> d;
	d.push_back(1);
	REQUIRE_THROWS_AS(d.at(1), std::out_of_range);
	REQUIRE_THROWS_AS(d.at(99), std::out_of_range);
}

TEST_CASE("at const overload returns element within bounds", "[deque][access]")
{
	tcl::deque<int> d;
	d.push_back(7);
	const tcl::deque<int>& cd = d;
	REQUIRE(cd.at(0) == 7);
}

TEST_CASE("at const overload throws on out-of-range index", "[deque][access]")
{
	tcl::deque<int> d;
	d.push_back(1);
	const tcl::deque<int>& cd = d;
	REQUIRE_THROWS_AS(cd.at(1), std::out_of_range);
}

TEST_CASE("front and back const overloads", "[deque][access]")
{
	tcl::deque<int> d;
	d.push_back(11);
	d.push_back(22);
	const tcl::deque<int>& cd = d;
	REQUIRE(cd.front() == 11);
	REQUIRE(cd.back() == 22);
}

// --- observers ---------------------------------------------------------------

TEST_CASE("empty returns true on default-constructed deque",
          "[deque][observers]")
{
	tcl::deque<int> d;
	REQUIRE(d.empty());
}

TEST_CASE("empty returns false after push", "[deque][observers]")
{
	tcl::deque<int> d;
	d.push_back(1);
	REQUIRE_FALSE(d.empty());
}

TEST_CASE("get_allocator returns the stored allocator", "[deque][observers]")
{
	std::allocator<int> a;
	tcl::deque<int> d(a);
	REQUIRE(d.get_allocator() == a);
}

// --- iterators ---------------------------------------------------------------

TEST_CASE("const begin and end iterate over elements", "[deque][iterator]")
{
	tcl::deque<int> d;
	d.push_back(1);
	d.push_back(2);
	d.push_back(3);
	const tcl::deque<int>& cd = d;
	std::vector<int> result(cd.begin(), cd.end());
	REQUIRE(result == std::vector<int>{1, 2, 3});
}

TEST_CASE("cbegin and cend iterate over elements", "[deque][iterator]")
{
	tcl::deque<int> d;
	d.push_back(4);
	d.push_back(5);
	std::vector<int> result(d.cbegin(), d.cend());
	REQUIRE(result == std::vector<int>{4, 5});
}

TEST_CASE("iterator equality between matching positions", "[deque][iterator]")
{
	tcl::deque<int> d;
	d.push_back(1);
	d.push_back(2);
	auto it  = d.begin();
	auto it2 = d.begin();
	REQUIRE(it == it2);
	++it;
	REQUIRE_FALSE(it == it2);
}

TEST_CASE("iterator end equals end", "[deque][iterator]")
{
	tcl::deque<int> d;
	d.push_back(1);
	REQUIRE(d.end() == d.end());
}

TEST_CASE("iterator operator[] random access", "[deque][iterator]")
{
	tcl::deque<int> d;
	d.push_back(10);
	d.push_back(20);
	d.push_back(30);
	auto it = d.begin();
	REQUIRE(it[0] == 10);
	REQUIRE(it[1] == 20);
	REQUIRE(it[2] == 30);
}

TEST_CASE("iterator arithmetic advance and retreat", "[deque][iterator]")
{
	tcl::deque<int> d;
	d.push_back(1);
	d.push_back(2);
	d.push_back(3);
	auto it  = d.begin();
	it      += 2;
	REQUIRE(*it == 3);
	it -= 1;
	REQUIRE(*it == 2);
}

TEST_CASE("iterator post-increment and post-decrement", "[deque][iterator]")
{
	tcl::deque<int> d;
	d.push_back(7);
	d.push_back(8);
	auto it  = d.begin();
	auto old = it++;
	REQUIRE(*old == 7);
	REQUIRE(*it == 8);
	auto old2 = it--;
	REQUIRE(*old2 == 8);
	REQUIRE(*it == 7);
}

TEST_CASE("iterator difference", "[deque][iterator]")
{
	tcl::deque<int> d;
	d.push_back(1);
	d.push_back(2);
	d.push_back(3);
	auto first = d.begin();
	auto last  = first + 3;
	REQUIRE(last - first == 3);
}

TEST_CASE("iterator spaceship ordering", "[deque][iterator]")
{
	tcl::deque<int> d;
	d.push_back(1);
	d.push_back(2);
	d.push_back(3);
	auto a = d.begin();
	auto b = a + 2;
	REQUIRE((a <=> b) == std::partial_ordering::less);
	REQUIRE((b <=> a) == std::partial_ordering::greater);
	REQUIRE((a <=> a) == std::partial_ordering::equivalent);
}

TEST_CASE("end iterators compare equivalent via spaceship", "[deque][iterator]")
{
	tcl::deque<int> d;
	d.push_back(1);
	REQUIRE((d.end() <=> d.end()) == std::partial_ordering::equivalent);
}

TEST_CASE("begin is less than end via spaceship", "[deque][iterator]")
{
	tcl::deque<int> d;
	d.push_back(1);
	REQUIRE((d.begin() <=> d.end()) == std::partial_ordering::less);
	REQUIRE((d.end() <=> d.begin()) == std::partial_ordering::greater);
}

TEST_CASE("iterator operator+ with free function lhs", "[deque][iterator]")
{
	tcl::deque<int> d;
	d.push_back(10);
	d.push_back(20);
	auto it       = d.begin();
	auto advanced = 1 + it;
	REQUIRE(*advanced == 20);
}

TEST_CASE("iterator operator->", "[deque][iterator]")
{
	tcl::deque<std::string> d;
	d.push_back("hello");
	auto it = d.begin();
	REQUIRE(it->size() == 5);
}
