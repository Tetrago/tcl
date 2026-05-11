#ifndef TCL_DEQUE_HPP
#define TCL_DEQUE_HPP

#include <compare>
#include <cstring>
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
		using allocator_type = Allocator;
		using size_type      = std::size_t;
		using value_type     = T;
		using const_iterator = iterator_impl<true>;
		using iterator       = iterator_impl<false>;

		deque(const Allocator& alloc = Allocator{});
		~deque() noexcept;

		void clear() noexcept;

		void reserve(std::size_t n);

		T& push_front(const T& value)
		    requires std::constructible_from<T, const T&>;

		T& push_front(T&& value)
		    requires std::constructible_from<T, T&&>;

		T& push_back(const T& value)
		    requires std::constructible_from<T, const T&>;

		T& push_back(T&& value)
		    requires std::constructible_from<T, T&&>;

		T pop_front() noexcept
		    requires std::constructible_from<T, T&&>;

		void pop_front() noexcept
		    requires(!std::constructible_from<T, T &&>);

		T pop_back() noexcept
		    requires std::constructible_from<T, T&&>;

		void pop_back() noexcept
		    requires(!std::constructible_from<T, T &&>);

		[[nodiscard]] T& operator[](std::size_t pos) noexcept;
		[[nodiscard]] const T& operator[](std::size_t pos) const noexcept;

		[[nodiscard]] T& at(std::size_t pos);
		[[nodiscard]] const T& at(std::size_t pos) const;

		[[nodiscard]] iterator begin() noexcept { return {this, front_}; }

		[[nodiscard]] iterator end() noexcept { return {}; }

		[[nodiscard]] const_iterator begin() const noexcept
		{
			return {this, front_};
		}

		[[nodiscard]] const_iterator end() const noexcept { return {}; }

		[[nodiscard]] const_iterator cbegin() const noexcept
		{
			return {this, front_};
		}

		[[nodiscard]] const_iterator cend() const noexcept { return {}; }

		[[nodiscard]] std::size_t size() const noexcept { return size_; }

		[[nodiscard]] std::size_t capacity() const noexcept
		{
			return capacity_;
		}

		[[nodiscard]] bool empty() const noexcept { return size_ == 0; }

		[[nodiscard]] T& front() noexcept { return data_[front_]; }

		[[nodiscard]] const T& front() const noexcept { return data_[front_]; }

		[[nodiscard]] T& back() noexcept { return data_[back_]; }

		[[nodiscard]] const T& back() const noexcept { return data_[back_]; }

		[[nodiscard]] Allocator get_allocator() const noexcept
		{
			return alloc_;
		}

	private:
		static constexpr std::size_t INITIAL_SIZE = 8;

		[[no_unique_address]] Allocator alloc_;
		std::size_t capacity_ = 0;
		T* data_              = nullptr;
		std::size_t size_     = 0;
		std::size_t front_    = 0; /**< Not valid when size_ == 0 */
		std::size_t back_     = 0; /**< Not valid when size_ == 0 */
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
		friend deque;

		using deque_pointer = std::conditional_t<C, const deque*, deque*>;

		[[nodiscard]] friend iterator_impl operator+(
		    difference_type lhs, const iterator_impl& rhs) noexcept
		{
			return lhs + rhs;
		}

		iterator_impl(deque_pointer deque, std::size_t pos) noexcept
		    : deque_(deque), pos_(pos)
		{}

		deque_pointer deque_ = nullptr;
		std::size_t pos_ = 0; /**< The position in the data array (not index) */
	};
} // namespace tcl

static_assert(std::input_iterator<tcl::deque<int>::iterator>);
static_assert(std::output_iterator<tcl::deque<int>::iterator, int>);
static_assert(std::random_access_iterator<tcl::deque<int>::iterator>);
static_assert(std::input_iterator<tcl::deque<int>::const_iterator>);
static_assert(!std::output_iterator<tcl::deque<int>::const_iterator, int>);

template <typename T, typename Allocator>
inline tcl::deque<T, Allocator>::deque(const Allocator& alloc)
    : alloc_(alloc),
      capacity_(INITIAL_SIZE),
      data_(alloc_traits::allocate(alloc_, capacity_))
{}

template <typename T, typename Allocator>
inline tcl::deque<T, Allocator>::~deque() noexcept
{
	if (data_)
	{
		clear();
		alloc_traits::deallocate(alloc_, data_, capacity_);
	}
}

template <typename T, typename Allocator>
inline void tcl::deque<T, Allocator>::clear() noexcept
{
	if constexpr (!std::is_trivially_destructible_v<T>)
	{
		for (T& value : *this)
		{
			alloc_traits::destroy(alloc_, &value);
		}
	}

	front_ = 0;
	back_  = 0;
	size_  = 0;
}

template <typename T, typename Allocator>
inline void tcl::deque<T, Allocator>::reserve(std::size_t n)
{
	if (n <= capacity_) [[unlikely]]
	{
		return;
	}

	T* data       = alloc_traits::allocate(alloc_, n);
	std::size_t i = 0;

	try
	{
		if constexpr (std::is_trivially_copyable_v<T>)
		{
			if (front_ < back_)
			{
				std::memcpy(data, &data_[front_], sizeof(T) * size_);
			}
			else
			{
				std::memcpy(data, &data_[front_], sizeof(T) * (size_ - front_));
				std::memcpy(&data[size_ - front_], data_, front_);
			}
		}
		else
		{
			for (T& value : *this)
			{
				alloc_traits::construct(
				    alloc_, &data[i], std::move_if_noexcept(value));
			}
		}

		[&]() noexcept {
			if constexpr (!std::is_trivially_destructible_v<T>)
			{
				for (T& value : *this)
				{
					alloc_traits::destroy(alloc_, &data_[i]);
				}
			}

			alloc_traits::deallocate(alloc_, data_, capacity_);
		}();

		capacity_ = n;
		data_     = data;
		front_    = 0;
		back_     = size_ - 1;
	}
	catch (...)
	{
		while (i--)
		{
			alloc_traits::destroy(alloc_, &data[i]);
		}

		alloc_traits::deallocate(alloc_, data, n);
		throw;
	}
}

template <typename T, typename Allocator>
inline T& tcl::deque<T, Allocator>::push_front(const T& value)
    requires std::constructible_from<T, const T&>
{
	if (size_ == capacity_)
	{
		reserve(capacity_ * 2);
	}

	std::size_t p = size_ == 0 ? front_ : (capacity_ + front_ - 1) % capacity_;
	alloc_traits::construct(alloc_, &data_[p], value);
	front_ = p;
	++size_;

	return data_[front_];
}

template <typename T, typename Allocator>
inline T& tcl::deque<T, Allocator>::push_front(T&& value)
    requires std::constructible_from<T, T&&>
{
	if (size_ == capacity_)
	{
		reserve(capacity_ * 2);
	}

	std::size_t p = size_ == 0 ? front_ : (capacity_ + front_ - 1) % capacity_;
	alloc_traits::construct(alloc_, &data_[p], std::move(value));
	front_ = p;
	++size_;

	return data_[front_];
}

template <typename T, typename Allocator>
inline T& tcl::deque<T, Allocator>::push_back(const T& value)
    requires std::constructible_from<T, const T&>
{
	if (size_ == capacity_)
	{
		reserve(capacity_ * 2);
	}

	std::size_t p = size_ == 0 ? back_ : (capacity_ + back_ + 1) % capacity_;
	alloc_traits::construct(alloc_, &data_[p], value);
	back_ = p;
	++size_;

	return data_[back_];
}

template <typename T, typename Allocator>
inline T& tcl::deque<T, Allocator>::push_back(T&& value)
    requires std::constructible_from<T, T&&>
{
	if (size_ == capacity_)
	{
		reserve(capacity_ * 2);
	}

	std::size_t p = size_ == 0 ? back_ : (capacity_ + back_ + 1) % capacity_;
	alloc_traits::construct(alloc_, &data_[p], std::move(value));
	back_ = p;
	++size_;

	return data_[back_];
}

template <typename T, typename Allocator>
inline T tcl::deque<T, Allocator>::pop_front() noexcept
    requires std::constructible_from<T, T&&>
{
	T value = std::move(data_[front_]);
	alloc_traits::destroy(alloc_, &data_[front_]);
	front_ = (front_ + 1) % capacity_;
	--size_;
	return value;
}

template <typename T, typename Allocator>
inline void tcl::deque<T, Allocator>::pop_front() noexcept
    requires(!std::constructible_from<T, T &&>)
{
	alloc_traits::destroy(alloc_, &data_[front_]);
	front_ = (front_ + 1) % capacity_;
	--size_;
}

template <typename T, typename Allocator>
inline T tcl::deque<T, Allocator>::pop_back() noexcept
    requires std::constructible_from<T, T&&>
{
	T value = std::move(data_[back_]);
	alloc_traits::destroy(alloc_, &data_[back_]);
	back_ = (capacity_ + back_ - 1) % capacity_;
	--size_;
	return value;
}

template <typename T, typename Allocator>
inline void tcl::deque<T, Allocator>::pop_back() noexcept
    requires(!std::constructible_from<T, T &&>)
{
	alloc_traits::destroy(alloc_, &data_[back_]);
	back_ = (capacity_ + back_ - 1) % capacity_;
	--size_;
}

template <typename T, typename Allocator>
inline T& tcl::deque<T, Allocator>::operator[](std::size_t pos) noexcept
{
	return data_[(front_ + pos) % capacity_];
}

template <typename T, typename Allocator>
inline const T& tcl::deque<T, Allocator>::operator[](
    std::size_t pos) const noexcept
{
	return data_[(front_ + pos) % capacity_];
}

template <typename T, typename Allocator>
inline T& tcl::deque<T, Allocator>::at(std::size_t pos)
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
inline const T& tcl::deque<T, Allocator>::at(std::size_t pos) const
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
inline tcl::deque<T, Allocator>::iterator_impl<C>::reference
tcl::deque<T, Allocator>::iterator_impl<C>::operator[](
    difference_type n) const noexcept
{
	return *(*this + n);
}

template <typename T, typename Allocator>
template <bool C>
inline tcl::deque<T, Allocator>::iterator_impl<C>
tcl::deque<T, Allocator>::iterator_impl<C>::operator+(
    difference_type n) const noexcept
{
	if (deque_->size_ != 0 && pos_ == deque_->back_)
	{
		return {};
	}
	else
	{
		return {deque_, (deque_->capacity_ + pos_ + n) % deque_->capacity_};
	}
}

template <typename T, typename Allocator>
template <bool C>
inline tcl::deque<T, Allocator>::iterator_impl<C>
tcl::deque<T, Allocator>::iterator_impl<C>::operator-(
    difference_type n) const noexcept
{
	return *this + -n;
}

template <typename T, typename Allocator>
template <bool C>
inline tcl::deque<T, Allocator>::iterator_impl<C>&
tcl::deque<T, Allocator>::iterator_impl<C>::operator+=(
    difference_type n) noexcept
{
	return *this = *this + n;
}

template <typename T, typename Allocator>
template <bool C>
inline tcl::deque<T, Allocator>::iterator_impl<C>&
tcl::deque<T, Allocator>::iterator_impl<C>::operator-=(
    difference_type n) noexcept
{
	return *this += -n;
}

template <typename T, typename Allocator>
template <bool C>
inline tcl::deque<T, Allocator>::iterator_impl<C>&
tcl::deque<T, Allocator>::iterator_impl<C>::operator++() noexcept
{
	return *this += 1;
}

template <typename T, typename Allocator>
template <bool C>
inline tcl::deque<T, Allocator>::iterator_impl<C>
tcl::deque<T, Allocator>::iterator_impl<C>::operator++(int) noexcept
{
	iterator_impl it = *this;
	++(*this);
	return it;
}

template <typename T, typename Allocator>
template <bool C>
inline tcl::deque<T, Allocator>::iterator_impl<C>&
tcl::deque<T, Allocator>::iterator_impl<C>::operator--() noexcept
{
	return *this -= 1;
}

template <typename T, typename Allocator>
template <bool C>
inline tcl::deque<T, Allocator>::iterator_impl<C>
tcl::deque<T, Allocator>::iterator_impl<C>::operator--(int) noexcept
{
	iterator_impl it = *this;
	--(*this);
	return it;
}

template <typename T, typename Allocator>
template <bool C>
inline tcl::deque<T, Allocator>::iterator_impl<C>::difference_type
tcl::deque<T, Allocator>::iterator_impl<C>::operator-(
    const iterator_impl& rhs) const noexcept
{
	return ((deque_->capacity_ + pos_) - rhs.pos_) % deque_->capacity_;
}

template <typename T, typename Allocator>
template <bool C>
inline bool tcl::deque<T, Allocator>::iterator_impl<C>::operator==(
    const iterator_impl& rhs) const noexcept
{
	return deque_ == rhs.deque_ && (!deque_ || pos_ == rhs.pos_);
}

template <typename T, typename Allocator>
template <bool C>
inline std::partial_ordering
tcl::deque<T, Allocator>::iterator_impl<C>::operator<=>(
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
