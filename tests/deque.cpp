#include <catch2/catch_test_macros.hpp>
#include <tcl/deque.hpp>

template class tcl::deque<int>;

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
