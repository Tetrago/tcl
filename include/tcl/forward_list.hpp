#ifndef TCL_FORWARD_LIST_HPP
#define TCL_FORWARD_LIST_HPP

#include <initializer_list>
#include <iterator>
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
		using value_type     = T;
		using size_type      = std::size_t;
		using iterator       = iterator_impl<false>;
		using const_iterator = iterator_impl<true>;
		using allocator_type = Allocator;

		forward_list(const Allocator& alloc = Allocator{}) noexcept(
		    noexcept(Allocator{alloc}));

		~forward_list() noexcept;

		forward_list(std::size_t size, const Allocator& alloc = Allocator{})
		    requires std::constructible_from<T>;

		forward_list(std::size_t size,
		             const T& value         = T{},
		             const Allocator& alloc = Allocator{})
		    requires(!std::constructible_from<T> &&
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
		    requires std::move_constructible<T>;
		T& push_front(const T& value)
		    requires std::constructible_from<T, const T&>;

		template <typename... Args>
		T& emplace_front(Args&&... args);

		T& insert_after(const_iterator pos, T&& value)
		    requires std::move_constructible<T>;
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
		    requires(!std::move_constructible<T>);

		T pop_front() noexcept(std::is_nothrow_move_constructible_v<T>)
		    requires std::move_constructible<T>;

		void erase_after(const_iterator pos) noexcept
		    requires(!std::move_constructible<T>);

		T erase_after(const_iterator pos) noexcept(
		    std::is_nothrow_move_constructible_v<T>)
		    requires std::move_constructible<T>;

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
		             (std::constructible_from<T> &&
		              std::is_nothrow_assignable_v<
		                  T&,
		                  std::iter_reference_t<InputIt>>);

		template <std::ranges::input_range R>
		void assign(R&& r)
		    requires std::constructible_from<
		                 T,
		                 std::ranges::range_reference_t<R>> ||
		             (std::constructible_from<T> &&
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
		    std::is_nothrow_copy_constructible_v<T>);

		void resize(std::size_t n)
		    requires std::constructible_from<T>;

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
		    std::is_nothrow_move_constructible_v<T>)
		    requires node_alloc_traits::is_always_equal::value
		             || std::is_nothrow_move_constructible_v<T> ||
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

		[[nodiscard]] allocator_type get_allocator() const noexcept
		{
			return typename node_alloc_traits::template rebind_alloc<T>(alloc_);
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
		using iterator_category = std::forward_iterator_tag;
		using difference_type   = std::ptrdiff_t;
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

		[[nodiscard]] pointer operator->() const noexcept
		{
			return &node_->value;
		}

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

#ifndef TCL_FORWARD_LIST_IPP
	#include "forward_list.ipp"
#endif

#endif
