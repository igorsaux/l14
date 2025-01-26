//----------------------- Copyright 2025 Igor Spichkin ----------------------//

module;

#include <concepts>
#include <functional>

export module istd.defer;

namespace istd
{

export template <typename F>
	requires std::invocable<F>
class Defer final
{
  public:
	explicit Defer( F&& callback ) : m_callback( std::forward<F>( callback ) )
	{
	}

	Defer( const Defer& ) = delete;
	Defer( Defer&& ) = delete;
	Defer& operator=( const Defer& ) = delete;
	Defer& operator=( Defer&& ) = delete;

	~Defer()
	{
		m_callback();
	}

  private:
	F m_callback;
};

} // namespace istd
