#ifndef SVG2SFML_HPP_UGNRL9RY
#define SVG2SFML_HPP_UGNRL9RY

#include <memory>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace svg2sfml{
namespace pt = boost::property_tree;

typedef  std::vector<std::unique_ptr<sf::Drawable>> return_t;

return_t readSVG( const std::string &path );

class Loader
{
public:
	//Loader();
	//virtual ~Loader ();

    return_t read( const std::string &path );
    
private:
	std::vector<pt::ptree::value_type*> m_tagStack; 

	//apply color as either a shape or a line, based on m_tagStack
	void applyColorAsShape( sf::Shape * shape );
	void applyColorAsLine( sf::Shape * shape );

	std::unique_ptr< sf::Drawable >
		readAsRect();
	std::unique_ptr< sf::Drawable >
		readAsCircle();
	std::unique_ptr< sf::Drawable >
		readAsLine();
	return_t
		readAsPolyline();
    return_t
        readAsPolygon();
	
	return_t shapesFromSVGTag();
};


sf::Color colorFromSVGColor( const std::string &name );
std::unique_ptr< sf::Drawable >rectForLineSegment(
	const float &x1,
	const float &y1,
	const float &x2,
	const float &y2,
	const float &width );
std::vector< std::pair< float, float > > parsePointsAttribute();
}; /* svg2sfml */

#endif /* end of include guard: SVG2SFML_HPP_UGNRL9RY */
