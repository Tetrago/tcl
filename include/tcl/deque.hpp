#ifndef TCL_DEQUE_HPP
#define TCL_DEQUE_HPP

#include <compare>
#include <format>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace tcl
{
	template <typename T, typename Allocator = std::allocator<T>>
	class deque
	{
	private:
		template <bool C>
		class iterator_impl;

		using alloc_traits = std::allocator_traits<Allocator>;

	public:
		using allocator_type  = Allocator;
		using size_type       = std::size_t;
		using value_type      = T;
		using const_iterator  = iterator_impl<true>;
		using iterator        = iterator_impl<false>;
		using reference       = T&;
		using const_reference = const T&;

		deque(const Allocator& alloc = Allocator{}) noexcept(
		    std::is_nothrow_constructible_v<Allocator, const Allocator&> &&
		    std::is_nothrow_invocable_v<typename alloc_traits::allocate>);

		~deque() noexcept;

		[[nodiscard]] reference operator[](size_type pos) noexcept;
		[[nodiscard]] const_reference operator[](size_type pos) const noexcept;

		[[nodiscard]] reference at(size_type pos);
		[[nodiscard]] const_reference at(size_type pos) const;

		[[nodiscard]] iterator begin() noexcept { return {this, front_}; }

		[[nodiscard]] iterator end() noexcept { return {}; }

		[[nodiscard]] const_iterator begin() const noexcept { return begin(); }

		[[nodiscard]] const_iterator end() const noexcept { return {}; }

		[[nodiscard]] const_iterator cbegin() const noexcept { return begin(); }

		[[nodiscard]] const_iterator cend() const noexcept { return {}; }

		[[nodiscard]] size_type size() const noexcept { return size_; }

		[[nodiscard]] bool empty() const noexcept { return size_ == 0; }

	private:
		static constexpr size_type INITIAL_SIZE = 8;

		[[no_unique_address]] Allocator alloc_;
		size_type capacity_ = 0;
		T* data_            = nullptr;
		size_type size_     = 0;
		size_type front_    = 0; /**< Not valid when size_ == 0 */
		size_type end_      = 0;
	};

	template <typename T, typename Allocator>
	template <bool C>
	class deque<T, Allocator>::iterator_impl
	{
	public:
		using difference_type   = std::ptrdiff_t;
		using iterator_category = std::random_access_iterator_tag;
		using pointer           = std::conditional_t<C, const T*, T*>;
		using reference         = std::conditional_t<C, const T&, T&>;
		using value_type        = T;

		iterator_impl() noexcept  = default;
		~iterator_impl() noexcept = default;

		iterator_impl(const iterator_impl&) noexcept            = default;
		iterator_impl& operator=(const iterator_impl&) noexcept = default;
		iterator_impl(iterator_impl&&) noexcept                 = default;
		iterator_impl& operator=(iterator_impl&&) noexcept      = default;

		/**
		 * @ref iterator to @ref const_iterator implicit conversion utility.
		 */
		iterator_impl(const iterator_impl<false>& other) noexcept
		    requires(C)
		    : deque_(other.deque_), pos_(other, pos_)
		{}

		pointer operator->() const noexcept { return &*this; }

		[[nodiscard]] reference operator*() const noexcept
		{
			return (*deque_)[pos_];
		}

		[[nodiscard]] reference operator[](difference_type d) const noexcept;

		[[nodiscard]] iterator_impl operator+(difference_type n) const noexcept;
		[[nodiscard]] iterator_impl operator-(difference_type n) const noexcept;

		iterator_impl& operator+=(difference_type n) noexcept;
		iterator_impl& operator-=(difference_type n) noexcept;

		iterator_impl& operator++() noexcept;
		[[nodiscard]] iterator_impl operator++(int) noexcept;

		iterator_impl& operator--() noexcept;
		[[nodiscard]] iterator_impl operator--(int) noexcept;

		[[nodiscard]] difference_type operator-(
		    const iterator_impl& rhs) const noexcept;

		[[nodiscard]] bool operator==(const iterator_impl& rhs) const noexcept;

		[[nodiscard]] std::partial_ordering operator<=>(
		    const iterator_impl& rhs) const noexcept;

	private:
		[[nodiscard]] friend iterator_impl operator+(
		    difference_type lhs, const iterator_impl& rhs) noexcept
		{
			return lhs + rhs;
		}

		iterator_impl(deque* deque, size_type pos) noexcept
		    : deque_(deque), pos_(pos)
		{}

		deque* deque_  = nullptr;
		size_type pos_ = 0;
	};
} // namespace tcl

static_assert(std::input_iterator<tcl::deque<int>::iterator>);
static_assert(std::output_iterator<tcl::deque<int>::iterator, int>);
static_assert(std::random_access_iterator<tcl::deque<int>::iterator>);
static_assert(std::input_iterator<tcl::deque<int>::const_iterator>);
static_assert(!std::output_iterator<tcl::deque<int>::const_iterator, int>);

template <typename T, typename Allocator>
inline tcl::deque<T, Allocator>::deque(const Allocator& alloc) noexcept(
    std::is_nothrow_constructible_v<Allocator, const Allocator&> &&
    std::is_nothrow_invocable_v<typename alloc_traits::allocate>)
    : alloc_(alloc),
      capacity_(INITIAL_SIZE),
      data_(alloc_traits::allocate(alloc_, capacity_))
{}

template <typename T, typename Allocator>
inline tcl::deque<T, Allocator>::~deque() noexcept
{
	if (data_)
	{
		for (auto& value : *this)
		{
			alloc_traits::destroy(alloc_, &value);
		}

		alloc_traits::deallocate(alloc_, data_, capacity_);
	}
}

template <typename T, typename Allocator>
tcl::deque<T, Allocator>::reference tcl::deque<T, Allocator>::operator[](
    size_type pos) noexcept
{
	return data_[(front_ + pos) % capacity_];
}

template <typename T, typename Allocator>
tcl::deque<T, Allocator>::const_reference tcl::deque<T, Allocator>::operator[](
    size_type pos) const noexcept
{
	return data_[(front_ + pos) % capacity_];
}

template <typename T, typename Allocator>
tcl::deque<T, Allocator>::reference tcl::deque<T, Allocator>::at(size_type pos)
{
	if (pos >= size_)
	{
		throw std::out_of_range(std::format("{} >= {}", pos, size_));
	}
	else
	{
		return (*this)[pos];
	}
}

template <typename T, typename Allocator>
tcl::deque<T, Allocator>::const_reference tcl::deque<T, Allocator>::at(
    size_type pos) const
{
	if (pos >= size_)
	{
		throw std::out_of_range(std::format("{} >= {}", pos, size_));
	}
	else
	{
		return (*this)[pos];
	}
}

template <typename T, typename Allocator>
template <bool C>
tcl::deque<T, Allocator>::iterator_impl<C>::reference
tcl::deque<T, Allocator>::iterator_impl<C>::operator[](
    difference_type n) const noexcept
{
	return *(*this + n);
}

template <typename T, typename Allocator>
template <bool C>
tcl::deque<T, Allocator>::iterator_impl<C>
tcl::deque<T, Allocator>::iterator_impl<C>::operator+(
    difference_type n) const noexcept
{
	// TODO: Convert to end iterator.
	return *this + n;
}

template <typename T, typename Allocator>
template <bool C>
tcl::deque<T, Allocator>::iterator_impl<C>
tcl::deque<T, Allocator>::iterator_impl<C>::operator-(
    difference_type n) const noexcept
{
	return *this + -n;
}

template <typename T, typename Allocator>
template <bool C>
tcl::deque<T, Allocator>::iterator_impl<C>&
tcl::deque<T, Allocator>::iterator_impl<C>::operator+=(
    difference_type n) noexcept
{
	return *this = *this + n;
}

template <typename T, typename Allocator>
template <bool C>
tcl::deque<T, Allocator>::iterator_impl<C>&
tcl::deque<T, Allocator>::iterator_impl<C>::operator-=(
    difference_type n) noexcept
{
	return *this += -n;
}

template <typename T, typename Allocator>
template <bool C>
tcl::deque<T, Allocator>::iterator_impl<C>&
tcl::deque<T, Allocator>::iterator_impl<C>::operator++() noexcept
{
	++pos_;
	return *this;
}

template <typename T, typename Allocator>
template <bool C>
tcl::deque<T, Allocator>::iterator_impl<C>
tcl::deque<T, Allocator>::iterator_impl<C>::operator++(int) noexcept
{
	iterator_impl it = *this;
	++(*this);
	return it;
}

template <typename T, typename Allocator>
template <bool C>
tcl::deque<T, Allocator>::iterator_impl<C>&
tcl::deque<T, Allocator>::iterator_impl<C>::operator--() noexcept
{
	--pos_;
	return *this;
}

template <typename T, typename Allocator>
template <bool C>
tcl::deque<T, Allocator>::iterator_impl<C>
tcl::deque<T, Allocator>::iterator_impl<C>::operator--(int) noexcept
{
	iterator_impl it = *this;
	--(*this);
	return it;
}

template <typename T, typename Allocator>
template <bool C>
tcl::deque<T, Allocator>::iterator_impl<C>::difference_type
tcl::deque<T, Allocator>::iterator_impl<C>::operator-(
    const iterator_impl& rhs) const noexcept
{
	return ((deque_->capacity_ + pos_) - rhs.pos_) % deque_->capacity_;
}

template <typename T, typename Allocator>
template <bool C>
bool tcl::deque<T, Allocator>::iterator_impl<C>::operator==(
    const iterator_impl& rhs) const noexcept
{
	return deque_ == rhs.deque_ && (!deque_ || pos_ == rhs.pos_);
}

template <typename T, typename Allocator>
template <bool C>
std::partial_ordering tcl::deque<T, Allocator>::iterator_impl<C>::operator<=>(
    const iterator_impl& rhs) const noexcept
{
	if (!deque_ && !rhs.deque_)
	{
		return std::partial_ordering::equivalent;
	}
	else if (!deque_)
	{
		return std::partial_ordering::greater;
	}
	else if (!rhs.deque_)
	{
		return std::partial_ordering::less;
	}
	else
	{
		return *this - rhs;
	}
}

#endif
