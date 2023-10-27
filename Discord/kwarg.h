#ifndef KWARG_H
#define KWARG_H
#include <concepts>
#include <cstddef>
#include <exception>
#include <new>
#include <utility>

namespace kw {
	/* An exception to be thrown when accessing an arg with no value; similar to std::bad_optional_access */
	class bad_arg_access : public std::exception {
	public:
		~bad_arg_access() override = default;

		const char* what() const noexcept override {
			return "bad_arg_access";
		}
	};
	/* A constexpr string that allows string literals to be used as template parameters */
	template<std::size_t Size>
	struct key {
		char value[Size];

		consteval key(const char(&str)[Size]) noexcept {
			for (std::size_t i = 0; i < Size; ++i)
				value[i] = str[i];
		}
	};
	/* The nullarg keyword; similar to std::nullopt */
	struct nullarg_t {
		explicit consteval nullarg_t(int) noexcept {}
	};
	inline constexpr nullarg_t nullarg(0);
	/* Customization point for associating strings to types */
	template<key Key> struct arg_type;
	template<key Key> using arg_type_t = typename arg_type<Key>::value_type;
	/* The arg type */
	template<key Key>
	class arg final {
	public:
		/* The type associated with the arg */
		using value_type = arg_type_t<Key>;
		/* Destructor */
		~arg() { reset(); }
		/* Disable copying */
		arg(const arg&) = delete;
		arg& operator=(const arg&) = delete;
		/* Assignment of values */
		template<typename Arg = value_type> requires std::constructible_from<value_type, Arg&&>
		arg& operator=(Arg&& arg) {
			emplace(std::forward<Arg>(arg));
			return *this;
		}
		arg& operator=(std::initializer_list<int> l) requires std::constructible_from<value_type, std::initializer_list<int>> {
			emplace(l);
			return *this;
		}
		arg& operator=(nullarg_t) noexcept {
			reset();
			return *this;
		}
		/* Static method returning the unique instance of the arg */
		static arg& instance() noexcept { return m_instance; }
	private:
		/* The unique instance */
		static arg m_instance;
		/* Pointer to the held value; nullptr if the arg has no value */
		value_type* m_value;
		/* The buffer that holds the actual value */
		alignas(alignof(value_type)) std::byte m_buff[sizeof(value_type)];
		/* The private constructor; the constructed arg holds no value */
		explicit arg(nullarg_t) noexcept : m_value(nullptr) {}
		/* reset - set the arg to hold no value */
		void reset() noexcept {
			if (m_value) {
				m_value->~value_type();
				m_value = nullptr;
			}
		}
		/* emplace - initialize a new value */
		template<typename... Args>
		auto& emplace(Args&&... args) {
			reset();
			return *(m_value = new(m_buff) value_type(std::forward<Args>(args)...));
		}
		/* Enable access to private members for the getter class */
		template<key, key...> friend struct getter;
	};
	/* The type that holds a collection of keys, through which values can be retrieved */
	template<key... Keys>
	struct pack {
		pack() = default;
		explicit pack(arg<Keys>&...) {}
	};
	template<>
	struct pack<> {};
	/* The getter class - by default the pack is empty */
	template<key Key, key... Keys>
	struct getter {
		template<typename... Args>
		static arg_type_t<Key>& get(Args&&... args) {
			return arg<Key>::instance().emplace(std::forward<Args>(args)...);
		}
		static arg_type_t<Key>& get(nullarg_t) {
			throw bad_arg_access();
		}
		template<typename... Args>
		static arg_type_t<Key>* get_if(Args&&... args) {
			return &get(std::forward<Args>(args)...);
		}
		static arg_type_t<Key>* get_if(nullarg_t) {
			return nullptr;
		}
	};
	/* The getter class - specialization for then the user key is found in the non-empty pack */
	template<key Key, key... Keys>
	struct getter<Key, Key, Keys...> {
		template<typename... Args>
		static arg_type_t<Key>& get(Args&&...) {
			if (auto p = arg<Key>::instance().m_value; p)
				return *p;
			throw bad_arg_access();
		}
		template<typename... Args>
		static arg_type_t<Key>* get_if(Args&&...) {
			return arg<Key>::instance().m_value;
		}
	};
	/* The getter class - specialization for when the user key isn't found in the non-empty pack */
	template<key Key, key First, key... Rest>
	struct getter<Key, First, Rest...> {
		template<typename... Args>
		static arg_type_t<Key>& get(Args&&... args) {
			return getter<Key, Rest...>::get(std::forward<Args>(args)...);
		}
		template<typename... Args>
		static arg_type_t<Key>* get_if(Args&&... args) {
			return getter<Key, Rest...>::get_if(std::forward<Args>(args)...);
		}
	};
	/* Top level get and get_if functions with appropriate argument checks */
	template<key Key, key... Keys>
	requires std::default_initializable<arg_type_t<Key>>
	inline arg_type_t<Key>* get_if(pack<Keys...> p) {
		return getter<Key, Keys...>::get_if();
	}
	template<key Key, key... Keys, typename Arg = arg_type_t<Key>, typename... Args>
	requires std::constructible_from<arg_type_t<Key>, Arg&&, Args&&...>
	inline arg_type_t<Key>* get_if(pack<Keys...> p, Arg&& arg, Args&&... args) {
		return getter<Key, Keys...>::get_if(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<key Key, key... Keys>
	inline arg_type_t<Key>* get_if(pack<Keys...> p, nullarg_t n) {
		return getter<Key, Keys...>::get_if(n);
	}
	template<key Key, key... Keys>
	requires std::default_initializable<arg_type_t<Key>>
	inline arg_type_t<Key>& get(pack<Keys...> p) {
		return getter<Key, Keys...>::get();
	}
	template<key Key, key... Keys, typename Arg = arg_type_t<Key>, typename... Args>
	requires std::constructible_from<arg_type_t<Key>, Arg&&, Args&&...>
	inline arg_type_t<Key>& get(pack<Keys...> p, Arg&& arg, Args&&... args) {
		return getter<Key, Keys...>::get(std::forward<Arg>(arg), std::forward<Args>(args)...);
	}
	template<key Key, key... Keys>
	inline arg_type_t<Key>& get(pack<Keys...> p, nullarg_t n) {
		return getter<Key, Keys...>::get(n);
	}
}
/* The macro which declares a kwarg */
#define KW_DECLARE(name, type) \
namespace kw { \
    template<> struct arg_type<#name> { using value_type = type; }; \
    template<> inline arg<#name> arg<#name>::m_instance{ nullarg }; \
    inline auto& name = arg<#name>::instance(); \
}
#endif // KWARG_H