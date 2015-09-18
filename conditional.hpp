
namespace conditional{

template<typename T, typename First>
bool equals_any( const T &a, const First &b ){
	return ( a == b );
}


template<typename T, typename First, typename... Rest>
bool equals_any( const T &a, const First &b, const Rest... r ){
	if( a == b )
		return true;
	return equals_any( a, r... );
}

} // namespace conditional
