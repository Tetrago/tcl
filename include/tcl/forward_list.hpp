#ifndef TCL_FORWARD_LIST_HPP
#define TCL_FORWARD_LIST_HPP

#include <initializer_list>
#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>

namespace tcl
{
	/**
	 * A forward linked list.
	 *
	 * All operations provide strong exception guarentees, or are otherwise
	 * marked as `noexcept`.
	 *
	 * @tparam T         The list element type.
	 * @tparam Allocator The @ref std::allocator_traits compliant allocator
	 * type.
	 */
	template <typename T, typename Allocator = std::allocator<T>>
	class forward_list
	{
	private:
		template <bool C>
		class iterator_impl;

		struct Node;

		using alloc_traits        = std::allocator_traits<Allocator>;
		using node_allocator_type = alloc_traits::template rebind_alloc<Node>;
		using node_alloc_traits   = std::allocator_traits<node_allocator_type>;

	public:
		using allocator_type = Allocator;
		using const_iterator = iterator_impl<true>;
		using iterator       = iterator_impl<false>;
		using size_type      = std::size_t;
		using value_type     = T;

		forward_list(const Allocator& alloc = Allocator{}) noexcept(
		    std::is_nothrow_constructible_v<Allocator, const Allocator&>);
		~forward_list() noexcept;

		forward_list(std::size_t size, const Allocator& alloc = Allocator{})
		    requires std::is_default_constructible_v<T>;

		forward_list(std::size_t size,
		             const T& value         = T{},
		             const Allocator& alloc = Allocator{})
		    requires(!std::is_default_constructible_v<T> &&
		             std::constructible_from<T, const T&>);

		template <std::input_iterator InputIt>
		forward_list(InputIt first,
		             InputIt last,
		             const Allocator& alloc = Allocator{})
		    requires std::constructible_from<T, std::iter_reference_t<InputIt>>;

		template <std::ranges::input_range R>
		forward_list(R&& r, const Allocator& alloc = Allocator{})
		    requires std::constructible_from<T,
		                                     std::ranges::range_reference_t<R>>;

		forward_list(std::initializer_list<T> init,
		             const Allocator& alloc = Allocator{})
		    requires std::constructible_from<T, const T&>;

		forward_list(const forward_list& other)
		    requires std::constructible_from<T, const T&>;
		forward_list& operator=(const forward_list& other)
		    requires std::constructible_from<T, const T&>;
		forward_list(forward_list&& other) noexcept;
		forward_list& operator=(forward_list&& other) noexcept(
		    node_alloc_traits::is_always_equal::value ||
		    node_alloc_traits::propagate_on_container_move_assignment::value);

		template <std::ranges::input_range R>
		    requires std::constructible_from<T,
		                                     std::ranges::range_reference_t<R>>
		forward_list& operator=(R&& r);

		template <typename U, typename A>
		[[nodiscard]] bool operator==(
		    const forward_list<U, A>& rhs) const noexcept
		    requires std::equality_comparable_with<T, U>;

		template <typename U, typename A>
		[[nodiscard]] bool operator!=(
		    const forward_list<U, A>& rhs) const noexcept
		    requires std::equality_comparable_with<T, U>;

		T& push_front(T&& value)
		    requires std::constructible_from<T, T&&>;
		T& push_front(const T& value)
		    requires std::constructible_from<T, const T&>;

		template <typename... Args>
		T& emplace_front(Args&&... args);

		T& insert_after(const_iterator pos, T&& value)
		    requires std::constructible_from<T, T&&>;
		T& insert_after(const_iterator pos, const T& value)
		    requires std::constructible_from<T, const T&>;

		template <std::input_iterator InputIt>
		void insert_after(const_iterator pos, InputIt first, InputIt last)
		    requires std::constructible_from<T, std::iter_reference_t<InputIt>>;

		template <std::ranges::input_range R>
		void insert_after(const_iterator pos, R&& r)
		    requires std::constructible_from<T,
		                                     std::ranges::range_reference_t<R>>;

		template <typename... Args>
		T& emplace_after(const_iterator pos, Args&&... args);

		void pop_front() noexcept
		    requires(!std::constructible_from<T, T &&>);

		T pop_front() noexcept(std::is_nothrow_move_constructible_v<T>)
		    requires std::constructible_from<T, T&&>;

		void erase_after(const_iterator pos) noexcept
		    requires(!std::constructible_from<T, T &&>);

		T erase_after(const_iterator pos) noexcept(
		    std::is_nothrow_move_constructible_v<T>)
		    requires std::constructible_from<T, T&&>;

		/**
		 * Erases elements in the given range.
		 *
		 * @param first One before the first element to erase.
		 * @param last  One past the last element to erase.
		 */
		void erase_after(const_iterator first, const_iterator last) noexcept;

		/**
		 * Replaces the existing list structure, truncating
		 * or expanding as necessary.
		 *
		 * @param first First element in assignment range.
		 * @param last  One past the last valid element in assignment range.
		 */
		template <std::input_iterator InputIt>
		void assign(InputIt first, std::sentinel_for<InputIt> auto last)
		    requires std::constructible_from<T,
		                                     std::iter_reference_t<InputIt>> ||
		             (std::is_default_constructible_v<T> &&
		              std::is_nothrow_assignable_v<
		                  T&,
		                  std::iter_reference_t<InputIt>>);

		template <std::ranges::input_range R>
		void assign(R&& r)
		    requires std::constructible_from<
		                 T,
		                 std::ranges::range_reference_t<R>> ||
		             (std::is_default_constructible_v<T> &&
		              std::is_nothrow_assignable_v<
		                  T&,
		                  std::ranges::range_reference_t<R>>);

		/**
		 * Splits all elements after the given iterator into a new distinct
		 * list.
		 *
		 * May perform copies if required by the allocator.
		 *
		 * @param pos The last element to retain in the current list.
		 *
		 * @return New list consisting of elements after `pos`.
		 */
		[[nodiscard]] forward_list split_after(const_iterator pos) noexcept(
		    node_alloc_traits::is_always_equal::value &&
		        noexcept(Allocator{}) ||
		    std::is_nothrow_constructible_v<T, const T&>);

		void resize(std::size_t n)
		    requires std::is_default_constructible_v<T>;

		void clear() noexcept;

		/**
		 * Reverses the list in O(n) time.
		 *
		 * @return Iterator for the new back element.
		 */
		iterator reverse() noexcept;

		/**
		 * Performs an in-place (non-moving) merge sort on the list.
		 */
		template <
		    std::strict_weak_order<const T&, const T&> Compare = std::less<>>
		void sort(Compare cmp = Compare{}) noexcept;

		/**
		 * Removes consecutive duplicate elements.
		 */
		template <std::equivalence_relation<const T&, const T&> Compare =
		              std::equal_to<>>
		void unique(Compare cmp = Compare{}) noexcept;

		template <std::equivalence_relation<const T&, const T&> Compare =
		              std::equal_to<>>
		void remove(const T& value, Compare cmp = Compare{}) noexcept;

		template <std::predicate<const T&> Pred>
		void remove_if(Pred pred) noexcept;

		/**
		 * Joins elements in `other` to the current list, removing them from
		 * `other`.
		 *
		 * May perform copies if required by the allocator.
		 *
		 * Will choose to perform copies over non-noexcept moves.
		 *
		 * @return The current list.
		 */
		forward_list& join(forward_list& other) noexcept(
		    node_alloc_traits::is_always_equal::value ||
		    std::is_nothrow_constructible_v<T, T&&>)
		    requires node_alloc_traits::is_always_equal::value
		             || std::is_nothrow_constructible_v<T, T&&> ||
		             std::constructible_from<T, const T&>;

		forward_list& swap(forward_list& other) noexcept;

		[[nodiscard]] std::size_t size() const noexcept { return size_; }

		[[nodiscard]] bool empty() const noexcept { return size_ == 0; }

		[[nodiscard]] iterator before_begin() noexcept
		    requires std::is_standard_layout_v<T>;

		[[nodiscard]] iterator begin() noexcept { return node_; }

		[[nodiscard]] iterator end() noexcept { return {}; }

		[[nodiscard]] const_iterator before_begin() const noexcept
		    requires std::is_standard_layout_v<T>;

		[[nodiscard]] const_iterator begin() const noexcept { return node_; }

		[[nodiscard]] const_iterator end() const noexcept { return {}; }

		[[nodiscard]] const_iterator cbefore_begin() const noexcept
		    requires std::is_standard_layout_v<T>;

		[[nodiscard]] const_iterator cbegin() const noexcept { return node_; }

		[[nodiscard]] const_iterator cend() const noexcept { return {}; }

		[[nodiscard]] T& front() noexcept { return node_->value; }

		[[nodiscard]] const T& front() const noexcept { return node_->value; }

		[[nodiscard]] Allocator get_allocator() const noexcept
		{
			return alloc_;
		}

	private:
		forward_list(Node* node,
		             std::size_t size) noexcept(noexcept(Allocator{}))
		    requires alloc_traits::is_always_equal::value;

		/**
		 * Merge sorts [`first`, `first` + `n`) elements.
		 *
		 * @param first First element in sort range.
		 * @param n     Number of elements in the range starting from `first`.
		 * @param cmp   Compare function for sorting.
		 * @return      Include range of [first, before_last] elements.
		 */
		template <std::strict_weak_order<const T&, const T&> Compare>
		static std::pair<const_iterator, const_iterator> range_sort(
		    const_iterator first, std::size_t n, Compare cmp) noexcept;

		/**
		 * Exception-safe wrapper around the allocator.
		 */
		template <typename... Args>
		[[nodiscard]] Node* create_node(Node* next, Args&&... args);

		/**
		 * Forced noexcept wrapper around the allocator.
		 */
		void destroy_node(Node* node) noexcept;

		[[no_unique_address]] node_allocator_type alloc_;
		Node* node_       = nullptr;
		std::size_t size_ = 0;
	};

	template <typename T, typename Allocator>
	template <bool C>
	class forward_list<T, Allocator>::iterator_impl
	{
	public:
		using difference_type   = std::ptrdiff_t;
		using iterator_category = std::forward_iterator_tag;
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
		    : node_(other.node_)
		{}

		pointer operator->() const noexcept { return &node_->value; }

		[[nodiscard]] reference operator*() const noexcept
		{
			return node_->value;
		}

		iterator_impl& operator++() noexcept;
		[[nodiscard]] iterator_impl operator++(int) noexcept;

		[[nodiscard]] bool operator==(const iterator_impl&) const noexcept =
		    default;

	private:
		friend forward_list<T, Allocator>;

		iterator_impl(Node* node) noexcept
		    : node_(node)
		{}

		Node* node_ = nullptr;
	};

	template <typename T, typename Allocator>
	struct forward_list<T, Allocator>::Node
	{
		template <typename... Args>
		Node(Node* next, Args&&... args)
		    : next(next), value(std::forward<Args>(args)...)
		{}

		Node* next;
		T value;
	};
} // namespace tcl

static_assert(std::ranges::range<tcl::forward_list<int>>);
static_assert(std::ranges::forward_range<tcl::forward_list<int>>);
static_assert(std::ranges::common_range<tcl::forward_list<int>>);

static_assert(std::input_iterator<tcl::forward_list<int>::iterator>);
static_assert(std::output_iterator<tcl::forward_list<int>::iterator, int>);
static_assert(std::forward_iterator<tcl::forward_list<int>::iterator>);
static_assert(!std::bidirectional_iterator<tcl::forward_list<int>::iterator>);
static_assert(std::input_iterator<tcl::forward_list<int>::const_iterator>);
static_assert(
    !std::output_iterator<tcl::forward_list<int>::const_iterator, int>);

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>::
    forward_list(const Allocator& alloc) noexcept(
        std::is_nothrow_constructible_v<Allocator, const Allocator&>)
    : alloc_(alloc)
{}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>::forward_list(
    Node* node, std::size_t size) noexcept(noexcept(Allocator{}))
    requires alloc_traits::is_always_equal::value
    : node_(node), size_(size), alloc_(Allocator{})
{}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>::~forward_list() noexcept
{
	clear();
}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>::forward_list(std::size_t size,
                                                     const Allocator& alloc)
    requires std::is_default_constructible_v<T>
    : alloc_(alloc)
{
	try
	{
		while (size_ < size)
		{
			Node* node = create_node(node_);
			node_      = node;
			++size_;
		}
	}
	catch (...)
	{
		clear();
		throw;
	}
}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>::forward_list(std::size_t size,
                                                     const T& value,
                                                     const Allocator& alloc)
    requires(!std::is_default_constructible_v<T> &&
             std::constructible_from<T, const T&>)
    : alloc_(alloc)
{
	try
	{
		while (size_ < size)
		{
			Node* node = create_node(node_, value);
			node_      = node;
			++size_;
		}
	}
	catch (...)
	{
		clear();
		throw;
	}
}

template <typename T, typename Allocator>
template <std::input_iterator InputIt>
inline tcl::forward_list<T, Allocator>::forward_list(InputIt first,
                                                     InputIt last,
                                                     const Allocator& alloc)
    requires std::constructible_from<T, std::iter_reference_t<InputIt>>
    : alloc_(alloc)
{
	assign(first, last);
}

template <typename T, typename Allocator>
template <std::ranges::input_range R>
inline tcl::forward_list<T, Allocator>::forward_list(R&& r,
                                                     const Allocator& alloc)
    requires std::constructible_from<T, std::ranges::range_reference_t<R>>
    : alloc_(alloc)
{
	assign(std::forward<R>(r));
}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>::forward_list(
    std::initializer_list<T> init, const Allocator& alloc)
    requires std::constructible_from<T, const T&>
    : alloc_(alloc)
{
	assign(init.begin(), init.end());
}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>::forward_list(const forward_list& other)
    requires std::constructible_from<T, const T&>
    : alloc_(node_alloc_traits::select_on_container_copy_construction(
          other.alloc_))
{
	assign(other.begin(), other.end());
}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>&
tcl::forward_list<T, Allocator>::operator=(const forward_list& other)
    requires std::constructible_from<T, const T&>
{
	if (this != &other)
	{
		if constexpr (node_alloc_traits::
		                  propagate_on_container_copy_assignment::value)
		{
			if (alloc_ != other.alloc_)
			{
				clear();
				alloc_ = other.alloc_;
			}
		}

		assign(other.begin(), other.end());
	}

	return *this;
}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>::forward_list(
    forward_list&& other) noexcept
    : node_(std::exchange(other.node_, nullptr)),
      size_(std::exchange(other.size_, 0)),
      alloc_(std::move(other.alloc_))
{}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>&
tcl::forward_list<T, Allocator>::operator=(forward_list&& other) noexcept(
    node_alloc_traits::is_always_equal::value ||
    node_alloc_traits::propagate_on_container_move_assignment::value)
{
	if (this != &other)
	{
		if (alloc_ == other.alloc_)
		{
			std::swap(node_, other.node_);
			std::swap(size_, other.size_);
		}
		else if constexpr (node_alloc_traits::
		                       propagate_on_container_move_assignment::value)
		{
			clear();
			alloc_ = std::move(other.alloc_);
			std::swap(node_, other.node_);
			std::swap(size_, other.size_);
		}
		else
		{
			Node* prev = node_;

			try
			{
				Node* node = nullptr;

				for (iterator it = other.begin(); it != other.end(); ++it)
				{
					Node* next = create_node(nullptr, std::move(*it));

					if (node == nullptr)
					{
						node_ = next;
					}
					else
					{
						node->next = next;
					}

					node = next;
				}

				size_ = other.size_;
				other.clear();

				while (prev)
				{
					destroy_node(std::exchange(prev, prev->next));
				}
			}
			catch (...)
			{
				while (node_)
				{
					destroy_node(std::exchange(node_, node_->next));
				}

				node_ = prev;
				throw;
			}
		}
	}

	return *this;
}

template <typename T, typename Allocator>
template <std::ranges::input_range R>
    requires std::constructible_from<T, std::ranges::range_reference_t<R>>
inline tcl::forward_list<T, Allocator>&
tcl::forward_list<T, Allocator>::operator=(R&& r)
{
	assign(std::forward<R>(r));
	return *this;
}

template <typename T, typename Allocator>
template <typename U, typename A>
inline bool tcl::forward_list<T, Allocator>::operator==(
    const forward_list<U, A>& rhs) const noexcept
    requires std::equality_comparable_with<T, U>
{
	if (this == &rhs)
	{
		return true;
	}
	else if (size_ != rhs.size_)
	{
		return false;
	}
	else
	{
		const_iterator l = begin();
		const_iterator r = rhs.begin();

		while (l != end())
		{
			if (*(l++) != *(r++))
			{
				return false;
			}
		}

		return true;
	}
}

template <typename T, typename Allocator>
template <typename U, typename A>
inline bool tcl::forward_list<T, Allocator>::operator!=(
    const forward_list<U, A>& rhs) const noexcept
    requires std::equality_comparable_with<T, U>
{
	return !(*this == rhs);
}

template <typename T, typename Allocator>
inline T& tcl::forward_list<T, Allocator>::push_front(T&& value)
    requires std::constructible_from<T, T&&>
{
	Node* node = create_node(node_, std::move(value));
	node_      = node;
	++size_;
	return node->value;
}

template <typename T, typename Allocator>
inline T& tcl::forward_list<T, Allocator>::push_front(const T& value)
    requires std::constructible_from<T, const T&>
{
	Node* node = create_node(node_, value);
	node_      = node;
	++size_;
	return node->value;
}

template <typename T, typename Allocator>
template <typename... Args>
inline T& tcl::forward_list<T, Allocator>::emplace_front(Args&&... args)
{
	Node* node = create_node(node_, std::forward<Args>(args)...);
	node_      = node;
	++size_;
	return node->value;
}

template <typename T, typename Allocator>
inline T& tcl::forward_list<T, Allocator>::insert_after(const_iterator pos,
                                                        T&& value)
    requires std::constructible_from<T, T&&>
{
	Node* node      = create_node(pos.node_->next, std::move(value));
	pos.node_->next = node;
	++size_;
	return node->value;
}

template <typename T, typename Allocator>
inline T& tcl::forward_list<T, Allocator>::insert_after(const_iterator pos,
                                                        const T& value)
    requires std::constructible_from<T, const T&>
{
	Node* node      = create_node(pos.node_->next, value);
	pos.node_->next = node;
	++size_;
	return node->value;
}

template <typename T, typename Allocator>
template <std::input_iterator InputIt>
inline void tcl::forward_list<T, Allocator>::insert_after(const_iterator pos,
                                                          InputIt first,
                                                          InputIt last)
    requires std::constructible_from<T, std::iter_reference_t<InputIt>>
{
	if (first == last) [[unlikely]]
	{
		return;
	}

	InputIt it = first;

	try
	{
		const_iterator p = pos;

		while (it != last)
		{
			insert_after(p, *it);
			++p;
			++it;
		}
	}
	catch (...)
	{
		while (pos.node_->next)
		{
			erase_after(pos);
		}

		throw;
	}
}

template <typename T, typename Allocator>
template <std::ranges::input_range R>
inline void tcl::forward_list<T, Allocator>::insert_after(const_iterator pos,
                                                          R&& r)
    requires std::constructible_from<T, std::ranges::range_reference_t<R>>
{
	try
	{
		const_iterator p = pos;

		for (auto&& value : r)
		{
			insert_after(p, std::forward<decltype(value)>(value));
			++p;
		}
	}
	catch (...)
	{
		while (pos.node_->next)
		{
			erase_after(pos);
		}

		throw;
	}
}

template <typename T, typename Allocator>
template <typename... Args>
inline T& tcl::forward_list<T, Allocator>::emplace_after(const_iterator pos,
                                                         Args&&... args)
{
	Node* node      = create_node(pos.node_->next, std::forward<Args>(args)...);
	pos.node_->next = node;
	++size_;
	return node->value;
}

template <typename T, typename Allocator>
inline void tcl::forward_list<T, Allocator>::pop_front() noexcept
    requires(!std::constructible_from<T, T &&>)
{
	destroy_node(std::exchange(node_, node_->next));
	--size_;
}

template <typename T, typename Allocator>
inline T tcl::forward_list<T, Allocator>::pop_front() noexcept(
    std::is_nothrow_move_constructible_v<T>)
    requires std::constructible_from<T, T&&>
{
	T value = std::move(node_->value);
	destroy_node(std::exchange(node_, node_->next));
	--size_;
	return value;
}

template <typename T, typename Allocator>
inline void tcl::forward_list<T, Allocator>::erase_after(
    const_iterator pos) noexcept
    requires(!std::constructible_from<T, T &&>)
{
	destroy_node(std::exchange(pos.node_->next, pos.node_->next->next));
	--size_;
}

template <typename T, typename Allocator>
inline T tcl::forward_list<T, Allocator>::erase_after(
    const_iterator pos) noexcept(std::is_nothrow_move_constructible_v<T>)
    requires std::constructible_from<T, T&&>
{
	T value = std::move(pos.node_->next->value);
	destroy_node(std::exchange(pos.node_->next, pos.node_->next->next));
	--size_;
	return value;
}

template <typename T, typename Allocator>
inline void tcl::forward_list<T, Allocator>::erase_after(
    const_iterator first, const_iterator last) noexcept
{
	while (std::next(first) != last)
	{
		erase_after(first);
	}
}

template <typename T, typename Allocator>
template <std::input_iterator InputIt>
inline void tcl::forward_list<T, Allocator>::assign(
    InputIt first, std::sentinel_for<InputIt> auto last)
    requires std::constructible_from<T, std::iter_reference_t<InputIt>> ||
             (std::is_default_constructible_v<T> &&
              std::is_nothrow_assignable_v<T&, std::iter_reference_t<InputIt>>)
{
	if (first == last) [[unlikely]]
	{
		clear();
		return;
	}

	if constexpr (std::is_default_constructible_v<T> &&
	              std::is_nothrow_assignable_v<T&,
	                                           std::iter_reference_t<InputIt>>)
	{
		resize(std::distance(first, last));

		for (iterator it = begin(); it != end(); ++it, ++first)
		{
			*it = *first;
		}
	}
	else
	{
		Node* node       = std::exchange(node_, nullptr);
		std::size_t size = std::exchange(size_, 0);

		try
		{
			push_front(*(first++));

			for (const_iterator it = begin(); first != last; ++it)
			{
				insert_after(it, *(first++));
			}

			while (node)
			{
				destroy_node(std::exchange(node, node->next));
			}
		}
		catch (...)
		{
			while (node_)
			{
				destroy_node(std::exchange(node_, node_->next));
			}

			node_ = node;
			size_ = size;

			throw;
		}
	}
}

template <typename T, typename Allocator>
template <std::ranges::input_range R>
inline void tcl::forward_list<T, Allocator>::assign(R&& r)
    requires std::constructible_from<T, std::ranges::range_reference_t<R>> ||
             (std::is_default_constructible_v<T> &&
              std::is_nothrow_assignable_v<T&,
                                           std::ranges::range_reference_t<R>>)
{
	if constexpr (std::ranges::sized_range<R> &&
	              std::is_default_constructible_v<T> &&
	              std::is_nothrow_assignable_v<
	                  T&,
	                  std::ranges::range_reference_t<R>>)
	{
		resize(std::ranges::size(r));

		iterator it = begin();

		for (auto&& value : r)
		{
			*(it++) = std::forward<decltype(value)>(value);
		}
	}
	else
	{
		Node* node       = std::exchange(node_, nullptr);
		std::size_t size = std::exchange(size_, 0);

		try
		{
			const_iterator it = end();

			for (auto&& value : r)
			{
				if (size_ == 0)
				{
					push_front(std::forward<decltype(value)>(value));
					it = begin();
				}
				else
				{
					insert_after(it, std::forward<decltype(value)>(value));
					++it;
				}
			}

			while (node)
			{
				destroy_node(std::exchange(node, node->next));
			}
		}
		catch (...)
		{
			while (node_)
			{
				destroy_node(std::exchange(node_, node_->next));
			}

			node_ = node;
			size_ = size;

			throw;
		}
	}
}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>
tcl::forward_list<T, Allocator>::split_after(const_iterator pos) noexcept(
    node_alloc_traits::is_always_equal::value && noexcept(Allocator{}) ||
    std::is_nothrow_constructible_v<T, const T&>)
{
	if (pos == end()) [[unlikely]]
	{
		return forward_list();
	}
	else if constexpr (node_alloc_traits::is_always_equal::value)
	{
		auto size  = std::distance(std::next(pos), cend());
		size_     -= size;

		Node* node = std::exchange(pos.node_->next, nullptr);
		return forward_list(node, size);
	}
	else
	{
		forward_list list(std::next(pos), end());
		erase_after(std::next(pos), end());
		return std::move(list);
	}
}

template <typename T, typename Allocator>
inline void tcl::forward_list<T, Allocator>::resize(std::size_t n)
    requires std::is_default_constructible_v<T>
{
	if (n == 0) [[unlikely]]
	{
		clear();
	}
	else if (size_ < n)
	{
		Node* tail = node_;
		while (tail && tail->next) tail = tail->next;

		try
		{
			Node* node = tail;

			for (std::size_t i = size_; i < n; ++i)
			{
				if (node) [[likely]]
				{
					node->next = create_node(nullptr);
					node       = node->next;
				}
				else
				{
					node_ = create_node(nullptr);
					node  = node_;
				}
			}

			size_ = n;
		}
		catch (...)
		{
			if (size_ == 0)
			{
				tail  = node_;
				node_ = nullptr;
			}
			else
			{
				tail = std::exchange(tail->next, nullptr);
			}

			while (tail)
			{
				delete std::exchange(tail, tail->next);
			}

			throw;
		}
	}
	else if (size_ > n)
	{
		size_ = n;

		Node* node = node_;
		while (--n) node = node->next;
		node = std::exchange(node->next, nullptr);

		while (node)
		{
			delete std::exchange(node, node->next);
		}
	}
}

template <typename T, typename Allocator>
inline void tcl::forward_list<T, Allocator>::clear() noexcept
{
	while (node_)
	{
		delete std::exchange(node_, node_->next);
	}

	size_ = 0;
}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>::iterator
tcl::forward_list<T, Allocator>::reverse() noexcept
{
	iterator it = begin();

	Node* prev = nullptr;
	Node* next = nullptr;

	while (node_)
	{
		next = std::exchange(node_->next, prev);
		prev = std::exchange(node_, next);
	}

	node_ = prev;
	return it;
}

template <typename T, typename Allocator>
template <std::strict_weak_order<const T&, const T&> Compare>
inline void tcl::forward_list<T, Allocator>::sort(Compare cmp) noexcept
{
	if (size_ > 1)
	{
		node_ = range_sort(begin(), size_, cmp).first.node_;
	}
}

template <typename T, typename Allocator>
template <std::equivalence_relation<const T&, const T&> Compare>
inline void tcl::forward_list<T, Allocator>::unique(Compare cmp) noexcept
{
	if (size_ > 1)
	{
		const_iterator first  = begin();
		const_iterator second = std::next(begin());

		while (second != end())
		{
			if (cmp(*first, *second))
			{
				erase_after(first);
				second = std::next(first);
			}
			else
			{
				++first;
				++second;
			}
		}
	}
}

template <typename T, typename Allocator>
template <std::equivalence_relation<const T&, const T&> Compare>
inline void tcl::forward_list<T, Allocator>::remove(const T& value,
                                                    Compare cmp) noexcept
{
	Node* node = node_;

	while (node)
	{
		if (cmp(value, node->value))
		{
			if (node_ == node)
			{
				node_ = node->next;
			}

			destroy_node(std::exchange(node, node->next));
			--size_;
		}
		else
		{
			node = node->next;
		}
	}
}

template <typename T, typename Allocator>
template <std::predicate<const T&> Pred>
inline void tcl::forward_list<T, Allocator>::remove_if(Pred pred) noexcept
{
	Node* node = node_;

	while (node)
	{
		if (pred(node->value))
		{
			if (node_ == node)
			{
				node_ = node->next;
			}

			destroy_node(std::exchange(node, node->next));
			--size_;
		}
		else
		{
			node = node->next;
		}
	}
}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>& tcl::forward_list<T, Allocator>::join(
    forward_list& other) noexcept(node_alloc_traits::is_always_equal::value ||
                                  std::is_nothrow_constructible_v<T, T&&>)
    requires node_alloc_traits::is_always_equal::value
             || std::is_nothrow_constructible_v<T, T&&> ||
             std::constructible_from<T, const T&>
{
	if (size_ == 0) [[unlikely]]
	{
		*this = std::move(other);
	}
	else if (alloc_ == other.alloc_)
	{
		Node* node = node_;
		while (node->next) node = node->next;

		node->next  = std::exchange(other.node_, nullptr);
		size_      += std::exchange(other.size_, 0);
	}
	else if constexpr (std::is_nothrow_constructible_v<T, T&&>)
	{
		const_iterator before_end = std::next(begin(), size_ - 1);

		for (iterator it = other.begin(); it != other.end(); ++it)
		{
			insert_after(before_end, std::move(*it));
			++before_end;
		}

		other.clear();
	}
	else
	{
		const_iterator before_end = std::next(begin(), size_ - 1);
		const_iterator tail       = before_end;

		try
		{
			for (const_iterator it = other.begin(); it != other.end(); ++it)
			{
				insert_after(before_end, *it);
				++before_end;
			}

			other.clear();
		}
		catch (...)
		{
			erase_after(tail, end());
			throw;
		}
	}

	return *this;
}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>& tcl::forward_list<T, Allocator>::swap(
    forward_list& other) noexcept
{
	return *this = std::move(other);
}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>::iterator
tcl::forward_list<T, Allocator>::before_begin() noexcept
    requires std::is_standard_layout_v<T>
{
	static_assert(offsetof(Node, next) == 0);
	return reinterpret_cast<Node*>(&node_);
}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>::const_iterator
tcl::forward_list<T, Allocator>::before_begin() const noexcept
    requires std::is_standard_layout_v<T>
{
	return cbefore_begin();
}

template <typename T, typename Allocator>
inline tcl::forward_list<T, Allocator>::const_iterator
tcl::forward_list<T, Allocator>::cbefore_begin() const noexcept
    requires std::is_standard_layout_v<T>
{
	static_assert(offsetof(Node, next) == 0);
	return const_cast<Node*>(reinterpret_cast<const Node*>(&node_));
}

template <typename T, typename Allocator>
template <std::strict_weak_order<const T&, const T&> Compare>
inline std::pair<typename tcl::forward_list<T, Allocator>::const_iterator,
                 typename tcl::forward_list<T, Allocator>::const_iterator>
tcl::forward_list<T, Allocator>::range_sort(const_iterator first,
                                            std::size_t n,
                                            Compare cmp) noexcept
{
	if (n == 1)
	{
		return std::make_pair(first, first);
	}
	else if (n == 2)
	{
		const_iterator next = std::next(first);
		if (!cmp(*first, *next))
		{
			first.node_->next = std::exchange(next.node_->next, first.node_);
			std::swap(first, next);
		}

		return std::make_pair(first, next);
	}
	else
	{
		auto [front, left] = range_sort(first, n / 2, cmp);
		auto [right, back] = range_sort(std::next(left), (n + 1) / 2, cmp);

		left.node_->next = nullptr;
		Node* end        = std::exchange(back.node_->next, nullptr);

		Node* head = cmp(*front, *right)
		                 ? std::exchange(front.node_, front.node_->next)
		                 : std::exchange(right.node_, right.node_->next);
		Node* tail = head;

		while (--n)
		{
			tail->next = [&]() {
				if (front.node_ && right.node_)
				{
					return cmp(*front, *right)
					           ? std::exchange(front.node_, front.node_->next)
					           : std::exchange(right.node_, right.node_->next);
				}
				else if (front.node_)
				{
					return std::exchange(front.node_, front.node_->next);
				}
				else
				{
					return std::exchange(right.node_, right.node_->next);
				}
			}();

			tail = tail->next;
		}

		tail->next = end;
		return std::make_pair<const_iterator, const_iterator>(head, tail);
	}
}

template <typename T, typename Allocator>
template <typename... Args>
inline tcl::forward_list<T, Allocator>::Node*
tcl::forward_list<T, Allocator>::create_node(Node* next, Args&&... args)
{
	Node* node = node_alloc_traits::allocate(alloc_, 1);

	try
	{
		node_alloc_traits::construct(
		    alloc_, node, next, std::forward<Args>(args)...);
		return node;
	}
	catch (...)
	{
		node_alloc_traits::deallocate(alloc_, node, 1);
		throw;
	}
}

template <typename T, typename Allocator>
inline void tcl::forward_list<T, Allocator>::destroy_node(Node* node) noexcept
{
	node_alloc_traits::destroy(alloc_, node);
	node_alloc_traits::deallocate(alloc_, node, 1);
}

template <typename T, typename Allocator>
template <bool C>
inline tcl::forward_list<T, Allocator>::iterator_impl<C>&
tcl::forward_list<T, Allocator>::iterator_impl<C>::operator++() noexcept
{
	node_ = node_->next;
	return *this;
}

template <typename T, typename Allocator>
template <bool C>
inline tcl::forward_list<T, Allocator>::iterator_impl<C>
tcl::forward_list<T, Allocator>::iterator_impl<C>::operator++(int) noexcept
{
	iterator_impl it = *this;
	++(*this);
	return it;
}

#endif
