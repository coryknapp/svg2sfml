#ifndef SVG2SFML_HPP_UGNRL9RY
#define SVG2SFML_HPP_UGNRL9RY

#include <memory>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

namespace svg2sfml
{

typedef  std::vector<std::unique_ptr<sf::Shape>> return_t;

return_t readSVG( const std::string &path );

} /* svg2sfml */ 

#endif /* end of include guard: SVG2SFML_HPP_UGNRL9RY */
