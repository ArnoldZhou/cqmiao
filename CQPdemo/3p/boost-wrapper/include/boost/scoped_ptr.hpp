#pragma once

#include <memory>

namespace boost
{
	template<class...Args>
	using scoped_ptr = std::unique_ptr<Args...>;
}