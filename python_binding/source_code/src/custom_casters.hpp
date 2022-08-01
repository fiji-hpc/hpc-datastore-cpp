#include <i3d/vector3d.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace pybind11::detail {
template <typename Value>
struct vector3d_caster {
  public:
	using value_conv = make_caster<Value>;

	bool load(handle src, bool convert) {
		if (!isinstance<sequence>(src))
			return false;

		auto s = reinterpret_borrow<sequence>(src);

		if (s.size() != 3)
			return false;

		for (std::size_t i = 0; i < 3; ++i) {
			value_conv conv;
			if (!conv.load(s[i], convert))
				return false;

			value[i] = cast_op<Value&&>(std::move(conv));
		}
		return true;
	}

	template <typename T>
	static handle cast(T&& src, return_value_policy policy, handle parent) {
		tuple t(3);
		for (std::size_t i = 0; i < 3; ++i) {
			auto value_ = reinterpret_steal<object>(
			    value_conv::cast(forward_like<T>(src[i]), policy, parent));

			if (!value_)
				return handle();

			PyTuple_SET_ITEM(t.ptr(), i,
			                 value_.release().ptr()); // steals a reference
		}

		return t.release();
	}

	PYBIND11_TYPE_CASTER(i3d::Vector3d<Value>,
	                     const_name("Tuple[") + value_conv::name +
	                         const_name("]"));
};

template <typename T>
struct type_caster<i3d::Vector3d<T>> : vector3d_caster<T> {};
} // namespace pybind11::detail
