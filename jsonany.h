#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <typeindex>
#include "static_json.h"

using namespace std;
using namespace static_json;

namespace json {

// template<class T>
// struct data {
// 	T value;
// };

// template<class Archive>
// void serialize(Archive& ar, data& a) {
// 	ar	& JSON_NI_SERIALIZATION_NVP(a, value);
// }

//template<class D>
class any
{
	//friend std::ostream& operator<< (std::ostream&, const any&);
	public:
		any() : m_tpIndex(std::type_index(typeid(void))) {}
		any(const any& other) : m_ptr(other.clone()), m_tpIndex(other.m_tpIndex) {}
		any(any&& other) : m_ptr(std::move(other.m_ptr)), m_tpIndex(std::move(other.m_tpIndex)) {}

		//通用的右值构造
		//template<typename U, class = typename std::enable_if<!std::is_same<typename std::decay<U>::type, any>::value, U>::type> 
		template<class T, class = typename std::enable_if<!std::is_same<typename std::decay<T>::type, any>::value, T>::type>
		any(T&& t)
			: m_ptr(new Derived<typename std::decay<T>::type>(std::forward<T>(t)))
			  //m_ptr(new Derived < typename std::decay<U>::type>(forward<U>(value)))
			, m_tpIndex(typeid(typename std::decay<T>::type)) {
				// cout << "=====> " << typeid(T).name() << " to "
				// 	<< m_tpIndex.name() << endl;
			}

		bool isNull()
		{
			return !bool(m_ptr);
		}

		template<class T>
		bool is()
		{
			return m_tpIndex == type_index(typeid(T));
		}

		template<class T>
		T& anycast()
		{
			if (!is<T>())
			{
				cout << "can not cast " << typeid(T).name() << " to "
					<< m_tpIndex.name() << endl;
				throw bad_cast();
			}
			auto ptr = dynamic_cast<Derived<T>*>(m_ptr.get());
			return ptr->m_value;
		}

		template<class Archive>
		friend void serialize(Archive& ar, any& a)
		{
			//ar	& JSON_NI_SERIALIZATION_NVP(a, m_ptr);
		}

    	template<typename U, class = typename std::enable_if<!std::is_same<typename std::decay<U>::type, any>::value, U>::type> 
		static inline std::string dump(U && value) {
			auto ptr = new Derived<typename std::decay<U>::type>(forward<U>(value));
        	//auto m_tpIndex = type_index(typeid(typename std::decay<U>::type));
			return to_json_string(ptr->m_value);
		}

		template<typename U, class = typename std::enable_if<!std::is_same<typename std::decay<U>::type, any>::value, U>::type>
		static inline U& parse(U && value,  std::string json_str) {
			auto ptr = new Derived<typename std::decay<U>::type>(forward<U>(value));
        	//auto m_tpIndex = type_index(typeid(typename std::decay<U>::type));
			from_json_string(ptr->m_value, json_str);
			return ptr->m_value;
		}

		any& operator=(const any& other)
		{
			if (m_ptr == other.m_ptr)
			{
				return *this;
			}
			m_ptr = other.clone();
			m_tpIndex = other.m_tpIndex;
			return *this;
		}

	private:
		struct Base;
		using BasePtr = std::unique_ptr<Base>;

		struct Base
		{
			virtual BasePtr clone() const = 0;
		};

		template<typename T>
		struct Derived : public Base
		{
			template<typename...Args>
			Derived(Args&&...args) : m_value(std::forward<Args>(args)...)
			{
			}
			BasePtr clone() const
			{
				return BasePtr(new Derived(m_value));
			}
			T m_value;
			// template<class Archive>
			// void serialize(Archive& ar, Derived& a)
			// {
			// 	ar	& JSON_NI_SERIALIZATION_NVP(a, m_value);
			// }
		};

		BasePtr clone() const
		{
			if (m_ptr)
			{
				return m_ptr->clone();
			}
			return nullptr;
		}

		BasePtr         m_ptr;
		std::type_index m_tpIndex;
	};
}
